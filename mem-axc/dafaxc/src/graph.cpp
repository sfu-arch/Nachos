#include "graph.hpp"
#include <iostream>
#include <set>
#include <fstream>
#include "data_type.hpp"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/max_cardinality_matching.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/bipartite.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/filesystem/fstream.hpp>
#include <regex>


#define MY_DEBUG

using namespace std;
using namespace boost;

#define MEMORYLATENCY 100002
#define FORCE_STOP 150000

/* Constructor */
Graph::Graph(uint64_t n, std::string ldstfile_path, string input_filestring, std::shared_ptr<O3sim_ruby> _ruby,bool _is_stall_on_bloom , std::string _bloom_type, bool ishash2 , uint64_t mem_lat, int _mem_req_stack, bool _is_nachos, bool _is_LSQ, uint64_t _issue_width ):orig_graph(0) ,in_graph(0), graph_limit(n), m_stall_on_bloom(_is_stall_on_bloom) , 
  m_memory ( Memory(_ruby, _bloom_type,ishash2, mem_lat, _mem_req_stack)),
  m_is_nachos(_is_nachos), 
  m_is_LSQ(_is_LSQ),
  g_issue_width(_issue_width)
{

  g_iter = 0;
  g_input_filestring = input_filestring;
  cout<<"MSHR_SIZE: "<<_mem_req_stack<<endl;


  std::string mlpstr;

  ldstfile.open(ldstfile_path.c_str(), fstream::in );
  if (!ldstfile.is_open())
  {
    cerr << "graph: Error Mem Dump ldst file not reached end Foey!\n";
  }

  cerr<<"ldst: "<<ldstfile_path.c_str()<<endl;

  read_memcsv(ldstfile);
  // D(print_memmap());
  g_en =true;
  g_mem_req_count =0;

  g_total_ruby_requests=0;
  g_total_termination_cycles =0;
  g_total_bloom_yes_ld =0;
  g_total_bloom_yes_st =0;
  g_total_bloom_no_ld =0;
  g_total_bloom_no_st =0;
  g_total_bloom_fp_ld=0;
  g_total_bloom_fp_st=0;
  g_total_bloom_stall_cycles=0;

  g_total_memops=0;
  g_total_ldops=0;
  g_total_LiveOut =0;

  readGraph();
  if(g_total_memops*ITER_COUNT >= ITER_COUNT_MEM)
  {
    cerr<<"MEM OPS ARE GREATER THAN EXPECTED : CHANGE ITER_COUNT_MEM IN data_type.hpp\n";
    throw std::exception();

  }
  in_graph = orig_graph;
  this->set_state_maps();
}

Graph::~Graph()
{
}

  void
Graph::printstats()
{

  print_disamg_delay();
  cout<<"Iter. # of Vertices: "<< num_vertices(in_graph)<<endl;
  cout<<"Iter. # of Edges:  "<< num_edges(in_graph)<<endl;
  cout<<"Iter. # of MemOps (excluding LiveOuts): "<< g_total_memops - g_total_LiveOut<<endl;
  cout<<"Iter. # of Store LiveOuts: "<< g_total_LiveOut<<endl;
  cout<<"Iter. # of Total Ld Ops: "<< g_total_ldops<<endl;

  cout<<"Cumm. Port Busy Cycles:  "<<m_memory.m_port_unavailable<<endl;
  cout<<"Cumm. Total Memory Requests: "<<g_mem_req_count<<endl;

  auto g_total_comp_delay_1 = 0;
  if(m_is_nachos)
  {
    g_total_comp_delay_1 = g_total_comp_delay;
    g_total_termination_cycles += g_total_comp_delay;
  }
  cout<<"Cumm. total comparator delay: "<< g_total_comp_delay_1 <<endl;


  // cout<<"Cumm. Total axc termination cycles: " <<g_total_termination_cycles<<endl;
  cout<<"Cumm. TERMINATION CALLED :Total axc termination cycles: " <<g_total_termination_cycles <<endl;

  // if(m_stall_on_bloom)
  // {
  //   // Since LSQ will be accessed after bloom filter is accessed
  //   // NOTE: LSQ Latency has already taken into account of 1 cycle counted by g_total_termination_cycles
  //   auto inc_lsq_lat = g_total_termination_cycles + (g_total_bloom_yes_ld + g_total_bloom_yes_st)*(LSQ_Latency+1); 
  //
  // }
  // else 
  // {
  //
  //   // NOTE: LSQ Latency has already taken into account of 1 cycle counted by g_total_termination_cycles
  //   auto inc_lsq_lat = g_total_termination_cycles + g_mem_req_count*LSQ_Latency; 
  //   // cout<<"Cumm. TERMINATION CALLED :Total axc termination cycles: " <<inc_lsq_lat <<endl;
  // }
  


  cout<<"Cumm. Memory:Total_ruby_requests: "<< g_total_ruby_requests <<endl;
  cout<<"Cumm. Memory:Bloom_filter_ld:Yes: "<< g_total_bloom_yes_ld <<endl;
  cout<<"Cumm. Memory:Bloom_filter_st:Yes: "<< g_total_bloom_yes_st <<endl;
  cout<<"Cumm. Memory:Bloom_filter_ld:NO: "<< g_total_bloom_no_ld <<endl;
  cout<<"Cumm. Memory:Bloom_filter_st:NO: "<< g_total_bloom_no_st <<endl;
  cout<<"Cumm. Memory:Bloom False Positive_LD: "<<g_total_bloom_fp_ld <<endl;
  cout<<"Cumm. Memory:Bloom False Positive_ST: "<<g_total_bloom_fp_st <<endl;
  cout<<"Cumm. Memory:Bloom False Positive percent : "<<
    (g_total_bloom_fp_ld + g_total_bloom_fp_st)/ (float)(g_total_bloom_no_ld + g_total_bloom_no_st + g_total_bloom_fp_ld + g_total_bloom_fp_st )*100 <<endl;

  cout<<"Cumm. mlp_hist 0 "<<m_memory.m_mlp_hist[0]<<endl;
  cout<<"Cumm. mlp_hist 1 "<<m_memory.m_mlp_hist[1]<<endl;
  cout<<"Cumm. mlp_hist 2 "<<m_memory.m_mlp_hist[2]<<endl;
  cout<<"Cumm. mlp_hist 3 "<<m_memory.m_mlp_hist[3]<<endl;
  cout<<"Cumm. mlp_hist 4 "<<m_memory.m_mlp_hist[4]<<endl;
  cout<<"Cumm. mlp_hist 5 "<<m_memory.m_mlp_hist[5]<<endl;
  cout<<"Cumm. mlp_hist 6 "<<m_memory.m_mlp_hist[6]<<endl;

  //NOTE: False Positive rate = FP/(FP+TN)
  // cout<<"Cumm. Memory:Bloom Total stall cycles : "<< g_total_bloom_stall_cycles<<endl;
}

/**
 * Loading dot file and load into BG
 */
  void
Graph::readGraph()
{
  D(cout<<"reading graph\n");
  boost::filesystem::ifstream input_file(g_input_filestring,ios::in);
  assert(input_file.is_open() && "Error! input_file File couldn't open");
  //Vertex dp

  boost::dynamic_properties dp(boost::ignore_other_properties);
  auto vname_dp = get(&vertex_prop::vname, orig_graph);
  dp.property("name", vname_dp);

  auto vir_dp = get(&vertex_prop::vir, orig_graph);
  dp.property("ir", vir_dp);

  auto vlabel_dp = get(&vertex_prop::vlabel, orig_graph);
  dp.property("label", vlabel_dp);

  auto vcolor_dp = get(&vertex_prop::vcolor, orig_graph);
  dp.property("color", vcolor_dp);

  auto vopcode_dp = get(&vertex_prop::vopcode, orig_graph);
  dp.property("opcode", vopcode_dp);

  auto vlatency_dp = get(&vertex_prop::vlatency, orig_graph);
  dp.property("vlatency", vlatency_dp);

  auto vstate_dp = get(&vertex_prop::vstate, orig_graph);
  dp.property("vstate", vstate_dp);

  auto vissue_dp = get(&vertex_prop::vissue, orig_graph);
  dp.property("vissue", vissue_dp);

  auto vCDEcomp_dp = get(&vertex_prop::vCDEcomp, orig_graph);
  dp.property("vCDEcomp", vCDEcomp_dp);
  
  auto vOPcomp_dp = get(&vertex_prop::vOPcomp, orig_graph);
  dp.property("vOPcomp", vOPcomp_dp);

  auto vcde_comparator_delay_dp = get(&vertex_prop::vcde_comparator_delay, orig_graph);
  dp.property("vcde_comparator_delay", vcde_comparator_delay_dp);




  //Edge dp
  auto ecolor_dp = get(&edge_prop::ecolor, orig_graph);
  dp.property("color",ecolor_dp);


  auto elatency_dp = get(&edge_prop::elatency, orig_graph);
  dp.property("elatency",elatency_dp);

  auto estate_dp = get(&edge_prop::estate, orig_graph);
  dp.property("estate",estate_dp);
 
  auto ecde_comparator_delay_dp = get(&edge_prop::ecde_comparator_delay, orig_graph);
  dp.property("ecde_comparator_delay", ecde_comparator_delay_dp);


  auto ecde_served_dp = get(&edge_prop::ecde_served, orig_graph);
  dp.property("ecde_served", ecde_served_dp);



  // ecde_comparator_delay
  //ecde_served
  //vcde_comparator_delay



  //-----------------------------------------------------------------------------------
  try
  {
    read_graphviz(input_file, orig_graph, dp, "name");

  }
  catch(std::exception &err)
  {
    cerr << err.what() << endl;
    cerr << "read_graphviz failed \n";
    exit(0);
  }

  //-----------------------------------------------------------------------------------
  //TODO
  // Add termination vertex
  VERTEX u = boost::add_vertex(orig_graph);

  put(&vertex_prop::vstate,orig_graph,u,VNOT_READY);
  put(&vertex_prop::vname,orig_graph,u,99999999);
  put(&vertex_prop::vir,orig_graph,u,"END (TERMINATE)");
  put(&vertex_prop::vlatency,orig_graph,u,1);
  put(&vertex_prop::vopcode,orig_graph,u,"END");
  put(&vertex_prop::vlabel,orig_graph,u,"END");
  put(&vertex_prop::vcolor,orig_graph,u,"Black");
  put(&vertex_prop::vcde_comparator_delay,orig_graph,u,0);


  std::pair<VERTEX_ITER, VERTEX_ITER> v;

  // Add Termination Edge
  for (v = vertices(orig_graph); v.first != v.second; ++v.first)
  {
    auto EI = out_edges(*v.first,orig_graph);
    if(EI.first ==EI.second && *v.first != u)
    {
      boost::add_edge(*v.first,u,orig_graph);
    }
  } 

  //Iterate over property maps of opcode and fill latency in property opcode

  //Add Vertex Properties : vstate based on opcode

  std::pair<VERTEX_ITER, VERTEX_ITER> vp;
  std::set<int> g_LSQ;

  for (vp = vertices(orig_graph); vp.first != vp.second; ++vp.first)
  {
    vlatency_dp[*vp.first] = getOpcodeStr(vopcode_dp[*vp.first]);
    vissue_dp[*vp.first] = false; //Only Used for memory ops
    vCDEcomp_dp[*vp.first] = 0;
    vOPcomp_dp[*vp.first] = 0;
    vcde_comparator_delay_dp[*vp.first] = 0;

    //-------------
    if (vir_dp[*vp.first].find("!LO") != std::string::npos)
    {
      ++g_total_LiveOut;
      vlatency_dp[*vp.first] = 1; // Latency for Live Outs is 1 : which are store operations by default
      D(cerr<<"graph: LIveOuts \n "<< vir_dp[*vp.first] <<endl);
    }
    // if it is not a !LO  Live Out and it is  a Ld/St then store nodeids in g_LSQ
    else if (vopcode_dp[*vp.first] == "Load" || vopcode_dp[*vp.first] == "Store") 
    {
      g_LSQ.insert(vname_dp[*vp.first]);

    }
    //--------------

    if( vopcode_dp[*vp.first] == "Arg" || vopcode_dp[*vp.first] == "Const" 
        || vopcode_dp[*vp.first] == "BB")
    {
      vstate_dp[*vp.first] = VCOMPLETE  ; 
    }
    else 
    {
      vstate_dp[*vp.first] = VNOT_READY  ; 
    }

  }

  //print g_LSQ
  std::cout << "g_LSQ contains:";
  for (auto it=g_LSQ.begin(); it!=g_LSQ.end(); ++it)
    std::cout << ' ' << *it;
  std::cout << '\n';


  //Add Edge Properties: etate and elatency

  EDGE_ITER ei, ei_end;
  for (tie(ei, ei_end) = edges(orig_graph); ei != ei_end; ++ei)
  {

    VERTEX v = source(*ei,orig_graph);
    auto tmp_edge_latency = get(&vertex_prop::vlatency,orig_graph,v)  ;
    //Check if the source vertex is a live out then add latency=1 and not MEMORYLATENCY
    //since liveouts are connected to a fake termination edge
    auto tmp_ir = get(&vertex_prop::vir,orig_graph,v);

    //Set Edge Properties : Latency and ESTATE
    if(tmp_ir.find("!LO") == std::string::npos)
    {
      elatency_dp[*ei] = tmp_edge_latency;
    }
    else
      elatency_dp[*ei] = 1;//latency is 1 for Live Out stores

    estate_dp[*ei] = ENOT_READY;

    ecde_comparator_delay_dp[*ei] = 0;
    ecde_served_dp[*ei] = false;

  }



  try
  {
    D(cout<<" Constructed graph:"<<endl);
    D(write_graphviz_dp(std::cout, orig_graph, dp, "name"));
  }
  catch(std::exception &err)
  {
    cerr << err.what() << endl;
    // cerr << "read_graphviz failed for " << input_file << "\n";
    exit(0);
  }


  return;
}

  void
Graph::set_state_maps()
{

  //------------------------------------------------------------------------------------
  //Iterate over property maps of opcode and fill latency in property opcode

  //Add Vertex Properties : vstate based on opcode

  std::pair<VERTEX_ITER, VERTEX_ITER> vp;

  for (vp = vertices(in_graph); vp.first != vp.second; ++vp.first)
  {
    auto vopcode = get(&vertex_prop::vopcode,in_graph,*vp.first);

    if( vopcode == "Arg" || vopcode == "Const" 
        || vopcode == "BB"   )
    {

      //Initialize Vertex State Maps
      vertex_state_map[VCOMPLETE].push_back(*vp.first);
    }
    else 
    {
      //Initialize Vertex State Maps
      vertex_state_map[VNOT_READY].push_back(*vp.first);
    }

  }

  //Add Edge Properties: etate and elatency
  EDGE_ITER ei, ei_end;
  for (tie(ei, ei_end) = edges(in_graph); ei != ei_end; ++ei)
  {
    //Initialize all Edges to NOT_READY: EDGE STATE_MAPS
    edge_state_map[ENOT_READY].push_back(*ei);
  }
}


  void
Graph::initialize(uint64_t _iter)
{

  //TODO Update global stats for histogram
  update_disamg_delay();

  g_iter = _iter;
  global_clock=0;
  in_graph =0;

  // D(printGraph());

  for( auto &X : vertex_state_map)
  {
    X.clear();
  }
  for( auto &Y : edge_state_map)
  {
    Y.clear();
  }

  in_graph = orig_graph;
  this->set_state_maps();

  m_memory.initialize();

  g_rdy = true;
}


/**
 * Writing BG into output file
 */
  void
Graph::printGraph()
{
  cout<<"Printing Graph \n";

  //Vertex dp

  boost::dynamic_properties dp(boost::ignore_other_properties);
  auto vname_dp = get(&vertex_prop::vname, in_graph);
  dp.property("vname", vname_dp);

  auto vir_dp = get(&vertex_prop::vir, in_graph);
  dp.property("vir", vir_dp);

  auto vlabel_dp = get(&vertex_prop::vlabel, in_graph);
  dp.property("vlabel", vlabel_dp);

  auto vcolor_dp = get(&vertex_prop::vcolor, in_graph);
  dp.property("vcolor", vcolor_dp);

  auto vopcode_dp = get(&vertex_prop::vopcode, in_graph);
  dp.property("vopcode", vopcode_dp);

  auto vlatency_dp = get(&vertex_prop::vlatency, in_graph);
  dp.property("vlatency", vlatency_dp);

  auto vstate_dp = get(&vertex_prop::vstate, in_graph);
  dp.property("vstate", vstate_dp);

  auto vissue_dp = get(&vertex_prop::vissue, in_graph);
  dp.property("vissue", vissue_dp);

  auto vCDEcomp_dp = get(&vertex_prop::vCDEcomp, in_graph);
  dp.property("vCDEcomp", vCDEcomp_dp);
  
  auto vOPcomp_dp = get(&vertex_prop::vOPcomp, in_graph);
  dp.property("vOPcomp", vOPcomp_dp);

  auto vcde_comparator_delay_dp = get(&vertex_prop::vcde_comparator_delay, in_graph);
  dp.property("vcde_comparator_delay", vcde_comparator_delay_dp);







  //----------------------------------------------------
  //Edge dp
  auto ecolor_dp = get(&edge_prop::ecolor, in_graph);
  dp.property("ecolor",ecolor_dp);


  auto elatency_dp = get(&edge_prop::elatency, in_graph);
  dp.property("elatency",elatency_dp);

  auto estop_dp = get(&edge_prop::estop_time, in_graph);
  dp.property("estop_time",estop_dp);

  auto estart_dp = get(&edge_prop::estart_time, in_graph);
  dp.property("estart_time",estart_dp);


  auto estate_dp = get(&edge_prop::estate, in_graph);
  dp.property("estate",estate_dp);

  auto ecde_comparator_delay_dp = get(&edge_prop::ecde_comparator_delay, in_graph);
  dp.property("ecde_comparator_delay", ecde_comparator_delay_dp);


  auto ecde_served_dp = get(&edge_prop::ecde_served, in_graph);
  dp.property("ecde_served", ecde_served_dp);



  //--------------------------

  try
  {
    write_graphviz_dp(std::cout, in_graph, dp, "vname");
  }
  catch(std::exception &err)
  {
    std::cerr << err.what() << endl;
    std::cerr << "wirte_graphviz failed : ";// << output_file << "\n";
    exit(0);
  }

  return;
}


  unsigned int 
Graph::getOpcodeStr(string op_code) 
{
  map<string, unsigned> mymap;
#define HANDLE_INST(N, OPCODE, CLASS) \
  mymap.insert({string(#OPCODE), N});


  unsigned int  FIRST_TERM_INST;
  unsigned int LAST_TERM_INST;

  unsigned int FIRST_BINARY_INST       ;              
  unsigned int LAST_BINARY_INST        ; 
  unsigned int FIRST_MEMORY_INST       ; 
  unsigned int LAST_MEMORY_INST        ; 
  unsigned int FIRST_CAST_INST         ; 
  unsigned int LAST_CAST_INST          ; 
  unsigned int FIRST_FUNCLETPAD_INST   ;
  unsigned int LAST_FUNCLETPAD_INST    ;
  unsigned int FIRST_OTHER_INST        ; 
  unsigned int LAST_OTHER_INST         ; 

#define FIRST_TERM_INST(num) \
  FIRST_TERM_INST = num;

#define LAST_TERM_INST(num) \
  LAST_TERM_INST = num;

#define     FIRST_BINARY_INST(num)            FIRST_BINARY_INST             = num;      
#define     LAST_BINARY_INST(num)             LAST_BINARY_INST              = num;      
#define     FIRST_MEMORY_INST(num)            FIRST_MEMORY_INST             = num;                
#define     LAST_MEMORY_INST(num)             LAST_MEMORY_INST              = num;       
#define     FIRST_CAST_INST(num)              FIRST_CAST_INST               = num;
#define     LAST_CAST_INST(num)               LAST_CAST_INST                = num;      
#define     FIRST_FUNCLETPAD_INST(num)        FIRST_FUNCLETPAD_INST         = num;   
#define     LAST_FUNCLETPAD_INST(num)         LAST_FUNCLETPAD_INST          = num;
#define     FIRST_OTHER_INST(num)             FIRST_OTHER_INST              = num;       
#define     LAST_OTHER_INST(num)              LAST_OTHER_INST               = num;        

  // #include "llvm/IR/Instruction.def"
  // llvm version 3.8 
#include "Instruction.def"

  // string cur_opcode = SExt;
  unsigned int latency;
  unsigned int opcode_no = mymap[op_code];
  if (FIRST_BINARY_INST     <= opcode_no && opcode_no <= LAST_BINARY_INST \
      && FIRST_CAST_INST  <= opcode_no && opcode_no <= LAST_CAST_INST     \
      && FIRST_TERM_INST  <= opcode_no && opcode_no <= LAST_TERM_INST     \
      && FIRST_OTHER_INST <= opcode_no && opcode_no <= LAST_OTHER_INST    \
     )
  {
    // ("Add"), 11);
    // ("FAdd"), 12);
    // ("Sub"), 13);
    // ("FSub"), 14);
    // ("Mul"), 15);
    // ("FMul"), 16);
    // ("UDiv"), 17);
    // ("SDiv"), 18);
    // ("FDiv"), 19);
    // ("URem"), 20);
    // ("SRem"), 21);
    // ("FRem"), 22);



    if(op_code.compare("FDiv") == 0 )
      return latency = 20;

    else if(op_code.compare("FAdd") == 0 )
      return latency = 5;

    else if(op_code.compare("FMul") == 0 )
      return latency = 7;

    else if(op_code.compare("Mul") == 0 )
      return latency = 4;

    else if (op_code.find("Div") != std::string::npos)
      // else if(op_code.compare("Div") == 0 )
      return latency = 15;
    else 
      return latency=1;
  }

  else if ( FIRST_FUNCLETPAD_INST <= opcode_no && opcode_no <= LAST_FUNCLETPAD_INST)
    return latency= 1;

  else if (FIRST_MEMORY_INST <= opcode_no && opcode_no <= LAST_MEMORY_INST)
  {
    if ( (op_code.compare("Load") == 0) ||
        // (op_code.compare("LoadInst") == 0) ||
        // (op_code.compare("StoreInst") == 0) ||
        (op_code.compare("Store") == 0) 
       )
    {
      if(op_code.compare("Load") == 0) 
        ++g_total_ldops;


      ++g_total_memops;
      return latency = MEMORYLATENCY;
    }
    else return latency = 1;
  }
  else 
    return latency = 1;

}

  int 
Graph::traverse_graph()
{
  //termination condition 
  // while(1)
  // {
  //--------------------------------------------------------
  // Termination Condition
  if( vertex_state_map[VNOT_READY].empty() 
      && vertex_state_map[VWAIT].empty() 
      && edge_state_map[ENOT_READY].empty() 
      && edge_state_map[EWAIT].empty() 
      && edge_state_map[ECDE].empty() 
      && edge_state_map[EMEM].empty() 
    )

  {

    g_total_comp_delay += update_fanin_cde_delay_VCOMPELTE();
    update_disamg_delay();


    D(print_states());
    // D(cout<<"TERMINATION CALLED : Global Clock: " <<global_clock<<endl);
    // D(cout<<"Memory:Bloom_filter_ld:Yes: "<<m_memory.bloom_yes_ld()<<endl);
    // D(cout<<"Memory:Bloom_filter_st:Yes: "<<m_memory.bloom_yes_st()<<endl);
    // D(cout<<"Memory:Bloom_filter_ld:No:  "<<m_memory.bloom_no_ld()<<endl);
    // D(cout<<"Memory:Bloom_filter_st:No:  "<<m_memory.bloom_no_st()<<endl);

    //------- test ---------------------

    g_total_termination_cycles += global_clock;
    g_total_bloom_yes_ld += m_memory.bloom_yes_ld();
    g_total_bloom_yes_st += m_memory.bloom_yes_st();
    g_total_bloom_no_ld +=  m_memory.bloom_no_ld();
    g_total_bloom_no_st +=  m_memory.bloom_no_st();
    g_total_bloom_fp_ld +=  m_memory.bloom_fp_ld();
    g_total_bloom_fp_st +=  m_memory.bloom_fp_st();
    g_total_ruby_requests += m_memory.m_ruby_requests;

    D(m_memory.print_mem_stats());
    g_rdy = false;
    m_prev_completion_cycles = global_clock;
    return global_clock;

  }


  // --------------------------------------------------------
  // Edge Functions
  // --------------------------------------------------------

  D(std::cout<< " update_states_ECDE_and_EMEM_queue: \n");
  update_states_ECDE_and_EMEM_queue();
  D(print_states());

  D(std::cout<< " update_states_ENOT_READY_queue: \n");
  update_states_ENOT_READY_queue();
  D(print_states());

  D(std::cout<< " update_states_EWAIT_queue: \n");
  update_states_EWAIT_queue();
  D(print_states());


  // --------------------------------------------------------
  // Vertex Functions
  // --------------------------------------------------------

  D(std::cout<< " update_states_VNOT_READY_queue: \n");
  update_states_VNOT_READY_queue();
  D(print_states());

  D(std::cout<< " update_states_VWAIT_queue: \n");
  update_states_VWAIT_queue();
  D(print_states());

  // --------------------------------------------------------


  //--------------------------------------------------------
  // Force Stop 

  if(global_clock >= /*g_start_clock +*/  FORCE_STOP)
  {

    g_rdy = false;
    m_memory.print_mem_stats();
    print_states();
    cout<<"FORCE STOP AFTER "<<FORCE_STOP <<"cycles:\n";
    cerr<<"FORCE STOP AFTER "<<FORCE_STOP << "cycles:\n";
    printGraph();
    assert(0 && "FORCE STOP \n");
    return -1;
  }
  // }

  return 0;
}
  int 
Graph::run_a_cycle()
{

  ++global_clock;
  if(g_en && g_rdy)
  {
    // cout<<"DAFAXC TRAVERSING GRAPH:"<<endl;
    return this->traverse_graph();
  }
  return 0;
}


//-------------------------------------------------------------------------------------------------------------------------------
// Updating Vertex States
//-------------------------------------------------------------------------------------------------------------------------------

  void
Graph:: update_states_VNOT_READY_queue()
{

  //Transition 
  //VNR -> VC
  //VNR -> VW

  // Iterate over vertex in wait states
  // get values in array
   uint64_t issue_width = g_issue_width;
  vector<VERTEX> vector_state_vnot_ready( vertex_state_map[VNOT_READY].begin(), vertex_state_map[VNOT_READY].end() );
  for (auto &IT: vector_state_vnot_ready)
  {
    --issue_width;
    if(issue_width < 1) {

      // cout<<"Done Issuing " <<g_issue_width<<" instructions out of "<< vector_state_vnot_ready.size()  <<"clock: "<<global_clock <<endl;
      D(cout<<"Done Issuing " <<g_issue_width<<" instructions out of "<< vector_state_vnot_ready.size()  <<"clock: "<<global_clock <<endl);
      break;

    }

    //update_fanin_cde_delay
    update_fanin_cde_delay_VNR_VWAIT(IT);

    auto vstate = new_vertex_state(IT);
    // is_vertex_ready ?
    // If all in_edges are either in CDE or EComplete then vertex is_vertex_ready == 2
    //    mv V: NR -> VC
    if( vstate == VCOMPLETE)
    {
      // Update vertex states
      update_vertex_state(VNOT_READY,VCOMPLETE,IT);
    }

    // If some but not all in_edges are either in CDE or EComplete then vertex is_vertex_ready == 1
    else if(vstate  == VWAIT)
    {
      // Update vertex states
      update_vertex_state(VNOT_READY,VWAIT,IT);
    }

  }
}


  void
Graph:: update_states_VWAIT_queue()
{
  //Transition 
  // VW -> VC

  // Iterate over vertex in wait states
  // get values in array
  vector<VERTEX> vector_state_vwait( vertex_state_map[VWAIT].begin(), vertex_state_map[VWAIT].end() );
  for (auto &IT: vector_state_vwait)
  {

    //update_fanin_cde_delay
    update_fanin_cde_delay_VNR_VWAIT(IT);
    
    if(new_vertex_state(IT) == VCOMPLETE)
    {
      // Update vertex states
      update_vertex_state(VWAIT,VCOMPLETE,IT);

    }
  }

}

//-------------------------------------------------------------------------------------------------------------------------------
// Auxillary VERTEX Functions
//-------------------------------------------------------------------------------------------------------------------------------

//DO IT AFTER ALL EDGE STATES ARE COMPLETED FOR THE CLOCK
//Read all in_edges from VWAIT or VNOT_READY_Q
// if(!CDE edges not served)
// then mark the first CDE as served and add a delay of +1 for others
// 
  void
Graph:: update_fanin_cde_delay_VNR_VWAIT(VERTEX _vertex)
{

  auto EI = in_edges(_vertex,in_graph);
  auto is_comparator_free = false;

  auto nodeid = get(&vertex_prop::vname,in_graph,_vertex);
  auto num_cde = 0;

  // cout<<"-S-----------------------------------------------\n";
  for (auto ES = EI.first, EE = EI.second; ES != EE; ES++) 
  {
    auto ecde_served = get(&edge_prop::ecde_served,in_graph,*ES);//bool 
    auto ecde_comparator_delay = get(&edge_prop::ecde_comparator_delay,in_graph,*ES); //uint32_t 
    auto cur_estate = get(&edge_prop::estate,in_graph,*ES); //ESTATE


    if(cur_estate == ECDE)
    {

      auto source_nodeid = get(&vertex_prop::vname,in_graph,source(*ES,in_graph));
      num_cde++;
      if(!ecde_served)
      {
        if(!is_comparator_free)
        {
          is_comparator_free = true;
          put(&edge_prop::ecde_served,in_graph,*ES,true); 
          //same as old simulator so no delay added
          D(cout<<"serving: src_nodeid: "<<source_nodeid <<
            " ecde_comparator_delay: "<<ecde_comparator_delay<<"\n");

        }
        else {
          put(&edge_prop::ecde_comparator_delay,in_graph,*ES,ecde_comparator_delay+1); 
          cout<<"not_served: src_nodeid: "<<source_nodeid <<
            " ecde_comparator_delay: "<<ecde_comparator_delay+1<<"\n";
        }
      } 
      else {
          D(cout<<"served: src_nodeid: "<<source_nodeid <<
            " ecde_comparator_delay: "<<ecde_comparator_delay<<"\n");
      }
    }
  }

  D(if(num_cde != 0)
  {
    cout<<"nodeid: "<<nodeid <<" num_cde: "<<num_cde <<" \n";
    cout<<"-E-----------------------------------------------\n";
  })
}

uint64_t
Graph:: update_fanin_cde_delay_VCOMPELTE()
{
  //update fanin_cde delay in Vertices
  vector<VERTEX> vector_state_vcomplete( vertex_state_map[VCOMPLETE].begin(), 
      vertex_state_map[VCOMPLETE].end() );
  uint64_t  g_all_vertices_comp_delay = 0;
  for (auto &IT: vector_state_vcomplete)
  {


    uint64_t  g_comp_delay = 0;
    auto nodeid = get(&vertex_prop::vname,in_graph,IT);
    auto EI = in_edges( IT,in_graph);

    for (auto ES = EI.first, EE = EI.second; ES != EE; ES++) 
    {
      auto comp_delay = get(&edge_prop::ecde_comparator_delay,in_graph,*ES); 
      auto source_nodeid = get(&vertex_prop::vname,in_graph,source(*ES,in_graph));
      g_comp_delay = std::max(g_comp_delay, comp_delay);
      put(&vertex_prop::vcde_comparator_delay,in_graph,IT,g_comp_delay);
      D(cout<<"src:NODEID: "<<source_nodeid <<
        " tgt:NODEID: "<<nodeid <<" COMPARATOR DELAY: "<< g_comp_delay<<"\n";
       )

    }

    g_all_vertices_comp_delay += g_comp_delay;

  }

  return g_all_vertices_comp_delay;
}



// Checks for all the in_edges, if all in_Edges are in ECDE or ECOMPLETE 
// then the vertex is ready
  VSTATE
Graph::new_vertex_state( VERTEX _vertex)
{
  bool is_vertex_ready = true;
  bool is_vertex_wait = false;

  auto EI = in_edges(_vertex,in_graph);
  for (auto ES = EI.first, EE = EI.second; ES != EE; ES++) 
  {
    // if( get(&edge_prop::estate,*ES,in_graph) != ECOMPLETE)
    // auto S = source(*ES, BG);
    auto cur_estate = get(&edge_prop::estate,in_graph,*ES); 
    if( !(cur_estate == ECOMPLETE || cur_estate == ECDE) )
    {
      is_vertex_ready = false;
      break;
    } 

  }

  //Atleaset one is CDE or ECOMP  
  for (auto ES = EI.first, EE = EI.second; ES != EE; ES++) 
  {
    auto cur_estate = get(&edge_prop::estate,in_graph,*ES); 
    if( (cur_estate == ECOMPLETE || cur_estate == ECDE) )
    {
      is_vertex_wait = true;
      break;
    }
  }




  // IF ALL in_edges are ECDE or ECOMP, then VNR -> VCOMP
  // ELSE IF ATLEAST one in_edge is ECDE or ECOMP, then VNR-> VW
  // ELSE DO NOTHING
  if(is_vertex_ready)
    return VCOMPLETE;
  else if(is_vertex_wait)
    return VWAIT;
  else 
    return VNOT_READY;
}


  void
Graph::update_vertex_state(VSTATE _From_state,VSTATE  _To_state,VERTEX  _Vertex)
{

  //Update vertex states
  put(&vertex_prop::vstate,in_graph,_Vertex,_To_state);

  //Update vertex state maps
  //move vertex from wait to ready queue
  //delete entry from VWAIT -- without deleting the object
  vertex_state_map[_To_state].push_back(_Vertex);
  vertex_state_map[_From_state].erase(find(vertex_state_map[_From_state].begin(), 
        vertex_state_map[_From_state].end(),_Vertex));

}

  void
Graph::update_out_vertex_prop(std::string _string, EDGE  _EI)
{

  auto target_vertex = target(_EI,in_graph);

  //Update vertex states
  if (_string.compare("vCDEcomp") == 0)
    put(&vertex_prop::vCDEcomp,in_graph,target_vertex,global_clock);
  else if (_string.compare("vOPcomp") == 0)
    put(&vertex_prop::vOPcomp,in_graph,target_vertex,global_clock);
  else if (_string.compare("vEMEM") == 0) //FOR NACHOS-SW SINCE ECDE IS CONVERTED TO EMEM
  {

    auto tgt_opcode = get(&vertex_prop::vopcode,in_graph,target_vertex);
    auto tgt_ir = get(&vertex_prop::vir,in_graph,target_vertex);
    //DO NOT calculate  LIVE OUTS 
    if ( (tgt_opcode == "Load" || tgt_opcode =="Store") 
        && (tgt_ir.find("!LO") == std::string::npos))
    {

      cout<<"CDE_clock_comp: "<<global_clock <<"\t";
      put(&vertex_prop::vCDEcomp,in_graph,target_vertex,global_clock);
      //NOTE : WE WANTED TO ADD vCDEcomp FOR EMEM CASES WHEN IT IS ACTUALLY A CDE 
      //HOWEVER, MADE EMEM FOR SERIALIZATION
    }

  }
  else
    assert(0 && "Error! update_out_vertex_prop : wrong string input");

}
//-------------------------------------------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------------------------------------------



//-------------------------------------------------------------------------------------------------------------------------------
// Updating Edge States
//-------------------------------------------------------------------------------------------------------------------------------

  void
Graph::update_states_ENOT_READY_queue()
{


  vector<EDGE> vector_state_enot_ready( edge_state_map[ENOT_READY].begin(), edge_state_map[ENOT_READY].end() );
  for (auto &IT: vector_state_enot_ready)
  {
    auto estate = new_edge_state(IT);

    if(estate == ECDE)
    {
      // Update edge states
      update_edge_state(ENOT_READY,ECDE,IT);
    }

    else if(estate  == EMEM)
    {
      // Update edge states
      update_edge_state(ENOT_READY,EMEM,IT);
    }

    else if(estate  == EWAIT)
    {
      // Update edge states
      update_edge_state(ENOT_READY,EWAIT,IT);
    }

  }
}




  void
Graph::update_states_ECDE_and_EMEM_queue()
{

  //-------------------------------------------------------------------------------------------
  // Response from Memory 
  //insert ruby here --recv resp 
  auto nodeid_list = m_memory.recv_resp(m_prev_completion_cycles);

  D(cout<<"nodeid_list size: "<<nodeid_list.size() << endl);

  for( auto &n : nodeid_list)
  {
    D(cout<<"graph: get resp from memory nodeid: "<<n<<endl);
    D(cerr<<"graph: get resp from memory nodeid: "<<n
        <<" g_clk: "<<global_clock 
        <<endl);
  }



  //-------------------------------------------------------------------------------------------
  // ECDE EDGES COMING FROM MEMORY
  vector<EDGE> vector_state_ecde( edge_state_map[ECDE].begin(), edge_state_map[ECDE].end() );

  // Remember target vertex of CDE edges have already moved on 
  for (auto &EI: vector_state_ecde)
  {

    bool is_edge_cde = false;

    auto source_vertex = source(EI,in_graph);
    auto src_nodeid = get(&vertex_prop::vname,in_graph,source_vertex);

    for( auto &n : nodeid_list)
    {
      if(src_nodeid == n)
      {
        is_edge_cde = true;
        break;
      }
    }

    if(is_edge_cde)
    {
      D(cout<< "edge is cde: nodeid: "
          << src_nodeid 
          <<"\n");
      update_edge_state( ECDE, ECOMPLETE, EI) ;

      //TODO Update target Vertex vCDEcomp: Cycles
      update_out_vertex_prop("vCDEcomp",EI);
    }
    else 
      update_out_vertex_prop("vEMEM",EI);

  }

  //-------------------------------------------------------------------------------------------
  // EMEM EDGES COMING FROM MEMORY
  vector<EDGE> vector_state_emem( edge_state_map[EMEM].begin(), edge_state_map[EMEM].end() );

  // Similarly for EWAIT memory edges
  for (auto &EI: vector_state_emem)
  {

    bool is_edge_match = false;

    auto source_vertex = source(EI,in_graph);
    auto src_nodeid = get(&vertex_prop::vname,in_graph,source_vertex);

    for( auto &n : nodeid_list)
    {
      if(src_nodeid == n)
      {
        is_edge_match = true;
        break;
      }
    }

    if(is_edge_match)
    {

      D(cout<<"edge is emem \n");

      update_edge_state( EMEM, ECOMPLETE, EI) ;
    }

  }

  //-------------------------------------------------------------------------------------------

}

  void
Graph::update_states_EWAIT_queue()
{


  //-----------------------------------
  // Iterate over Edges in wait states
  // If End time == global_clock: 
  //    mv E: W->C :  
  //    mv targetV NR-> W: 
  //    update next_event_edge
  //-----------------------------------

  vector<EDGE> vector_state_ewait( edge_state_map[EWAIT].begin(), edge_state_map[EWAIT].end() );

  for (auto &EI: vector_state_ewait)
  {
    auto estop_time = get(&edge_prop::estop_time,in_graph,EI);



    if( estop_time <= global_clock) 
    {


      //TODO Update target Vertex vOPcomp: Cycles
      update_out_vertex_prop("vOPcomp",EI);

      //Update edge state from EWAIT to ECOMP
      update_edge_state(EWAIT, ECOMPLETE, EI);

    }

  }
}


//-------------------------------------------------------------------------------------------------------------------------------
// Auxillary Edge Functions 
//-------------------------------------------------------------------------------------------------------------------------------

  ESTATE
Graph::new_edge_state( EDGE IT)
{

  auto source_vertex = source(IT,in_graph);
  auto src_opcode = get(&vertex_prop::vopcode,in_graph,source_vertex);
  auto src_vstate = get(&vertex_prop::vstate,in_graph,source_vertex);
  auto src_ir = get(&vertex_prop::vir,in_graph,source_vertex);
  auto src_vissue = get(&vertex_prop::vissue,in_graph,source_vertex);
  auto is_src_mem = false;

  auto target_vertex = target(IT,in_graph);
  auto tgt_opcode = get(&vertex_prop::vopcode,in_graph,target_vertex);
  auto tgt_ir = get(&vertex_prop::vir,in_graph,target_vertex);
  auto is_tgt_mem = false;

  auto ecolor = get(&edge_prop::ecolor,in_graph,IT);
  //DO NOT SEND LIVE OUTS TO MEMORY
  if ( (src_opcode == "Load" || src_opcode =="Store") 
      && (src_ir.find("!LO") == std::string::npos))
    is_src_mem = true;

  //DO NOT SEND LIVE OUTS TO MEMORY
  if ( (tgt_opcode == "Load" || tgt_opcode =="Store") 
      && (tgt_ir.find("!LO") == std::string::npos))
    is_tgt_mem = true;

  //-- FOR MEMORY OPERATION 
  // Need to send address to memory and set estop, estart etc

  if(is_src_mem && src_vstate == VCOMPLETE &&  !src_vissue)
  {
    put(&vertex_prop::vissue,in_graph,source_vertex,true);
    m_memory.send_req(g_iter,get(&vertex_prop::vname,in_graph,source_vertex));
    D(cout<<"Sending to Memory nodeid: "<<get(&vertex_prop::vname,in_graph,source_vertex) <<endl);
    D(cerr<<"Sending to Memory nodeid: "<<get(&vertex_prop::vname,in_graph,source_vertex) 
        <<" g_clk: "<<global_clock
        <<endl);
    ++g_mem_req_count;
  }


  //NOTE: fixing a bug -> there could be a data dependency from src to tgt node
  //in that case the src has to wait to finish before the tgt can even start
  //thus, no matter if it is software approach or LSQ or nachos 
  //dest has to stall and should be go to ESTATE=EMEM
  //As of now, MDEs are found by the color of the edges
  //and existing scripts use color= orange and red to determine MDEs
 
  if(is_src_mem && is_tgt_mem && src_vstate == VCOMPLETE && m_is_LSQ)
  {
    if(ecolor=="orange"|| ecolor=="red")
      return ECDE;
    //for true data dependency edge
    else  
      return EMEM;
  }
  else if(is_src_mem && is_tgt_mem && src_vstate == VCOMPLETE && !m_is_LSQ)
    return EMEM;

  else if (is_src_mem && !is_tgt_mem  && src_vstate == VCOMPLETE)
    return EMEM;
  else if ( src_vstate == VCOMPLETE)
    return EWAIT;
  else 
    return ENOT_READY;


}



  void
Graph::update_edge_state(ESTATE _From_state, ESTATE _To_state, EDGE EI)
{


  auto elatency = get(&edge_prop::elatency,in_graph,EI);

  if(_To_state != ECOMPLETE)
  {
    put(&edge_prop::estart_time,in_graph,EI,global_clock); 
    put(&edge_prop::estop_time,in_graph,EI,global_clock+elatency); 
  }
  else 
    put(&edge_prop::estop_time,in_graph,EI,global_clock); 


  put(&edge_prop::estate,in_graph,EI,_To_state); 
  //Update states
  edge_state_map[_To_state].push_back(EI);
  //update queues
  edge_state_map[_From_state].erase(find(edge_state_map[_From_state].begin(),
        edge_state_map[_From_state].end(),EI));

}

//-------------------------------------------------------------------------------------------------------------------------------
// Generic Functions 
//-------------------------------------------------------------------------------------------------------------------------------

  void 
Graph::print_states()
{

  cout<<"-----------------------------------------\n";
  cout<<"VERTEX STATE MAP:\n";
  cout<<"-----------------------------------------\n";

  cout<<"VNOT_READY:\n";
  for( auto &VI: vertex_state_map[VNOT_READY])
  {
    cout<<get(&vertex_prop::vname,in_graph,VI) <<" ";
  }

  cout<<"\n";
  cout<<"-----------------------------------------\n";

  cout<<"VWAIT:\n";
  for( auto &VI: vertex_state_map[VWAIT])
  {
    cout<<get(&vertex_prop::vname,in_graph,VI) <<" ";
  }
  cout<<"\n";
  cout<<"-----------------------------------------\n";

  cout<<"VCOMPLETE:\n";
  for( auto &VI: vertex_state_map[VCOMPLETE])
  {
    cout<<get(&vertex_prop::vname,in_graph,VI) <<" ";
  }
  cout<<"\n";
  cout<<"-----------------------------------------\n";



  cout<<"-----------------------------------------\n";
  cout<<"EDGE STATE MAP:\n";
  cout<<"-----------------------------------------\n";

  cout<<"ENOT_READY:\n";
  for( auto &EI: edge_state_map[ENOT_READY])
  {

    auto target_vertex = target(EI, in_graph);
    auto source_vertex = source(EI, in_graph);

    cout<<"E:"<< get(&vertex_prop::vname,in_graph,source_vertex)  
      << "->" 
      << get(&vertex_prop::vname,in_graph,target_vertex) 
      <<"\n";
  }
  cout<<"\n";
  cout<<"----------------------------------\n";


  cout<<"EWAIT:\n";
  for( auto &EI: edge_state_map[EWAIT])
  {

    auto target_vertex = target(EI, in_graph);
    auto source_vertex = source(EI, in_graph);

    cout<<"E:"<< get(&vertex_prop::vname,in_graph,source_vertex)  
      << "->" 
      << get(&vertex_prop::vname,in_graph,target_vertex) 
      <<"\n";
  }
  cout<<"\n";
  cout<<"----------------------------------\n";


  cout<<"ECOMPLETE:\n";
  for( auto &EI: edge_state_map[ECOMPLETE])
  {

    auto target_vertex = target(EI, in_graph);
    auto source_vertex = source(EI, in_graph);

    cout<<"E:"<< get(&vertex_prop::vname,in_graph,source_vertex)  
      << "->" 
      << get(&vertex_prop::vname,in_graph,target_vertex) 
      <<"\n";
  }
  cout<<"\n";
  cout<<"----------------------------------\n";

  cout<<"ECDE:\n";
  for( auto &EI: edge_state_map[ECDE])
  {

    auto target_vertex = target(EI, in_graph);
    auto source_vertex = source(EI, in_graph);

    cout<<"E:"<< get(&vertex_prop::vname,in_graph,source_vertex)  
      << "->" 
      << get(&vertex_prop::vname,in_graph,target_vertex) 
      <<"\n";
  }
  cout<<"\n";
  cout<<"----------------------------------\n";


  cout<<"EMEM:\n";
  for( auto &EI: edge_state_map[EMEM])
  {

    auto target_vertex = target(EI, in_graph);
    auto source_vertex = source(EI, in_graph);

    cout<<"E:"<< get(&vertex_prop::vname,in_graph,source_vertex)  
      << "->" 
      << get(&vertex_prop::vname,in_graph,target_vertex) 
      <<"\n";
  }
  cout<<"\n";
  cout<<"----------------------------------\n";


}




  void
Graph::print_memmap()
{
  m_memory.print_global_memaddr_map();
}
//----------------------------------
  void
Graph::read_memcsv(std::ifstream& file)
{
  boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
  inbuf.push(boost::iostreams::gzip_decompressor());
  inbuf.push(file);

  //Convert streambuf to istream
  std::istream instream(&inbuf);


  regex mem_parse("^([[:digit:]]+),([^,]*),([[:digit:]]+),([[:digit:]]+).*$");
  smatch match;

  std::string line;
  while(std::getline(instream, line) && m_memory.global_memaddr_map_size() < ITER_COUNT_MEM ){
    regex_search(line, match, mem_parse);

    uint64_t iter;
    uint64_t nodeid;
    string   opcode;
    bool isWrite =false;
    uint64_t addr;

    iter = stoi(match[1]);
    opcode = match[2];
    nodeid = stoi(match[3]);
    if(opcode == "Store")
      isWrite = true;
    else
      isWrite = false;
    addr = stoll(match[4]);

    //FIXME
    m_memory.fill_global_memaddr_map( iter, nodeid , MemValue {isWrite,addr});
  }

  cout<<"read_memcsv is complete: Loaded Memory iteration table \n";
}

//Functions for getting memory disambiguation delay
  void
Graph::update_disamg_delay()
{

  std::pair<VERTEX_ITER, VERTEX_ITER> v;

  for (v = vertices(in_graph); v.first != v.second; ++v.first)
  {

    auto vname = get(&vertex_prop::vname,in_graph,*v.first)  ;
    auto vCDEcomp = get(&vertex_prop::vCDEcomp,in_graph,*v.first)  ;
    auto vOPcomp = get(&vertex_prop::vOPcomp,in_graph,*v.first)  ;

    uint64_t disamg_delay=0;

    // We want to exclude cases which do not have CDE's
    if(vCDEcomp ==0 && vOPcomp !=0)
      disamg_delay =0;
    else
    {
      //NOTE: there can be cases where disamg_delay is negative 
      //It might be interesting to see that
      disamg_delay = vCDEcomp - vOPcomp;
    }

    //Increment Every iteration
    g_disamg_delay_map[vname] += disamg_delay;
  }
}

  void 
Graph::print_disamg_delay()
{
  uint64_t cum_pos_delay=0;
  uint64_t cum_neg_delay=0;

  uint64_t cum_pos_ops=0;
  uint64_t cum_neg_ops=0;

  cout<<"Printing Average Disambiguation Delay"<<endl;
  for (auto &val : g_disamg_delay_map)
  {
    if(val.second != 0)
      cout<<" nodeid: "<<val.first <<" cumm_disamg_delay: "<< val.second;

    if(val.second >0)
    {
      cum_pos_delay += val.second; 
      cum_pos_ops +=1;
    }
    else 
    {
      cum_neg_delay += val.second;
      cum_neg_ops +=1;
    }
  }
  cout<<endl;

  cout<<"Cumm. mem_disamb_delay positive: "<< cum_pos_delay<<endl;
  cout<<"Cumm. mem_disamb_delay negative: "<< cum_neg_delay<<endl;
  cout<<"Cumm. mem_disamb_pos_ops: "<< cum_pos_ops<<endl;
  cout<<"Cumm. mem_disamb_neg_ops: "<< cum_neg_ops<<endl;
  cout<<"Cumm. avg_pos_disamg_delay_per_op: "<< (float)cum_pos_delay/cum_pos_ops;

  cout<<endl;
}
