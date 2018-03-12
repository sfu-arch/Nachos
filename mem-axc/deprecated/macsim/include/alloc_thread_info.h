#ifndef ALLOC_THREAD_INFO_H
#define ALLOC_THREAD_INFO_H

#include "global_defs.h"
#include "global_types.h"



typedef struct alloc_thread_info {
  // add thread data
  bool m_fetch_ended;
  bool m_thread_finished;
  bool m_thread_reach_end;
  Counter m_inst_fetched;
  Counter m_last_fetch_cycle;
  Counter m_ops_to_be_dispatched;

  // allocate heartbeat
  heartbeat_s* m_heartbeat;

  // allocate bp recovery
  bp_recovery_info_c* m_bp_recovery_info;
  bp_data_c*  m_bp_data;
  // alloc_thread_info (int tid, core_c* core);
  // alloc_thread_info (alloc_thread_info &alloc); 
  alloc_thread_info();
  ~alloc_thread_info();
} alloc_thread_info;

#endif
