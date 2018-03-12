#include "graph.hpp"
#include "data_type.hpp"

#include <iostream>
#include <iterator>
#include <fstream> 
#include <string>
#include <cassert>
#include <algorithm>
#include <typeinfo>
#include <set>
#include <exception>
//--------------------------
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem.hpp>
#include <regex>
//---------------------------
#include <boost/filesystem/fstream.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/program_options.hpp>

#include "O3sim_ruby.h"

#include <set>
#include <sstream>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
namespace po = boost::program_options;

std::set<uint64_t> create_iteration_vec( string _ldst_file)
{
  std::ifstream ldst_file(_ldst_file, std::ios_base::in | std::ios_base::binary);

  if (!ldst_file.is_open())
  {
    cerr << "Main: Error Mem Dump ldst file not reached end Foey!\n";
  }

  boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
  inbuf.push(boost::iostreams::gzip_decompressor());
  inbuf.push(ldst_file);

  //Convert streambuf to istream
  std::istream instream(&inbuf);

  std::string line;

  std::set<uint64_t> data;

  regex mem_parse("^([[:digit:]]+),([^,]*),([[:digit:]]+),([[:digit:]]+).*$");
  smatch match;
  while(std::getline(instream, line) && data.size() < ITER_COUNT ){
    regex_search(line, match, mem_parse);
    uint64_t iter = std::stoi(match[1]);
    // cout<< "iter: " << iter <<" ";
    data.insert(iter);
  }

  return data;
}

// std::set<uint64_t> create_iteration_vec( string _ldst_file)
// {
//   std::set <uint64_t> data;
//   std::ifstream infile(_ldst_file.c_str());
//
//
//   while (infile)
//   {
//     string line;
//     if (!getline( infile, line )) break;
//
//     istringstream ss( line );
//
//     if (ss)
//     {
//       string s;
//       if (!getline( ss, s, ',' )) break;
//       data.insert( stoi(s) );//vector of each line
//     }
//
//   }
//
//   if (!infile.eof())
//   {
//     cerr << "Main: Error Mem Dump ldst file not reached end Foey!\n";
//   }
//
//   infile.close();
//
//   return data;
// }

  int 
main(int argc, char* argv[])
{

  bool stall_on_bloom;
  bool is_nachos;
  bool is_LSQ;
  uint64_t bfp_size;
  uint64_t issue_width;
  uint64_t bfp_counter_size;
  bool bfp_ishash2;
  uint64_t m_latency;
  int m_mem_req_stack;

  string m_file_path;
  string m_ldstfile_path;
  path input_dotfile_path;
  path ldst_file_path;
  path p_m;
  std::set<uint64_t> m_iter_vec;

  //Processing program input
  try {

    po::options_description desc("Usage: chain [Program's options] <input_file>\n"
        "Chain's options:");
    desc.add_options()
      ("help", "Display this help and exit")
      ("input-file,i", po::value<string>(&m_file_path)->required(),
       "Input file [should be in dot format]")
      ("ldst-file,m", po::value<string>(&m_ldstfile_path)->required(),
       "ld store dynamic information file ")
      ("stall_on_bloom,s", po::value<bool>(&stall_on_bloom)->default_value(false),
       "Stall on bloom")

      ("is_nachos,n", po::value<bool>(&is_nachos)->default_value(false),
       "is nachos")


      ("issue_width,w", po::value<uint64_t>(&issue_width)->default_value(10000),
       "Issue Width :")

      ("is_LSQ,q", po::value<bool>(&is_LSQ)->default_value(true),
       "is LSQ")
      ("bfp_size,f", po::value<uint64_t>(&bfp_size)->default_value(256),
       "Bloom filter size:")

      ("bfp_counter_size,c", po::value<uint64_t>(&bfp_counter_size)->default_value(64),
       "Bloom filter counter size:")

      ("bfp_ishash2,h", po::value<bool>(&bfp_ishash2)->default_value(false),
       "Bloom filter is_hash2")

      ("latency,l", po::value<uint64_t>(&m_latency)->default_value(20),
       "latency of memory")

      ("mshr,r", po::value<int>(&m_mem_req_stack)->default_value(32),
       "mshr size")

      ;

    po::variables_map vm;        
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      cout << desc << "\n";
      return 0;
    }

    if (vm.count("input-file")) {
      cout << "input file was set to " 
        << vm["input-file"].as<string>() << ".\n";
      input_dotfile_path = vm["input-file"].as<string>();
      if (input_dotfile_path.extension() != ".dot") {
        cerr << "DOT FILE is needed" << endl;
      }
    }

    if (vm.count("ldst-file")) {
      cout << "ldst file was set to " 
        << vm["ldst-file"].as<string>() << ".\n";

      p_m = vm["ldst-file"].as<string>();
      if (p_m.extension() != ".gz") 
        cerr << "GZ FILE is needed" << endl;


      ldst_file_path = vm["ldst-file"].as<string>();
    }

    if (vm.count("stall_on_bloom")){
      stall_on_bloom = vm["stall_on_bloom"].as<bool>(); 
      cout << "is stall on bloom:  "
        << vm["stall_on_bloom"].as<bool>() << ".\n";
    }

    if (vm.count("is_nachos")){
      is_nachos = vm["is_nachos"].as<bool>(); 
      cout << "is nachos is set:  "
        << vm["is_nachos"].as<bool>() << ".\n";
    }

    if (vm.count("is_LSQ")){
      is_LSQ = vm["is_LSQ"].as<bool>(); 
      cout << "is LSQ set:  "
        << vm["is_LSQ"].as<bool>() << ".\n";
    }

    if(vm.count("latency")){
      cout << "latency of memory:  "
        << vm["latency"].as<uint64_t>() << ".\n";
    }


    if(vm.count("issue_width")){
      cout <<"Issue Width:"
        << vm["issue_width"].as<uint64_t>() <<".\n";
    } 



    if(vm.count("bfp_size")){
      cout <<"Bloom filter size:"
        << vm["bfp_size"].as<uint64_t>() <<".\n";
    } 



    if(vm.count("bfp_counter_size")){
      cout <<"Bloom filter counter  size:"
        << vm["bfp_counter_size"].as<uint64_t>() <<".\n";
    }

    if (vm.count("bfp_ishash2")){
      bfp_ishash2 = vm["bfp_ishash2"].as<bool>(); 
      cout << "is bfp hash2:  "
        << vm["bfp_ishash2"].as<bool>() << ".\n";
    }


    po::notify(vm);    
  }
  catch(std::exception& e) {
    cerr << "error: " << e.what() << "\n";
    return 1;
  }
  catch(...) {
    cerr << "Exception of unknown type!\n";
  }

  cout<<"Main: Loading Start Iteration nos. \n";
  cerr<<"Main: Loading Start Iteration nos. \n";

  m_iter_vec= create_iteration_vec(ldst_file_path.c_str());

  cout<< "m_iter_vec : size: "<<m_iter_vec.size() <<endl;

  if(m_iter_vec.size() == 0)
  {
    cerr<<"Error: m_iter_vec : size = 0"<<endl;
    exit(0);
  }


  cout<<"Main: Loaded Iteration nos. \n";
  cerr<<"Main: Loaded Iteration nos. \n";


  std::shared_ptr<O3sim_ruby> m_ruby (new O3sim_ruby(1, 1,4,1, true, true, 1 ,"","med","/dev/null"));
  m_ruby->initialize();
  uint64_t memory_latency;
  int _mem_req_stack;
  _mem_req_stack = m_mem_req_stack;
  memory_latency= m_latency;
  uint32_t m_resource=256;
  Graph input_graph_c(m_resource, ldst_file_path.c_str(), input_dotfile_path.c_str(),m_ruby ,stall_on_bloom,"LSB_Counting-" + to_string(bfp_size) + ":"+ to_string(128),bfp_ishash2, memory_latency, _mem_req_stack,is_nachos,is_LSQ,issue_width );

#define STOP 1000000
  // #define STOP 5000 

  uint64_t iter0=0;
  for( auto &iterno :m_iter_vec )
  {
    D(cout<<"dafaxc: initialized Iteration No: "<<iterno<<"\n");
    //iteration no -- test file has only 1
    input_graph_c.initialize(iterno);

    int count=0;
    while(count< STOP) {
      auto ret = input_graph_c.run_a_cycle();
      // cout<<"run_a_cycle: "<<ret<<endl;
      m_ruby->advance_time();
      assert(ret != -1);
      if(ret == -1 ) {

        cout<< "ERROR FORCE STOP \n";
        exit(0);
      }
      if(ret >0)
        break;
      if(count > STOP -2)
      {
        assert(0 && "MAIN: program not stalling\n");
        exit(0);
      }


      if(count %100 == 0)
        cout<<"%100th cycle :"<< count <<endl;

      count++;
    }

    if(iter0 %10 == 0)
      cout<<"%10th iteration :"<< iter0 <<endl;

    iter0++;
  }


  input_graph_c.printGraph();
  input_graph_c.printstats();
  cout<<"Cumm. Total Iteration Count: "<<m_iter_vec.size()<<endl;
  std::ofstream ruby_stat_file("ruby.stat.out", ios::out);
  m_ruby->print_stats(ruby_stat_file);
  ruby_stat_file.close();
  m_ruby->destroy();
  return 0;
}

//----------------------------------------------------------------------------------------------------------------



