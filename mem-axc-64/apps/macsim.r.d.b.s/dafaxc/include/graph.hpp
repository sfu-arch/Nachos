#ifndef __GRAPH__
#define __GRAPH__

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/filesystem/fstream.hpp>
#include "data_type.hpp"
#include "memory.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <queue>


//------------------
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
//----------
//from bloom


using namespace std;
using namespace boost;



class Graph{

  private:
        BoostGraph orig_graph;
        BoostGraph in_graph;
        uint64_t graph_limit;
        bool g_en   = false;
        bool g_rdy  = false;
        uint64_t g_iter = 0;
        uint64_t g_mem_req_count =0;
        std::string g_input_filestring;
        uint64_t m_prev_completion_cycles=0;
        int g_mem_req_stack=0;

        uint64_t g_total_termination_cycles=0;
        uint64_t g_total_bloom_yes_ld =0;
        uint64_t g_total_bloom_yes_st =0;
        uint64_t g_total_bloom_no_ld =0;
        uint64_t g_total_bloom_no_st =0;
        uint64_t g_total_bloom_fp =0;
        uint64_t g_total_bloom_stall_cycles =0;
        bool m_stall_on_bloom = false;

  public:

        Memory* m_memory;
        //variables 
        uint64_t next_event_time_vertex=0;
        uint64_t next_event_time_edge=0;
        uint64_t global_clock=0;
        uint64_t mlp=0;
        std::fstream mlpfile;
        std::ifstream ldstfile;

        uint64_t g_total_memops=0;
        uint64_t g_total_ldops=0;
        uint64_t g_total_LiveOut =0;
        //-----------------------------------------------------

        //VEctor of arrays
        //Vertex queues
        std::vector< graph_traits<BoostGraph>::vertex_descriptor>  vertex_state_map [LEN_VSTATE];

        //Edge queues
        std::vector< graph_traits<BoostGraph>::edge_descriptor> edge_state_map [LEN_ESTATE];

        //functions
        
        //constructors
        Graph(uint64_t n, string ldstfile_path, string input_filestring, bool _is_stall_on_bloom/*, uint64_t start_clock*/, std::string _bloom_type, bool ishash2, uint64_t mem_lat, int _mem_req_stack);

        //destructor
        ~Graph();

        // Genergic Functions
        void set_state_maps();
        void readGraph();
        void initialize(uint64_t _iter );
        void printGraph();
        void printstats();
				unsigned int getOpcodeStr(string op_code); 
        void print_states();
        void read_memcsv(std::ifstream& file);
        void print_memmap();
        int run_a_cycle();



        int traverse_graph();
        // Edge Functions
        void update_states_ECDE_and_EMEM_queue();
        void update_states_ENOT_READY_queue();
        void update_states_EWAIT_queue();
        void update_edge_state(ESTATE _From_state, ESTATE _To_state, EDGE EI);
        ESTATE new_edge_state( EDGE IT);


        // Vertex Functions
        void update_states_VNOT_READY_queue();
        void update_states_VWAIT_queue();
        void update_vertex_state(VSTATE _From_state,VSTATE _To_state,VERTEX _Vertex);
        VSTATE new_vertex_state( VERTEX _vertex);

};

#endif
