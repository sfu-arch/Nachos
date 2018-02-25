#include <iostream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <regex>
//---------------------------
#include <boost/filesystem/fstream.hpp>
#include <set>
#include <sstream>

using namespace std;
using namespace boost;
using namespace boost::filesystem;
namespace po = boost::program_options;

#ifdef MY_DEBUG
#  define D(x) x
#else
#  define D(x)
#endif // DEBUG


#define ITER_COUNT 1000000

std::map< std::pair<uint64_t,uint64_t>, uint64_t> g_may_NO_map;
std::map< std::pair<uint64_t,uint64_t>, uint64_t> g_may_MUST_map;
std::set<std::pair<uint64_t,uint64_t>> g_E_pair;

  void 
update_g_stats(std::map<uint64_t,uint64_t> mem_map )
{
  D(cout<<"-- ITER -----\n");
  for (auto &Epair : g_E_pair)
  {
    D(cout<<"S: "<<Epair.first<<"\t"
      <<"T: "<<Epair.second<<endl);
    if(mem_map.count(Epair.first) ==0 || mem_map.count(Epair.second) == 0)
    {
      continue;
      // cerr<<"nodeid not found in mem_map"<<endl;
      // exit(0);
    }
    auto src_addr = mem_map[Epair.first];
    auto tgt_addr = mem_map[Epair.second];

    // cout<<src_addr <<" : "<<tgt_addr<<endl;
    if(src_addr != tgt_addr)
      g_may_NO_map[Epair] +=1 ; 
    else
      g_may_MUST_map[Epair] +=1;
  }
}

  void
print_g_stats()
{
  // cout<<"CASE: NO ALIAS"<<endl;
  // for(auto &it : g_may_NO_map)
  //   cout<<it.first.first <<"->" <<it.first.second<<" "<<it.second <<endl;
  //
  // cout<<"CASE: MUST ALIAS"<<endl;
  // for(auto &it : g_may_MUST_map)
  //   cout<<it.first.first <<"->" <<it.first.second<<" "<<it.second <<endl;

  // cout<<"S E : NO MUST"<<endl;
  for(auto &it: g_E_pair)
    cout<<it.first <<" "<<it.second<<" : "
      <<g_may_NO_map[it] <<" "<<g_may_MUST_map[it] <<endl;
}

  void
create_iteration_vec( path mem_f_path)
{
  std::ifstream memfile(mem_f_path.c_str(), std::ios_base::in | std::ios_base::binary);

  if (!memfile.is_open())
  {
    cerr << "Main: Error Mem Dump ldst file not reached end Foey!\n";
  }

  boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
  inbuf.push(boost::iostreams::gzip_decompressor());
  inbuf.push(memfile);

  //Convert streambuf to istream
  std::istream instream(&inbuf);

  std::string line;

  std::list<int> data;
  std::map<uint64_t,uint64_t> mem_map;

  regex mem_parse("^([[:digit:]]+),([^,]*),([[:digit:]]+),([[:digit:]]+).*$");
  smatch match;
  int prev_iter=-1;
  uint64_t count=0; 

  while(std::getline(instream, line) && count < ITER_COUNT ){
    count++;
    regex_search(line, match, mem_parse);
    int iter;
    uint64_t nodeid;
    // string   opcode;
    // bool isWrite =false;
    uint64_t addr;

    iter = stoi(match[1]);
    nodeid = stoi(match[3]);
    addr = stoll(match[4]);
    // cout<<"iter: "<<iter
      // <<"nodeid: "<<nodeid
      // <<"addr: "<<addr<<endl;
    // opcode = match[2];
    // if(opcode == "Store")
    //   isWrite = true;
    // else                
    //   isWrite = false; 

    if(prev_iter==iter) 
    {
      data.push_back(iter);
      D(cout<<"N: "<<nodeid
        <<"addr: "<<addr<<endl);
      mem_map[nodeid] =addr;
    }
    else
    {
      prev_iter=iter;
      // for(auto &x :data) cout<<x<<" ";
      D(cout<<endl<<"--------------"<<endl);
      //----------------------------------
      //Analysis for may_profile
      update_g_stats(mem_map);
      D(cout<<endl<<"--------------"<<endl);
      //----------------------------------
      data.clear();
      data.push_back(iter);
      mem_map.clear();
      D(cout<<"N: "<<nodeid
        <<"addr: "<<addr<<endl);
      mem_map[nodeid]=addr;
    }

  }

  D(cout<<endl<<"#####-------------"<<endl);
  update_g_stats(mem_map);

  print_g_stats();

}

  void
parse_edge_pairs(path may_f_path)
{
  //Read Edges
  std::ifstream mayfile(may_f_path.c_str(),ios::in);
  std::set<std::pair<uint64_t,uint64_t>> E_pair;
  string line;
  while(getline(mayfile,line))
  {
    std::stringstream ss(line);
    uint64_t src_E,tgt_E;
    ss>>src_E;
    ss>>tgt_E;
    g_E_pair.insert(std::make_pair(src_E,tgt_E));
  }

  D(for( auto &E : g_E_pair)
  {
    cout<<E.first << " " <<E.second << endl;
  })
}

  int 
main(int argc, char* argv[])
{

  string m_file_path;
  string m_ldstfile_path;
  path may_f_path;
  path mem_f_path;
  path p_m;
  std::set<uint64_t> m_iter_vec;

  //Processing program input
  try {

    po::options_description desc("Usage: [Program's options] <input_file>\n"
        " options:");
    desc.add_options()
      ("help", "Display this help and exit")
      ("may-file,i", po::value<string>(&m_file_path)->required(),
       "format 2 3")
      ("memfile,m", po::value<string>(&m_ldstfile_path)->required(),
       "ld store dynamic information file ")
      ;

    po::variables_map vm;        
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
      cout << desc << "\n";
      return 0;
    }

    if (vm.count("may-file")) {
      cerr << "may file was set to " 
        << vm["may-file"].as<string>() << ".\n";
      may_f_path = vm["may-file"].as<string>();
    }

    if (vm.count("memfile")) {
      cerr << "ldst file was set to " 
        << vm["memfile"].as<string>() << ".\n";

      p_m = vm["memfile"].as<string>();
      if (p_m.extension() != ".gz") 
        cerr << "GZ FILE is needed" << endl;

      mem_f_path = vm["memfile"].as<string>();
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

  //---------------------------------------------------------
  //Initialize g_NO and g_Must
  for(auto &epair: g_E_pair)
  {
    g_may_NO_map[epair]=0;
    g_may_MUST_map[epair]=0;
  }

  //Read Edges
  parse_edge_pairs(may_f_path.c_str());
  //---------------------------------------------------------
  // Read per Iteration from the file and profile may alias cases
  create_iteration_vec(mem_f_path);


  return 0;
}

