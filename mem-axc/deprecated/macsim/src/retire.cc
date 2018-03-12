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
 * File         : retire.cc
 * Author       : Hyesoon Kim
 * Date         : 3/17/2008
 * CVS          : $Id: retire.cc 867 2009-11-05 02:28:12Z kacear $:
 * Description  : retirement logic
 origial author: Jared W.Stark  imported from
 *********************************************************************************************/


///////////////////////////////////////////////////////////////////////////////////////////////
/// \page retire Retirement stage
///
/// This models retire (commit) stage in the processor pipeline. All instructions are retired 
/// in-order. However, basic execution is in the micro-op granularity. Thus, retirement 
/// should carefully handle this cases.
/// \li <c>Instruction termination condition</c> - All uops of an instruction retired in-order
/// \li <c>Thread termination condition</c> - Last uop of a thread
/// \li <c>Process termination condition</c> - # of thread terminated == # of thread created
///
/// \section retire_cpu CPU retirement stage
/// Check the front uop in the rob (in-order retirement).
///
/// \section retire_gpu GPU retirement stage
/// Since there are possibly many ready-to-retire uops from multiple threads. From all
/// ready-to-retire uops from all threads, we sort them based on the ready cycle (age).
///
/// \todo We need to check thread termination condition carefully.
///////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////
/// \page repeat Repeating traces
///
/// \section repeat_1 What is repeating traces?
/// When an application has been terminated, run this application again
///
/// \section repeat_2 Why do we need to repeat traces?
/// In typical multi-programmed workloads (not multi-threaded), whan an application is 
/// terminated earlier than other applications, typically we keep running early-terminated 
/// application until the last application is terminated.
///
/// \section repeat_3 How to enable repeating trace?
/// There are two ways to enable repeating traces.
/// \li Multi-programmed workloads - set <b>*m_simBase->m_knobs->KNOB_REPEAT_TRACE 1</b>
/// \li Single-application - set <b>*m_simBase->m_knobs->KNOB_REPEAT_TRACE 1</b> and 
/// <b>set *m_simBase->m_knobs->KNOB_REPEAT_TRACE_N positive number</b>
///////////////////////////////////////////////////////////////////////////////////////////////


#include "bug_detector.h"
#include "core.h"
#include "frontend.h"
#include "process_manager.h"
#include "retire.h"
#include "rob.h"
#include "rob_smc.h"
#include "uop.h"

#include "config.h"

#include "knob.h"
#include "debug_macros.h"
#include "statistics.h"

#include "all_knobs.h"
#include "trace_read.h"
#include "memory.h"
#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE, ## args)


#include "graph.hpp"
#include "data_type.hpp"





//---------------------------------------
// retire_c constructor
retire_c::retire_c(RETIRE_INTERFACE_PARAMS(), macsim_c* simBase) : RETIRE_INTERFACE_INIT() 
{
  m_simBase = simBase;

  m_retire_running      = false;
  m_total_insts_retired = 0;

  RETIRE_CONFIG();

  if (m_knob_ptx_sim)
    m_knob_width = 1000;

  std::string input_dotfile_path = "/home/nvedula/hotpaths/run-framework/blackscholes/_Z19BlkSchlsEqEuroNoDivfffffif/dfg.__offload_func_0.dot";

  std::string ldst_file_path     = "/home/nvedula/summer2016/mem-axc-64/traces/trace-ldst-gen/blackscholes/mem-dump.out";
  // std::string ldst_file_path =   "/home/nvedula/summer2016/mem-axc-64/dafaxc/test-graph/test-mem-dump.out";

  // Graph input_graph_c(64, input_file, ldst_file_path.c_str(), input_dotfile_path.c_str() );
  // call constructor with retire constructor- create a graph object inside retire
  // Initialize dafaxc when cpu threads are disabled : graph->initialize()
  // int start_clock=0;
  input_graph_c = new Graph(64, ldst_file_path, input_dotfile_path, m_simBase->m_memory->m_ruby);
  // input_graph_c.printGraph();
  m_dafaxc_rdy = false;


}


// retire_c destructor
retire_c::~retire_c()
{
  delete input_graph_c;
}


// run_a_cycle : try to commit (retire) instructions every cycle
// Check front ROB (reorder buffer) entry and see whehther it is completed
// If there are multiple completed uops, commit until pipeline width
void retire_c::run_a_cycle()
{
  // check whether retire stage is running
  if (!m_retire_running) {

    return;
  }

  m_cur_core_cycle = m_simBase->m_core_cycle[m_core_id];
  core_c *core = m_simBase->m_core_pointers[m_core_id];

  vector<uop_c*>* uop_list = NULL;
  unsigned int uop_list_index = 0;
  if (m_knob_ptx_sim && *m_simBase->m_knobs->KNOB_GPU_SCHED) {
    // GPU : many retireable uops from multiple threads. Get entire retireable uops
    uop_list = m_gpu_rob->get_n_uops_in_ready_order(m_knob_width, m_cur_core_cycle);
  }


  // retire instructions : all micro-ops within an inst. need to be retired for an inst.
  for (int count = 0; count < m_knob_width; ++count) {
    uop_c* cur_uop;
    rob_c* rob;

    // we need to handle retirement for x86 and ptx separately

    // retirement logic for GPU
    if (m_knob_ptx_sim && *m_simBase->m_knobs->KNOB_GPU_SCHED) {
      // GPU : many retireable uops from multiple threads. Get entire retireable uops
      if (uop_list_index == uop_list->size()) {
        uop_list->clear();
        break;
      }

      cur_uop = uop_list->at(uop_list_index++);

      rob = m_gpu_rob->get_thread_rob(cur_uop->m_thread_id);
      rob->pop();
    }
    // retirement logic for x86 simulation
    else {

      //nav DAFAXC LOGIC AND RE-ENABLE CPU LOGIC
      //------------------------------------------------------------
      //nav1

      // if any cpu_thread is stalled -- retrieve it from disabled_threads_queue and restart cpu core
      //------------------------------------------------------------------------------ 
      // DAFAXC CALLED
      //------------------------------------------------------------------------------ 
      if(!m_simBase->m_disabled_cpu_threads.empty() && !m_dafaxc_rdy)
      {
        // report("DAFAXC initialized \n");
        cout<<"DAFAXC initialized \n";
        m_dafaxc_rdy=true;
        //sending iteration no. for offload function
        input_graph_c->initialize(m_simBase->m_disabled_cpu_threads.front().offload_iter );
      } 

      if(m_dafaxc_rdy && !m_simBase->m_disabled_cpu_threads.empty() )
      {
        // report("DAFAXC READY \n");
        // cout<<"DAFAXC READY \n";
        //default return value is 0: do nothing
        //returns -1 on error, # of cycles taken on completion and 0 otherwise (default value)
        int is_dafaxc_complete = input_graph_c->run_a_cycle();
        ASSERT(is_dafaxc_complete != -1);

        if( is_dafaxc_complete>0 )
        {
          cout<<"DAFAXC COMPLETE \n";
          m_dafaxc_rdy = false;
          auto coreid = m_simBase->m_disabled_cpu_threads.front().coreid;
          auto localtid = m_simBase->m_disabled_cpu_threads.front().localtid;

          report("LARGE CORE: Retrieving CPU thread enabled coreid:"<<coreid<<"\n");
          m_simBase->m_disabled_cpu_threads.pop();

          auto appl_id = m_simBase->m_core_pointers[coreid]->get_appl_id(localtid);
          auto orig_tid = m_simBase->m_core_pointers[coreid]->get_trace_info(localtid)->m_orig_thread_id;
          auto cpu_thread_info = m_simBase->m_sim_processes[appl_id]->m_thread_trace_info[orig_tid];

          // Write Logic to enable CPU thread ---  
          //CPU thread mv_context 
          int cpu_core_id = -1;
          // axc_tid is orig_tid need to give tid - local to core in which axc_tid is allocated
          if (cpu_thread_info == NULL ) //Undefined behaviour 
            ASSERT(0);
          else 
          {

            cpu_core_id = cpu_thread_info->m_cur_core_id;         
            if(cpu_core_id == -1)
              ASSERT(0);
          }

          m_simBase->m_dafaxc_running = false;
          //enable cpu thread
          cpu_thread_info->m_thread_enable = true;
          if(m_simBase->m_process_manager->is_present_mem_wait_context(cpu_core_id,cpu_thread_info->m_thread_id,false))
            m_simBase->m_process_manager->mv_context_mem_wait_to_ready(cpu_thread_info->m_thread_id,cpu_core_id,false);

          m_simBase->m_process_manager->sim_thread_schedule();
          break;
        }
      }

      //------------------------------------------------------------




      rob = m_rob;

      // rob is empty;
      if (rob->entries() == 0) {

        if(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE) 
          cout<<"ROB entries empty \n";
        break;
      }
      else { 

        if(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE) 
          cout<<"ROb Entries : "<<rob->entries()<<"\n";
      }
      cur_uop = rob->front();

      auto appl_id = m_simBase->m_core_pointers[cur_uop->m_core_id]->get_appl_id(cur_uop->m_thread_id);
      auto orig_tid = m_simBase->m_core_pointers[cur_uop->m_core_id]->get_trace_info(cur_uop->m_thread_id)->m_orig_thread_id;
      auto num_req = m_simBase->m_sim_processes[appl_id]->m_thread_trace_info[orig_tid]->m_num_mem_requests; 

      // uop cannot be retired
      if (!cur_uop->m_done_cycle || cur_uop->m_done_cycle > m_cur_core_cycle) {

        if(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE) 
          report("uop cannot retire done_cycle: "<< cur_uop->m_done_cycle 
              <<"  cur_core_cycle:"<<m_cur_core_cycle
              <<"  uop_num :"<<cur_uop->m_uop_num
              <<"  inst_num:"<<cur_uop->m_inst_num
              <<"\n");


        //remember m_is_flush is set only for medium cores 

        if(cur_uop->m_is_flush && !core->m_fetch_ended[cur_uop->m_thread_id]  
            && core->is_time_to_context_switch() && num_req!=0 ) {
          if(cur_uop->m_last_uop) {

            // if(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE) 
            report(" LAST_UOP -- cur_uop  m_inst_num :"<<cur_uop->m_inst_num
                <<" uop_num:"<<cur_uop->m_uop_num
                <<" unique_num:"<<cur_uop->m_unique_num <<"\n"
                <<" core_id:"<< m_core_id
                <<" thread_id:"<< cur_uop->m_thread_id
                <<" retired_insts:"<<m_insts_retired[cur_uop->m_thread_id]
                <<" done_cycle:"<<cur_uop->m_done_cycle 
                <<" pc:0X"<<hexstr64s(cur_uop->m_pc)
                <<"\n");


            break;
          }

          //----- 
          // Get current context and modify the file pointer to read instructions after  memory ops 
          // clear the buffer read from the file
          auto  cur_alloc_thread_info = core->get_alloc_thread_info(cur_uop->m_thread_id); // contains heart beat information
          auto  cur_trace_info = m_simBase->m_process_manager->prepare_context_info(core,cur_uop,0);

          m_simBase->m_process_manager->context_switch(cur_trace_info, cur_alloc_thread_info, cur_uop->m_core_id, cur_uop->m_thread_id,true); 


          // if(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE) {

          report("SAVING CONTEXT:"
              <<" core_id:"<< m_core_id
              <<" m_inst_num :"<<cur_uop->m_inst_num
              <<" uop_num:"<<cur_uop->m_uop_num
              <<" unique_num:"<<cur_uop->m_unique_num <<"\n"
              <<" thread_id:"<< cur_uop->m_thread_id
              <<" retired_insts:"<<m_insts_retired[cur_uop->m_thread_id]
              <<" done_cycle:"<<cur_uop->m_done_cycle 
              <<" pc:0X"<<hexstr64s(cur_uop->m_pc)
              <<"\n");
          // }

          auto uop_thread_id = cur_uop->m_thread_id;
          // //flush all queues and call stall thread
          // If trace read the last instruction then this flag will be set 
          // which needs to be unset since we are context switching  OR Second approach do not context switch in that case
          // core->m_fetch_ended[uop_thread_id] = false;
          core->sched_stop();
          core->flush_pqueues(uop_thread_id);

          break;
        }


        break;
      }


      //Value arrived from memory when the instruction was about to retire
      //So reverse the changes made in memory.cc access function which stopped the cores frontend allocate etc
      if(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE && core->m_fetch_ended[cur_uop->m_thread_id] ) 
        report("is_fetch_ended :"<< core->m_fetch_ended[cur_uop->m_thread_id]); 
      //remember m_is_flush is set only for medium cores 
      if(cur_uop->m_is_flush  ) {
        if(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE) 
          report("reverse context switch change since data arrived frm memory when instruction is about to retire : Time to CS: "<< core->is_time_to_context_switch() );

        if( core->is_time_to_context_switch() !=0 ) { 
          core->reset_context_switch_timer();
          // if(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE) 
          //   report("reseting context switch timer"   );

        }


        if(num_req == 0) report ("num_req =0 core_id:"<<cur_uop->m_core_id <<" orig_tid:"<<orig_tid <<" tid:"<<cur_uop->m_thread_id);
        core->start_frontend();

      }

      // all uops belong to previous instruction have been retired : inst_count++
      // nagesh - why are we marking the instruction as retired when the uop
      // marked BOM is retired? shouldn't we do that when the EOM is retired?
      // nagesh - ISTR that I tried changing and something failed - not 100% 
      // sure though : (jaekyu) I think this is the matter of the design. we can update
      // everything from the first uop of an instruction.
      if (cur_uop->m_isitBOM) {
        if (cur_uop->m_uop_type >= UOP_FCF && cur_uop->m_uop_type <= UOP_FCMOV) {
          STAT_EVENT(FP_OPS_TOT);
          STAT_CORE_EVENT(cur_uop->m_core_id, FP_OPS);
        }

        ++m_insts_retired[cur_uop->m_thread_id];
        ++m_total_insts_retired;
        ++m_period_inst_count;

        STAT_CORE_EVENT(cur_uop->m_core_id, INST_COUNT);
        POWER_CORE_EVENT(cur_uop->m_core_id, POWER_PIPELINE);
        STAT_EVENT(INST_COUNT_TOT);
      }


      // GPU : barrier
      if (m_knob_ptx_sim && cur_uop->m_bar_type == BAR_FETCH) {
        frontend_c *frontend = core->get_frontend();
        frontend->synch_thread(cur_uop->m_block_id, cur_uop->m_thread_id);
      }


      // Terminate thread : current uop is last uop of a thread, so we can retire a thread now
      thread_s* thread_trace_info = core->get_trace_info(cur_uop->m_thread_id);
      process_s *process = thread_trace_info->m_process;
      if (cur_uop->m_last_uop || m_insts_retired[cur_uop->m_thread_id] >= *m_simBase->m_knobs->KNOB_MAX_INSTS) {
        core->m_thread_reach_end[cur_uop->m_thread_id] = true;
        ++core->m_num_thread_reach_end;
        // If thread needs to be stalled then just termnate the thread


        if (!core->m_thread_finished[cur_uop->m_thread_id]) {
          // DEBUG("core_id:%d thread_id:%d terminated\n", m_core_id, cur_uop->m_thread_id);
          report("core_id:"<< m_core_id <<" thread_id:" << cur_uop->m_thread_id <<" orig_thread_id:"<<cur_uop->m_orig_thread_id  <<" terminated\n"); 

          // terminate thread
          m_simBase->m_process_manager->terminate_thread(m_core_id, thread_trace_info, \
              cur_uop->m_thread_id, cur_uop->m_block_id);

          // disable current thread's fetch engine
          if (!core->m_fetch_ended[cur_uop->m_thread_id]) {
            core->m_fetch_ended[cur_uop->m_thread_id] = true;
            core->m_fetching_thread_num--;
          }

          // all threads in an application have been retired. Thus, we can retire an appliacation
          if (process->m_no_of_threads_terminated == process->m_no_of_threads_created) {
            if (process->m_current_vector_index == process->m_applications.size() 
                || (*m_simBase->m_ProcessorStats)[INST_COUNT_TOT].getCount() >= *KNOB(KNOB_MAX_INSTS1)) {
              update_stats(process);
              m_simBase->m_process_manager->terminate_process(process);
              if (m_simBase->m_process_count_without_repeat == 0) {
                m_simBase->m_repeat_done = true;
              }
              repeat_traces(process);
            }
            else {
              m_simBase->m_process_manager->terminate_process(process);
            }

            // schedule new threads
            m_simBase->m_process_manager->sim_thread_schedule();
          }

          // schedule new threads
          m_simBase->m_process_manager->sim_thread_schedule();
        }
      } // terminate_thread


      // update number of retired uops
      ++m_uops_retired[cur_uop->m_thread_id];

      DEBUG("core_id:%d thread_id:%d retired_insts:%lld uop->inst_num:%lld uop_num:%lld " 
          "done_cycle:%lld\n",
          m_core_id, cur_uop->m_thread_id, m_insts_retired[cur_uop->m_thread_id], 
          cur_uop->m_inst_num, cur_uop->m_uop_num, cur_uop->m_done_cycle);


      // if(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE) {

      report("cur_uop  m_inst_num :"<<cur_uop->m_inst_num
          <<" uop_num:"<<cur_uop->m_uop_num
          <<" unique_num:"<<cur_uop->m_unique_num <<"\n"
          <<" core_id:"<< m_core_id
          <<" thread_id:"<< cur_uop->m_thread_id
          <<" retired_insts:"<<m_insts_retired[cur_uop->m_thread_id]
          <<" done_cycle:"<<cur_uop->m_done_cycle 
          <<" pc:0X"<<hexstr64s(cur_uop->m_pc)
          <<"\n"
          <<" rob_space:"<<rob->space()
          <<" rob entries:"<< rob->entries()
          <<" rob head:"<<rob->head()
          <<" rob tail:"<< rob->tail()
          <<"\n");

      // }

      rob->pop();
      POWER_CORE_EVENT(m_core_id, POWER_REORDER_BUF_R);
      POWER_CORE_EVENT(m_core_id, POWER_INST_COMMIT_SEL_LOGIC_R);

      if( cur_uop->m_ds_flag  )
      {

        report("Markers Read: RETIRE stage start is_chunk_begin :"<<cur_uop->m_is_chunk_begin
            <<" orig_thread_id : "<<cur_uop->m_orig_thread_id
            <<" core_id : "<< cur_uop->m_core_id
            <<" chunk_id : "<<cur_uop->m_chunk_id
            <<" opcode :NOP : "<<cur_uop->m_opcode <<"\n" 
            );
        //CPU core hits a begin marker - note CPU core will not have an end marker
        if(core->get_unit_type() == UNIT_LARGE && cur_uop->m_is_chunk_begin) {

          auto thread_count = m_simBase->m_sim_processes[appl_id]->m_total_threads_in_appl; 
          thread_count/=2;
          auto cpu_tid = cur_uop->m_orig_thread_id;

          auto cpu_thread_info = m_simBase->m_sim_processes[appl_id]->m_thread_trace_info[cpu_tid];
          //Update CPU current chunk_id 
          cpu_thread_info->m_cur_chunk_id = cur_uop->m_chunk_id;
          //thread_enable false - currenty CPU thread_enable not used 
          cpu_thread_info->m_thread_enable = false;

          uint64_t offload_iter = cur_uop->m_chunk_id; 

          //nav1
          // auto axc_tid = cpu_tid + thread_count; 
          //stall current thread on current core and  send cpu trace to mem_wait with offset 1
          //prerequisites for stalling a thread
          auto  cur_alloc_thread_info = core->get_alloc_thread_info(cur_uop->m_thread_id); // contains heart beat information
          //remember offset is 1 since we want to start from the next instruction 
          auto  cur_trace_info = m_simBase->m_process_manager->prepare_context_info(core,cur_uop,1);
          m_simBase->m_process_manager->context_switch(cur_trace_info, cur_alloc_thread_info, cur_uop->m_core_id, cur_uop->m_thread_id,false); 
          core->m_is_context_switch=true;
          core->stop();
          core->sched_stop();
          core->flush_pqueues(cur_uop->m_thread_id);

          //nav1
          // auto axc_thread_info = m_simBase->m_sim_processes[appl_id]->m_thread_trace_info[axc_tid];

          //if axc_thread_info == NULL 
          // axc_thread is not created yet - so when axc thread is created for chunk_id =0 
          //it should immediaely start executing - this can be done by looking at CPU thread_enable
          // ASSERT(0);
          //Check if axc_core_id is -1 - that would mean thread is not allocated to any core 

          report("LARGE CORE: CPU thread disabled after chunk begin marker \n orig_tid:"<<cpu_tid<<"\n");
          //nav10
          //cpu_tid : orig_thread_id
          //push the tid in a global queue retrieve it later when restartng it
          m_simBase->m_disabled_cpu_threads.push(CPUID_TID{ .coreid = cur_uop->m_core_id, .localtid = cur_uop->m_thread_id,  .offload_iter = offload_iter });
          m_simBase->m_dafaxc_running = true;

          //nav1
          // if(axc_thread_info != NULL) {
          //   report("axc_thread != NULL: axc_tid:"<<axc_tid);
          //   auto axc_core_id = axc_thread_info->m_cur_core_id;
          //   //enable_axc thread - used only for chunk_id = 0 
          //   axc_thread_info->m_thread_enable = true;
          //   //if axc thread is already waiting in mem_wait send it to mv_context for scheduling
          //   // axc_tid is orig_tid need to give tid - local to core in which axc_tid is allocated
          //
          //   //if axc_tid is allocated to a core then returns core_id else returns returns -1 
          //   // if axc tid is present in mem_wait , then move it to ready_context . Next time axc core schedules a thread it will 
          //   //pick threads from the ready q - and currently if the thread is not in mem_wait or if the thread is not allocated to a core 
          //   //then axc_thread_info->m_thread_enable will ensure it starts executing the axc core 
          //
          //   if( axc_core_id != -1 &&  m_simBase->m_process_manager->is_present_mem_wait_context(axc_core_id, axc_thread_info->m_thread_id,false) ) 
          //   {
          //     report("axc mv_context");
          //     //Need to find the core in which the axc_tid is currently present
          //     m_simBase->m_process_manager->mv_context_mem_wait_to_ready(axc_thread_info->m_thread_id,axc_core_id,false);
          //   }
          // }

          //FIXME Need to start the core
          //    ----sim_thread_schedule - 
          m_simBase->m_process_manager->sim_thread_schedule();
          break;
        }
        else if (core->get_unit_type() == UNIT_LARGE && !cur_uop->m_is_chunk_begin)
          ASSERT(0); //Undefined behaviour   -- chunk end marker


        //Axc core hits a marker 
        if(core->get_unit_type() == UNIT_MEDIUM ) 
        {
          report("Medium Core");
          auto axc_thread_info = m_simBase->m_sim_processes[appl_id]->m_thread_trace_info[cur_uop->m_orig_thread_id];

          //Update Chunk id 
          axc_thread_info->m_cur_chunk_id = cur_uop->m_chunk_id;
          // FOR chunk_id = 0 - when axc execution starts 
          // If the axc_thread is enabled by CPU  then the axc execution continues else send the thread to mem_wait

          auto thread_count = m_simBase->m_sim_processes[appl_id]->m_total_threads_in_appl;
          thread_count/=2;
          auto axc_tid = cur_uop->m_orig_thread_id;
          auto cpu_tid = axc_tid - thread_count;
          auto cpu_thread_info = m_simBase->m_sim_processes[appl_id]->m_thread_trace_info[cpu_tid];
          //Below condition means that axc thread was not created when cpu thread reached begin marker so start executing axc
          if(cur_uop->m_is_chunk_begin) 
          {
            if(cpu_thread_info != NULL && !cpu_thread_info->m_thread_enable  )
            {
              report(" chunk begin : CPU thread disabled CPU_tid : "<< cpu_tid);
              axc_thread_info->m_thread_enable = true;

            }

            if(!axc_thread_info->m_thread_enable  )
            {
              report("chunk begin : axc thread disabled  context switch");
              auto  cur_alloc_thread_info = core->get_alloc_thread_info(cur_uop->m_thread_id); // contains heart beat information
              //remember offset is 1 since we want to start from the next instruction 
              auto  cur_trace_info = m_simBase->m_process_manager->prepare_context_info(core,cur_uop,1);          
              m_simBase->m_process_manager->context_switch(cur_trace_info,cur_alloc_thread_info,cur_uop->m_core_id,cur_uop->m_thread_id,false); 
              core->m_is_context_switch=true;
              core->stop();
              core->sched_stop();
              core->flush_pqueues(cur_uop->m_thread_id);

              m_simBase->m_process_manager->sim_thread_schedule();
              break;
            }

            else if(axc_thread_info->m_thread_enable ) {
              report("chunk begin : update chunkid : "<<cur_uop->m_chunk_id);
              //Update Chunk id 
              axc_thread_info->m_cur_chunk_id = cur_uop->m_chunk_id;

            }

          }
          else //is_chunk_begin == 0 end marker 
          {
            // axc core  - end marker 
            if(axc_thread_info->m_thread_enable /* && !m_simBase->m_sim_processes[appl_id]->m_thread_trace_info[cur_uop->m_orig_thread_id]->m_trace_ended */ )
            {
              report("chunk end : axc_thread enable");
              if( !m_simBase->m_sim_processes[appl_id]->m_thread_trace_info[cur_uop->m_orig_thread_id]->m_trace_ended)
              {
                auto  cur_alloc_thread_info = core->get_alloc_thread_info(cur_uop->m_thread_id); // contains heart beat information
                //remember offset is 1 since we want to start from the next instruction 
                auto  cur_trace_info = m_simBase->m_process_manager->prepare_context_info(core,cur_uop,1);          
                m_simBase->m_process_manager->context_switch(cur_trace_info,cur_alloc_thread_info,cur_uop->m_core_id,cur_uop->m_thread_id,false); 
                core->m_is_context_switch=true;
                core->stop();
                core->sched_stop();
                core->flush_pqueues(cur_uop->m_thread_id);
                axc_thread_info->m_thread_enable = false;
              }

              //CPU thread mv_context 
              int cpu_core_id = -1;
              // axc_tid is orig_tid need to give tid - local to core in which axc_tid is allocated
              if (cpu_thread_info == NULL ) //Undefined behaviour 
                ASSERT(0);
              else 
              {

                cpu_core_id = cpu_thread_info->m_cur_core_id;         
                if(cpu_core_id == -1)
                  ASSERT(0);
              }

              //enable cpu thread
              cpu_thread_info->m_thread_enable = true;
              if(m_simBase->m_process_manager->is_present_mem_wait_context(cpu_core_id,cpu_thread_info->m_thread_id,false))
                m_simBase->m_process_manager->mv_context_mem_wait_to_ready(cpu_thread_info->m_thread_id,cpu_core_id,false);

              m_simBase->m_process_manager->sim_thread_schedule();
              break;
            }

            //Undefined behaviour :- if  thread_enable is false for end marker  
            else 
              ASSERT(0);
          }
        }


        report("Markers Read: RETIRE is_chunk_begin :"<<cur_uop->m_is_chunk_begin
            <<" orig_thread_id : "<<cur_uop->m_orig_thread_id
            <<" core_id : "<< cur_uop->m_core_id
            <<" chunk_id : "<<cur_uop->m_chunk_id
            <<" opcode :NOP : "<<cur_uop->m_opcode <<"\n" 
            );
      } //END of DS_FLAG   


    }



    // free uop
    for (int ii = 0; ii < cur_uop->m_num_child_uops; ++ii) {
      if (*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
        m_simBase->m_bug_detector->deallocate(cur_uop->m_child_uops[ii]);
      m_uop_pool->release_entry(cur_uop->m_child_uops[ii]->free());
    }

    if (*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
      m_simBase->m_bug_detector->deallocate(cur_uop);

    delete [] cur_uop->m_child_uops;
    m_uop_pool->release_entry(cur_uop->free());

    // release physical registers
    if (cur_uop->m_req_lb) {
      rob->dealloc_lb();
    }
    if (cur_uop->m_req_sb) {
      rob->dealloc_sb();
    }
    if (cur_uop->m_req_int_reg) {
      rob->dealloc_int_reg();
    }
    if (cur_uop->m_req_fp_reg) {
      rob->dealloc_fp_reg();
    }

  }

  if (m_core_id == 0) {
    m_simBase->m_core0_inst_count = m_insts_retired[0];
  }

  if(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE) 
    report("---------------- END OF RETIRE  CYCLE ----------");
}


// When a new thread has been scheduled, reset data
void retire_c::allocate_retire_data(int tid)
{
  m_insts_retired[tid] = 0;
  m_uops_retired[tid]  = 0;

}


void retire_c::start()
{
  m_retire_running = true;
}


void retire_c::stop()
{
  m_retire_running = false;
}


bool retire_c::is_running()
{
  return m_retire_running;
}



#if 0
// return number of retired instructions per thread
inline Counter retire_c::get_instrs_retired(int thread_id) 
{ 
  return m_insts_retired[thread_id]; 
}
#endif


// return number of retired uops per thread
Counter retire_c::get_uops_retired(int thread_id) 
{ 
  return m_uops_retired[thread_id]; 
}


// return total number of retired instructions
Counter retire_c::get_total_insts_retired() 
{ 
  return m_total_insts_retired; 
}


// whan an application is completed, update corresponding stats
void retire_c::update_stats(process_s* process)
{
  core_c* core = m_simBase->m_core_pointers[m_core_id];

  // repeating traces in case of running multiple applications
  // TOCHECK I will get back to this later
  if (*KNOB(KNOB_REPEAT_TRACE) && process->m_repeat < *KNOB(KNOB_REPEAT_TRACE_N) &&
      core->get_core_type() == "ptx") {
    if ((process->m_repeat+1) == *m_simBase->m_knobs->KNOB_REPEAT_TRACE_N) {
      --m_simBase->m_process_count_without_repeat;
      STAT_EVENT_N(CYC_COUNT_PTX, CYCLE);

      if(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE) 
        report("application " << process->m_process_id << " terminated " 
            << "(" << process->m_applications[process->m_current_vector_index-1] 
            << "," << process->m_repeat << ") at " << CYCLE);
    }
  } 
  else {
    if (process->m_repeat == 0) {
      if (core->get_core_type() == "ptx") {
        STAT_EVENT_N(CYC_COUNT_PTX, CYCLE);
      }
      else {
        STAT_EVENT_N(CYC_COUNT_X86, CYCLE);
      }
      --m_simBase->m_process_count_without_repeat;

      if(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE) 
        report("----- application " << process->m_process_id << " terminated (" 
            << process->m_applications[process->m_current_vector_index-1] 
            << "," << process->m_repeat << ") at " << CYCLE);
    }
  }
}


// repeat (terminated) trace, if necessary
void retire_c::repeat_traces(process_s* process)
{
  if ((*KNOB(KNOB_REPEAT_TRACE) || (*KNOB(KNOB_REPEAT_TRACE) && *KNOB(KNOB_REPEAT_TRACE_N) > 0)) && 
      m_simBase->m_process_count_without_repeat > 0) {
    // create duplicate process once previous one is terminated
    m_simBase->m_process_manager->create_process(process->m_kernel_config_name, process->m_repeat+1, 
        process->m_orig_pid);
    STAT_EVENT(NUM_REPEAT);
  }
}
