#include "alloc_thread_info.h"

//alloc_thread_info::alloc_thread_info (alloc_thread_info &alloc) {
//  m_fetch_ended           = alloc.m_fetch_ended;
//  m_thread_finished       = alloc.m_thread_finished;
//  m_thread_reach_end      = alloc.m_thread_reach_end;
//  m_inst_fetched          = alloc.m_inst_fetched;
//  m_last_fetch_cycle      = alloc.m_last_fetch_cycle;
//  m_ops_to_be_dispatched  = alloc.m_ops_to_be_dispatched;
//  m_heartbeat             = alloc.m_heartbeat;
//  m_bp_recovery_info      = alloc.m_bp_recovery_info;
//  m_bp_data               = alloc.m_bp_data;
//}

alloc_thread_info::alloc_thread_info()  
{
  m_fetch_ended           = false;    
  m_thread_finished       = false;
  m_thread_reach_end      =false; 
  m_inst_fetched          =0;
  m_last_fetch_cycle      =0;
  m_ops_to_be_dispatched  =0;
  m_heartbeat             =NULL;
  m_bp_recovery_info      =NULL;
  m_bp_data  = NULL           ; 
}

alloc_thread_info::~alloc_thread_info() {}
