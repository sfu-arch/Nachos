#include "dummy_ruby.hpp"
#include <iostream>
using namespace std;


Dummy_ruby::Dummy_ruby(uint64_t _ruby_latency): m_ruby_latency(_ruby_latency){}



void Dummy_ruby::send_req(Addr req_addr, uint64_t curr_clock)
{
  std::pair<Addr,uint64_t> memset(req_addr,curr_clock+ m_ruby_latency);
  dummy_mem_set.insert(memset);
}


std::queue<Addr> Dummy_ruby::recv_resp(uint64_t curr_clock)
{
  std::queue<Addr> out_q;
  for (auto &memset : dummy_mem_set)
  {
    if(memset.second < curr_clock)
    {
      out_q.push(memset.first);
      dummy_mem_set.erase(memset);
    }
    // cout<<"size:"<<dummy_mem_set.size()<<endl;
  }

  return out_q;
}
