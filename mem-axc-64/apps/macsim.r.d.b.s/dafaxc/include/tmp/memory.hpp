#ifndef __MEMORY__
#define __MEMORY__


#include <functional>
#include <fstream>
#include <list>
#include <map>
#include <unordered_map>
#include <assert.h>
#include "data_type.hpp"
#include "O3sim_ruby.h"

#include "GenericBloomFilter_1.h" 
#include "Address.h"

using namespace std;

#define MEM_MSHR_SIZE  16
#define MAX_BLOOM_COUNT 128

  template <class T>
inline void hash_combine(std::size_t & seed, const T & v)
{
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std
{
  template<typename S, typename T> struct hash<pair<S, T>>
  {
    inline size_t operator()(const pair<S, T> & v) const
    {
      size_t seed = 0;
      ::hash_combine(seed, v.first);
      ::hash_combine(seed, v.second);
      return seed;
    }
  };
}




class Memory
{

  private:
    O3sim_ruby* m_ruby;
    //--------------------------------
    std::unordered_map< std::pair<uint64_t,uint64_t>, MemValue > memaddr_map;
    // std::unordered_map<uint64_t, MemValue_t> ldst_addr_buffer;
    std::list<MemValue_t> ldst_addr_buffer;
    //--------------------------------
    int m_num_core=1; /**< number of cores */
    int m_num_l3=1; /**< number of l3 caches */
    int m_num_mc=1; /**< number of memory controllers */
    int m_core_id=0;
    int m_thread_id=0;

    uint64_t m_mem_mshr_stack = MEM_MSHR_SIZE;

    uint64_t m_bloom_ld_no=0;
    uint64_t m_bloom_ld_yes=0;
    uint64_t m_bloom_ld_fp=0;

    uint64_t m_bloom_st_no=0;
    uint64_t m_bloom_st_yes=0;
    uint64_t m_bloom_st_fp=0;
    uint64_t m_bloom_stall_cycles=0;

    GenericBloomFilter_1* bloom_filter_ld;
    GenericBloomFilter_1* bloom_filter_st;

    bool m_stall_on_bloom;
  public:

    int  mem_req_stack;
    uint64_t m_mshr_full_cycles =0;
    Memory(O3sim_ruby* _ruby, bool _stall_on_bloom, std::string _bloom_type );
    uint64_t port_unavailable();
    uint64_t bloom_yes_ld();
    uint64_t bloom_yes_st();
    uint64_t bloom_no_ld();
    uint64_t bloom_no_st();
    uint64_t bloom_fp();
    uint64_t bloom_stall_cycles();

    uint64_t m_port_unavailable=0;
    ~Memory();
    
    uint64_t global_memaddr_map_size();
    void print_mem_stats();
    void initialize(bool _stall_on_bloom);
    bool access(uint64_t prev_completion_cycles);
    void send_req( uint64_t iter, uint64_t nodeid /*, bool isWrite*/ );
    std::list<uint64_t> recv_resp(uint64_t prev_completion_cycles);
    void advance_time();
    std::list<uint64_t> process_mshr();
    MemValue_t* search_req(Addr addr, uint64_t size);
    // void run_a_cycle(void);

    uint64_t line_size();
    Addr base_addr(Addr addr);
    void run_a_cycle_core(int core_id);
    void fill_global_memaddr_map(uint64_t iter, uint64_t nodeid , MemValue mem_value);
    void print_global_memaddr_map();


    void decrement_bloom_st_count(const MemValue_t* matching_req);
    void decrement_bloom_ld_count(const MemValue_t* matching_req);
    void increment_bloom_st_count(MemValue_t mem_value);
    void increment_bloom_ld_count(MemValue_t mem_value);

    bool update_bloom_ld(MemValue_t mem_value);
    bool update_bloom_st(MemValue_t mem_value);
};
#endif 


/*
 *Iter:
 Ld/St
 Memory addr
nodeid:
-----------------------------------------
for every above info -- create a uop
-- to integrate with macsim
-------------

 *
 *
 */
