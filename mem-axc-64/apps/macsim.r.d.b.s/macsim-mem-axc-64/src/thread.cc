#include "thread.h"
#include "frontend.h"
#include "trace_read.h"
#include "all_knobs.h"

////////////////////////////////////////////////////////////////////////////////
//  marker_s() - constructor - used for detecting chunk_begin and chunk_end
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//  thread_s() - constructor
////////////////////////////////////////////////////////////////////////////////
thread_s::thread_s(macsim_c* simBase)
{
  m_simBase         = simBase;
  m_fetch_data      = new frontend_s; 
  m_buffer          = new char[1000 * TRACE_SIZE];
  m_prev_trace_info = new trace_info_s;
  m_next_trace_info = new trace_info_s;
  m_num_mem_requests = 0;
  for (int ii = 0; ii < MAX_PUP; ++ii) {
    m_trace_uop_array[ii] = new trace_uop_s;
    if (static_cast<string>(*m_simBase->m_knobs->KNOB_FETCH_POLICY) == "row_hit") {
      m_next_trace_uop_array[ii] = new trace_uop_s;
    }
  }
  // m_enable_chunk = false;
  m_thread_enable = false;
  m_cur_chunk_id  = -1;
  m_cur_core_id   = -1;
}


int  thread_s::get_thread_id()  
{
  return m_thread_id;
}


////////////////////////////////////////////////////////////////////////////////
//  thread_s() - destructor
////////////////////////////////////////////////////////////////////////////////
thread_s::~thread_s()
{
#if 0
  delete m_fetch_data;
  delete[] m_buffer;
  delete m_prev_trace_info;
  delete m_next_trace_info;

  for (int ii = 0; ii < MAX_PUP; ++ii) {
    delete m_trace_uop_array[ii];
    if (static_cast<string>(*m_simBase->m_knobs->KNOB_FETCH_POLICY) == "row_hit") {
      delete m_next_trace_uop_array[ii];
    }
  }
#endif
}



