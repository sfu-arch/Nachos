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
#define FORCE_STOP 15000

/* Constructor */
Graph::Graph(uint64_t n, std::string ldstfile_path, string input_filestring,bool _is_stall_on_bloom , std::string _bloom_type, bool ishash2 , uint64_t mem_lat, int _mem_req_stack ):orig_graph(0) ,in_graph(0), graph_limit(n), m_stall_on_bloom(_is_stall_on_bloom)
{


  g_iter = 0;
  g_input_filestring = input_filestring;
  g_mem_req_stack = _mem_req_stack;
  cout<<"MSHR_SIZE: "<<_mem_req_stack<<endl;
  m_memory = new Memory(m_stall_on_bloom, _bloom_type,ishash2, mem_lat, g_mem_req_stack);
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

  g_total_termination_cycles =0;
  g_total_bloom_yes_ld =0;
  g_total_bloom_yes_st =0;
  g_total_bloom_no_ld =0;
  g_total_bloom_no_st =0;
  g_total_bloom_fp=0;
  g_total_bloom_stall_cycles=0;

  g_total_memops=0;
  g_total_ldops=0;
  g_total_LiveOut =0;

  readGraph();
  //Create Global LSQ nodeids = g_LSQ -> m_memory
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
  cout<<"Iter. # of Vertices: "<< num_vertices(in_graph)<<endl;
  cout<<"Iter. # of Edges:  "<< num_edges(in_graph)<<endl;
  cout<<"Iter. # of MemOps (excluding LiveOuts): "<< g_total_memops - g_total_LiveOut<<endl;
  cout<<"Iter. # of Store LiveOuts: "<< g_total_LiveOut<<endl;
  cout<<"Iter. # of Total Ld Ops: "<< g_total_ldops<<endl;

  cout<<"Cumm. Port Busy Cycles:  "<<m_memory->m_port_unavailable<<endl;
  cout<<"Cumm. Total Memory Requests: "<<g_mem_req_count<<endl;
  cout<<"Cumm. Total m_mshr_full_cycles:  "<<m_memory->m_mshr_full_cycles<<endl;

  cout<<"Cumm. TERMINATION CALLED :Total axc termination cycles: " <<g_total_termination_cycles<<endl;
  cout<<"Cumm. Memory:Bloom_filter_ld:Yes: "<< g_total_bloom_yes_ld <<endl;
  cout<<"Cumm. Memory:Bloom_filter_st:Yes: "<< g_total_bloom_yes_st <<endl;
  cout<<"Cumm. Memory:Bloom_filter_ld:NO: "<< g_total_bloom_no_ld <<endl;
  cout<<"Cumm. Memory:Bloom_filter_st:NO: "<< g_total_bloom_no_st <<endl;
  cout<<"Cumm. Memory:Bloom False Positive: "<<g_total_bloom_fp <<endl;
  cout<<"Cumm. Memory:Bloom False Positive percent : "<<
    g_total_bloom_fp/ (float)(g_total_bloom_yes_ld + g_total_bloom_yes_st )*100 <<endl;
  cout<<"Cumm. Memory:Bloom Total stall cycles : "<< g_total_bloom_stall_cycles<<endl;
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

  //Edge dp
  auto ecolor_dp = get(&edge_prop::ecolor, orig_graph);
  dp.property("color",ecolor_dp);

  auto vlatency_dp = get(&vertex_prop::vlatency, orig_graph);
  dp.property("vlatency", vlatency_dp);

  auto vstate_dp = get(&vertex_prop::vstate, orig_graph);
  dp.property("vstate", vstate_dp);

  auto vissue_dp = get(&vertex_prop::vissue, orig_graph);
  dp.property("vissue", vissue_dp);


  auto elatency_dp = get(&edge_prop::elatency, orig_graph);
  dp.property("elatency",elatency_dp);

  auto estate_dp = get(&edge_prop::estate, orig_graph);
  dp.property("estate",estate_dp);


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


  std::pair<VERTEX_ITER, VERTEX_ITER> v;

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
      vertex_state_map[VREADY].push_back(*vp.first);
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
  m_memory->initialize(m_stall_on_bloom);
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
  //----------------------------------------------------

  auto vissue_dp = get(&vertex_prop::vissue, in_graph);
  dp.property("vissue", vissue_dp);



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
  if( vertex_state_map[VREADY].empty() && vertex_state_map[VWAIT].empty() && 
      edge_state_map[EWAIT].empty() && edge_state_map[ENOT_READY].empty() 
      && edge_state_map[ECDE].empty() 
    )

  {
    assert((m_memory->mem_req_stack == g_mem_req_stack /*MEM_MSHR_SIZE*/) && "memory request stack is not zero"); 
    D(print_states());
    D(cout<<"TERMINATION CALLED : Global Clock: " <<global_clock<<endl);
    D(cout<<"Memory:Bloom_filter_ld:Yes: "<<m_memory->bloom_yes_ld()<<endl);
    D(cout<<"Memory:Bloom_filter_st:Yes: "<<m_memory->bloom_yes_st()<<endl);
    D(cout<<"Memory:Bloom_filter_ld:No:  "<<m_memory->bloom_no_ld()<<endl);
    D(cout<<"Memory:Bloom_filter_st:No:  "<<m_memory->bloom_no_st()<<endl);

    //------- test ---------------------

    g_total_termination_cycles += global_clock;
    g_total_bloom_yes_ld += m_memory->bloom_yes_ld();
    g_total_bloom_yes_st += m_memory->bloom_yes_st();
    g_total_bloom_no_ld += m_memory->bloom_no_ld();
    g_total_bloom_no_st += m_memory->bloom_no_st();
    g_total_bloom_fp += m_memory->bloom_fp();
    g_total_bloom_stall_cycles += m_memory->bloom_stall_cycles();

    D(m_memory->print_mem_stats());
    g_rdy = false;
    m_prev_completion_cycles = global_clock;
    return global_clock;

  }

  // update_mlp_stats_1();

  // Iterate over all vertices and all Edges --
  // --------------------------------------------------------
  D(std::cout<< "Calling update_states_VNOT_READY_queue: \n");
  update_states_VNOT_READY_queue();
  D(print_states());

  // --------------------------------------------------------
  D(std::cout<< "Calling update_states_VWAIT_queue: \n");
  update_states_VWAIT_queue();
  D(print_states());

  // --------------------------------------------------------
  // Iterate over vertex in READY states
  D(cout<< "Calling update_states_VREADY_queue: \n");
  update_states_VREADY_queue();
  D(print_states());

  // --------------------------------------------------------
  D(cout<<"Calling update_states_VCOMPLETE_queue: \n");
  update_states_VCOMPLETE_queue();
  D(print_states());


  // Iterate over Edges in EWAIT states from Non Memory operations
  D(cout<< "Calling update_states_EWAIT_non_memory_queue: \n");
  update_states_EWAIT_non_memory_queue();
  D(print_states());

  // --------------------------------------------------------
  // Iterate over Edges in EWAIT states from Memory operations
  D(cout<< "Calling update_states_EWAIT_memory_queue: \n");
  update_states_EWAIT_memory_queue();
  D(print_states());


  
  //--------------------------------------------------------
  // Advance Clock

  // if(global_clock< min(next_event_time_vertex,next_event_time_edge))
  //   global_clock += min(next_event_time_vertex, next_event_time_edge);
  // else
  // cout<<"##   --- Global CLOCK:"<< global_clock++ << endl;

  // if(mlpfile.is_open())
  //   mlpfile<<mlp<<"\n";
  // else
  // {
  //   g_rdy = false;
  //   cerr<<"Error opening file \n";
  //
  //   assert(0 && "dafaxc: graph.cpp Error! File couldn't open");
  //   return -1;
  // }
  // global_clock++;

  //--------------------------------------------------------
  // Force Stop 

  if(global_clock >= /*g_start_clock +*/  FORCE_STOP)
  {

    g_rdy = false;
    m_memory->print_mem_stats();
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




  void
Graph:: update_states_VNOT_READY_queue()
{

  // Iterate over vertex in wait states
  // get values in array
  vector<VERTEX> vector_state_vnot_ready( vertex_state_map[VNOT_READY].begin(), vertex_state_map[VNOT_READY].end() );
  for (auto &IT: vector_state_vnot_ready)
  {


    // is_vertex_ready ?
    // If all in_edges are either in CDE or EComplete then vertex is ready
    //    mv V: NR -> W
    //    mv out_edges: NR -> W or NR-> CDE  
    //    add start time and end time
    if(is_vertex_ready(IT))
    {

      // Update vertex states
      update_vertex_states(VNOT_READY,VWAIT,IT);

      // Update Edge States
      // OUt Edges mv : ENOT_READY -> EWAIT
      // OR 
      // OUt Edges mv : ENOT_READY -> ECDE 
      // add start and end time

      // Returns true if the edge is a CDE
      update_outedge_states_NR_to_CDE_or_EWAIT(ENOT_READY, EWAIT, IT);


      //Update next_event_vertex
      // next_event_time_vertex=global_clock+1;

    }
  }

}



  void
Graph:: update_states_VWAIT_queue()
{

  // Iterate over vertex in wait states
  // get values in array
  vector<VERTEX> vector_state_vwait( vertex_state_map[VWAIT].begin(), vertex_state_map[VWAIT].end() );
  for (auto &IT: vector_state_vwait)
  {


    // is_vertex_ready ?
    // If all in_edges are either in CDE or EComplete then vertex is ready
    //    mv V: W -> R
    //    mv out_edges: NR -> W or NR-> CDE  
    //    add start time and end time
    if(is_vertex_ready(IT))
    {

      // Update vertex states
      update_vertex_states(VWAIT,VREADY,IT);

      // Update Edge States
      // OUt Edges mv : ENOT_READY -> EWAIT
      // OR 
      // OUt Edges mv : ENOT_READY -> ECDE 
      // add start and end time

      // Returns true if the edge is a CDE
      update_outedge_states_NR_to_CDE_or_EWAIT(ENOT_READY, EWAIT, IT);


      //Update next_event_vertex
      // next_event_time_vertex=global_clock+1;

    }
  }

}


  void
Graph::update_states_VREADY_queue()
{
  // Iterate over vertex in ready states
  // get values in array
  vector<VERTEX> vector_state_vready( vertex_state_map[VREADY].begin(), vertex_state_map[VREADY].end() );
  for (auto &IT: vector_state_vready)
  {

    // mv out_edges: NR -> W: Update State, queues,  add start time and end time
    // mv V: R ->C
    update_outedge_states_NR_to_CDE_or_EWAIT(ENOT_READY, EWAIT, IT);
    //Update Vertex State
    update_vertex_states(VREADY, VCOMPLETE, IT);

  }
}

  void
Graph::update_states_EWAIT_memory_queue()
{

  //-------------------------------------------------------------------------------------------
  // Response from Memory 
  //insert ruby here --recv resp 
  auto nodeid_list = m_memory->recv_resp(m_prev_completion_cycles);
  for( auto &n : nodeid_list)
  {
    D(cerr<<"graph: get resp from memory nodeid: "<<n<<endl);
  }



  //-------------------------------------------------------------------------------------------
  // ECDE EDGES COMING FROM MEMORY
  vector<EDGE> vector_state_ecde( edge_state_map[ECDE].begin(), edge_state_map[ECDE].end() );

  bool is_edge_cde = false;
  // Remember target vertex of CDE edges have already moved on 
  for (auto &EI: vector_state_ecde)
  {

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

      update_edge_state( ECDE, ECOMPLETE, EI) ;
    }

  }

  //-------------------------------------------------------------------------------------------
  // EWAIT EDGES COMING FROM MEMORY
  vector<EDGE> vector_state_ewait( edge_state_map[EWAIT].begin(), edge_state_map[EWAIT].end() );

  // Similarly for EWAIT memory edges
  for (auto &EI: vector_state_ewait)
  {

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

      update_edge_state( EWAIT, ECOMPLETE, EI) ;
    }

  }

  //-------------------------------------------------------------------------------------------

}

  void
Graph::update_states_EWAIT_non_memory_queue()
{


  //-----------------------------------
  // Iterate over Edges in wait states
  // If End time == global_clock: 
  //    mv E: W->C :  
  //    mv targetV NR-> W: 
  //    update next_event_edge
  //-----------------------------------
  vector<VERTEX> mv_target_vec_NR_to_W;
  vector<EDGE> vector_state_ewait( edge_state_map[EWAIT].begin(), edge_state_map[EWAIT].end() );

  for (auto &EI: vector_state_ewait)
  {
    bool is_edge_complete = false;
    auto estop_time = get(&edge_prop::estop_time,in_graph,EI);
    auto source_vertex = source(EI,in_graph);
    auto src_nodeid = get(&vertex_prop::vname,in_graph,source_vertex);
    auto src_opcode = get(&vertex_prop::vopcode,in_graph,source_vertex);

    auto target_vertex = target(EI,in_graph);
    auto tgt_nodeid = get(&vertex_prop::vname,in_graph,target_vertex);
    auto tgt_opcode = get(&vertex_prop::vopcode,in_graph,target_vertex);



    D(cout<<"update_states_EWAIT_non_memory_queue: nodeid: "<< src_nodeid
        <<" estop_time: "<<estop_time
        <<" global_clock: "<<global_clock
        <<endl;
     );

    //By default its false , if its a mem-op and there is a response from the memory 
    //then check if the current edge's source node is the one that got response from the 
    //memory.
    //If it is some other op- then just check it is retired

    bool is_mem = false; 
    auto tmp_ir = get(&vertex_prop::vir,in_graph,source_vertex);

    //DO NOT SEND LIVE OUTS TO MEMORY
    if ( (src_opcode == "Load" || src_opcode =="Store") 
        && (tmp_ir.find("!LO") == std::string::npos))
      is_mem = true;


    if( !is_mem && (estop_time <= global_clock) )
      is_edge_complete = true;


    // if(estop_time <= global_clock && is_edge_complete)
    if(is_edge_complete )
    {
      //Update edge state from EWAIT to ECOMP
      update_edge_state(EWAIT, ECOMPLETE, EI);

    }

}


  void
Graph::update_states_VCOMPLETE_queue()
{

  // Iterate over vertex in ready states
  // get values in array
  vector<VERTEX> vertex_mv_C_to_R;
  for (auto &IT: vertex_state_map[VCOMPLETE])
  {
    // mv out_edges: if not already completed, or not already in wait - NR -> W: Update State, queues,  add start time and end time
    update_outedge_states_NR_to_CDE_or_EWAIT( ENOT_READY, EWAIT,IT);
  }

  for (auto &VI : vertex_mv_C_to_R)
  {
    update_vertex_states( VCOMPLETE, VRETIRE, VI);
  }

}

  void
Graph::update_mlp_stats( uint64_t elatency)
{
  if(elatency == MEMORYLATENCY)
    mlp++;
  //
  // if(mlpfile.is_open())
  //   mlpfile<<mlp<<"\n";
  // else
  //   cerr<<"Error opening mlp_file\n";
  //
}


//   void
// Graph::update_mlp_stats_1()
// {
//
//   mlp=0;
//   for( auto &EI: edge_state_map[EWAIT])
//   {
//
//     if(get(&edge_prop::elatency,in_graph,EI) == MEMORYLATENCY)
//       mlp++;
//   }
//
//   if(mlpfile.is_open())
//     mlpfile<<mlp<<"\n";
//   else
//     cerr<<"Error opening mlp_file\n";
//
// }


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

  cout<<"VREADY:\n";
  for( auto &VI: vertex_state_map[VREADY])
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

  cout<<"VRETIRE:\n";
  for( auto &VI: vertex_state_map[VRETIRE])
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



}




  void
Graph::print_memmap()
{
  m_memory->print_global_memaddr_map();
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
  while(std::getline(instream, line) && m_memory->global_memaddr_map_size() < ITER_COUNT_MEM ){
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
    m_memory->fill_global_memaddr_map( iter, nodeid , MemValue {isWrite,addr});
  }
  cout<<"read_memcsv is complete: Loaded Memory iteration table \n";
}


// Checks for all the in_edges, if all in_Edges are in ECDE or ECOMPLETE 
// then the vertex is ready
bool
Graph::is_vertex_ready( VERTEX _vertex)
{
  bool is_vertex_ready = true;
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
  return is_vertex_ready;
}


void
Graph::update_vertex_states(VSTATE _From_state,VSTATE  _To_state,VERTEX  _Vertex)
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




  bool
Graph::update_outedge_states_NR_to_CDE_or_EWAIT(ESTATE _From_state,ESTATE _To_state, VERTEX _Vertex)
{
  auto _From_state = ENOT_READY;
  auto _To_state = EWAIT;

  vector<EDGE> edge_mv_From_to_To;
  vector<EDGE> edge_mv_From_to_CDE;

  auto EO = out_edges(_Vertex,in_graph);

  // FOR all OUT EDGES 
  for (auto ES = EO.first, EE = EO.second; ES != EE; ES++) 
  {

    auto estate = get(&edge_prop::estate,in_graph,*ES);
    // update_mlp_stats( get(&edge_prop::elatency,in_graph,*ES));
    //insert ruby here  -- send request
    //FIXME add iteration no
    auto vopcode = get(&vertex_prop::vopcode,in_graph,_Vertex);

    //DO NOT SEND LIVE OUTS TO MEMORY
    auto tmp_ir = get(&vertex_prop::vir,in_graph,_Vertex);
    auto tmp_vissue = get(&vertex_prop::vissue,in_graph,_Vertex);
    auto is_mem = false;

    //CHECK IF ITS A CDE 
    if ( (vopcode == "Load" || vopcode =="Store") 
        && (tmp_ir.find("!LO") == std::string::npos)
        && tmp_vissue == false)
    {
      is_mem = true;
      put(&vertex_prop::vissue,in_graph,_Vertex,true);
      m_memory->send_req(g_iter,get(&vertex_prop::vname,in_graph,_Vertex));
      D(cerr<<"Sending to Memory nodeid: "<<get(&vertex_prop::vname,in_graph,_Vertex)<<" :VWAIT" <<endl);
      ++g_mem_req_count;
    }

    // bool check_ifCDE(*ES)

    auto target_vertex = target(*ES,in_graph);
    auto tgt_nodeid = get(&vertex_prop::vname,in_graph,target_vertex);
    auto tgt_opcode = get(&vertex_prop::vopcode,in_graph,target_vertex);
    auto tmp_tgt_ir = get(&vertex_prop::vir,in_graph,target_vertex);
    auto is_tgt_mem = false;

    if ( (tgt_opcode == "Load" || tgt_opcode =="Store") 
        && (tmp_tgt_ir.find("!LO") == std::string::npos)
       )
    {

      is_tgt_mem = true;
    }


    auto elatency = get(&edge_prop::elatency,in_graph,*ES);


    //UPDATE STATE TO CDE
    if(is_mem && is_tgt_mem)
      put(&edge_prop::estate,in_graph,*ES,ECDE); 

    //UPDATE STATE TO _To_STATE
    else 
      put(&edge_prop::estate,in_graph,*ES,_To_state); 

    put(&edge_prop::estart_time,in_graph,*ES,global_clock); 
    put(&edge_prop::estop_time,in_graph,*ES,global_clock+elatency); 

    if(find( edge_mv_From_to_To.begin(),edge_mv_From_to_To.end(),*ES) == edge_mv_From_to_To.end() && !(is_mem && is_tgt_mem) )
      edge_mv_From_to_To.push_back(*ES);
    else if(find( edge_mv_From_to_CDE.begin(),edge_mv_From_to_CDE.end(),*ES) == edge_mv_From_to_CDE.end() && is_mem && is_tgt_mem)
      edge_mv_From_to_CDE.push_back(*ES);


  }

  //Update Edge Queues
  for(auto &ED : edge_mv_From_to_To)
  {

    edge_state_map[_To_state].push_back(ED);
    edge_state_map[_From_state].erase( find( edge_state_map[_From_state].begin(),
          edge_state_map[_From_state].end(), ED));
  }

  //Update Edge Queues
  for(auto &ED : edge_mv_From_to_CDE)
  {

    edge_state_map[ECDE].push_back(ED);
    edge_state_map[_From_state].erase( find( edge_state_map[_From_state].begin(),
          edge_state_map[_From_state].end(), ED));
  }

  if(is_mem && is_tgt_mem)
    return true;
  else 
    return false;

}

void
Graph::update_edge_state(ESTATE _From_state, ESTATE _To_state, & EDGE EI)
{
      put(&edge_prop::estate,in_graph,EI,_To_state); 

      //Update states
      edge_state_map[_To_state].push_back(EI);
      //update queues
      edge_state_map[_From_state].erase(find(edge_state_map[_From_state].begin(),
            edge_state_map[_From_state].end(),EI));

}
