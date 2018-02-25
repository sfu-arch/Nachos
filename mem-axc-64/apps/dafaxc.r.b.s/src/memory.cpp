#include "memory.hpp"
// #include "dummy_ruby.hpp"

Memory::Memory(std::shared_ptr<O3sim_ruby> _ruby , std::string _bloom_type, bool ishash2, 
    uint64_t mem_lat, uint64_t _mem_req_stack) : 
  m_ruby(_ruby), 
  bloom_filter_ld (GenericBloomFilter_1(_bloom_type, ishash2)),
  bloom_filter_st (GenericBloomFilter_1(_bloom_type, ishash2) )
  
{
  cout<<"Memory Constructor called \n";

  for(auto i=0; i< NUM_BANKS;i++)
    g_BANK_SIZE[i]= _mem_req_stack;


  for(auto i=0; i< MLP_HIST_SIZE;i++)
    m_mlp_hist[i] =0;


  m_ruby_requests =0;
  mem_clk=0;
  m_num_core = 1;
  m_num_l3   = 1;
  m_num_mc   = 1;

  m_core_id =0;
  m_thread_id =0;

  m_bloom_st_yes=0;
  m_bloom_st_no=0;
  m_bloom_st_fp=0;

  m_bloom_ld_yes=0;
  m_bloom_ld_no=0;
  m_bloom_ld_fp=0;
  this->initialize();

  // dummy_ruby = new Dummy_ruby(mem_lat);
  m_port_unavailable=0;
  m_bloom_stall_cycles =0;
  m_mshr_full_cycles =0;
  m_SIZE_LSQ = 0;

  for(auto i=0; i< NUM_BANKS;i++)
    m_BANK_SIZE[i]= 0;


}


void Memory::initialize()
{


  m_ruby_requests =0;
  mem_clk=0;
  m_bloom_st_yes=0;
  m_bloom_st_no=0;
  m_bloom_st_fp=0;

  m_bloom_ld_yes=0;
  m_bloom_ld_no=0;
  m_bloom_ld_fp=0;

  m_bloom_stall_cycles =0;

  m_SIZE_LSQ = 0;

  for(auto i=0; i< NUM_BANKS;i++)
    m_BANK_SIZE[i]= 0;



  LSQ.clear();
  MSHR.clear();
}

void Memory::print_mem_stats()
{
  cout<<"----------------------------------\n";
  cout<<"Printing dafaxc memory state \n";

  std::cout<<"g_BANK_SIZE: "<<g_BANK_SIZE[0] <<endl;
  // std::cout<<"ldst_addr_buffer size: "<<ldst_addr_buffer.size()<<endl;
  std::cout<<"ldst_addr_buffer size: "<<LSQ.size()<<endl;
  std::cout<<"MSHR: size: "<<MSHR.size()<<endl;

  for(auto & entry: LSQ)
  {
    cout<<"block addr: "<< entry.first <<endl;
    for(auto & mem_value : entry.second)
    {
      cout<<"isWrite:"<<mem_value.isWrite<<"\t";
      cout<<"nodeid:"<<mem_value.nodeid<<"\t";
      cout<<"---------"<<endl;
    }
  }
}

Memory::~Memory()
{

  m_port_unavailable=0;
  bloom_filter_ld.clear();
  bloom_filter_st.clear();


  m_bloom_stall_cycles =0;
  m_bloom_st_yes=0;
  m_bloom_st_no=0;
  m_bloom_st_fp=0;

  m_bloom_ld_yes=0;
  m_bloom_ld_no=0;
  m_bloom_ld_fp=0;




  LSQ.clear();
  MSHR.clear();
  memaddr_map.clear();

}

void Memory::send_req( uint64_t iter, uint64_t nodeid/*, bool isWrite*/ )
{

  auto key =std::make_pair (iter,nodeid);
  //TODO: Find Bankid, check if bank is Full
  //If yes send to overflow buffer 
  //else send to memory
  if (memaddr_map.find(key) == memaddr_map.end())
  {
    assert(0 && "dafaxc: sent request not in memaddr_map \n");
  }


  add_to_mlp_hist(m_SIZE_LSQ);
  MemValue mem_value = memaddr_map[key];

  bool isWrite = mem_value.isWrite;
  Addr vaddr = mem_value.addr;
  Addr req_addr = base_addr(vaddr);
  Addr bankid = get_bankid(req_addr);
  D(cerr<<"bankid : "<<bankid);
  D(cout<<"bankid : "<<bankid);

  // if(g_BANK_SIZE <= m_SIZE_LSQ)
  if(g_BANK_SIZE[bankid] <= m_BANK_SIZE[bankid])
  {
    overflow_buffer.push(key);
    // cerr<<"overflow_buffer size: "<<overflow_buffer.size()<<endl;
    m_port_unavailable++;
    return;
  }

  D(cout<<"m_ruby: send_req:  orig_addr :"<<mem_value.addr<<"\t"
      <<"block_addr: "<<req_addr<<"\t"
      <<"nodeid: "<<nodeid<<"\t"
      <<"\n");
  //Before adding an entry in LSQ
  //- Check if its a Load
  //- if yes, 
  //    then find the latest matching store in LSQ
  //    - if yes 
  //        retire Load instruction without making an entry
  //    - if no 
  //        make an entry in LSQ
  //        Check if an entry exists in MSHR  - (It can be an earlier matching Load)
  //          - if yes
  //              do nothing
  //          - if no 
  //              make an entry in MSHR, is_issue = true 
  //              issue memop to ruby
  //- else it is a Store
  //    make an entry in LSQ
  //    Check if an entry exists in MSHR  - (It can be an earlier matching Load/Store)
  //      - if yes
  //        do nothing
  //      - if no 
  //        make an entry in MSHR, is_issue = true 
  //        issue memop to ruby


  if(!isWrite)
  {
    D(cout<<"m_ruby: LD:"
        <<"\t");
    MemValue_t matching_st = MemValue_t{false,UINT_MAX};
    //Check if an entry exist 
    //if Yes : There is atleast 1 matching Ld/St in the LSQ
    if(LSQ.find(req_addr) != LSQ.end() )
    {
      std::vector<MemValue_t> entry = LSQ[req_addr];
      //Look for the previous store which is youngest
      //Note that input dot file should maintain the dependency edges between Ld/St 
      //and so a dependent Ld/St which should occur before will have an entry earlier
      //than the current store

      for (auto &it : entry)
      {

        if(it.isWrite && it.nodeid < nodeid)
        {
          matching_st = it;
          D(cout<<" LSQ earlier MATCHING nodeids: "<< it.nodeid <<"\t");
        }

      }
      D(cout<<endl);

      // assert(matching_st.nodeid != UINT_MAX);
      //Note: entry contains the youngest store, with a nodeid which occurs before the load in
      //program order;
      //it.nodeid < nodeid may not be required since, the input dot graph should already ensure that

      //if Matching Store found -- add it to early_retire 
      if(matching_st.isWrite)
      {
        D(cout<<" Earlier Matching Store Exist with nodeid: "<<matching_st.nodeid
            <<"\n");
        early_retire.push_back(nodeid);
        //FIXME
        //Note: since an entry in LSQ was never created
        //and we are not storing any info apart from nodeid in early_retire
        //we are incrementing bloom counter + updating bloom stats + decrementing counter 
        //all at once here. 



        // NOte: It is a Load Instruction
        // true = it says there is a true dependency with an earlier Store in LSQ
        // Note: it also takes care of incementing corresponding bloom_st
        bloom_stats_for_loads(req_addr,true);
        // true = it is a Load instruction;
        decrement_bloom_counters(req_addr, true);

      }

      // Earlier matching Load exist
      else 
      {

        for (auto &it : entry)
        {

          if(!it.isWrite && it.nodeid < nodeid)
          {
            matching_st = it;
            D(cout<<" LSQ earlier MATCHING nodeids: "<< it.nodeid <<"\t");
          }

        }
        D(cout<<endl);


        D(cout<<" Earlier Matching Load Exists : " <<"\n");
        //make an entry in LSQ with is_issue=1 
        //Need to retire this Load with the previous Load
        //Since there is no other store in between and thus,
        //can retire together
        LSQ[req_addr].push_back(MemValue_t{isWrite,nodeid});
        m_SIZE_LSQ++;
        m_BANK_SIZE[bankid]++;


        //NOTE:
        //Due to Hack for early_retires  
        //We deal with LSQ entries separately 
        // we delete bloom counters when LSQ entry is deleted


        // NOte: It is a Load Instruction
        // Note: it also takes care of incementing corresponding bloom_st
        // false = there is no  dependency with an earlier Store in LSQ
        bloom_stats_for_loads(req_addr,false);

        //Decrement bloom_LD counter when this LSQ entry is deleted 
        //after response from ruby

      }

    }

    //There is no matching entry in LSQ
    else 
    {
      D(cout<<" No Matching LD/ST Exist" 
          <<"\n");
      // Add an entry to LSQ
      // Make an entry in MSHR
      // Issue memop
      LSQ[req_addr].push_back(MemValue_t{isWrite,nodeid});
      MSHR[req_addr] = false;
      m_SIZE_LSQ++;
      m_BANK_SIZE[bankid]++;



      //NOTE:
      //Due to Hack for early_retires  
      //We deal with LSQ entries separately 
      // we delete bloom counters when LSQ entry is deleted


      // NOte: It is a Load Instruction
      // Note: it also takes care of incementing corresponding bloom_st
      // false = there is no  dependency with an earlier Store in LSQ
      bloom_stats_for_loads(req_addr,false);

      //Decrement bloom_LD counter when this LSQ entry is deleted 
      //after response from ruby


    }


  }

  //is is a Store instruction
  else 
  {

    bool is_matching_lsq = false;
    D(cout<<"m_ruby: ST:"
        <<"\t");

    //---------------------------------------------------------------------------------
    if(LSQ.find(req_addr) != LSQ.end() )
    {

      std::vector<MemValue_t> entry = LSQ[req_addr];
      //Look for the previous Load which is youngest
      //Note that input dot file should maintain the dependency edges between Ld/St 
      //and so a dependent Ld/St which should occur before will have an entry earlier
      //than the current store

      for (auto &it : entry)
      {

        if(!it.isWrite && it.nodeid < nodeid)
        {
          is_matching_lsq = true;
          D(cout<<" LSQ earlier MATCHING nodeids: "<< it.nodeid <<"\t");
        }
      }
      D(cout<<endl);

      //Note: entry contains the youngest Load, with a nodeid which occurs before the Store in
      //program order;
      //it.nodeid < nodeid may not be required since, the input dot graph should already ensure that

      if(is_matching_lsq)
      {
        D(cout<<"Earlier Matching Load found \n");

      }
      else 
      {
        D(for (auto &it : entry)
        {

          if(it.isWrite && it.nodeid < nodeid)
          {
            cout<<" LSQ earlier MATCHING nodeids: "<< it.nodeid <<"\t";
          }
        }
        cout<<endl ;
        )

        is_matching_lsq = true;

        D(cout<<"Earlier Matching Store found \n");

      }
    }
    else 
    {

      is_matching_lsq=false;
      D(cout<<" No Matching LD/St"
          <<"\n");

      MSHR[req_addr] = false;

    }


    //Create an LSQ entry
    //for all cases 
    LSQ[req_addr].push_back(MemValue_t{isWrite,nodeid});
    m_SIZE_LSQ++;
    m_BANK_SIZE[bankid]++;


    // Check Bloom Filter and update stats
    // Note all we need is the stats of False +ves
    // Since, we are not adding a latency to LSQ
    // Total cycles can be calculated separately later
    // based on bloom Stats, # of mem_requests
    // Note: Implemented as Ld -> checks bloom_st
    // and St-> Checks bloom_ld 
    // thats all no st->st checking, just like sethumadhavan paper
    // "Scalable Hardware Memory Disambiguation for High ILP Processors"


    // NOte: It is a Store Instruction
    // Note: it also takes care of incementing corresponding bloom_st
    // is_LSQ_yes = there is no  dependency with an earlier Load in LSQ
    bloom_stats_for_stores(req_addr,is_matching_lsq);
    bloom_stats_for_loads(req_addr,is_matching_lsq);

    //Decrement bloom_ST counter when this LSQ entry is deleted 
    //after response from ruby


  }


}

//checks each cycle
//
std::list<uint64_t> Memory::recv_resp(uint64_t prev_completion_cycles)
{


  // For every cycle Overflow buffer has an entry waiting
  // to be sent to memory, because of LSQ size constraints
  // we add it to m_port_unavailable

  if(overflow_buffer.size() > 0)
  {
    m_port_unavailable++;
  }


  ++mem_clk;

  //----------
  // We assume a single port issue to memory 
  //TODO: instead of front look for addresses that belong to a particular bank
  //which is not full and send that address to memory - Just 1 for now

  for(auto i=0; i< NUM_BANKS; i++)
  {
    if(g_BANK_SIZE[i] > m_BANK_SIZE[i])
      // if(g_BANK_SIZE > m_SIZE_LSQ)
    {

      if(overflow_buffer.size() > 0)
      {
        auto key = overflow_buffer.front();
        D(cerr<<" sending memory addr: from overflow_buffer iter: "<<key.first <<" node:"<<key.second <<" overflow_buffer_size: "<<overflow_buffer.size()<<endl);
        send_req(key.first, key.second);
        overflow_buffer.pop();
        break;
      }

    }
  }

  //----------
  this->access(prev_completion_cycles);
  return process_mshr();
}

//checks each cycle
//
bool Memory::access(uint64_t prev_completion_cycles)
{

  if(!MSHR.empty() )
  {

    //-------------
    // Issuing only 1 memop per cycle
    for( auto &it: MSHR)
    {
      Addr req_addr = it.first;
      uint64_t req_size  = line_size();
      auto mem_value = LSQ[req_addr].front();
      bool isWrite = mem_value.isWrite;
      bool is_issue = MSHR[req_addr]; 


      if(m_ruby->isReady(req_addr,req_size,m_core_id, m_thread_id, false, false, NULL) && !is_issue )
        // if(!is_issue )
      {
        it.second = true;

        D(std::cerr<<"m_ruby: access: sending to ruby req_addr:"<<req_addr<<"\t"
            // "req_size:"<<req_size<<"\t"
            <<"isWrite: "<<mem_value.isWrite<<"\t"
            <<"nodeid: "<<mem_value.nodeid <<"\t"
            <<"is_issue:"<<it.second
            <<endl);
        // dummy_ruby->send_req(req_addr,mem_clk);
        req_size = line_size();
        m_ruby->send_request(req_addr, req_size, m_core_id,m_thread_id, isWrite, false, NULL);

        m_ruby_requests++;
        return true;
      }
    }

    return false;
    //-------------
  }


  return false;
}
// get base line address
// FIXME (jaekyu, 3-7-2012)
// replace 63 with the cache line size
  Addr 
Memory::base_addr(Addr addr)
{
  return (addr & ~63);
}


// get cache line size
  uint64_t 
Memory::line_size()
{
  return 64; 
}


  std::list<uint64_t> 
Memory::process_mshr()
{
  std::list<uint64_t> m_mem_resp_list;

  // auto resp_dummy_ruby = dummy_ruby->recv_resp(mem_clk);
  //Copy early retires to send response - St-Ld Forwarding 
  while(!early_retire.empty())
  {
    D(std::cout<<"m_ruby: get response: early_retires: nodeid " << early_retire.front() <<endl 
     );
    m_mem_resp_list.push_back(early_retire.front());
    early_retire.pop_front();
  }

  //Checking response 

  // while(!resp_dummy_ruby.empty())
  while(!m_ruby->RubyQueueEmpty(m_core_id))
  {

    // Addr req_addr = resp_dummy_ruby.front();
    Addr req_addr = m_ruby->RubyQueueTop(m_core_id);
    Addr bankid = get_bankid(req_addr);
    D(std::cout<<"m_ruby: get response: addr "<<req_addr<<"\t"
     );
    //Remember there should not be any Load, if a Store address is returning from memory
    //Since we are forwarding 
    //and if Load address is returning, we can retire all Loads after that together
    //till we hit a Store
    // m_mem_resp_list.push_back(matching_req->nodeid);

    auto & entry = LSQ[req_addr];
    bool isSt = entry.front().isWrite;
    D(std::cout<<" isST: "<<isSt<<"\n");


    
    if(!isSt)
    {

      D(std::cout<<"m_ruby: Before erase LSQ_size: "<<LSQ[req_addr].size()  <<" addr: "<<req_addr<<"\n");
      // Till we keep finding Loads  keep retiring
      while ( !entry.empty() &&  !entry.front().isWrite)
      {

        D(std::cout<<"m_ruby: get response: LD nodeid:" << entry.front().nodeid <<endl 
         );

        //send responses 
        m_mem_resp_list.push_back(entry.front().nodeid);
        //delete corresponding Load Store Entry
        entry.erase(entry.begin());
        m_SIZE_LSQ--;
        m_BANK_SIZE[bankid]--;

        // Bloom Stats - Decrement corresponding Bloom Counter
        // true = it is a Load instruction;
        auto isLd = !isSt;
        // isLd : to reduce confusion
        decrement_bloom_counters(req_addr, isLd);


      }

      D(std::cout<<"m_ruby: After erase LSQ_size: "<<LSQ[req_addr].size()  <<" addr: "<<req_addr<<"\n");

      if(entry.empty())
        LSQ.erase(req_addr);
    }
    //If it was a Store instruction
    else 
    {

      D(std::cout<<"m_ruby: get response: ST nodeid:" << entry.front().nodeid 
          <<"  LSQ size before deletion: "<<entry.size()
          <<endl 
       );

      //send responses 
      m_mem_resp_list.push_back(entry.front().nodeid);
      //delete corresponding Load Store Entry
      entry.erase(entry.begin());
      m_SIZE_LSQ--;
      m_BANK_SIZE[bankid]--;
      D(std::cout<<"entry size after deletion: "<<entry.size()
          <<"is_empty: "<<entry.empty()
          <<endl
       );



      // Bloom Stats - Decrement corresponding Bloom Counter
      // true = it is a Load instruction;
      auto isLd = !isSt;
      // isLd : to reduce confusion
      decrement_bloom_counters(req_addr, isLd);





      if(entry.empty())
        LSQ.erase(req_addr);


    }

    // If there are entries in LSQ with same addresses
    // So that you issue the address to memory next time for remaining Ld/St entries
    if( LSQ.find(req_addr) != LSQ.end() )
    {

      MSHR[req_addr] = false; 
      D(std::cout<<"MSHR False  LSQ corresp entry size: "<<LSQ[req_addr].size() << endl 
       );

    }
    // Delete the MSHR entry
    else 
    {
      D(std::cout<<"MSHR Erase" << endl 
       );


      MSHR.erase(req_addr);
    }
    //Delete ruby response 
    // resp_dummy_ruby.pop();
    m_ruby->RubyQueuePop(m_core_id);


  }

  return m_mem_resp_list;
}


void Memory::fill_global_memaddr_map(uint64_t iter, uint64_t nodeid , MemValue mem_value)
{
  memaddr_map[std::make_pair (iter,nodeid)] = ( mem_value );
}

uint64_t Memory::global_memaddr_map_size()
{
  return memaddr_map.size();
}

void Memory::print_global_memaddr_map()
{
  for( auto &x : memaddr_map)
  {
    cout<<"iter:"<<x.first.first <<"\t";
    cout<<"nodeid:"<<x.first.second <<"\t";
    // cout<<"opcode:"<<x.second.opcode <<"\t";
    cout<<"isWrite:"<<x.second.isWrite <<"\t";
    cout<<"mem_addr::"<<x.second.addr<<endl;

  }
}

uint64_t Memory::bloom_fp_ld()
{
  return m_bloom_ld_fp ;
}


uint64_t Memory::bloom_fp_st()
{
  return m_bloom_st_fp;
}


uint64_t Memory::bloom_yes_ld()
{
  return m_bloom_ld_yes ;
}

uint64_t Memory::bloom_yes_st()
{
  return m_bloom_st_yes;
}

uint64_t Memory::bloom_no_ld()
{
  return m_bloom_ld_no ;
}

uint64_t Memory::bloom_no_st()
{
  return m_bloom_st_no;
}

uint64_t Memory::port_unavailable()
{
  return m_port_unavailable;
}


// uint64_t Memory::bloom_stall_cycles()
// {
//   return m_bloom_stall_cycles;
// }


bool

Memory::bloom_stats_for_stores(Addr req_addr, bool is_LSQ_yes)
{

  physical_address_t _paddr = req_addr;
  Address addr_c(_paddr);
  //XXX: Only St instructions should enter this 

  // Note: Implemented as Ld -> checks bloom_st
  // and St-> Checks bloom_ld 
  // thats all no st->st checking, just like sethumadhavan paper
  // "Scalable Hardware Memory Disambiguation for High ILP Processors"

  //NOTE: its a St Instruction which updates the stats of 
  // bloom_st 
  // and then checks inside bloom_st for conflicts

  // As soon as a memory instruction enters an LSQ
  // update the stats inside bloom filter 

  // UPDATING PHASE INCREMENTING COUNTER - SHOULD HAPPEN IN BLOOM_ST
  //increment bloom filter
  if(bloom_filter_st.getCount(addr_c) < MAX_BLOOM_COUNT )
    bloom_filter_st.increment(addr_c);
  else 
  {
    assert(0 && "bloom filter st max count reached");
    cerr<<"ERROR:bloom_filter_st max count reached - To_Stall Memory";
  }




  //CHECK PHASE  BLOOM : CHECKING AGAINST BLOOM_LD

  //load bloom says no
  if(bloom_filter_ld.getCount(addr_c) ==0) 
  {
    m_bloom_ld_no++;
    //NOte: Bloom can never be wrong, when it says no
    //Do NOT have False negatives
    D(cout<<"bloom_filter_ld: NO addr:"<<req_addr
        <<endl);
    return false;
  }
  //load bloom says yes
  else 
  {
    m_bloom_ld_yes++;

    // IF LSQ said NO there is not a dependency WITH A PREVIOUS LOAD
    // and bloom said YES, then its a False Positive 
    if(!is_LSQ_yes)
      ++m_bloom_ld_fp;

    D(cout<<"bloom_filter_ld: YES addr:"<<req_addr
        <<endl);
    return true;
  }
}







bool

Memory::bloom_stats_for_loads(Addr req_addr, bool is_LSQ_yes)
{


  physical_address_t _paddr = req_addr;
  Address addr_c(_paddr);

  //XXX: Only Ld instructions should enter this 

  // Note: Implemented as Ld -> checks bloom_st
  // and St-> Checks bloom_ld 
  // thats all no st->st checking, just like sethumadhavan paper
  // "Scalable Hardware Memory Disambiguation for High ILP Processors"

  //NOTE: its a Ld Instruction which updates the stats of 
  // bloom_ld 
  // and then checks inside bloom_st for conflicts

  // As soon as a memory instruction enters an LSQ
  // update the stats inside bloom filter 

  // UPDATING PHASE INCREMENTING COUNTER - SHOULD HAPPEN IN BLOOM_LD
  //increment bloom filter
  if(bloom_filter_ld.getCount(addr_c) < MAX_BLOOM_COUNT )
    bloom_filter_ld.increment(addr_c);
  else 
  {
    assert(0 && "bloom filter LD max count reached");
    cerr<<"ERROR:bloom_filter_ld max count reached - To_Stall Memory";
  }




  //CHECK PHASE  BLOOM : CHECKING AGAINST BLOOM_ST

  //store bloom says no
  if(bloom_filter_st.getCount( addr_c) ==0) 
  {
    m_bloom_st_no++;
    //NOte: Bloom can never be wrong, when it says no
    //Do NOT have False negatives
    D(cout<<"bloom_filter_st: NO addr:"<<req_addr
        <<endl);
    return false;
  }
  //store bloom says yes
  else 
  {
    m_bloom_st_yes++;

    // IF LSQ said NO there is not a dependency WITH A PREVIOUS STORE 
    // and bloom said YES, then its a False Positive 
    if(!is_LSQ_yes)
      ++m_bloom_st_fp;

    D(cout<<"bloom_filter_ld: YES addr:"<<req_addr
        <<endl);
    return true;
  }
}

  void 
Memory::decrement_bloom_counters(Addr req_addr, bool is_Ld)
{

  physical_address_t _paddr = req_addr;
  Address addr_c(_paddr);
  //  NOTE: When LD/St instruction leaves an LSQ 
  //  Need to decrement bloom counters for that memory address
  //  in corresponding bloom filter


  // For a Load Instruction
  if(is_Ld)
  {
    if(bloom_filter_ld.getCount(addr_c) > 0)
      bloom_filter_ld.decrement(addr_c);
    //FIXME
    else 
    {
      assert(0 && "bloom filter ld count ==0\n");
      std::cerr<<"Error: bloom_filter_ld count=0 when entry is in Ruby\n";
    }
  }

  // For a Store Instruction
  else 
  {
    if(bloom_filter_st.getCount(addr_c) > 0)
      bloom_filter_st.decrement(addr_c);
    //FIXME
    else 
    {
      assert(0 && "bloom filter st count ==0\n");
      std::cerr<<"Error: bloom_filter_st count=0 when entry is in Ruby\n";
    }


    if(bloom_filter_ld.getCount(addr_c) > 0)
      bloom_filter_ld.decrement(addr_c);
    //FIXME
    else 
    {
      assert(0 && "bloom filter ld1 count ==0\n");
      std::cerr<<"Error: bloom_filter_ld1 count=0 when entry is in Ruby\n";
    }
  }
}




  Addr 
Memory::get_bankid(Addr req_addr)
{
  //FIXME
  return (req_addr>>6)%NUM_BANKS;
}

void
Memory::add_to_mlp_hist( uint64_t m_SIZE_LSQ)
{
  if(m_SIZE_LSQ<2)
    m_mlp_hist[0]++;
 else if(m_SIZE_LSQ < 4)
    m_mlp_hist[1]++;
 else if(m_SIZE_LSQ < 8)
    m_mlp_hist[2]++;
 else if(m_SIZE_LSQ < 16)
    m_mlp_hist[3]++;
 else if(m_SIZE_LSQ < 32)
    m_mlp_hist[4]++;
 else if(m_SIZE_LSQ < 64)
    m_mlp_hist[5]++;
 else
    m_mlp_hist[6]++;
}
