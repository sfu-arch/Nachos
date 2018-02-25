#ifndef __DATA_TYPE__
#define __DATA_TYPE__

// #include <iostream>
#include <string>
// #include <vector>
// #include <cassert>
// #include <queue>
typedef unsigned long long uns64;
typedef uns64 Addr;

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/graph_utility.hpp>
#include "Address.h"
// enum COLOR {black,green,red,blue};
// #define NUM_BANKS 4



using namespace boost;
// Edge
enum ESTATE :uint32_t { ENOT_READY=0,EWAIT=1, ECDE=2, EMEM=3, ECOMPLETE=4};
#define LEN_ESTATE  5

struct edge_prop{ 
  std::string   ecolor;
  uint64_t      elatency;
  uint32_t      estate;
  uint64_t      estart_time;
  uint64_t      estop_time;
  // uint64_t      esource_nodeid;
  uint64_t      ecde_comparator_delay;
  bool          ecde_served;

};



// Vertex 
enum VSTATE:uint32_t { VNOT_READY=0, VWAIT=1, VCOMPLETE=2};
#define LEN_VSTATE  3


struct vertex_prop {
    uint64_t      vname;
    std::string   vcolor;
    std::string   vlabel;
    std::string   vir;
    std::string   vopcode;
    uint64_t      vlatency;
    uint32_t      vstate;  
    bool          vissue;
    uint64_t      vCDEcomp;
    uint64_t      vOPcomp;
    uint64_t      vcde_comparator_delay;
};

typedef boost::adjacency_list <boost::listS, boost::vecS, 
        boost::bidirectionalS, vertex_prop, edge_prop> BoostGraph;

typedef graph_traits<BoostGraph>::vertex_descriptor VERTEX;
typedef graph_traits<BoostGraph>::edge_descriptor EDGE;


typedef graph_traits<BoostGraph>::vertex_iterator VERTEX_ITER;

typedef graph_traits<BoostGraph>::edge_iterator EDGE_ITER;

struct MemValue 
{
  bool  isWrite;
  // std::string opcode;
  Addr addr;
};

struct MemValue_t
{
  bool isWrite;
  uint64_t nodeid;
  // bool is_issue;
  // Addr vaddr;
  bool operator==(const MemValue_t& lhs)
  {
    return (lhs.isWrite == isWrite && lhs.nodeid == nodeid /*&& lhs.vaddr == vaddr*/);
  }
};

// struct MemValue_t
// {
//   bool isWrite;
//   uint64_t nodeid;
//   Addr vaddr;
//   Address addr_c;
//   bool is_issue;
//   bool operator==(const MemValue_t& lhs)
//   {
//     return (lhs.isWrite == isWrite && lhs.nodeid == nodeid && lhs.vaddr == vaddr 
//         && lhs.addr_c== addr_c);
//   }
// };

#ifdef MY_DEBUG
#  define D(x) x
#else
#  define D(x)
#endif // DEBUG


#define ITER_COUNT  100
//ASSUMING PER ITERATION WILL NOT HAVE MORE THAN 500 OPS
#define ITER_COUNT_MEM  50000

#endif
