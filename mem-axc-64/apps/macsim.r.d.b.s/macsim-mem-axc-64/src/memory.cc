/*
   Copyright (c) <2012>, <Georgia Institute of Technology> All rights reserved.

   Redistribution and use in source and binary forms, with or without modification, are permitted
   provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list of conditions
   and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this list of
   conditions and the following disclaimer in the documentation and/or other materials provided
   with the distribution.

   Neither the name of the <Georgia Institue of Technology> nor the names of its contributors
   may be used to endorse or promote products derived from this software without specific prior
   written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   */


/**********************************************************************************************
 * File         : memory.cc
 * Author       : Jaekyu Lee
 * Date         : 3/4/2011
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : memory system
 *********************************************************************************************/


#include <iostream>
#include <cmath>

#include "assert_macros.h"
#include "cache.h"
#include "core.h"
#include "debug_macros.h"
#include "dram.h"
#include "frontend.h"
#include "memory.h"
#include "noc.h"
#include "port.h"
#include "uop.h"
#include "factory_class.h"
#include "bug_detector.h"
#include "router.h"

#include "config.h"

#include "all_knobs.h"
#include "statistics.h"
#include "process_manager.h"

#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MEM, ## args)




int g_mem_priority[] = {
  10, // IFETCH
  10, // DFETCH
  10, // DSTORE
  0, // IPRF
  0, // DPRF
  5, // WB
  0, // SW_DPRF
  0, // SW_NTA
  0, // SW_T0
  0, // SW_T1
  0, // SW_T2
};


///////////////////////////////////////////////////////////////////////////////////////////////


inline bool IsStore(Mem_Type type)
{
  switch (type) {
    case MEM_ST:
    case MEM_ST_LM:
    case MEM_ST_SM:
    case MEM_ST_GM:
      return true;
    default:
      return false;
  }
}


inline bool IsLoad(Mem_Type type)
{
  return !IsStore(type);
}


// dcache fill wrapper function
bool dcache_fill_line_wrapper(mem_req_s* req)
{
  bool result = true;

  list<mem_req_s*> done_list;
  for (auto I = req->m_merge.begin(), E = req->m_merge.end(); I != E; ++I) {
    if ((*I)->m_done_func && !((*I)->m_done_func((*I)))) {
      result = false;
      continue;
    }
    done_list.push_back((*I));
  }

  // process merged requests
  for (auto I = done_list.begin(), E = done_list.end(); I != E; ++I) {
    req->m_merge.remove((*I));
    req->m_simBase->m_memory->free_req((*I)->m_core_id, (*I));
  }

  // process the request
  // done func sets the m_rdy_cycle of a uop if uop is not NULL 
  // Even on a context switch it is safe to use this function 
  if (result && req->m_done_func && !req->m_simBase->m_memory->done(req)) {
    // This will never be executed
    result = false;
  }


  return result;
}


///////////////////////////////////////////////////////////////////////////////////////////////


// register base branch predictor
memory_c *default_mem(macsim_c* m_simBase)
{
  memory_c* new_mem;
  new_mem = new no_cache_c(m_simBase);
  return new_mem;
}




///////////////////////////////////////////////////////////////////////////////////////////////


bool queue_c::sort_func::operator()(mem_req_s* a, mem_req_s* b)
{
  if (*m_simBase->m_knobs->KNOB_HETERO_MEM_PRIORITY_CPU) {
    if (a->m_ptx != true && b->m_ptx == true) {
      return true;
    }
    else if (a->m_ptx == true && b->m_ptx != true)
      return false;
  }
  else if (*m_simBase->m_knobs->KNOB_HETERO_MEM_PRIORITY_GPU) {
    if (a->m_ptx != true && b->m_ptx == true) {
      return false;
    }
    else if (a->m_ptx == true && b->m_ptx != true)
      return true;
  }

  return a->m_priority > b->m_priority;
}


queue_c::queue_c(macsim_c* simBase, int size)
{
  m_simBase = simBase;
  m_size = size;
}


queue_c::~queue_c()
{
}


// search an entry with address and size.
mem_req_s* queue_c::search(Addr addr, int size)
{
  for (auto I = m_entry.begin(), E = m_entry.end(); I != E; ++I) {
    mem_req_s* req = (*I);

    // matching should be inclusive
    if (req->m_addr <= addr && req->m_addr+req->m_size >= addr+size) {
      return req;
    }
  }

  return NULL;
}


// search a request.
bool queue_c::search(mem_req_s* req)
{
  for (auto I = m_entry.begin(), E = m_entry.end(); I != E; ++I) {
    if ((*I) == req)
      return true;
  }

  return false;
}


// delete an entry.
void queue_c::pop(mem_req_s* req)
{
  m_entry.remove(req);
}


// insert en entry.
bool queue_c::push(mem_req_s* req)
{
  if (m_entry.size() == m_size)
    return false;

  req->m_queue = this;
  m_entry.push_back(req);
  m_entry.sort(sort_func(m_simBase));

  return true;
}


// check buffer space.
bool queue_c::full()
{
  if (m_entry.size() == m_size)
    return true;
  return false;
}

// Check the m_rdy_cycle of each request in the MSHR
// and trigger done_func if required
// New: Check ruby queues here instead of m_ready_cycle.
// When request avl from ruby, set rdy cycle and call done function

void memory_c::process_mshr(int core_id)
{

  int m_line_size = line_size(core_id);
  // if(*m_simBase->m_knobs->KNOB_DEBUG_MEM)
  //   report("Processing mshrs \n");
  while(!m_ruby->RubyQueueEmpty(core_id))
  {
    Addr addr = m_ruby->RubyQueueTop(core_id);
    // Search the mshr for the request
    mem_req_s* matching_req = search_req(core_id, addr, m_line_size);
    ASSERTM(matching_req != NULL, "Addr returned from Ruby not found in MacSim MSHR");
    for (auto I = m_mshr[core_id].begin(), E = m_mshr[core_id].end(); I != E; ++I) {
      if((*I) == matching_req)
      {
        (*I)->m_rdy_cycle = m_cycle;
         auto tid = (*I)->m_thread_id;
         auto orig_tid = (*I)->m_orig_thread_id;
         //Decrement the number of mem requests for a corresponding thread 
          auto num_req = -- m_simBase->m_sim_processes[(*I)->m_appl_id]
          ->m_thread_trace_info[orig_tid]->m_num_mem_requests;
          if(*m_simBase->m_knobs->KNOB_DEBUG_MEM)
            report("num_mem_req out:"<<num_req <<" core_id:"<<core_id <<" thread_id:"<< tid <<" orig_tid:"<<orig_tid);
          //If num of pending memory request for a thread is 0 and it is not the current thread_id
          //or if it is the current thread_id then the processor is not stalled
          // then mv_context from mem_wait to ready_context
          // && m_simBase->m_cur_thread_id[core_id ] != tid 
          // TODO add logic - if its  a single thread application do not context switch
          

          // Context switch onl for MEDIUM_CORE  
          if(*m_simBase->m_knobs->KNOB_MEM_CONTEXT_SWITCH_ENABLE)
          {
            cout<<"mem_context_switch is enabled\n";

            if(num_req == 0  && m_simBase->m_core_pointers[core_id]->get_unit_type() == UNIT_MEDIUM  )
            {
              if(m_simBase->m_process_manager->mv_context_mem_wait_to_ready(tid,core_id,true)) { 
                // if(*m_simBase->m_knobs->KNOB_DEBUG_MEM)
                report("mv_context_switch called for tid: "<<tid<<" core_id: "<<core_id);
                m_simBase->m_process_manager->sim_thread_schedule();
              }
            }
          }

      }
    }
    m_ruby->RubyQueuePop(core_id);
  }


  list<mem_req_s*> done_list;
  for (auto I = m_mshr[core_id].begin(), E = m_mshr[core_id].end(); I != E; ++I) {
    if((*I)->m_rdy_cycle != 0)
    {
      if((*I)->m_rdy_cycle <= m_cycle ){
        if((*I)->m_state != MEM_MERGED)
          done_list.push_back((*I));
      }
    }
  }

  list<mem_req_s*> free_list;
  for (auto I = done_list.begin(), E = done_list.end(); I != E; ++I) {
    if((*I)->m_done_func((*I))){
      free_list.push_back((*I));
    }
  }
  for(auto I = free_list.begin(), E = free_list.end(); I!=E; ++I)
  {
    m_simBase->m_memory->free_req((*I)->m_core_id, (*I));
  }

}


///////////////////////////////////////////////////////////////////////////////////////////////


int memory_c::m_unique_id = 0;


// =======================================
// memory_c constructor
// =======================================
memory_c::memory_c(macsim_c* simBase)
{
  mem_req_no =0;
  m_simBase = simBase;
  REPORT("Memory system(%s) has been initialized.\n",
      KNOB(KNOB_MEMORY_TYPE)->getValue().c_str());

  m_num_core = *m_simBase->m_knobs->KNOB_NUM_SIM_CORES;
  m_num_l3   = *m_simBase->m_knobs->KNOB_NUM_L3;
  m_num_mc   = *m_simBase->m_knobs->KNOB_DRAM_NUM_MC;

  //  ASSERT(m_num_core == m_num_l3);

  // Init void Ruby

  //  O3sim_ruby::O3sim_ruby( unsigned num_processors, unsigned SMT, unsigned num_L2_banks, unsigned num_memories, bool CMP, bool garnet, unsigned debug_start_time, char *\
  //  debug_filter_string, char *debug_verbosity_string, char *debug_output_filename)

  m_ruby = new O3sim_ruby(m_num_core, *m_simBase->m_knobs->KNOB_MAX_THREADS_PER_CORE, 
      *KNOB(KNOB_RUBY_NUM_BANKS), *KNOB(KNOB_RUBY_NUM_DIR), true, true, 1 ,"","med","/dev/null");
  m_ruby->initialize();

  // allocate mshr
  m_mshr = new list<mem_req_s*>[m_num_core];
  m_mshr_free_list = new list<mem_req_s*>[m_num_core];

  for (int ii = 0; ii < m_num_core; ++ii) {
    for (int jj = 0; jj < *m_simBase->m_knobs->KNOB_MEM_MSHR_SIZE; ++jj) {
      mem_req_s* entry = new mem_req_s(simBase);
      m_mshr_free_list[ii].push_back(entry);
    }
  }

}


// memory class destructor
memory_c::~memory_c()
{
  for (int ii = 0; ii < m_num_core; ++ii) {
    m_mshr_free_list[ii].clear();
    m_mshr[ii].clear();
  }



  delete[] m_mshr;
  delete[] m_mshr_free_list;


  std::ofstream ruby_stat_file("ruby.stat.out", ios::out);
  m_ruby->print_stats(ruby_stat_file);
  ruby_stat_file.close();

  m_ruby->destroy();
  delete m_ruby;
}


// =======================================
// initialize the memory system (noc interface)
// =======================================
void memory_c::init(void)
{
  if (*KNOB(KNOB_ENABLE_IRIS) == false && *KNOB(KNOB_ENABLE_NEW_NOC) == false)
    return;
}


// generate a new memory request
// called from 1) data cache, 2) instruction cache, and 3) prefetcher
bool memory_c::new_mem_req(Mem_Req_Type type, Addr addr, uns size, uns delay, uop_c* uop, \
    function<bool (mem_req_s*)> done_func, Counter unique_num, pref_req_info_s* pref_info, \
    int core_id, int thread_id, bool ptx)
{
  DEBUG("MSHR[%d] new_req type:%s (%d)\n",
      core_id, mem_req_c::mem_req_type_name[type], (int)m_mshr[core_id].size());
  DEBUG("NEWMEM= UOP: %d ADDR %s\n",(int)uop->m_uop_num,hexstr64s(addr));

  if (m_stop_prefetch > m_cycle && type == MRT_DPRF)
    return true;

  // find a matching request
  mem_req_s* matching_req = search_req(core_id, addr, size);

  if (type == MRT_IFETCH) {
    POWER_CORE_EVENT(core_id, POWER_ICACHE_MISS_BUF_R_TAG);
  }
  else {
    POWER_CORE_EVENT(core_id, POWER_DCACHE_MISS_BUF_R_TAG);
  }

  if (matching_req) {
    ASSERT(type != MRT_WB);
    // redundant hardware prefetch request
    if (type == MRT_DPRF) {
      // adjust priority of matching entry
      return true;
    }
    else if (matching_req->m_type == MRT_DPRF) {
      // promotion from hardware prefetch to demand
      DEBUG("req:%d has been promoted type:%s\n", matching_req->m_id,
          mem_req_c::mem_req_type_name[matching_req->m_type]);
      adjust_req(matching_req, type, addr, size, delay, uop, done_func, unique_num, \
          g_mem_priority[type], core_id, thread_id, ptx);
      return true;
    }
    else if (matching_req->m_type == MRT_WB) {
      ASSERT(0);
    }
  }



  // allocate an entry

  mem_req_s* new_req = allocate_new_entry(core_id);

  // mshr full
  if (new_req == NULL) {
    m_stop_prefetch = m_cycle + 500;
    flush_prefetch(core_id);
    if (type == MRT_DPRF)
      return true;

    new_req = allocate_new_entry(core_id);

    if (new_req == NULL) {
      STAT_EVENT(MSHR_FULL);
      return false;
    }
  }


  if (type == MRT_IFETCH) {
    POWER_CORE_EVENT(core_id, POWER_ICACHE_MISS_BUF_W);
  }
  else if (type == MRT_DSTORE) {
    POWER_CORE_EVENT(core_id, POWER_DCACHE_MISS_BUF_W);
  }
  else {
    POWER_CORE_EVENT(core_id, POWER_DCACHE_MISS_BUF_W);
  }
  STAT_EVENT(TOTAL_MEMORY);

  Counter priority = g_mem_priority[type];

  // init new request - Add ready cycle time - ska
  init_new_req(new_req, type, addr, size, delay, uop, done_func, unique_num, priority, \
      core_id, thread_id, ptx);


  // merge to existing request
  if (matching_req) {
    STAT_EVENT(TOTAL_MEMORY_MERGE);
    DEBUG("req:%d addr:%llx has matching entry req:%d addr:%llx type:%s\n",
        new_req->m_id, new_req->m_addr, matching_req->m_id, matching_req->m_addr, \
        mem_req_c::mem_req_type_name[matching_req->m_type]);

    matching_req->m_merge.push_back(new_req);
    new_req->m_merged_req = matching_req;
    new_req->m_state = MEM_MERGED;

    //report("Merged: " <<hex << matching_req->m_addr <<dec);

    // adjust priority
    //    if (matching_req->m_priority < priority)
    //      matching_req->m_priority = priority;

    return true;
  }

  // insert to queue
  //m_l2_cache[core_id]->insert(new_req);

  return true;
}



// allocate a new memory request
mem_req_s* memory_c::allocate_new_entry(int core_id)
{
  if (m_mshr_free_list[core_id].empty())
    return NULL;

  mem_req_s* new_req = m_mshr_free_list[core_id].back();
  m_mshr_free_list[core_id].pop_back();
  m_mshr[core_id].push_back(new_req);

  return new_req;
}


// search matching request
mem_req_s* memory_c::search_req(int core_id, Addr addr, int size)
{
  for (auto I = m_mshr[core_id].begin(), E = m_mshr[core_id].end(); I != E; ++I) {
    mem_req_s* req = (*I);
    if (req->m_addr <= addr && req->m_addr+req->m_size >= addr+size) {
      return req;
    }
  }

  return NULL;
}


// initialize a new request
void memory_c::init_new_req(mem_req_s* req, Mem_Req_Type type, Addr addr, int size, \
    int delay, uop_c* uop, function<bool (mem_req_s*)> done_func, Counter unique_num, \
    Counter priority, int core_id, int thread_id, bool ptx)
{
  req->m_id                     = m_unique_id++;
  req->m_appl_id                = m_simBase->m_core_pointers[core_id]->get_appl_id(thread_id);
  req->m_core_id                = core_id;
  req->m_thread_id              = thread_id;
  req->m_orig_thread_id         = m_simBase->m_core_pointers[core_id]->get_trace_info(thread_id)->m_orig_thread_id;
  req->m_block_id               = 0;
  req->m_state                  = MEM_NEW;
  req->m_type                   = type;
  req->m_priority               = priority;
  req->m_addr                   = addr;
  req->m_size                   = size;
  req->m_rdy_cycle              = 0; /* ska - this is set by process_mshr after checking ruby_queue*/
  req->m_pc                     = uop ? uop->m_pc : 0;
  req->m_prefetcher_id          = 0;
  req->m_pref_loadPC            = 0;
  req->m_ptx                    = ptx;
  req->m_done_func              = done_func;
  req->m_uop                    = uop ? uop : NULL;
  req->m_in                     = m_cycle;
  req->m_dirty                  = false;
  req->m_done                   = false;
  req->m_merged_req             = NULL;
  req->m_bypass                 = uop ? uop->m_bypass_llc : false;
  req->m_skip                   = uop ? uop->m_skip_llc : false;

  ASSERT(req->m_merge.empty());
  //set_cache_id(req);
}


// adjust a new request
void memory_c::adjust_req(mem_req_s* req, Mem_Req_Type type, Addr addr, int size, \
    int delay, uop_c* uop, function<bool (mem_req_s*)> done_func, Counter unique_num, \
    Counter priority, int core_id, int thread_id, bool ptx)
{
  req->m_appl_id                = m_simBase->m_core_pointers[core_id]->get_appl_id(thread_id);;
  req->m_core_id                = core_id;
  req->m_thread_id              = thread_id;
  req->m_block_id               = 0;
  req->m_type                   = type;
  req->m_priority               = priority;
  req->m_addr                   = addr;
  req->m_size                   = size;
  req->m_pc                     = uop ? uop->m_pc : 0;
  req->m_prefetcher_id          = 0;
  req->m_pref_loadPC            = 0;
  req->m_ptx                    = ptx;
  req->m_done_func              = done_func;
  req->m_uop                    = uop ? uop : NULL;
  req->m_in                     = m_cycle;
  req->m_dirty                  = false;
  req->m_done                   = false;
  req->m_merged_req             = NULL;
  req->m_bypass                 = false;
  req->m_skip                   = false;

  //set_cache_id(req);
}



// deallocate a memory request
void memory_c::free_req(int core_id, mem_req_s* req)
{
  STAT_EVENT(AVG_MEMORY_LATENCY_BASE);
  STAT_EVENT_N(AVG_MEMORY_LATENCY, m_cycle - req->m_in);

  // when there are still merged requests, call done wrapper function
  if (!req->m_merge.empty()) {
    DEBUG("req:%d has merged req type:%s\n",
        req->m_id, mem_req_c::mem_req_type_name[req->m_type]);
    dcache_fill_line_wrapper(req);
  }

  ASSERTM(req->m_merge.empty(), "type:%s\n", mem_req_c::mem_req_type_name[req->m_type]);

  if (req->m_type == MRT_WB) {
    delete req;
  }
  else {
    req->m_addr = 0;
    req->m_id   = -1;
    m_mshr[core_id].remove(req);
    m_mshr_free_list[core_id].push_back(req);
  }
}


// get number of available mshr entries
int memory_c::get_num_avail_entry(int core_id)
{
  return m_mshr_free_list[core_id].size();
}

// access L1 cache from execution stage
int memory_c::access(uop_c* uop)
{
  bool isWrite = false;
  bool isPrefetch = false;

  // DCU->Mem Hacks - Fix later
  bool m_ptx_sim = false;
  int m_latency = 8;
  int m_line_size = line_size(uop->m_thread_id);

  uop->m_state = OS_DCACHE_BEGIN;
  Addr vaddr     = uop->m_vaddr;
  Mem_Type type  = uop->m_mem_type;
  Addr line_addr = base_addr(uop->m_thread_id,vaddr);
  uop->m_uop_info.m_dcmiss = true;

  // set type;
  Mem_Req_Type req_type;
  switch (type) {
    case MEM_LD:
    case MEM_LD_LM:
    case MEM_LD_CM:
    case MEM_LD_TM:
      req_type = MRT_DFETCH;
      break;
    case MEM_ST:
    case MEM_ST_LM:
    case MEM_ST_GM:
      req_type = MRT_DSTORE;
      isWrite = true;
      break;
    case MEM_PF: //nav -27-06-2016
    case MEM_SWPREF_NTA:
    case MEM_SWPREF_T0:
    case MEM_SWPREF_T1:
    case MEM_SWPREF_T2:
      req_type = MRT_DPRF;
      isPrefetch = true;
      break;
    default:
      ASSERTM(0, "type:%d\n", type);
      break;
  }

//TODO: MEM_PF,MEM_LD_SM,-- ??
  // -------------------------------------
  // set address and size
  // -------------------------------------
  int req_size;
  Addr req_addr;
  if (m_ptx_sim && *m_simBase->m_knobs->KNOB_BYTE_LEVEL_ACCESS) {
    req_size = uop->m_mem_size;
    req_addr = vaddr;
  }
  else {
    req_size = m_line_size;
    req_addr = line_addr;
  }

  // int result;
  // function<bool (mem_req_s*)> done_func = dcache_fill_line_wrapper;
  // result = new_mem_req(req_type, req_addr, req_size, m_latency, uop, done_func, uop->m_unique_num, NULL, uop->m_core_id,
  //     uop->m_thread_id, m_ptx_sim);
  //
  // if(!result)
  // {
  //   return 0;
  // }
  // //Cache Miss 
  // else {
  //
  //
  //
  //   uop->m_state = OS_DCACHE_MEM_ACCESS_DENIED;
  //   if(!m_simBase->m_core_pointers[uop->m_core_id]->m_fetch_ended[uop->m_thread_id]) {
  //     m_simBase->m_core_pointers[uop->m_core_id]->set_context_switch();
  //
  //     if(*m_simBase->m_knobs->KNOB_DEBUG_MEM)
  //       report("rob_entry for cur_uop during context switch :- "<< uop->m_rob_entry <<" # of mem instructions "<< mem_req_no++
  //         <<"\n");
  //     uop->m_is_flush = true; // Mark the instruction  after which the rob entries will be flushed;
  //   }

  // }
  // // need to remove to enable O3 sim 
  // return -1;

  if(m_ruby->isReady(req_addr,req_size,uop->m_core_id, uop->m_thread_id, false, false, NULL)){
    function<bool (mem_req_s*)> done_func = dcache_fill_line_wrapper;
    int result;
    if(!isPrefetch)
    {
      result = new_mem_req(req_type, req_addr, req_size, m_latency, uop, done_func, uop->m_unique_num, NULL, uop->m_core_id,
          uop->m_thread_id, m_ptx_sim);

      // Pushing into Ruby from here -- ska

      if(!result)
      {
        // Did not get a free mshr entry
        uop->m_state = OS_DCACHE_MEM_ACCESS_DENIED;
        return 0;
      }
    }
    if(isPrefetch)
    {
      m_ruby->send_prefetch(req_addr, req_size, uop->m_core_id, uop->m_thread_id, false, false, NULL);
      return 1;
    }
    else
    {

      auto appl_id = m_simBase->m_core_pointers[uop->m_core_id]->get_appl_id(uop->m_thread_id);
      //increment the  num of  mem_req for each thread 
      auto orig_tid =  m_simBase->m_core_pointers[uop->m_core_id]->get_trace_info(uop->m_thread_id)->m_orig_thread_id;
      auto num_req = ++ m_simBase->m_sim_processes[appl_id]->m_thread_trace_info[orig_tid]->m_num_mem_requests;
      if(*m_simBase->m_knobs->KNOB_DEBUG_MEM)
        report("num_mem_req in:"<<num_req);
      m_ruby->send_request(req_addr, req_size, uop->m_core_id, uop->m_thread_id, isWrite, false, NULL);

      //if L2 Cache Miss and L1 cache miss :- Since inside ruby there can be cases when L1 is in intermediate state and waiting for data 
      if(!m_ruby->tryAccCacheAccess(req_addr) && !m_ruby->tryL1CacheAccess(req_addr)  )  
      {
         // if(!m_ruby->tryL1CacheAccess(req_addr) )
         //     report("L1 Cache Miss addr:"<< req_addr);
        if(*m_simBase->m_knobs->KNOB_DEBUG_MEM)
          report("Cache miss addr:"<<req_addr );
        //If the last uop is not fetched 
        uop->m_state = OS_DCACHE_MEM_ACCESS_DENIED;


        //if fetch has not ended and is a memory op then set_context_switch -> this stops the frontend_q and alloc_q 
        auto cur_core = m_simBase->m_core_pointers[uop->m_core_id];

        if(*m_simBase->m_knobs->KNOB_MEM_CONTEXT_SWITCH_ENABLE)
        {
          if(!cur_core->m_fetch_ended[uop->m_thread_id] && cur_core->get_unit_type() == UNIT_MEDIUM ) {
            m_simBase->m_core_pointers[uop->m_core_id]->set_context_switch();

            if(uop->m_ds_flag && *m_simBase->m_knobs->KNOB_DEBUG_MEM)
              cout<<"ds_flag in memory.cc is true\n";
            //nav if core type is a large core do not set flush for context switch
            uop->m_is_flush = true; // Mark the instruction  after which the rob entries will be flushed;

            // if(*m_simBase->m_knobs->KNOB_DEBUG_MEM)
              report("set_context_switch rob_entry :"<< uop->m_rob_entry <<" # of mem instructions "<< mem_req_no++
                  <<" uop_num:"<<uop->m_uop_num
                  <<" core_id:"<<uop->m_core_id
                  <<" thread_id:"<<uop->m_orig_thread_id
                  <<"\n");
          }
        }
      }
    }
    return -1;
  }
  else{
    uop->m_state = OS_DCACHE_MEM_ACCESS_DENIED;
    return 0;
  }


  // return m_l1_cache[uop->m_core_id]->access(uop);
}


// get base line address
// FIXME (jaekyu, 3-7-2012)
// replace 63 with the cache line size
Addr memory_c::base_addr(int core_id, Addr addr)
{
  return (addr & ~63);
}


// get cache line size
int memory_c::line_size(int core_id)
{
  return 64; 
}




// cache done function
bool memory_c::done(mem_req_s* req)
{
  if (req->m_uop)
  {
    uop_c* uop = req->m_uop;

    DEBUG("uop:%lld done\n", uop->m_uop_num);
    uop->m_done_cycle = m_simBase->m_core_cycle[uop->m_core_id] + 1;
    uop->m_state = OS_SCHEDULED;
  }

  return true;
}


// get cache bank id
int memory_c::bank_id(int core_id, Addr addr)
{
  ASSERTM(false, "Unsupported function -- Using Ruby as memory system");
  return 0;
}


// get a cache read port
bool memory_c::get_read_port(int core_id, int bank_id)
{
  ASSERTM(false, "Unsupported function -- Using Ruby as memory system");
  return 0;
}


// access data cache
dcache_data_s* memory_c::access_cache(int core_id, Addr addr, Addr *line_addr, \
    bool update, int appl_id)
{
  ASSERTM(false, "Unsupported function -- Using Ruby as memory system");
  return 0;
}


// memory run_a_cycle
void memory_c::run_a_cycle(void)
{
  //run_a_cycle_uncore();
  // Increment RubyTime here

  m_ruby->advance_time();
  ++m_cycle;
  // std::cout <<"m_cycle<<"<<m_cycle<<"\n";
}

void memory_c::run_a_cycle_core(int core_id)
{
  if(!m_simBase->m_dafaxc_running)
    process_mshr(core_id);
}






// print all mshr entries
void memory_c::print_mshr(void)
{


  // print mshr
  FILE* fp = fopen("bug_detect_mem.out", "w");
  for (int ii = 0; ii < m_num_core; ++ii) {
    fprintf(fp, "== Core %d ==\n", ii);
    fprintf(fp, "%-20s %-10s %-10s %-15s %-15s %-7s %-20s %-15s %-15s\n", \
        "ID", "IN_TIME", "DELTA", "TYPE", "STATE", "MERGED", "MERGED_ID", \
        "MERGED_TYPE", "MERGED_STATE");

    for (auto I = m_mshr[ii].begin(), E = m_mshr[ii].end(); I != E; ++I) {
      mem_req_s* req = (*I);
      fprintf(fp, "%-20d %-10llu %-10llu %-15s %-15s %-7d %-20d %-15s %-15s\n",
          req->m_id, req->m_in, m_cycle - req->m_in, \
          mem_req_c::mem_req_type_name[req->m_type], mem_req_c::mem_state[req->m_state], \
          (req->m_merged_req ? 1 : 0), (req->m_merged_req ? req->m_merged_req->m_id : -1), \
          (req->m_merged_req ? mem_req_c::mem_req_type_name[req->m_merged_req->m_type] : "NULL"), \
          (req->m_merged_req ? mem_req_c::mem_state[req->m_merged_req->m_state] : NULL));
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}


// flush all prefetches in the mshr
void memory_c::flush_prefetch(int core_id)
{
  list<mem_req_s*> done_list;
  for (auto I = m_mshr[core_id].begin(), E = m_mshr[core_id].end(); I != E; ++I) {
    if ((*I)->m_type == MRT_DPRF && (*I)->m_merge.empty()) {
      done_list.push_back((*I));
    }
  }

  for (auto I = done_list.begin(), E = done_list.end(); I != E; ++I) {
    if ((*I)->m_queue != NULL) {
      (*I)->m_queue->pop((*I));
      free_req(core_id, (*I));
    }
  }
}


no_cache_c::no_cache_c(macsim_c* simBase) : memory_c(simBase)
{
}


no_cache_c::~no_cache_c()
{
}


void no_cache_c::set_cache_id(mem_req_s* req)
{
}

