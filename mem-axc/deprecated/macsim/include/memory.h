/*
Copyright (c) <2012>, <Georgia Institute of Technology> All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted
provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions
and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or other materials provided
with the distribution.

Neither the name of the <Georgia Institue of Technology> nor the names of its contributors
may be used to endorse or promote products derived from this software without specific prior
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/


/**********************************************************************************************
 * File         : memory.h
 * Author       : Jaekyu Lee
 * Date         : 3/4/2011
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : memory system
 *********************************************************************************************/


#ifndef MEMORY_H
#define MEMORY_H


#include <functional>
#include <fstream>

#include "memreq_info.h"
#include "pref_common.h"
#include "uop.h"
#include "macsim.h"
#include "../../gems-lib-ooo/ruby_clean/O3sim_ruby/O3sim_ruby.h"

#define MemType_Prefetch(x)      (x >= MEM_SWPREF_NTA && x <= MEM_SWPREF_T2)
#define MRT_Prefetch(x)     (x >= MRT_SW_DPRF && x <= MRT_SW_DPRF_T2)


///////////////////////////////////////////////////////////////////////////////////////////////


bool IsStore(Mem_Type type);
bool IsLoad(Mem_Type type);
bool dcache_fill_line_wrapper(mem_req_s *req);


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief data cache data structure
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct dcache_data_s {
  bool    m_dirty;              /**< line dirty */
  Counter m_fetch_cycle;        /**< fetched cycle */
  int     m_core_id;            /**< core id */
  Addr    m_pc;                 /**< pc address */
  int     m_tid;                /**< thread id */
} dcache_data_s;



///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Memory queue class
///////////////////////////////////////////////////////////////////////////////////////////////
class queue_c
{
  /**
   * Queue sort function based on the priority
   */
  struct sort_func {
    macsim_c* m_simBase; /**< pointer to the base simulation class */
    /**
     * Assign a base class pointer
     */
    sort_func(macsim_c* simBase) {m_simBase = simBase;};
    bool operator()(mem_req_s* a, mem_req_s* b); /**< comparison function */
  }; /**< sort function */

  public:
    /**
     * Memory queue constructor
     */
    queue_c(macsim_c* simBase, int size);

    /**
     * Memory queue destructor
     */
    ~queue_c();

    /**
     * Search entries with the given address
     */
    mem_req_s* search(Addr addr, int size);

    /**
     * Search the list with the given request
     */
    bool search(mem_req_s*);

    /**
     * Delete a request
     */
    void pop(mem_req_s* req);

    /**
     * Insert a new request
     */
    bool push(mem_req_s* req);

    /**
     * Check buffer availability
     */
    bool full();

  private:
    queue_c(); //Do not implement

  public:
    list<mem_req_s*> m_entry; /**< queue entries */

  private:
    unsigned int m_size;        /**< queue size */
    macsim_c* m_simBase;        /**< macsim_c base class for simulation globals */

};




///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief memory system
///////////////////////////////////////////////////////////////////////////////////////////////
class memory_c
{
  public:
    /**
     * Constructor
     */
    memory_c(macsim_c* simBase);

    /**
     * Destructor
     */
    virtual ~memory_c() = 0;

    /**
     * Generate a new memory request
     * @param type - memory request type
     * @param addr - memory request address
     * @param size - memory request size
     * @param delay - delay
     * @param uop - request generating uop
     * @param done_func - done function
     * @param unique_num - uop unique number
     * @param pref_info - prefetch information structure
     * @param core_id - core id
     * @param thread_id - thread id
     * @param ptx - GPU request
     * @return false, if mshr full or l2 queue full
     * @return true, otherwise
     */
    bool new_mem_req(Mem_Req_Type type, Addr addr, uns size, uns delay, uop_c* uop,
        std::function<bool (mem_req_s*)> done_func, Counter unique_num, pref_req_info_s* pref_info,
        int core_id, int thread_id, bool ptx);

    /**
     * Access first-level cache from execution stage
     * @return 0, if not accessed correctly
     * @return -1, generate a new memory request
     * @return L1_latency, if hit
     */
    int access(uop_c* uop);

    /**
     * Return base line address
     */
    Addr base_addr(int core_id, Addr addr);

    /**
     * Return cache line size
     */
    int  line_size(int core_id);

    /**
     * Return cache bank id
     */
    int  bank_id(int core_id, Addr addr);

    /**
     * Acquire dcache read port with the specified bank
     */
    bool get_read_port(int core_id, int bank_id);

    /**
     * Tick a cycle for the memory system
     */
    void run_a_cycle(void);

    /**
     * Tick a cycle for L1/L2 (core, private) caches
     */
    void run_a_cycle_core(int);

    /**
     * Tick a cycle for L3 cache
     */
    //void run_a_cycle_uncore();

    /**
     * Deallocate completed memory request
     */
    void free_req(int core_id, mem_req_s* req);

    /**
     * Receive a message from NoC
     */
    //bool receive(int src, int dst, int msg, mem_req_s* req);

    /**
     * Using level and its id, get network id
     */
    //int get_dst_id(int level, int id);

    /**
     * Using level and its id, get destination router id
     */
    //int get_dst_router_id(int level, int id);

    /**
     * Using network id, get level and its id
     */
    //void get_level_id(int noc_id, int* level, int* id);

    /**
     * Cache line fill function
     */
    bool done(mem_req_s* req);

    /**
     * Access L1 data cache
     */
    dcache_data_s* access_cache(int core_id, Addr addr, Addr *line_addr, bool update,
        int appl_id);

    /**
     * Generate a new write-back request
     */
    //mem_req_s* new_wb_req(Addr addr, int size, bool ptx, dcache_data_s* data, int level);

    /**
     * Print all entries in MSHR
     */
    void print_mshr(void);

    /*
     * Process MSHR directly
     */

    void process_mshr(int core_id);

    /**
     * Check available entry in mshr
     */
    int get_num_avail_entry(int core_id);

    /**
     * Initialize the memory system. \n
     * Setup interconnection network interface.
     */
    void init(void);


  public:
    static int m_unique_id; /**< unique memory request id */


    // Add O3sim_ruby here
    O3sim_ruby* m_ruby;

  protected:
    /**
     * Allocate a new request from free list
     */
    mem_req_s* allocate_new_entry(int core_id);

    /**
     * Initialize a new request
     */
    void init_new_req(mem_req_s* req, Mem_Req_Type type, Addr addr, int size, int delay, \
        uop_c* uop, std::function<bool (mem_req_s*)> done_func, Counter unique_num, \
        Counter priority, int core_id, int thread_id, bool ptx);

    /**
     * Adjust a new request. In case of finding matching entry, we need to adjust
     * fields of the matching request
     */
    void adjust_req(mem_req_s* req, Mem_Req_Type type, Addr addr, int size, int delay, \
        uop_c* uop, std::function<bool (mem_req_s*)> done_func, Counter unique_num, \
        Counter priority, int core_id, int thread_id, bool ptx);

    /**
     * Search a request from queues
     */
    mem_req_s* search_req(int core_id, Addr addr, int size);

    /**
     * Set the level of each cache level
     */
    //virtual void set_cache_id(mem_req_s* req);

    /**
     * When MSHR is full, try to evict a prefetch request
     */
    mem_req_s* evict_prefetch(int core_id);

    /**
     * Flush all prefetches in MSHR
     */
    void flush_prefetch(int core_id);



  protected:
    list<mem_req_s*>* m_mshr; /**< mshr entry per L1 cache */
    list<mem_req_s*>* m_mshr_free_list; /**< mshr entry free list */
    int m_num_core; /**< number of cores */
    int m_num_l3; /**< number of l3 caches */
    int m_num_mc; /**< number of memory controllers */
    Counter m_stop_prefetch; /**< when set, no prefetches will be inserted */
    int m_l3_interleave_factor; /**< mask bit for L3 id */
    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
    map<int, int>* m_dst_map; /**< destination id mapping function */

    Counter m_cycle; /**< clock cycle */
   int  mem_req_no;
};

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief  No cache architecture (NVIDIA G80, G200)
///////////////////////////////////////////////////////////////////////////////////////////////
class no_cache_c : public memory_c
{
  public:
    /**
     * Constructor
     */
    no_cache_c(macsim_c* simBase);

    /**
     * Destructor
     */
    ~no_cache_c();

  private:
    /**
     * Set the level of each cache level
     */
    void set_cache_id(mem_req_s* req);
};

#endif
