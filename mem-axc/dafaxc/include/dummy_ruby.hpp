#ifndef __DUMMY_RUBY__
#define __DUMMY_RUBY__
#include <set> 
#include <queue> 
#include <cstdint>
#include "data_type.hpp"


// #define RUBYLATENCY 12

// typedef unsigned long long uns64;
// typedef uns64 Addr;


class Dummy_ruby
{
  private:
    // std::set<DummyRubyType> dummy_mem_set;
    std::set<std::pair<Addr,uint64_t>> dummy_mem_set;
    uint64_t m_ruby_latency;
  public:
    void clear(){
      dummy_mem_set.clear();
    }

    Dummy_ruby(uint64_t _ruby_latency);
    void send_req(Addr req_addr, uint64_t curr_clock);
    std::queue<Addr> recv_resp(uint64_t clock);

};


#endif
