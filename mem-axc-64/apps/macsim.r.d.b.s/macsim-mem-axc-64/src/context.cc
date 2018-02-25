#include "context.h"

#include <iostream>






///////////////////////
/// COnstructor   //////
//////////////////////




// context_s::context_s(thread_s* _th, alloc_thread_info _a_th_info)  {
// }


co_switch_handler_c::co_switch_handler_c() { // : m_mem_wait_context_vec = [] , m_ready_context_vec = []  {

  // m_ready_context_map =  std::unordered_map<int,context_s* > ;
  // m_mem_wait_context_map.clear();
  // =  std::unordered_map<int,context_s* > ;
}


bool co_switch_handler_c::context_switch( thread_s * _th, alloc_thread_info _a_th_info, int core_id, int tid , bool is_mem)
{

  context_s*  cur_context = new  context_s(_th,_a_th_info);
  int hash_value = hash_func(tid,core_id);
  //check if the entry already exist if yes just ignore the new entry
  // m_mem_wait_context_map[hash_value] = cur_context;
  m_wait_context_map[is_mem][hash_value] = cur_context;
  return true;

}


bool  co_switch_handler_c::is_present_mem_wait_context(int core_id, int tid, bool is_mem) {
  int hash_value =  hash_func(tid,core_id);
  if( m_wait_context_map[is_mem].count(hash_value)> 0 )
    return true;
  else return false;
}



bool  co_switch_handler_c::mv_context_mem_wait_to_ready(int tid,int core_id, bool is_mem)  
{
  int hash_value = hash_func(tid,core_id);
  auto t = m_wait_context_map[is_mem].find(hash_value);
  if(t == m_wait_context_map[is_mem].end() )
    return false;
  else {
    m_ready_context_map[hash_value] = m_wait_context_map[is_mem][hash_value];
    m_wait_context_map[is_mem].erase(hash_value);
    // cout<<"mem_wait_map entry exists : moving context to ready_map \n";
    return true;
  }
}



context_s* co_switch_handler_c::is_ready_context_vec_empty(int core_id) {
  for (auto it = m_ready_context_map.begin(); it != m_ready_context_map.end();it++)
  {
    if(get_core_id_frm_hash_value(it->first) == core_id)
    {
      //TODO need to delete heap memory       
      auto temp = it->second;

      m_context_map_pool[it->first] = m_ready_context_map[it->first];
      m_ready_context_map.erase(it->first);
      return temp;
    }
  }

  return  NULL;
}

context_s* co_switch_handler_c::rr_ready_context(int core_id) {
  struct Key_least {
    int m_chunk_id;
    int m_tid;
    Key_least() : m_chunk_id (-1), m_tid(-1) {};
  }cur_least_s;

  for (auto it = m_ready_context_map.begin(); it != m_ready_context_map.end();it++)
  {
    if(get_core_id_frm_hash_value(it->first) == core_id)
    {
      //TODO need to delete heap memory       
      auto temp = it->second;
      if (cur_least_s.m_chunk_id == -1 )
      {
        cur_least_s.m_chunk_id = temp->thread_info->m_cur_chunk_id;
        cur_least_s.m_tid = temp->thread_info->m_thread_id;
      }
      else if(cur_least_s.m_chunk_id > temp->thread_info->m_cur_chunk_id)
      {
        cur_least_s.m_chunk_id = temp->thread_info->m_cur_chunk_id;
        cur_least_s.m_tid = temp->thread_info->m_thread_id;
      }
    }
  }

  if(cur_least_s.m_chunk_id == -1 )
    return  NULL;
  else 
  {

    int hash_value = hash_func(cur_least_s.m_tid,core_id);
    m_context_map_pool[hash_value]  = m_ready_context_map[hash_value];
    auto temp = m_ready_context_map[hash_value];
    m_ready_context_map.erase(hash_value);
    return  temp;
  }
}

void co_switch_handler_c::ready_context_flush() {
  for (auto it = m_context_map_pool.begin(); it != m_context_map_pool.end();it++)
  {
    auto temp  = it->second;
    delete  temp ;
  }
}

co_switch_handler_c::~co_switch_handler_c() {
  this->ready_context_flush();
}


