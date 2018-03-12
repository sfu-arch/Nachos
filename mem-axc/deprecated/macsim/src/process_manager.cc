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
 * File         : process_manager.cc
 * Author       : Jaekyu Lee
 * Date         : 1/6/2011
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : process manager (creation, termination, scheduling, ...)
 *                combine sim_process and sim_thread_schedule
 *********************************************************************************************/


///////////////////////////////////////////////////////////////////////////////////////////////
/// \page process Process management
///
/// In process, we manage each application as a process which consists of one or more
/// threads (for multi-threaded CPU or GPU applications).
///
/// \section process_core Core Partition
///
/// \section process_queue Queues
/// We maintain two separate queues for the simulation; thread queue and block queue. Thread
/// queue has CPU traces and block queue has GPU traces. Especially, block queue consists of
/// thread list. Each list in the block queue will have threads from the same thread block.
/// \see process_manager_c::m_thread_queue
/// \see process_manager_c::m_block_queue
///
/// \section process_schedule Thread Scheduling
/// When a core becomes available, based on the core type, it pulls out a thread from 
/// the corresponding queue. Each core has maximum number of threads that can run concurrently.
/// 
/// \section process_storage Storage Management
/// Since each thread requires its own data structure and GPU simulation will have huge number
/// of threads, GPU simulation will require huge memory consumption. However, since there are
/// limited number of threads at a certain time, we just need to maintain data structure for
/// those active threads. In each thread/block queue, it will have dummy thread structures.
/// When a thread is actually scheduled to the core, its data structure will be allocated.
/// 
/// \todo We need to carefully handle core allocation process. multiple x86s, multi-threaded..
///////////////////////////////////////////////////////////////////////////////////////////////


#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>

#include "assert_macros.h"
#include "knob.h"
#include "core.h"
#include "utils.h"
#include "statistics.h"
#include "frontend.h"
#include "process_manager.h"
#include "pref_common.h"
#include "trace_read.h"
#include "debug_macros.h"
#include "all_knobs.h"
#include "thread.h"

///////////////////////////////////////////////////////////////////////////////////////////////


#define BLOCK_ID_SHIFT 16 
#define THREAD_ID_MASK 0xFFF 
#define BLOCK_ID_MOD 16384 

#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_SIM_THREAD_SCHEDULE, ## args)
///////////////////////////////////////////////////////////////////////////////////////////////


// thread_stat_s constructor
thread_stat_s::thread_stat_s()
{
  m_thread_id          = 0;
  m_unique_thread_id   = 0;
  m_block_id           = 0;
  m_thread_sched_cycle = 0;
  m_thread_fetch_cycle = 0;
  m_thread_end_cycle   = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//  process_s() - constructor
////////////////////////////////////////////////////////////////////////////////
process_s::process_s()
{
  m_process_id = 0;
  m_orig_pid = 0;
  m_max_block = 0;
  m_thread_start_info = NULL;
  m_thread_trace_info = NULL;
  m_no_of_threads = 0;
  m_no_of_threads_created = 0;
  m_no_of_threads_terminated = 0;
  m_core_pool = NULL;
  m_ptx = false;
  m_repeat = 0;
  m_current_file_name_base = "";
  m_kernel_config_name = "";
  m_current_vector_index = 0;
  m_inst_count_tot = 0;
  m_block_count          = 0;
}


////////////////////////////////////////////////////////////////////////////////
//  process_s() - constructor
////////////////////////////////////////////////////////////////////////////////
process_s::~process_s()
{
}

////////////////////////////////////////////////////////////////////////////////
//  block_schedule_info_s() - constructor
////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////


// block_schedule_info_s constructor
block_schedule_info_s::block_schedule_info_s()
{
  m_start_to_fetch        = false;
  m_dispatched_core_id    = -1;
  m_retired               = false;
  m_dispatched_thread_num = 0;
  m_retired_thread_num    = 0;
  m_dispatch_done         = false;
  m_trace_exist           = false;
  m_total_thread_num      = 0;
}


block_schedule_info_s::~block_schedule_info_s()
{
}


///////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//  process_manager_c() - constructor
//   m_thread_queue - contains the list of unassigned threads (from all 
//  applications) that are ready to be launched
//   m_block_queue - contains the list of unassigned blocks (from all 
//  applications) that are ready to be launched
////////////////////////////////////////////////////////////////////////////////
process_manager_c::process_manager_c(macsim_c* simBase)
{
  //base simulation reference
  m_simBase = simBase;
  counter=0;
  // allocate queues
  m_thread_queue = new list<thread_trace_info_node_s *>;
  m_thread_queue_med = new list<thread_trace_info_node_s *>;
  m_block_queue  = new unordered_map<int, list<thread_trace_info_node_s *> *>;
  m_inst_hash_pool = new pool_c<hash_c<inst_info_s> >(1, "inst_hash_pool");
  m_context_handler = new co_switch_handler_c();
}


////////////////////////////////////////////////////////////////////////////////
//  process_manager_c() - destructor
////////////////////////////////////////////////////////////////////////////////
process_manager_c::~process_manager_c()
{
  // deallocate thread queue
  m_thread_queue->clear();
  m_thread_queue_med->clear();
  delete m_thread_queue;
  delete m_thread_queue_med;

  // deallocate block queue
  m_block_queue->clear();
  delete m_block_queue;
  delete m_context_handler;
}


////////////////////////////////////////////////////////////////////////////////
//  process_manager_c::create_thread_node()
//   called for each thread/warp when it becomes ready to be launched (started);
//  allocates a node for the thread/warp and add its to m_thread_queue (for x86)
//  or m_block_queue (for ptx)
////////////////////////////////////////////////////////////////////////////////
void process_manager_c::create_thread_node(process_s* process, int tid, bool main)
{
  // create a new thread node
  thread_trace_info_node_s* node = m_simBase->m_trace_node_pool->acquire_entry();

  // initialize the node
  node->m_trace_info_ptr = NULL;
  node->m_process        = process;
  node->m_tid            = tid;
  node->m_main           = main;
  node->m_ptx            = process->m_ptx;

  // create a new thread start information
  thread_start_info_s *start_info = &(process->m_thread_start_info[tid]);

  // TODO (jaekyu, 8-2-2010)
  // check block id assignment policy

  // block id assignment in case of multiple applications
  int block_id = start_info->m_thread_id >> BLOCK_ID_SHIFT; 
  node->m_block_id = m_simBase->m_block_id_mapper->find(process->m_process_id, 
      block_id + process->m_kernel_block_start_count[process->m_current_vector_index-1]);

  // new block has been executed.
  if (node->m_block_id == -1) {
    node->m_block_id = m_simBase->m_block_id_mapper->insert(process->m_process_id, 
        block_id + process->m_kernel_block_start_count[process->m_current_vector_index-1]);

    // increase total block count
    ++process->m_block_count;
  }

  // update process block list
  process->m_block_list[node->m_block_id] = true;


  // add a new node to m_thread_queue (for x86) or m_block_queue (for ptx)
  if (process->m_ptx == true) 
    insert_block(node);
  else  
    insert_thread(node);

  // increase number of active threads
  ++m_simBase->m_num_active_threads;
}


////////////////////////////////////////////////////////////////////////////////
//  process_manager_c::create_thread_node()
//   creates a process and opens the trace for the main thread of the process
////////////////////////////////////////////////////////////////////////////////
int process_manager_c::create_process(string appl)	
{
  return create_process(appl, 0, 0);
}


// Creates a process and opens the trace for the main thread of the process
int process_manager_c::create_process(string appl, int repeat, int pid)
{
  ///
  /// Create a process and setup id and other fields
  ///

  // create a new process
  process_s* process = new process_s;

  // create new instruction hash table to reduce cost of decoding trace instructions
  string name;
  stringstream sstr;

  sstr << "inst_info_" << m_simBase->m_process_count;
  sstr >> name;

  hash_c<inst_info_s>* new_inst_hash = m_inst_hash_pool->acquire_entry();
  m_simBase->m_inst_info_hash[m_simBase->m_process_count]  = new_inst_hash;


  // process data structure setup
  process->m_repeat             = repeat;
  process->m_process_id         = m_simBase->m_process_count++;
  process->m_kernel_config_name = appl;

  // keep original process id in case of the repeated application
  if (repeat == 0) {
    ++m_simBase->m_process_count_without_repeat;
    process->m_orig_pid = process->m_process_id;
  }
  else {
    process->m_orig_pid = pid;
  }

  m_simBase->m_sim_processes[process->m_orig_pid] = process; 


  ///
  /// Get configurations from the file
  ///


  // Setup file name of process trace
  unsigned int dot_location = appl.find_last_of(".");
  if (dot_location == string::npos)
    ASSERTM(0, "file(%s) formats should be <appl_name>.<extn>\n", appl.c_str());

  // open trace configuration file
  ifstream trace_config_file;
  trace_config_file.open(appl.c_str(), ifstream::in);

  // open trace configuration file
  string trace_type;
  if (trace_config_file.fail()) {
    STAT_EVENT(FILE_OPEN_ERROR);
    ASSERTM(0, "filename:%s cannot be opened\n", appl.c_str());
  }

  // Read the first line of trace configuration file
  // Get (#thread, type)
  int thread_count;
  if (!(trace_config_file >> thread_count >> trace_type)) 
    ASSERTM(0, "error reading from file:%s", appl.c_str());


  // To support new trace version : each kernel has own configuration and some applications
  // may have multiple kernels
  if (thread_count == -1) {
    string kernel_directory;
    while (trace_config_file >> kernel_directory) {
      string kernel_path = appl.substr(0, appl.find_last_of('/'));
      kernel_path += kernel_directory.substr(kernel_directory.rfind('/', 
            kernel_directory.find_last_of('/')-1), kernel_directory.length());
      // When a trace directory is moved to the different path,
      // since everything is coded in absolute path, this will cause problem.
      // By taking relative path here, the problem can be solved
      process->m_applications.push_back(kernel_path);
      //process->m_applications.push_back(kernel_directory);
      process->m_kernel_block_start_count.push_back(0);
    }
  }
  else {
    process->m_total_threads_in_appl = thread_count; //total number of threads read from trace.txt
    process->m_applications.push_back(appl);
    process->m_kernel_block_start_count.push_back(0);
  }

  // setup core pool
  if (trace_type == "ptx" || trace_type == "newptx") {
    process->m_ptx = true;
    process->m_core_pool = &m_simBase->m_ptx_core_pool;
  }
  else {
    process->m_ptx = false;
    process->m_core_pool = &m_simBase->m_x86_core_pool;
  }

  m_simBase->m_PCL->set_appl_type(process->m_orig_pid, process->m_ptx);

  // now we set up a new process to execute it
  setup_process(process);

  return 0;
}


// setup a process to run. Each process has vector which holds all sub-applications.
// (GPU with multiple kernel calls). 
void process_manager_c::setup_process(process_s* process)
{
  if(*m_simBase->m_knobs->KNOB_DEBUG_SIM_THREAD_SCHEDULE)
    report("setup_process:" << process->m_orig_pid << " " 
        << process->m_applications[process->m_current_vector_index]
        << " current_index:" << process->m_current_vector_index << " (" 
        << process->m_applications.size() << ")");

  ASSERT(process->m_current_vector_index < process->m_applications.size());

  // Each block within an application (across multiple kernels) has unique id
  process->m_kernel_block_start_count[process->m_current_vector_index] = 
    process->m_block_count;

  // trace file name
  string trace_info_file_name = process->m_applications[process->m_current_vector_index++];

  unsigned int dot_location = trace_info_file_name.find_last_of(".");
  if (dot_location == string::npos)
    ASSERTM(0, "file(%s) formats should be <appl_name>.<extn>\n", 
        trace_info_file_name.c_str());

  // get the base name of current application (without extension)
  process->m_current_file_name_base = trace_info_file_name.substr(0, dot_location);


  // open TRACE_CONFIG file
  ifstream trace_config_file;
  trace_config_file.open(trace_info_file_name.c_str(), ifstream::in);
  if (trace_config_file.fail()) {
    STAT_EVENT(FILE_OPEN_ERROR);
    ASSERTM(0, "trace_config_file:%s\n", trace_info_file_name.c_str());
  }

  // -------------------------------------------
  // TRACE_CONFIG Format
  // -------------------------------------------
  // #Threads | Trace Type | (Optional Fields)
  // 1st Thread ID    | #Instructions
  // 2nd Thread ID    | #Instructions
  // ....
  // nth Thread ID    | #Instructions

  // read the first line of TRACE_CONFIG file 
  // X86 Traces : (#Threads | x86)
  // GPU Traces (OLD) : (#Warps | ptx)
  // GPU Traces (NEW) : (#Warps | newptx | #MaxBlocks per Core)
  int thread_count;
  string trace_type;
  if (!(trace_config_file >> thread_count >> trace_type)) 
    ASSERTM(0, "error reading from file:%s", trace_info_file_name.c_str());

  if (trace_type == "ptx") {
    process->m_max_block = *m_simBase->m_knobs->KNOB_MAX_BLOCK_PER_CORE;
  }
  if (trace_type == "newptx") {
    if (!(trace_config_file >> process->m_max_block))
      ASSERTM(0, "error reading from file:%s", trace_info_file_name.c_str());
    trace_type = "ptx";
    if (*m_simBase->m_knobs->KNOB_MAX_BLOCK_PER_CORE_SUPER > 0) {
      process->m_max_block = *m_simBase->m_knobs->KNOB_MAX_BLOCK_PER_CORE_SUPER;
    }
  }

  // # thread_count > 0
  if (thread_count <= 0) 
    ASSERTM(0, "invalid thread count:%d", thread_count);

  if(*m_simBase->m_knobs->KNOB_DEBUG_SIM_THREAD_SCHEDULE)
    report("thread_count:" << thread_count);


  // create data structures
  thread_stat_s *new_stat = new thread_stat_s[thread_count];
  m_simBase->m_thread_stats[process->m_process_id] = new_stat;

  m_simBase->m_all_threads += thread_count;

  process->m_no_of_threads     = thread_count;	
  process->m_thread_start_info = new thread_start_info_s[thread_count];	
  process->m_thread_trace_info = new thread_s*[thread_count];	

  if (process->m_thread_start_info == NULL || process->m_thread_trace_info == NULL)	{
    ASSERTM(0, "unable to allocate memory\n");
  }

  // read each thread's information (thread id, # of starting instruction (spawn))
  // This should start to read the first field of the second line in TRACE_CONFIG.
  for (int ii = 0; ii < thread_count; ++ii) { 
    if (!(trace_config_file >> process->m_thread_start_info[ii].m_thread_id 
          >> process->m_thread_start_info[ii].m_inst_count)) { 
      ASSERTM(0, "error reading from file:%s ii:%d\n", trace_info_file_name.c_str(), ii);
    }
  }

  // close TRACE_CONFIG file
  trace_config_file.close();


  // GPU simulation
  if (true == process->m_ptx) { 
    string path = process->m_current_file_name_base;
    path += "_info.txt";

    // read trace information file
    ifstream trace_lengths_file(path.c_str());
    if (!trace_lengths_file.good())
      ASSERTM(0, "could not open file: %s\n", path.c_str());

    // FIXME
    Counter inst_count_tot = 0; 
    Counter inst_count;
    Counter thread_id;

    for (int ii = 0; ii < thread_count; ++ii) {
      trace_lengths_file >> thread_id >> inst_count;
      inst_count_tot += inst_count;
    }
    trace_lengths_file.close();

    process->m_inst_count_tot = inst_count_tot;


    // FIXME
    // TODO (jaekyu, 1-30-2009)
    // fix this

    // Calculate the number of threads (warps) per block 
    // Currently, (n * 65536) is assigned to the first global warp ID for the nth block.
    m_simBase->m_no_threads_per_block = 0;
    for (int ii = 0; ii < thread_count; ++ii) { 
      if (process->m_thread_start_info[ii].m_thread_id < 100)
        ++m_simBase->m_no_threads_per_block;
      else
        break;
    }

    queue<int> *core_pool = process->m_core_pool;

    // Allocate cores to this application (bi-directonal)
    // get maximum allowed
    if (*m_simBase->m_knobs->KNOB_MAX_NUM_CORE_PER_APPL == 0) {
      while (!core_pool->empty()) {
        int core_id = core_pool->front();
        core_pool->pop();

        process->m_core_list[core_id] = true;
        m_simBase->m_core_pointers[core_id]->init();
        m_simBase->m_core_pointers[core_id]->add_application(0, process);
      }
    }
    // get limited (*m_simBase->m_knobs->KNOB_MAX_NUM_CORE_PER_APPL) number of cores
    else {
      int count = 0;
      while (!core_pool->empty() && count < *m_simBase->m_knobs->KNOB_MAX_NUM_CORE_PER_APPL) {
        int core_id = core_pool->front();
        core_pool->pop();

        process->m_core_list[core_id] = true;
        m_simBase->m_core_pointers[core_id]->init();
        m_simBase->m_core_pointers[core_id]->add_application(0, process);

        ++count;
      }
    }
  }


  // TODO (jaekyu, 1-30-2009)
  // FIXME
  if (trace_type == "ptx" && *KNOB(KNOB_BLOCKS_TO_SIMULATE)) {
    if ((*KNOB(KNOB_BLOCKS_TO_SIMULATE) * m_simBase->m_no_threads_per_block) < 
        static_cast<unsigned int>(thread_count)) { 
      uns temp = thread_count;
      thread_count = *KNOB(KNOB_BLOCKS_TO_SIMULATE) * m_simBase->m_no_threads_per_block;

      //print the thread count of the application at the beginning
      if(*m_simBase->m_knobs->KNOB_DEBUG_SIM_THREAD_SCHEDULE)
        report("new thread count " << thread_count<<"\n");	
      process->m_no_of_threads = thread_count;	//assign thread_count to the number of threads of the process

      m_simBase->m_all_threads += thread_count;

      if (*KNOB(KNOB_SIMULATE_LAST_BLOCKS)) {
        for (int i = 0; i < thread_count; ++i) {
          process->m_thread_start_info[i].m_thread_id = 
            process->m_thread_start_info[temp - thread_count + i].m_thread_id;
          process->m_thread_start_info[i].m_inst_count = 
            process->m_thread_start_info[temp - thread_count + i].m_inst_count;
        }
      }
    }
  }

  // Initialize stats (this will be used to determine process termination
  process->m_no_of_threads_created    = 1;
  process->m_no_of_threads_terminated = 0;

  // Insert the main thread to the pool
  create_thread_node(process, 0, true);
}


// terminate a process
bool process_manager_c::terminate_process(process_s* process)
{
  delete []process->m_thread_start_info;
  delete []process->m_thread_trace_info;
  process->m_block_list.clear();

  // TODO (jaekyu, 2-3-2010)
  // We may need to change this using pool_c
  m_simBase->m_inst_info_hash[process->m_process_id]->clear();

  // deallocate data structures
  thread_stat_s* thread_stat_data_to_delete = m_simBase->m_thread_stats[process->m_process_id];
  m_simBase->m_thread_stats.erase(process->m_process_id);
  delete[] thread_stat_data_to_delete;


  // Since there are more kernels within an application, we need to finish these
  // before terminate this application
  if (process->m_current_vector_index < process->m_applications.size() &&
      (*m_simBase->m_ProcessorStats)[INST_COUNT_TOT].getCount() < *KNOB(KNOB_MAX_INSTS1)) {
    setup_process(process);

    return false;
  }

  m_simBase->m_block_id_mapper->delete_table(process->m_process_id);

  // release allocated cores
  for (auto I = process->m_core_list.begin(), E = process->m_core_list.end(); I != E; ++I) {
    int core_id = (*I).first;
    process->m_core_pool->push(core_id); 

    m_simBase->m_core_pointers[core_id]->delete_application(process->m_process_id);
  }
  process->m_core_list.clear();


  hash_c<inst_info_s>* inst_info_hash = m_simBase->m_inst_info_hash[process->m_process_id];
  m_simBase->m_inst_info_hash.erase(process->m_process_id);
  inst_info_hash->clear();
  m_inst_hash_pool->release_entry(inst_info_hash);


  stringstream sstr;
  string ext;
  sstr << "." << process->m_process_id;
  sstr >> ext;

  int delta;
  if (m_appl_cyccount_info.find(process->m_orig_pid) == m_appl_cyccount_info.end()) {
    delta = static_cast<int>(CYCLE);
  }
  else {
    delta = static_cast<int>(CYCLE - m_appl_cyccount_info[process->m_orig_pid]);
  }
  m_appl_cyccount_info[process->m_orig_pid] = CYCLE;


  STAT_EVENT_N(APPL_CYC_COUNT0 + process->m_orig_pid, delta);
  STAT_EVENT(APPL_CYC_COUNT_BASE0 + process->m_orig_pid);

  // FIXME (jaekyu, 4-2-2010)
  // need to handle repeat_trace_n > 0 cases
  if (process->m_repeat == 0) {
    m_simBase->m_ProcessorStats->saveStats(ext);
  }

  return true;
}


// try to give a unique thread id
// counter to assign unique thread_ids to threads/warps
// nav*
// static int global_unique_thread_id = 0;


// create a new thread (actually, a thread has been created when create_thread_node()
// has been called. However, when a thread is actually scheduled, we allocate and initialize
// data in a thread.
thread_s *process_manager_c::create_thread(process_s* process, int tid, bool main)
{
  thread_s* trace_info = m_simBase->m_thread_pool->acquire_entry(m_simBase);
  process->m_thread_trace_info[tid] = trace_info;
  thread_start_info_s* start_info = &process->m_thread_start_info[tid];

  int block_id = start_info->m_thread_id >> BLOCK_ID_SHIFT;

  // original block id
  trace_info->m_orig_block_id = block_id;
  trace_info->m_block_id = m_simBase->m_block_id_mapper->find(process->m_process_id, block_id + 
      process->m_kernel_block_start_count[process->m_current_vector_index-1]);


  // FIXME
  trace_info->m_unique_thread_id = (start_info->m_thread_id) % BLOCK_ID_MOD; 
  //nav*
  // trace_info->m_orig_thread_id = global_unique_thread_id++;
  trace_info->m_orig_thread_id = tid;

  // set up trace file name
  stringstream sstr;
  sstr << process->m_current_file_name_base << "_" << start_info->m_thread_id << ".raw";

  string filename = "";
  sstr >> filename;

  // open trace file
  trace_info->m_trace_file = gzopen(filename.c_str(), "r");
  if (trace_info->m_trace_file == NULL) 
    ASSERTM(0, "error opening trace file:%s\n", filename.c_str());

  trace_info->m_file_opened      = true;
  trace_info->m_trace_ended      = false;
  trace_info->m_ptx              = process->m_ptx; 
  trace_info->m_buffer_index     = 0;
  trace_info->m_buffer_index_max = 0;
  trace_info->m_buffer_exhausted = true;
  trace_info->m_inst_count       = 0;
  trace_info->m_uop_count        = 0; 
  trace_info->m_process          = process;	
  trace_info->m_main_thread      = main;
  trace_info->m_temp_inst_count  = 0;
  trace_info->m_temp_uop_count   = 0;
  trace_info->m_thread_init      = true;
  trace_info->m_num_sending_uop  = 0;
  trace_info->m_bom              = true;
  trace_info->m_eom              = true;

  trace_info->m_fetch_data->init();

  //nav
  auto thread_count = process->m_total_threads_in_appl;
  thread_count = thread_count/2;
  if(thread_count > tid )  //Then its a CPU TRace 
    trace_info->m_thread_enable = true;

  return trace_info;
}


// terminate a thread
int process_manager_c::terminate_thread(int core_id, thread_s* trace_info, int thread_id, 
    int b_id)	
{
  core_c* core = m_simBase->m_core_pointers[core_id];

  ASSERT(core->m_running_thread_num); 

  int  block_id = trace_info->m_block_id; 
  --core->m_running_thread_num;

  // All threads have been terminated in a core. Mark core as ended.
  if (core->m_running_thread_num == 0)
    m_simBase->m_core_end_trace[core_id] = true;

  // Mark thread terminated
  core->m_thread_finished[thread_id] = 1; 

  // final heartbeat for the thread
  core->final_heartbeat(thread_id);

  // deallocate data structures
  core->deallocate_thread_data(thread_id);

  DEBUG("core_id:%d terminated block: %d thread - %d running_thread_num:%d block_id:%d \n", 
      core_id, trace_info->m_block_id, trace_info->m_thread_id, core->m_running_thread_num, 
      b_id);

  // update number of terminated thread for an application
  ++trace_info->m_process->m_no_of_threads_terminated;

  // decrement number of active threads
  --m_simBase->m_num_active_threads;

  // GPU simulation
  if (trace_info->m_ptx == true) { 
    int t_process_id = trace_info->m_process->m_process_id;
    int t_thread_id  = trace_info->m_unique_thread_id;
    m_simBase->m_thread_stats[t_process_id][t_thread_id].m_thread_end_cycle = 
      core->get_cycle_count();

    block_schedule_info_s* block_info = m_simBase->m_block_schedule_info[block_id];
    ++block_info->m_retired_thread_num;

    DEBUG("core_id:%d block_id:%d block_retired_thread_num:%d block_total_thread_num:%d "
        "running_block_num:%d\n",
        core_id, block_id, block_info->m_retired_thread_num, block_info->m_total_thread_num, 
        core->m_running_block_num); 


    // BLOCK RETIRE : all threads in a block have been retired, so the block is retired now
    if (block_info->m_retired_thread_num == block_info->m_total_thread_num) { 
      block_info->m_retired = true; 
      --core->m_running_block_num; 

      // deallocate block information
      block_schedule_info_s* block_schedule_info = m_simBase->m_block_schedule_info[block_id];
      m_simBase->m_block_schedule_info.erase(block_id);
      delete block_schedule_info;

      trace_info->m_process->m_block_list.erase(block_id);
      list<thread_trace_info_node_s*> *block_queue = (*m_block_queue)[block_id];
      m_block_queue->erase(block_id);
      block_queue->clear();
      delete block_queue;


      // stats
      STAT_EVENT_N(AVG_BLOCK_EXE_CYCLE, CYCLE - block_info->m_sched_cycle);
      STAT_EVENT(AVG_BLOCK_EXE_CYCLE_BASE);

      ++m_simBase->m_total_retired_block;
      if (*m_simBase->m_knobs->KNOB_MAX_BLOCKS_TO_SIMULATE > 0 && 
          m_simBase->m_total_retired_block >= *m_simBase->m_knobs->KNOB_MAX_BLOCKS_TO_SIMULATE) { 
        m_simBase->m_end_simulation = true;
      }
    }


    // FIXME TONAGESH

    // delete all synchronization information
    section_info_s* info;
    while (trace_info->m_sections.size() > 0) { 
      info = trace_info->m_sections.front();

      trace_info->m_sections.pop_front();
      m_simBase->m_section_pool->release_entry(info);
    }

    while (trace_info->m_bar_sections.size() > 0) { 
      info = trace_info->m_bar_sections.front();

      trace_info->m_bar_sections.pop_front();
      m_simBase->m_section_pool->release_entry(info);
    }

    while (trace_info->m_mem_sections.size() > 0) { 
      info = trace_info->m_mem_sections.front();

      trace_info->m_mem_sections.pop_front();
      m_simBase->m_section_pool->release_entry(info);
    }

    while (trace_info->m_mem_bar_sections.size() > 0) { 
      info = trace_info->m_mem_bar_sections.front();

      trace_info->m_mem_bar_sections.pop_front();
      m_simBase->m_section_pool->release_entry(info);
    }

    while (trace_info->m_mem_for_bar_sections.size() > 0) { 
      info = trace_info->m_mem_for_bar_sections.front();

      trace_info->m_mem_for_bar_sections.pop_front();
      m_simBase->m_section_pool->release_entry(info);
    }

    ASSERT(core->m_running_block_num >= 0); 
  }

  // cloase trace file
  gzclose(trace_info->m_trace_file); 

  // release thread_trace_info to the pool
  m_simBase->m_thread_pool->release_entry(trace_info);


  // if there are remaining threads, schedule it
  if (m_simBase->m_num_waiting_dispatched_threads) 
    sim_thread_schedule(); 

  return 0;
}


//Stall a thread 

int process_manager_c::stall_thread(int core_id, thread_s* trace_info, int thread_id, 
    int b_id)	
{
  core_c* core = m_simBase->m_core_pointers[core_id];
  cout<<"core->m_running_thread_num : "<<core->m_running_thread_num<<endl;
  ASSERT(core->m_running_thread_num); 
  DEBUG("core_id:%d terminated block: %d thread - %d running_thread_num:%d block_id:%d \n", 
      core_id, trace_info->m_block_id, trace_info->m_thread_id, core->m_running_thread_num, 
      b_id);

  sim_thread_schedule(); 

  return 0;
}



// insert a new thread
void process_manager_c::insert_thread(thread_trace_info_node_s *incoming)
{
  ++m_simBase->m_num_waiting_dispatched_threads;
  // if(incoming->m_tid > *KNOB(KNOB_NUM_SIM_MEDIUM_CORES)
  auto thread_count =  incoming->m_process->m_total_threads_in_appl ;
  thread_count = thread_count/2;

  //Map all the threads belonging to medium cores in m_thread_queue_med 
  //say total threads are 12 the tid - 0-5 belong to m_thread_queue 
  //and tid 6- 11 belong to m_thread_queue_med
  report(" thread_count:"<<thread_count<<" incoming_tid:"<<incoming->m_tid);
  //FIXME : nav
  //Medium cores are not allocated any threads
  if(thread_count > incoming->m_tid )
    m_thread_queue->push_back(incoming);

  //FIXME: nav 
  //Disabled Medium Core for our experiments
  // else 
  // {
  //
  //   m_thread_queue_med->push_back(incoming);
  //
  // }

}

// insert a new thread block
void process_manager_c::insert_block(thread_trace_info_node_s *incoming)
{
  ++m_simBase->m_num_waiting_dispatched_threads;
  int block_id = incoming->m_block_id; 
  if (m_simBase->m_block_schedule_info.find(block_id) == m_simBase->m_block_schedule_info.end()) {
    block_schedule_info_s* block_schedule_info = new block_schedule_info_s;
    m_simBase->m_block_schedule_info[block_id] = block_schedule_info;
  }

  ++m_simBase->m_block_schedule_info[block_id]->m_total_thread_num;
  m_simBase->m_block_schedule_info[block_id]->m_trace_exist = true; 

  DEBUG("block_schedule_info[%d].trace_exist:%d\n", 
      block_id, m_simBase->m_block_schedule_info[block_id]->m_trace_exist); 

  if (m_block_queue->find(block_id) == m_block_queue->end()) {
    list<thread_trace_info_node_s *> *new_list = new list<thread_trace_info_node_s *>;
    (*m_block_queue)[block_id] = new_list;
  }
  (*m_block_queue)[block_id]->push_back(incoming);
}


// fetch a new thread
thread_trace_info_node_s *process_manager_c::fetch_thread(void)
{
  if (m_thread_queue->empty()) {
    return NULL;
  }

  ASSERT(m_simBase->m_num_waiting_dispatched_threads > 0);

  --m_simBase->m_num_waiting_dispatched_threads;

  thread_trace_info_node_s *front = m_thread_queue->front();
  m_thread_queue->pop_front();

  return front;
}



// fetch a new thread
thread_trace_info_node_s *process_manager_c::fetch_thread(Unit_Type  _core_type)
{

  if (_core_type == UNIT_MEDIUM  && m_thread_queue_med->empty() ) {
    return NULL;
  }
  
  if (_core_type != UNIT_MEDIUM  && m_thread_queue->empty() ) {
    return NULL;
  }

  ASSERT(m_simBase->m_num_waiting_dispatched_threads > 0);

  --m_simBase->m_num_waiting_dispatched_threads;

  thread_trace_info_node_s *front;
  //For Medium Cores
  if(_core_type == UNIT_MEDIUM) {
    front = m_thread_queue_med->front();
    m_thread_queue_med->pop_front();

  }
  //if(_core_type == Large) {
  //For small and Large cores 
  else {
    front = m_thread_queue->front();
    m_thread_queue->pop_front();

  }

  return front;
}

  // get a thread from a block 
// fetch_block is a misnomer - it actually fetches a warp from the specified block
thread_trace_info_node_s *process_manager_c::fetch_block(int block_id)
{
  list<thread_trace_info_node_s *> *block_list = (*m_block_queue)[block_id];
  if (block_list->empty()) {
    return NULL;
  }

  ASSERT(m_simBase->m_num_waiting_dispatched_threads > 0);

  --m_simBase->m_num_waiting_dispatched_threads;

  thread_trace_info_node_s *front = block_list->front();
  block_list->pop_front();

  return front;
} 


// schedule a thread
// assigns a new thread/block to a core if the number of live threads/blocks
// on the core are fewer than the maximum allowed
void process_manager_c::sim_thread_schedule(void)
{
  thread_trace_info_node_s* trace_to_run;

  // assign the fetched thread to that core's active queue
  for (int core_id = 0; core_id < *KNOB(KNOB_NUM_SIM_CORES); ++core_id)  {
    core_c* core = m_simBase->m_core_pointers[core_id];

    if (*KNOB(KNOB_ROUTER_PLACEMENT) == 1 &&
        core->get_core_type() != "ptx" &&
        (core_id < *KNOB(KNOB_CORE_ENABLE_BEGIN) || core_id > *KNOB(KNOB_CORE_ENABLE_END))) 
      continue;
    //-----------------------------------------------
    //
    // IF thread entry exists in m_ready_context_vec  then just schedule that thread
    // Other wise create a new thread from the thread_pool
    // naveen
    // auto context_to_run =  m_context_handler->is_ready_context_vec_empty(core_id);
    // Round Robin schedular 
    context_s* context_to_run =  m_context_handler->rr_ready_context(core_id);
    if (core->m_running_thread_num <= core->get_max_threads_per_core() 
        && (core->get_core_type() != "ptx") 
        && (context_to_run != NULL)  )  {

      this->schedule_old_context(context_to_run,core_id);
      continue;
    }
    //-- else if schedlue_next_chunk from the chunk_finished thread 
    // ready q runs all the threads with current chunk id same as the core chunk id once a chunk is finished 
    // it goes to the chunk_finished_q which is executed here after all the chunks have finished executing 

    //--------------------------------------------
    if (core->m_running_thread_num < core->get_max_threads_per_core()) {
      // schedule a thread to x86 core
      if (core->get_core_type() != "ptx") {
        if (m_simBase->m_num_waiting_dispatched_threads) { 

          // fetch a new thread
          // trace_to_run = fetch_thread();
          // Core_Type core_type = UNIT_MEDIUM  ;
          trace_to_run = fetch_thread(core->get_unit_type() );


          if (trace_to_run != NULL) {

          report("Unit Type -core_id :"<<core_id <<" unit_type:" <<core->get_unit_type() <<" is mapped to :orig_tid:"<<trace_to_run->m_tid );
            // create a new thread
            trace_to_run->m_trace_info_ptr = create_thread(trace_to_run->m_process, 
                trace_to_run->m_tid, trace_to_run->m_main);


            //When a thread is allocated to a core - add cur_core_id info to thread_s (trace_to_run) default value is -1
            trace_to_run->m_trace_info_ptr->m_cur_core_id = core_id;

            // unique thread num of a core
            int unique_scheduled_thread_num = core->m_unique_scheduled_thread_num;          
            trace_to_run->m_trace_info_ptr->m_thread_id = unique_scheduled_thread_num; 

            // add a new application to the core
            core->add_application(unique_scheduled_thread_num, trace_to_run->m_process);

            // add a new thread trace information
            core->create_trace_info(unique_scheduled_thread_num, trace_to_run->m_trace_info_ptr); 

            //in case  there was a context switch - the core would stop - 
            //if the core is re-scedhuled later and there is a thread to be scheduled 
            //it should be able to run on the core - for that the core needs to restart 
            //- And so the below line ensures :- core restarts if a thread is fetched 
            
            // core->start();

            if(*m_simBase->m_knobs->KNOB_DEBUG_SIM_THREAD_SCHEDULE)
              report("schdeuling new  thread_id:"<<unique_scheduled_thread_num <<"\n");


            cout<<"thread to set :locla_tid :"<<trace_to_run->m_trace_info_ptr->m_thread_id
              <<"\t orig_tid:"<<trace_to_run->m_trace_info_ptr->m_orig_thread_id
              <<"\n";
            // set flag for the simulation
            m_simBase->m_core_end_trace[core_id] = false;
            m_simBase->m_sim_end[core_id]        = false;
            m_simBase->m_core_started[core_id]   = true; 

            // release the node entry
            m_simBase->m_trace_node_pool->release_entry(trace_to_run);
          }
        }
      }
      // GPU simulation
      else if (core->get_core_type() == "ptx") { 
        // get currently fetching id
        int prev_fetching_block_id = core->m_fetching_block_id;

        // get block id
        int block_id = sim_schedule_thread_block(core_id); 

        // no thread to schedule
        if (block_id == -1) 
          continue;

        // find a new thread
        trace_to_run = fetch_block(block_id);

        // try to schedule as many threads as possible in the same block
        while (trace_to_run != NULL) { 
          //create a new thread
          trace_to_run->m_trace_info_ptr = create_thread(trace_to_run->m_process, 
              trace_to_run->m_tid, trace_to_run->m_main);

          // increment dispatched thread number of a block
          ++m_simBase->m_block_schedule_info[block_id]->m_dispatched_thread_num;

          // unique thread num of a core
          int unique_scheduled_thread_num = core->m_unique_scheduled_thread_num;
          trace_to_run->m_trace_info_ptr->m_thread_id = unique_scheduled_thread_num; 

          // add a new application to the core
          core->add_application(unique_scheduled_thread_num, trace_to_run->m_process);

          // add a new thread trace information
          core->create_trace_info(unique_scheduled_thread_num, trace_to_run->m_trace_info_ptr); 


          m_simBase->m_trace_reader->pre_read_trace(trace_to_run->m_trace_info_ptr);

          // set flag for the simulation
          m_simBase->m_core_end_trace[core_id] = false;
          m_simBase->m_sim_end[core_id]        = false;
          m_simBase->m_core_started[core_id]   = true; 

          // for thread start end cycles
          uint32_t unique_thread_id = trace_to_run->m_trace_info_ptr->m_unique_thread_id;
          uint32_t process_id       = trace_to_run->m_trace_info_ptr->m_process->m_process_id;
          ASSERT(unique_thread_id < m_simBase->m_all_threads);

          thread_stat_s* thread_stat = &m_simBase->m_thread_stats[process_id][unique_thread_id]; 

          thread_stat->m_unique_thread_id   = unique_thread_id; 
          thread_stat->m_block_id           = block_id;	   
          thread_stat->m_thread_sched_cycle = core->get_cycle_count();

          if (core->m_running_thread_num == core->get_max_threads_per_core()) 
            break;

          // release the node entry
          m_simBase->m_trace_node_pool->release_entry(trace_to_run);

          // try to schedule other threads in the same block
          prev_fetching_block_id = core->m_fetching_block_id;
          block_id = sim_schedule_thread_block(core_id);
          if (block_id == -1) 
            break;

          // fetch a new thread
          trace_to_run = fetch_block(block_id);
        }
      }
    }
  }
}


// assign a new block to the core
// get the block id of the warp to be next assigned to the core
// assignment of warps from a block doesn't happen in one go, it is done one 
// warp at a time, hence this function is called once for each warp. however, 
// all warps of a block get assigned in the same cycle (TODO: make it 1 call)
int process_manager_c::sim_schedule_thread_block(int core_id) 
{
  core_c* core          = m_simBase->m_core_pointers[core_id];
  int new_block_id      = -1; 
  int fetching_block_id = core->m_fetching_block_id; 
  DEBUG("core:%d fetching_block_id:%d total_thread_num:%d dispatched_thread_num:%d "
      "running_block_num:%d block_retired:%d \n",
      core_id, fetching_block_id, 
      (m_simBase->m_block_schedule_info[fetching_block_id]->m_total_thread_num), 
      (m_simBase->m_block_schedule_info[fetching_block_id]->m_dispatched_thread_num), 
      core->m_running_block_num, m_simBase->m_block_schedule_info[fetching_block_id]->m_retired);


  // All threads from the currently serveced block should be scheduled before a new block
  // executes. Check currently fetching block has other threads to schedule
  if ((fetching_block_id != -1) && 
      m_simBase->m_block_schedule_info.find(fetching_block_id) != m_simBase->m_block_schedule_info.end() &&
      !(m_simBase->m_block_schedule_info[fetching_block_id]->m_retired)) {
    if (m_simBase->m_block_schedule_info[fetching_block_id]->m_total_thread_num > 
        m_simBase->m_block_schedule_info[fetching_block_id]->m_dispatched_thread_num) {
      DEBUG("fetching_continue block_id:%d total_thread_num:%d dispatched_thread_num:%d \n", 
          fetching_block_id, m_simBase->m_block_schedule_info[fetching_block_id]->m_total_thread_num, 
          m_simBase->m_block_schedule_info[fetching_block_id]->m_dispatched_thread_num);
      return fetching_block_id; 
    }
  }


  // All threads from previous block have been schedule. Thus, need to find a new block
  int appl_id = core->get_appl_id();
  int max_block_per_core = m_simBase->m_sim_processes[appl_id]->m_max_block;

  // If the core already has maximum blocks, do not fetch more
  if ((core->m_running_block_num + 1) > max_block_per_core) 
    return -1;

  bool block_found = true;
  process_s* process = m_simBase->m_sim_processes[appl_id];
  for (auto I = process->m_block_list.begin(), E = process->m_block_list.end(); I != E; ++I) {
    int block_id = (*I).first;

    // this block was not fetched yet and has traces to schedule
    if (!(m_simBase->m_block_schedule_info[block_id]->m_start_to_fetch) && 
        m_simBase->m_block_schedule_info[block_id]->m_trace_exist) {
      block_found = true;
      new_block_id = block_id;
      break; 
    }	    
  }

  // no block found to schedule
  if (new_block_id == -1) 
    return -1;  

  DEBUG("new block is %d \n", new_block_id); 

  // set up block
  m_simBase->m_block_schedule_info[new_block_id]->m_start_to_fetch     = true;
  m_simBase->m_block_schedule_info[new_block_id]->m_dispatched_core_id = core_id; 
  m_simBase->m_block_schedule_info[new_block_id]->m_sched_cycle        = core->get_cycle_count();	

  // set up core
  core->m_running_block_num = core->m_running_block_num + 1; 
  core->m_fetching_block_id = new_block_id; 

  DEBUG("new block is allocated to core:%d running_block:%d fetching_block_id:%d \n" ,
      core_id, new_block_id, core->m_fetching_block_id); 

  return new_block_id; 
}



//--------------------
//schedule  old threads
void process_manager_c::schedule_old_context( context_s* context_to_run, int core_id)
{
  core_c* core = m_simBase->m_core_pointers[core_id];
  core->start();
  core->exec_run_a_cycle();

  cout<<"ready_context scheduling thread :- for # times :"<<counter++;

  //  schedule an existing thread 
  int context_thread_id = context_to_run->thread_info->m_thread_id;
  // add a new application to the core
  core->add_application(context_thread_id, context_to_run->thread_info->m_process);

  // add a new thread trace information
  core->create_old_trace_info( context_to_run); 

  m_simBase->m_cur_thread_id[core_id]  = context_thread_id ; // adding cur_tid info 
  if(*m_simBase->m_knobs->KNOB_DEBUG_SIM_THREAD_SCHEDULE)
    report("schdeuling after context switch: thread_id:"<<context_thread_id <<"\n");

}

//-------------------

//Remember context_switch_core occurs only across chunk_begin and chunk_end boundaries 
//So dont have to worry about context switches for memory instructions
//--> Due to usage of prepare_context_info  if the cur_uop passed is chunk_begin then the dest_core's first uop 
// will be check

// bool process_manager_c::context_switch_core( int src_core_id,int src_local_tid,uop_c* src_uop,int dest_core_id)
// {
//   //------------------------
//   // forward progress check
//   // Counter m_last_forward_progress; /**< last checked cycle */
//   // Counter m_last_inst_count;       /**< last checked instruction count */
//   //
//   // map_c*                          m_map; /**< dependence information */
//
//   //----------------//------------------------------------
//   //----------------//------------------------------------
//   //FIXME
//   // The idea is to let the src_core believe that the thread is terminated preserving the thread data
//   // in the heap memory and add all the related thread info to the new core as a new thread with some old data
//   // This might create issues while deallocating thread data since the pointers will be shared across cores
//   // -- deleting all info from the src_core is cleaner way to do it 
//   //
//   //-------------------------
//   core_c* src_core = m_simBase->m_core_pointers[src_core_id];
//
//   //src_core  :- Changes 
//   src_core->m_last_terminated_tid =src_local_tid;
//   src_core->m_num_thread_reach_end++; /**< number of total terminated threads */
//   --src_core->m_running_thread_num;   /**< number of currently running threads */
//   //-------------------------
//   src_core->m_fetch_ended[src_local_tid] = true;
//   src_core->m_thread_reach_end[src_local_tid] = true ;
//   src_core->m_thread_finished[src_local_tid] = true;
//   src_core->m_terminated_tid[src_local_tid] = true;
//   //Delete dependence information from the src_core map
//   src_core->m_map->delete_map(src_local_tid);
//   //see if needs to remove thread data
//   //-------------------------
//
//   //dest_core :- Changes 
//   //-------------------------
//   core_c* dest_core = m_simBase->m_core_pointers[dest_core_id];
//   int     dest_local_tid  = ++dest_core->m_unique_scheduled_thread_num ;
//
//
//   // dest_core->m_running_block_num;   
//   ++dest_core->m_fetching_thread_num; 
//   // dest_core->m_num_thread_reach_end;
//   // dest_core->m_fetching_block_id;   
//   ++dest_core->m_running_thread_num;  
//
//
//
//   dest_core->m_fetch_ended      [dest_local_tid] = false; 
//   dest_core->m_thread_reach_end [dest_local_tid] = false;
//   dest_core->m_thread_finished  [dest_local_tid] = false;
//   dest_core->m_terminated_tid   [dest_local_tid] = false;
//
//   // additional fetch policies
//   dest_core->m_inst_fetched        [dest_local_tid]            = src_core->m_inst_fetched         [src_local_tid]; 
//   dest_core->m_ops_to_be_dispatched[dest_local_tid]            = src_core->m_ops_to_be_dispatched [src_local_tid];
//   dest_core->m_last_fetch_cycle    [dest_local_tid]            = src_core->m_last_fetch_cycle     [src_local_tid];
//   dest_core->m_heartbeat           [dest_local_tid]            = src_core->m_heartbeat            [src_local_tid];
//   dest_core->m_tid_to_appl_map     [dest_local_tid]            = src_core->m_tid_to_appl_map      [src_local_tid];
//
//   auto temp_thread_trace_info =  prepare_context_info( src_core , src_uop );
//   dest_core->m_thread_trace_info   [dest_local_tid]            = temp_thread_trace_info;
//   dest_core->m_bp_recovery_info    [dest_local_tid]            = src_core->m_bp_recovery_info     [src_local_tid];
//
//   //After the data is transfered to the dest core to avoid duplication make all the info at the src_core = 0
//   // Since context_switch across cores - is done at chunk boundaries there should not  be any map_c info dependency  
//   //src_core  :- Changes 
//   src_core->m_inst_fetched         [src_local_tid]=0; 
//   src_core->m_ops_to_be_dispatched [src_local_tid]=0;
//   src_core->m_last_fetch_cycle     [src_local_tid]=0;
//   src_core->m_heartbeat            [src_local_tid]=0;
//   
//   bp_recovery_info_c* bp_recovery_info = m_simBase->m_bp_recovery_info_pool->acquire_entry();
//   bp_recovery_info->m_recovery_cycle   = MAX_CTR;
//   bp_recovery_info->m_redirect_cycle   = MAX_CTR;
//
//   src_core->m_bp_recovery_info     [src_local_tid]=bp_recovery_info;
//
//
//
//
//
//   //-------------------------
//   return true;
// }



thread_s* process_manager_c:: prepare_context_info( core_c* core , uop_c* cur_uop, int offset )
{
  offset = offset -1;
  //----- 
  // Get current context and modify the file pointer to read instructions after  memory ops 
  // clear the buffer read from the file
  auto  cur_trace_info =  core->get_trace_info(cur_uop->m_thread_id); // contains file pointer information
  // auto  cur_alloc_thread_info = core->get_alloc_thread_info(cur_uop->m_thread_id); // contains heart beat information


  cur_trace_info->m_inst_count = cur_uop->m_inst_num + offset;
  cur_trace_info->m_uop_count = cur_uop->m_uop_num; //currently unused
  cur_trace_info->m_buffer_index = 0;
  cur_trace_info->m_buffer_exhausted = true;
  cur_trace_info->m_fetch_data->init();

  // cur_trace_info->m_prev_trace_info ;
  gzseek(cur_trace_info->m_trace_file,(cur_uop->m_inst_num + offset -1  )*TRACE_SIZE,SEEK_SET);
  gzread(cur_trace_info->m_trace_file, cur_trace_info->m_prev_trace_info,TRACE_SIZE);
  // cur_trace_info->m_next_trace_info
  //   = gzseek(cur_trace_info->m_trace_file,(cur_uop->m_uop_num + 1)*TRACE_SIZE,SEEK_SET);

  cur_trace_info->m_temp_inst_count = cur_uop->m_inst_num + offset;
  cur_trace_info->m_temp_uop_count  = cur_uop->m_uop_num;
  // cur_trace_info->m_num_sending_uop = cur_uop->m_unique_num;
  // cur_trace_info->m_bom  = true;
  // auto offset = cur_uop->m_uop_num;


  core->set_unique_uop_num(cur_uop->m_unique_num + offset);

  return cur_trace_info;
}
