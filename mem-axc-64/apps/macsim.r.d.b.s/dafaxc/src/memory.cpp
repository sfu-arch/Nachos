#include "memory.hpp"
#include "dummy_ruby.hpp"


Memory::Memory(bool _stall_on_bloom, std::string _bloom_type, bool ishash2, 
    uint64_t mem_lat, uint64_t _mem_req_stack) 
{
  dummy_ruby = new Dummy_ruby(mem_lat);
  // m_simBase = simBase;
  // REPORT("Memory system(%s) has been initialized.\n",
  //     KNOB(KNOB_MEMORY_TYPE)->getValue().c_str());

  // dummy_ruby(lat);
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
  this->initialize(_stall_on_bloom);
  // m_ruby = _ruby;
  m_port_unavailable=0;

  m_bloom_stall_cycles =0;

  bloom_filter_ld = new GenericBloomFilter_1(_bloom_type, ishash2);
  bloom_filter_st = new GenericBloomFilter_1(_bloom_type, ishash2);


  m_mshr_full_cycles =0;
  m_stall_on_bloom = _stall_on_bloom;
  g_mem_req_stack = _mem_req_stack;
  mem_req_stack = _mem_req_stack; 
  // mem_req_stack = MEM_MSHR_SIZE;
}


void Memory::initialize(bool _stall_on_bloom)
{


  mem_clk=0;
  m_bloom_st_yes=0;
  m_bloom_st_no=0;
  m_bloom_st_fp=0;

  m_bloom_ld_yes=0;
  m_bloom_ld_no=0;
  m_bloom_ld_fp=0;

  m_bloom_stall_cycles =0;
  m_stall_on_bloom = _stall_on_bloom;

  mem_req_stack = g_mem_req_stack;
  // mem_req_stack = MEM_MSHR_SIZE;

  // m_mshr_full_cycles =0;

  LSQ.clear();
  MSHR.clear();
  // ldst_addr_buffer.clear();
}

void Memory::print_mem_stats()
{
  cout<<"----------------------------------\n";
  cout<<"Printing dafaxc memory state \n";

  std::cout<<"mem_req_stack: "<<mem_req_stack<<endl;
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
  bloom_filter_ld->clear();
  bloom_filter_st->clear();

  delete bloom_filter_ld;
  delete bloom_filter_st;

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
  if (memaddr_map.find(key) != memaddr_map.end())
  {
    MemValue mem_value = memaddr_map[key];

    bool isWrite = mem_value.isWrite;
    Addr vaddr = mem_value.addr;
    Addr req_addr = base_addr(vaddr);
    cout<<"iter: "<<iter<<"  nodeid: "<<nodeid<<"isWrite: "<<isWrite<<endl;
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
            matching_st = it;
          // cout<<" "<< *it;

        }
        //Note: entry contains the youngest store, with a nodeid which occurs before the load in
        //program order;
        //it.nodeid < nodeid may not be required since, the input dot graph should already ensure that

        //if Matching Store found -- add it to early_retire 
        if(matching_st.isWrite)
        {
          D(cout<<" Earlier Matching Store Exist with nodeid: "<<matching_st.nodeid
              <<"\n");
          early_retire.push_back(nodeid);

        }

        // Earlier matching Load exist
        else 
        {
          D(cout<<" Earlier Matching Load Exists with nodeid: "<<matching_st.nodeid
              <<"\n");
          //make an entry in LSQ with is_issue=1 
          //Need to retire this Load with the previous Load
          //Since there is no other store in between and thus,
          //can retire together
          LSQ[req_addr].push_back(MemValue_t{isWrite,nodeid});
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
      }

    }
    //is is a Store instruction
    else 
    {
      D(cout<<"m_ruby: ST:"
          <<"\t");

      //check if there is a previous Ld/St of same block address
      //if there is no previous entry
      if(LSQ.find(req_addr) == LSQ.end() )
      {
        D(cout<<" No Matching LD/St"
            <<"\n");

        MSHR[req_addr] = false;

      }
      else 
      {
        D(cout<<" Some  Matching LD/St"
            <<"\n");
      }

      //Create an LSQ entry
      LSQ[req_addr].push_back(MemValue_t{isWrite,nodeid});

    }

  }
  else 
  {
    assert(0 && "dafaxc: sent request not in memaddr_map \n");
  }
}

// void Memory::send_req( uint64_t iter, uint64_t nodeid#<{(|, bool isWrite|)}># )
// {
//   auto key =std::make_pair (iter,nodeid);
//   if (memaddr_map.find(key) != memaddr_map.end())
//   {
//     MemValue mem_value = memaddr_map[key];
//     cout<<"m_ruby: send_req to ldst_addr_buffer-  mem_value :"<<mem_value.addr<<"\t"
//         <<"nodeid: "  << nodeid<<"\t"
//         <<"iter: "    << iter<<"\t"
//         <<"clk: "     << mem_clk
//         <<"\n";
//         
//     D(cout<<"m_ruby: send_req to ldst_addr_buffer-  mem_value :"<<mem_value.addr<<"\t"
//         <<"nodeid: "<<nodeid<<"\t"
//         <<"iter: "<<iter<<"\t"
//         <<"\n");
//     bool isWrite = mem_value.isWrite;
//     Addr vaddr = mem_value.addr;
//     // MemValue_t ldst_value = MemValue_t{isWrite,nodeid,vaddr,false};
//     physical_address_t _paddr = vaddr;
//     Address addr_c(_paddr);
//     MemValue_t ldst_value = MemValue_t{isWrite,nodeid,vaddr,addr_c,false};
//     // ldst_addr_buffer.insert( std::make_pair#<{(|<Addr,MemValue_t>|)}># (vaddr,ldst_value )); 
//     ldst_addr_buffer.push_back(ldst_value);
//   }
//   else
//     assert(0 && "dafaxc: sent request not in memaddr_map \n");
//
//
// }


//
// //checks each cycle
// bool Memory::access(uint64_t prev_completion_cycles)
// {
//
//   if(!ldst_addr_buffer.empty() && mem_req_stack!=0)
//   {
//
//     bool is_port_avail = true;
//     //-------------
//     for( auto &mem_value: ldst_addr_buffer)
//     {
//       // MemValue_t  mem_value = ldst_addr_buffer.front();
//       Addr req_addr = base_addr(mem_value.vaddr);
//       uint64_t req_size  = line_size();
//       bool isWrite = mem_value.isWrite;
//       bool is_issue = mem_value.is_issue;
//
//       if(!is_issue)
//       // if(m_ruby->isReady(req_addr,req_size,m_core_id, m_thread_id, false, false, NULL) && !is_issue )
//       {
//         bool is_issue_inst = false;
//         bool is_bloom_st_yes = false; 
//         bool is_bloom_ld_yes = false; 
//         //ld_blooms should be looked at by store instructions
//         //st_blooms should be looked by ld and st instructions
//         //----------------------------------------------------
//         // if its a load instruction
//         if(!isWrite) 
//         {
//           //Loads look at Stores 
//           is_bloom_st_yes = this->update_bloom_st(mem_value);
//
//
//           //LOGIC FOR LSQ+BLOOM and ONLY BLOOM
//           //when m_stall_on_bloom is true - that means its only bloom filter
//           //thus, if bloom filter says yes, you stall and wait till there is no conflict
//           //In case of a BLOOM + LSQ - m_stall_on_bloom is always false
//           //and You always issue the instruction and update the counter for bloom filter
//           //since, the memory instruction is being processed
//           //For performance: LSQ and LSQ + bloom will currently have the same cycles
//           //Assuming, the latency for a bloom check=1 cycle. One can add the
//           //total number of memory requests to the final performance number, both the 
//           //counters are printed at the end of the run.
//            //increment bloom_ld count 
//           //1- if m_stall_on_bloom is true - then increment only
//           //if bloom filter says no
//           //2- if m_stall_on_bloom is false - then is_bloom_st_yes
//           //is dont care. Thus theny follow an and logic to increment ld_count
//           
//           if( !(is_bloom_st_yes & m_stall_on_bloom))
//           {
//             is_issue_inst= true;
//             this->increment_bloom_ld_count(mem_value);
//           }
//           else 
//             ++m_bloom_stall_cycles;
//
//          
//           
//         }   
//         // its a store instruction
//         else 
//         {
//           //look at store bloom
//           is_bloom_st_yes = this->update_bloom_st(mem_value);
//
//           //look at load bloom
//           is_bloom_ld_yes = this->update_bloom_ld(mem_value);
//
//           //increment store bloom filter
//           //if m_stall_on_bloom true then
//           //if any of the bloom_ld or bloom_st is true - do not increment
//           //else dont care
//           if( !((is_bloom_st_yes|is_bloom_ld_yes) & m_stall_on_bloom))
//           {
//             is_issue_inst= true;
//             this->increment_bloom_st_count(mem_value);
//
//           }
//           else 
//             ++m_bloom_stall_cycles;
//         }
//
//
//         if(is_issue_inst)
//         {
//         //to notify ldst_addr_buffer : that the op is already issued to ruby
//         mem_value.is_issue = true;
//
//         std::cout<<"m_ruby: send_req: vaddr: "<<mem_value.vaddr<<"\t"
//             <<"req_addr:"<<req_addr<<"\t"
//             "req_size:"<<req_size<<"\t"
//             <<"nodeid: "<<mem_value.nodeid <<"\t"
//             <<"is_issue:"<<mem_value.is_issue
//             <<"issue_clock: "<<mem_clk<<"\t"
//             <<"ldst_addr_buffer.size " <<ldst_addr_buffer.size()
//             <<endl;
//         --mem_req_stack;
//         dummy_ruby->send_req(req_addr,mem_clk);
//         // m_ruby->send_request(req_addr, req_size, m_core_id,m_thread_id, isWrite, false, NULL);
//         return true;
//         }
//       }
//       else
//         is_port_avail=false;
//
//     }
//
//     if(!is_port_avail)
//       ++m_port_unavailable;
//
//     //-------------
//   }
//   else if( mem_req_stack == 0)
//   {
//     ++m_mshr_full_cycles;
//   }
//
//   return false;
// }


//checks each cycle
//
std::list<uint64_t> Memory::recv_resp(uint64_t prev_completion_cycles)
{

  ++mem_clk;
  // m_ruby->advance_time();
  this->access(prev_completion_cycles);
  return process_mshr();
}

//checks each cycle
//
bool Memory::access(uint64_t prev_completion_cycles)
{

  if(!MSHR.empty() && mem_req_stack!=0)
  {

    //-------------
    // Issuing only 1 memop per cycle
    for( auto &it: MSHR)
    {
      Addr req_addr = it.first;
      // uint64_t req_size  = line_size();
      auto mem_value = LSQ[req_addr].front();
      // bool isWrite = mem_value.isWrite;
      bool is_issue = MSHR[req_addr]; 


      // if(m_ruby->isReady(req_addr,req_size,m_core_id, m_thread_id, false, false, NULL) && !is_issue )
      if(!is_issue )
      {
        it.second = true;

        D(std::cout<<"m_ruby: access: sending to ruby req_addr:"<<req_addr<<"\t"
            // "req_size:"<<req_size<<"\t"
            <<"isWrite: "<<mem_value.isWrite<<"\t"
            <<"nodeid: "<<mem_value.nodeid <<"\t"
            <<"is_issue:"<<it.second
            <<endl);
        // mem_req_stack--;
        dummy_ruby->send_req(req_addr,mem_clk);
        // m_ruby->send_request(req_addr, req_size, m_core_id,m_thread_id, isWrite, false, NULL);
        return true;
      }
      else
        ++m_port_unavailable;
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

  auto resp_dummy_ruby = dummy_ruby->recv_resp(mem_clk);
  //Copy early retires to send response - St-Ld Forwarding 
  while(!early_retire.empty())
  {
    D(std::cout<<"m_ruby: get response: early_retires: nodeid " << early_retire.front() <<endl 
     );
    m_mem_resp_list.push_back(early_retire.front());
    early_retire.pop_front();
  }

  //Checking response 

  while(!resp_dummy_ruby.empty())
    // while(!m_ruby->RubyQueueEmpty(m_core_id))
  {

    Addr req_addr = resp_dummy_ruby.front();
    // Addr req_addr = m_ruby->RubyQueueTop(m_core_id);
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
      D(std::cout<<"entry size after deletion: "<<entry.size()
          <<"is_empty: "<<entry.empty()
          <<endl
       );

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
    resp_dummy_ruby.pop();
    // m_ruby->RubyQueuePop(m_core_id);


  }

  return m_mem_resp_list;
}

// std::list<uint64_t> 
// Memory::process_mshr()
// {
//
//   // uint64_t tmp_line_size = line_size();
//   std::list<uint64_t> m_mem_resp_list;
//   auto resp_dummy_ruby = dummy_ruby->recv_resp(mem_clk);
//
// while(!resp_dummy_ruby.empty())
//   // while(!m_ruby->RubyQueueEmpty(m_core_id))
//   {
//     ++mem_req_stack;
//     // Addr addr = m_ruby->RubyQueueTop(m_core_id);
//     Addr addr = resp_dummy_ruby.front();
//     // Search the mshr for the request
//     const MemValue_t* matching_req = search_req(addr, line_size());
//     std::cout<<"m_ruby_resp: base_addr:"<<addr
//       << "\t memclock:" << mem_clk
//       << endl;
//
//     D(this->print_mem_stats());
//     assert(matching_req != NULL && "Addr returned from Ruby not found in ldst_addr_buffer");
//
//     D(std::cout<<"m_ruby: get response: addr "<<addr<<"\t"
//         <<"nodeid: "<<matching_req->nodeid<<"\t"
//         <<endl);
//     //-------------
//
//     //decrement Ld bloom filter - its a load instruction
//     if(!matching_req->isWrite)
//     {
//       this->decrement_bloom_ld_count(matching_req);
//     }
//     //decrement ST bloom filter
//     else 
//     {
//       this->decrement_bloom_st_count(matching_req);
//     }
//
//     //--------------
//     m_mem_resp_list.push_back(matching_req->nodeid);
//     ldst_addr_buffer.remove(*matching_req);
//     // m_ruby->RubyQueuePop(m_core_id);
//     resp_dummy_ruby.pop();
//   }
//
//   return m_mem_resp_list;
// }


///////////////////////////////////////////////////////////////////////////////////////////////

// search matching request
//   MemValue_t* 
// Memory::search_req(Addr addr, uint64_t size)
// {
//   for( auto &mem_value : ldst_addr_buffer)
//   {
//     Addr vaddr = mem_value.vaddr;
//     // addr here below is base addr
//     //ORIGINAL: from macsim: req->m_addr <= addr && req->m_addr+req->m_size >= addr+size
//     //FIXME: vaddr + size => size corresponds to m_mem_size from macsim : in uop.cc 
//     //This size actually varies from opcode to opcode. and to set this we need a decoder i.e we need more info
//     //than just Load op or store op
//     if( (vaddr >= addr && vaddr < addr+size ) && mem_value.is_issue )
//       return &mem_value;
//   }
//
//   return NULL;
// }


void Memory::fill_global_memaddr_map(uint64_t iter, uint64_t nodeid , MemValue mem_value)
{
  if(nodeid == 3 && iter==1)
    cout<<"Global Memory: isWrite:"<<mem_value.isWrite <<"  addr:"<<mem_value.addr<<endl;

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

uint64_t Memory::bloom_fp()
{
  return m_bloom_ld_fp + m_bloom_st_fp;
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

uint64_t Memory::bloom_stall_cycles()
{
  return m_bloom_stall_cycles;
}




//  bool
// Memory::update_bloom_st(MemValue_t mem_value)
// {
//   //store bloom says no
//   if(bloom_filter_st->getCount(mem_value.addr_c) ==0) 
//   {
//     m_bloom_st_no++;
//     D(cout<<"bloom_filter_st: NO addr:"<<mem_value.vaddr
//         <<endl);
//     return false;
//   }
//
//   //store bloom says yes
//   else 
//   {
//     m_bloom_st_yes++;
//     D(cout<<"bloom_filter_st: YES addr:"<<mem_value.vaddr
//         <<endl);
//     //if bloom filter says yes and the entry is not in mshr- or 
//     //or if the entry is present but not issued 
//     //then its a false positive
//     const MemValue_t* matching_req_inldst = search_req(mem_value.vaddr,line_size());
//     if(matching_req_inldst== NULL)
//       ++m_bloom_st_fp ;
//     else
//     {
//       if(matching_req_inldst->is_issue == false)
//         ++m_bloom_st_fp;
//     }
//     return true;
//   }
//
// }

// bool
// Memory::update_bloom_ld(MemValue_t mem_value)
// {
//   //load bloom says no
//   if(bloom_filter_ld->getCount(mem_value.addr_c) ==0) 
//   {
//     m_bloom_ld_no++;
//     D(cout<<"bloom_filter_ld: NO addr:"<<mem_value.vaddr
//         <<endl);
//     return false;
//   }
//   //load bloom says yes
//   else 
//   {
//     m_bloom_ld_yes++;
//     //if bloom filter says yes and the entry is not in mshr- or 
//     //or if the entry is present but not issued 
//     //then its a false positive
//     const MemValue_t* matching_req_inldst = search_req(mem_value.vaddr,line_size());
//     if(matching_req_inldst== NULL)
//       ++m_bloom_ld_fp ;
//     else
//     {
//       if(matching_req_inldst->is_issue == false)
//         ++m_bloom_ld_fp;
//     }
//     D(cout<<"bloom_filter_ld: YES addr:"<<mem_value.vaddr
//         <<endl);
//     return true;
//   }
// }

//   void 
// Memory::increment_bloom_ld_count(MemValue_t mem_value)
// {
//   //increment bloom filter
//   if(bloom_filter_ld->getCount(mem_value.addr_c) < MAX_BLOOM_COUNT )
//     bloom_filter_ld->increment(mem_value.addr_c);
//   else 
//   {
//     assert(0 && "bloom filter ld max count reached");
//     cerr<<"ERROR:bloom_filter_ld max count reached - To_Stall Memory";
//   }
// }


//   void 
// Memory::increment_bloom_st_count(MemValue_t mem_value)
// {
//   if(bloom_filter_st->getCount(mem_value.addr_c) < MAX_BLOOM_COUNT )
//     bloom_filter_st->increment(mem_value.addr_c);
//   else 
//   {
//
//     assert(0 && "bloom filter st max count reached\n");
//     cerr<<"bloom_filter max count reached - To_Stall Memory";
//   }
// }
//
// void 
// Memory::decrement_bloom_ld_count(const MemValue_t* matching_req)
// {
//   if(bloom_filter_ld->getCount(matching_req->addr_c) > 0)
//     bloom_filter_ld->decrement(matching_req->addr_c);
//   //FIXME
//   else 
//   {
//     assert(0 && "bloom filter ld count ==0\n");
//     std::cerr<<"Error: bloom_filter_ld count=0 when entry is in Ruby\n";
//   }
// }
//
// void 
// Memory::decrement_bloom_st_count(const MemValue_t* matching_req)
// {
//   if(bloom_filter_st->getCount(matching_req->addr_c) > 0)
//     bloom_filter_st->decrement(matching_req->addr_c);
//   //FIXME
//   else 
//   {
//
//     assert(0 && "bloom filter st count ==0\n");
//     std::cerr<<"Error: bloom_filter_st count=0 when entry is in Ruby\n";
//   }
// }


