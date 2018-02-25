#include <iostream>
#include <map>
#include <set>
#include <assert.h>
// #include <zlib.h>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <cmath>
#include <queue>
#include <list>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cinttypes>


#include "pin.H"
#include "control_manager.H"
#include "portability.H"
#include <iomanip>

#include <string>
#include <unordered_set>

using namespace std;
using namespace CONTROLLER;


#if defined(TARGET_LINUX)
#define PINLINUX
#define WRITEM "w"
#else
#define WRITEM "wb"
#endif



/* ===================================================================== */
/* Names of FUNCTIONS */
/* ===================================================================== */

#define LDFUNC  "__InstruMem_load"
#define STFUNC  "__InstruMem_store"
#define STRING_SIZE 200
#define SUCCESS_THRESHOLD 10000
#define ITERATION_THRESHOLD 1000000

//#define ROI_FUNC "_Z19BlkSchlsEqEuroNoDivfffffif"
//#define OFFLOAD_FUNC "__offload_func_0"
// const char* ROI_FUNC = "table_pointer";
// const char* OFFLOAD_FUNC = "__offload_func_12";

char* ROI_FUNC ;
char* OFFLOAD_FUNC ;

  template <typename T>
string ToString(T val)
{
  stringstream stream;
  stream << val;
  return stream.str();
}


typedef struct {
    UINT64 IterNo;
    bool is_load;
    UINT32 NodeId;
    ADDRINT Addr;
    UINT64 byte;
} MemInfo;


/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

bool g_enable_instrument = false;
bool g_enable_offload = false;

UINT64 iteration_no=0;
UINT64 iteration_success=0;
UINT64 g_inst_count =0;
UINT64 g_inst_offload_count =0;

vector<MemInfo> g_mem_set_offload;
std::ofstream AxcMEMFile;
UINT64 ofload_mem = 0;



/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> Knob_funcfile          (KNOB_MODE_WRITEONCE, "pintool", "func", "function.txt", "File which contains the names of functions to be accelerated");

/* ===================================================================== */



//----------------------------------
//Function declaration
//----------------------------------

VOID offload_funcAfter(ADDRINT ret);

VOID EnterROI();
VOID ExitROI();
VOID offload_funcBefore();
VOID mem_func_ins_LD(UINT32 nodeid, UINT64 mem_addr, UINT64 _byte);
VOID mem_func_ins_ST(UINT32 nodeid, UINT64 mem_addr, UINT64 _byte);

void IInstruction(INS ins, void* v);
void IncrementNumInstruction();
void flush_axc_memset_to_file();

/* ===================================================================== */
/* Analysis routines                                                     */
/* ===================================================================== */
void flush_axc_memset_to_file()
{
  for (auto d : g_mem_set_offload)
  {
    AxcMEMFile <<  d.IterNo << ",";
    if(d.is_load) {
        AxcMEMFile << "Load,";
    } else {
        AxcMEMFile << "Store,";
    }
    AxcMEMFile << d.NodeId << "," << d.Addr
      <<","
      << d.byte
      
      << ",\n";
  }
}


/* ===================================================================== */

VOID Fini(INT32 code, VOID *v)
{

  cout<<"Total Iterations:"<<iteration_no<<"\n";
  cout<<"Total Iterations: SUCCESS "<<iteration_success<<"\n";
  cout<<"Total Instructions in Parent Function:"<<g_inst_count<<"\n";
  cout<<"Total Instructions in Offload Function:"<<g_inst_offload_count<<"\n";
  cout<<"Total Mem Ofload: " << ofload_mem << endl;
  cout<<"Exiting ...\n";
  AxcMEMFile.close();

}


VOID offload_funcAfter(ADDRINT ret)
{

  // AxcMEMFile << "---------------------------------------- " << endl;
  // AxcMEMFile << "  returns " << ret << endl;
  // AxcMEMFile << "---------------------------------------- " << endl;
  if(g_enable_instrument)
  {
    //if offload function is successful - then let ooo know that offload starts
    //and dump all ld st to axc mem file
    if(ret)
    {
      ++iteration_success;
      // cout<<"_OFFLOAD_TRUE iterno:"<<iteration_no<<endl;

      //flush to axc mem file
      flush_axc_memset_to_file();
      if(iteration_success > SUCCESS_THRESHOLD) {
          Fini(0,0);
          PIN_ExitProcess(0);
      }
    }
      g_mem_set_offload.clear();
    //else --if offload func fails -- dump all ld st to OOO

    //  AxcMEMFile<<"return value is: "<<ret<<endl;
    // AxcMEMFile << "---------------------------------------- " << endl;
    // -- empty stream containing instructions from offload func
  }

  g_enable_offload = false;
}


VOID offload_funcBefore()
{
  // cout << "NO!" << endl;
  g_enable_offload =true;
  iteration_no++;
      if(iteration_no > ITERATION_THRESHOLD) {
          Fini(0,0);
          PIN_ExitProcess(0);
      }
  // AxcMEMFile<<"#######################"<< endl;
}



VOID EnterROI()
{
  // std::cout << "App Enter ROI :" << endl;
  g_enable_instrument = true;
}

VOID ExitROI()
{
  // std::cout << "App Exit ROI :" << endl;
  g_enable_instrument = false;
}


//ONLY runs when offloaded region Load function is called
VOID mem_func_ins_LD(UINT32 nodeid, ADDRINT mem_addr, UINT64 byte)
{

  if(!g_enable_instrument)
    return;
  // cout<<"LD: node_id: "<<nodeid<<" mem_addr: "<<mem_addr<< " byte: "<< byte<<"\n";

  //vector<std::string> vec_set;
  //vec_set.push_back(ToString(iteration_no));
  //vec_set.push_back("Load");
  //vec_set.push_back(ToString(nodeid));
  //vec_set.push_back(ToString(mem_addr));
  // AxcMEMFile << "--LD Mem func -- " << endl; 

  ofload_mem++;
  //g_mem_set_offload.push_back(vec_set);
  g_mem_set_offload.push_back({iteration_no, true, nodeid, mem_addr, byte});

}


//ONLY runs when offloadedi region store function is called
VOID mem_func_ins_ST(UINT32 nodeid, ADDRINT mem_addr, UINT64 byte)
{
  if(!g_enable_instrument)
    return;

  // cout<<"ST: node_id: "<<nodeid<<" mem_addr: "<<mem_addr<< " byte: "<< byte<<"\n";
  //vector<std::string> vec_set;
  //vec_set.push_back(ToString(iteration_no));
  //vec_set.push_back("Store");
  //vec_set.push_back(ToString(nodeid));
  //vec_set.push_back(ToString(mem_addr));

  // AxcMEMFile << "--St Mem func -- " << endl; 

  ofload_mem++;
  g_mem_set_offload.push_back({iteration_no, false, nodeid, mem_addr, byte});

}



////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instruction - instrumentation
////////////////////////////////////////////////////////////////////////////////////////////////////////
void IInstruction(INS ins, void* v)
{
  // Count instructions
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IncrementNumInstruction, IARG_END);

}


////////////////////////////////////////////////////////////////////////////////////////////////////////
// Increment the number of instructions
////////////////////////////////////////////////////////////////////////////////////////////////////////
void IncrementNumInstruction()
{

  if(!g_enable_instrument)
    return;

  if(g_enable_offload)
    g_inst_offload_count++;

  g_inst_count++;

}

/////////////////////////////////////////////////////////////////////////////////////////
// Main instrumentation routine
// Depending on the type of an instruction, call corresponding subroutines.
// Do not modify this routine!!
/////////////////////////////////////////////////////////////////////////////////////////





/* ===================================================================== */
/* Instrumentation routines                                              */
/* ===================================================================== */

VOID Image(IMG img, VOID *v)
{

  // ROI_FUNC --- begin and end of region of interest
  RTN roi_funcRtn = RTN_FindByName(img, ROI_FUNC);
  if (RTN_Valid(roi_funcRtn))
  {
    RTN_Open(roi_funcRtn);

    RTN_InsertCall(roi_funcRtn, IPOINT_AFTER, (AFUNPTR)ExitROI,
        IARG_END);

    RTN_InsertCall(roi_funcRtn, IPOINT_BEFORE, (AFUNPTR)EnterROI,IARG_END);

    RTN_Close(roi_funcRtn);
  }




  //-- OFFLOADED FUNCTION --------
  RTN offload_funcRtn = RTN_FindByName(img, OFFLOAD_FUNC);
  if (RTN_Valid(offload_funcRtn))
  {
    RTN_Open(offload_funcRtn);

    RTN_InsertCall(offload_funcRtn, IPOINT_AFTER, (AFUNPTR)offload_funcAfter,
        IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);

    RTN_InsertCall(offload_funcRtn, IPOINT_BEFORE, (AFUNPTR)offload_funcBefore,
        IARG_END);

    RTN_Close(offload_funcRtn);
  }

  //---------- Instrument Offloaded Ld and St function 
  RTN Ld_funcRtn = RTN_FindByName(img, LDFUNC);
  if (RTN_Valid(Ld_funcRtn))
  {
    RTN_Open(Ld_funcRtn);

    RTN_InsertCall(Ld_funcRtn, IPOINT_BEFORE, (AFUNPTR)mem_func_ins_LD,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
        IARG_END);


    RTN_Close(Ld_funcRtn);
  }

  //---------- Instrument Offloaded Ld and St function 
  RTN St_funcRtn = RTN_FindByName(img, STFUNC);
  if (RTN_Valid(St_funcRtn))
  {
    RTN_Open(St_funcRtn);

    RTN_InsertCall(St_funcRtn, IPOINT_BEFORE, (AFUNPTR)mem_func_ins_ST,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
        IARG_END);

    RTN_Close(St_funcRtn);
  }



  cout << "Image Instrumentation\n";



}





/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
  cerr << "This tool produces a trace of calls to offload_func1." << endl;
  cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
  return -1;
}






/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{
  cout<<"start\n";
  // Initialize pin & symbol manager
  PIN_InitSymbols();
  if( PIN_Init(argc,argv) )
  {
    return Usage();
  }




  ifstream funcfile;
  funcfile.open(Knob_funcfile.Value().c_str(),ios::in);

  if(!funcfile.is_open())
    assert(false && "Unable to open accelerated function names file (default: function.txt)");

  ROI_FUNC = new char[STRING_SIZE];
  OFFLOAD_FUNC = new char[STRING_SIZE];


  int i = 0;
  for( std::string line; getline(funcfile, line); ){
    if(i == 0)
      line.copy(ROI_FUNC, line.size(), 0);
    else if (i == 1)
      line.copy(OFFLOAD_FUNC, line.size(), 0);
    else
      return 1;
    i++;
  }

  cout<<"ROI_FUNC:"<<ROI_FUNC<<"\t OFFLOAD_FUNC:"<<OFFLOAD_FUNC<<endl;



  // Write to a file since cout and cerr maybe closed by the application
  AxcMEMFile.open("axc-dump.out");
  //AxcMEMFile << hex;
  AxcMEMFile.setf(ios::showbase);


  // Register Image to be called to instrument functions.
  IMG_AddInstrumentFunction(Image, 0);
  PIN_AddFiniFunction(Fini, 0);

  INS_AddInstrumentFunction(IInstruction, 0);
  // Never returns
  PIN_StartProgram();

  return 0;
}

