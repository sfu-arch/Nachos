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
#include "queue"
// #include "dummy_ruby.hpp"

using namespace std;

#define MAX_BLOOM_COUNT 128
#define NUM_BANKS 4
#define MLP_HIST_SIZE 7

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
    std::shared_ptr<O3sim_ruby> m_ruby;
    uint64_t lat=20;
    uint64_t mem_clk;
    //--------------------------------
    std::queue<std::pair<uint64_t,uint64_t>> overflow_buffer;
    std::unordered_map< std::pair<uint64_t,uint64_t>, MemValue > memaddr_map;
    std::map<Addr, std::vector<MemValue_t>> LSQ;
    std::map<Addr, bool> MSHR;
    std::list<uint64_t> early_retire;
    //--------------------------------
    int m_num_core=1; /**< number of cores */
    int m_num_l3=1; /**< number of l3 caches */
    int m_num_mc=1; /**< number of memory controllers */
    int m_core_id=0;
    int m_thread_id=0;

    uint64_t m_SIZE_LSQ;

    void add_to_mlp_hist( uint64_t m_SIZE_LSQ);

    Addr  g_BANK_SIZE [NUM_BANKS];
    uint64_t  m_BANK_SIZE [NUM_BANKS];

    uint64_t m_bloom_ld_no=0;
    uint64_t m_bloom_ld_yes=0;
    uint64_t m_bloom_ld_fp=0;

    uint64_t m_bloom_st_no=0;
    uint64_t m_bloom_st_yes=0;
    uint64_t m_bloom_st_fp=0;
    uint64_t m_bloom_stall_cycles=0;
    std::list<int> m_ready_ops_nomem_list;

    GenericBloomFilter_1 bloom_filter_ld;
    GenericBloomFilter_1 bloom_filter_st;

  public:


    uint64_t m_ruby_requests;
    uint64_t m_mlp_hist [MLP_HIST_SIZE] ;
    uint64_t m_mshr_full_cycles =0;
    Memory( std::shared_ptr<O3sim_ruby> _ruby , std::string _bloom_type , bool ishash2, uint64_t mem_lat, uint64_t _mem_req_stack);
    // Dummy_ruby* dummy_ruby;
    uint64_t port_unavailable();
    uint64_t m_port_unavailable=0;

    ~Memory();
    
    uint64_t global_memaddr_map_size();
    void print_mem_stats();
    void initialize();
    bool access(uint64_t prev_completion_cycles);
    void send_req( uint64_t iter, uint64_t nodeid /*, bool isWrite*/ );
    std::list<uint64_t> recv_resp(uint64_t prev_completion_cycles);
    void advance_time();
    std::list<uint64_t> process_mshr();
    MemValue_t* search_req(Addr addr, uint64_t size);

    uint64_t line_size();
    Addr base_addr(Addr addr);
    Addr get_bankid(Addr req_addr);

    void run_a_cycle_core(int core_id);
    void fill_global_memaddr_map(uint64_t iter, uint64_t nodeid , MemValue mem_value);
    void print_global_memaddr_map();

    //Bloom Functions
    
    void  decrement_bloom_counters(Addr req_addr, bool is_Ld);
    bool bloom_stats_for_loads(Addr req_addr, bool is_LSQ_yes);
    bool bloom_stats_for_stores(Addr req_addr, bool is_LSQ_yes);


    uint64_t bloom_yes_ld();
    uint64_t bloom_yes_st();
    uint64_t bloom_no_ld();
    uint64_t bloom_no_st();

    uint64_t bloom_fp_ld();
    uint64_t bloom_fp_st();


};
#endif 


