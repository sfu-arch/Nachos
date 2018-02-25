
#ifndef CONTEXT_H
#define CONTEXT_H



#include <iostream>
#include <vector>
#include "thread.h"
#include "alloc_thread_info.h"
#include "assert.h"
#include "cmath"



int hash_func(int tid, int core_id);

int get_tid_frm_hash_value(int hash_value);

int get_core_id_frm_hash_value(int hash_value);





//-----------------------------
typedef int thread_id;


//------------------------------
typedef struct  context_s 

{
  //should contain current instruction pointer -- m_buffer_index contains the current instruction pointer from the struct "thread_s"  
  //and register dependencies  - map_c
  thread_s*  thread_info;
  alloc_thread_info  alloc_th_info;
  // alloc_thread_info*  alloc_th_info;
  // context_s(thread_s* _th ,int tid, core_c* core); 
  // ~context_s();
  context_s(thread_s* _th, alloc_thread_info _a_th_info){
  thread_info = _th;
  alloc_th_info =   _a_th_info;
  }

} context_s;


//------------------------------


class co_switch_handler_c {



  public:
    co_switch_handler_c();
    std::unordered_map<int,context_s* > m_ready_context_map;
    std::unordered_map<int,context_s* > m_mem_wait_context_map;
    std::unordered_map<int,context_s* > m_chunk_wait_context_map;
    std::array< unordered_map<int,context_s* >, 2> m_wait_context_map{ { m_chunk_wait_context_map,m_mem_wait_context_map } };
    std::unordered_map<int,context_s* > m_context_map_pool;

    bool   context_switch( thread_s * _th,alloc_thread_info _a_th_info, int core_id,int tid, bool is_mem);// current thread info and current instruction pointer

    bool  is_present_mem_wait_context(int core_id, int tid, bool is_mem); 
    bool   mv_context_mem_wait_to_ready(int tid,int core_id, bool is_mem)  ;

    void  ready_context_flush() ;
    bool  push_mem_wait_context_vec( context_s curr_context,int core_id);
    void  print_mem_wait_context_vec();
    void  print_ready_context_vec();
    context_s*  is_ready_context_vec_empty(int core_id);
    context_s* rr_ready_context(int core_id); 
    context_s fetch_ready_context(int core_id) ;
    ~co_switch_handler_c();

};




#endif
