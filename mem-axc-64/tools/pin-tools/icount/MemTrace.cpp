/*
 *  This file contains an ISA-portable PIN tool for tracing memory accesses.
 */

#include <stdio.h>
#include <assert.h>
#include "pin.H"
#include <iostream>
#include <fstream>
using namespace std;


/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> Knob_funcfile          (KNOB_MODE_WRITEONCE, "pintool", "func", "function.txt", "File which contains the names of functions to be accelerated");

/* ===================================================================== */

#define STRING_SIZE 200
#define ITERATION_THRESHOLD 30000000

UINT64 iteration_no=0;

UINT64 ins_count = 0;
char* ROI_FUNC ;
char* OFFLOAD_FUNC ;
FILE * trace;
bool g_enable_instrument = false;
std::ofstream StackFile;
std::ofstream HeapFile;

VOID Fini(INT32 code, VOID *v);


// Print a memory read record
// VOID RecordMemRead(VOID * ip, VOID * addr, UINT32 mem_read_size, BOOL stackop)
// {
//   iteration_no++;
//   if(iteration_no > ITERATION_THRESHOLD) {
//     Fini(0,0);
//     PIN_ExitProcess(0);
//   }
//   if(stackop)
//     StackFile << "R "<< addr <<" " << mem_read_size <<"\n";
//     // fprintf(StackFile,"R %p %d \n", addr, mem_read_size);
//   else 
//     HeapFile << "R "<< addr <<" " << mem_read_size <<"\n";
//     // fprintf(HeapFile,"R %p %d\n", addr, mem_read_size);
// }

// Print a memory write record
// VOID RecordMemWrite(VOID * ip, VOID * addr, UINT32 mem_write_size, BOOL stackop)
// {
//   iteration_no++;
//   if(iteration_no > ITERATION_THRESHOLD) {
//     Fini(0,0);
//     PIN_ExitProcess(0);
//   }
//   if(stackop)
//     StackFile << "W "<< addr <<" " << mem_write_size <<"\n";
//   // fprintf(StackFile,"W %p %d \n", addr, mem_write_size);
//   else 
//     HeapFile << "W "<< addr <<" " << mem_write_size <<"\n";
//     // fprintf(HeapFile,"W %p %d \n", addr, mem_write_size);
// }

VOID docount()
{
  ins_count++;
}
// Is called for every instruction and instruments reads and writes
VOID IInstruction(INS ins, VOID *v)
{
  if(!g_enable_instrument)
    return;

  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_END);
  // Instruments memory accesses using a predicated call, i.e.
  // the instrumentation is called iff the instruction will actually be executed.
  //
  // On the IA-32 and Intel(R) 64 architectures conditional moves and REP 
  // prefixed instructions appear as predicated instructions in Pin.
  // UINT32 memOperands = INS_MemoryOperandCount(ins);
  // BOOL stackop= false;
  // if(INS_IsStackRead(ins))
  //   stackop=true;
  //
  // if(INS_IsStackWrite(ins))
  //   stackop=true;
  //
  // // Iterate over each memory operand of the instruction.
  // for (UINT32 memOp = 0; memOp < memOperands; memOp++)
  // {
  //   if (INS_MemoryOperandIsRead(ins, memOp))
  //   {
  //     INS_InsertPredicatedCall(
  //         ins, IPOINT_BEFORE, (AFUNPTR)RecordMemRead,
  //         IARG_INST_PTR,
  //         IARG_MEMORYOP_EA, memOp,IARG_MEMORYREAD_SIZE, IARG_BOOL,stackop, 
  //         IARG_END);
  //   }
  //
  //   // Note that in some architectures a single memory operand can be 
  //   // both read and written (for instance incl (%eax) on IA-32)
  //   // In that case we instrument it once for read and once for write.
  //   if (INS_MemoryOperandIsWritten(ins, memOp))
  //   {
  //
  //     INS_InsertPredicatedCall(
  //         ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWrite,
  //         IARG_INST_PTR,
  //         IARG_MEMORYOP_EA, memOp,IARG_MEMORYREAD_SIZE,IARG_BOOL,stackop,
  //         IARG_END);
  //   }
  // }
}

VOID Fini(INT32 code, VOID *v)
{
  fprintf(trace, "#eof\n");
  // fclose(trace);

  cerr <<  "Total Instruction Count: " << ins_count  << endl;
  cout<<"Total Iterations: "<<iteration_no<<"\n";
  cout<<"Exiting ...\n";
  fprintf(trace,"Total Instruction Count: %zu \n",ins_count); 
  fprintf(trace,"Total Iterations: %zu \n", iteration_no); 
  fprintf(trace,"Exiting ...\n"); 
  fclose(trace);
  // StackFile.close();
  // HeapFile.close();


}


VOID EnterROI()
{
  // std::cout << "App Enter ROI :" << endl;
  g_enable_instrument = true;
  iteration_no++;
}

VOID ExitROI()
{
  // std::cout << "App Exit ROI :" << endl;
  g_enable_instrument = false;
}


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





  cout << "Image Instrumentation\n";



}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
  PIN_ERROR( "This Pintool prints a trace of memory addresses\n" 
      + KNOB_BASE::StringKnobSummary() + "\n");
  return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{

  //----------------------------------------------------------------
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

  cout<<"ROI_FUNC:"<<ROI_FUNC<<"\n"; // OFFLOAD_FUNC:"<<OFFLOAD_FUNC<<endl;


  trace = fopen("pinatrace.out", "w");

  // Write to a file since cout and cerr maybe closed by the application
  // StackFile.open("stack-dump.out");
  // StackFile.setf(ios::showbase);
  //
  // HeapFile.open("heap-dump.out");
  // HeapFile.setf(ios::showbase);

  // Register Image to be called to instrument functions.
  IMG_AddInstrumentFunction(Image, 0);
  PIN_AddFiniFunction(Fini, 0);

  INS_AddInstrumentFunction(IInstruction, 0);
  // Never returns
  PIN_StartProgram();

  return 0;



  //----------------------------------------------------------------
  // INS_AddInstrumentFunction(Instruction, 0);
  // PIN_AddFiniFunction(Fini, 0);
  //----------------------------------------------------------------

}
