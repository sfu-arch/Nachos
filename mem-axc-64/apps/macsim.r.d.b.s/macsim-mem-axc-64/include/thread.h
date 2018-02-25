
#ifndef THREAD_H
#define THREAD_H


#include <fstream>
#include <inttypes.h>
#include <list>
#include <zlib.h>
#include <unordered_map>
#include <queue>
#include "global_defs.h"
#include "global_types.h"
///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Thread data structure
///
/// Mapping between the trace of physical thread and simulation thread. For X86 simulation,
/// a simulation thread is mapped to a physical thread. However, for GPU simulation, a thread
/// is mapped to a SIMD thread group (warp).
///
/// task struct like structure maintained for each thread
/// contains pointer to trace file for the thread + state 
/// maintained for the thread
///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct marker_s {
  bool    ds_flag;
  Counter m_uop_num;
  Counter m_unique_num;
  bool    is_chunk_begin;
  marker_s () : ds_flag(false),m_uop_num( 0), m_unique_num(0) , is_chunk_begin(false){}

} marker_s;




typedef struct thread_s {
  /**
   * Constructor
   */
  thread_s(macsim_c* simBase);

  //thread_s(const thread_s  &thread);
  //thread_s operator =(const thread_s  &thread) ;
  /**
   * Destructor
   */
  ~thread_s();
  Counter              m_num_mem_requests;
  int                  m_unique_thread_id; /**< unique thread id */
  int                  m_thread_id; /**< current thread id */
  int                  m_orig_block_id; /**< original block id from a trace*/
  int                  m_orig_thread_id; /**< adjusted block id */
  int                  m_block_id; /**< block id */
  gzFile               m_trace_file; /**< gzip trace file */
  bool                 m_file_opened; /**< trace file opened? */
  bool                 m_main_thread; /**< main thread (usually thread id 0) */
  uint64_t             m_inst_count; /**< total instruction counts */
  uint64_t             m_uop_count;  /**< total uop counts */
  bool                 m_trace_ended; /**< trace ended */
  process_s           *m_process; /**< point to the application belongs to */
  bool                 m_ptx; /**< GPU thread */
  char*                m_buffer; /**< trace buffer */
  int                  m_buffer_index; /**< current trace buffer index */
  int                  m_buffer_index_max; /**< maximum buffer index */
  bool                 m_buffer_exhausted; /**< read all traces from the buffer */
  frontend_s*          m_fetch_data; /**< frontend fetch data */ 
  trace_info_s*        m_prev_trace_info; /**< prev instruction trace info */
  trace_info_s*        m_next_trace_info; /**< next instruction trace info */
  bool                 m_thread_init; /**< thread initialized */
  int                   m_temp_inst_count; /**< temp instruction count */
  int                   m_temp_uop_count; /**< temp uop count */
  int                  m_num_sending_uop; /**< number of sending uops */
  bool                 m_bom; /**< beginning of instruction */
  bool                 m_eom; /**< end of instruction */
  trace_uop_s*         m_trace_uop_array[MAX_PUP]; /**< trace uop array */
  trace_uop_s*         m_next_trace_uop_array[MAX_PUP]; /**< next trace uop array */

  macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

  // synchronization information
  list<section_info_s*> m_sections; /**< code sections */
  list<section_info_s*> m_bar_sections; /**< barrier sections */
  list<section_info_s*> m_mem_sections; /**< memory sections */
  list<section_info_s*> m_mem_bar_sections; /**< memory barrier sections */
  list<section_info_s*> m_mem_for_bar_sections; /**< memory for barrier sections */

  int get_thread_id();

  // std::queue<marker_s>  m_chunk_marker_q;
  // bool m_enable_chunk;
  bool                  m_thread_enable;
  int                   m_cur_chunk_id;
  int                   m_cur_core_id;
} thread_s;

#endif
