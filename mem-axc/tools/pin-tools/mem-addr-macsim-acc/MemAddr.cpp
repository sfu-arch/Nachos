#include <iostream>
#include <map>
#include <set>
#include <assert.h>
#include <zlib.h>
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

#include "MemAddr.h"

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

//#define ROI_FUNC "_Z19BlkSchlsEqEuroNoDivfffffif"
//#define OFFLOAD_FUNC "__offload_func_0"

char* ROI_FUNC ;
char* OFFLOAD_FUNC;

template <typename T>
string ToString(T val)
{
    stringstream stream;
    stream << val;
    return stream.str();
}


/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

bool g_enable_instrument = false;

bool g_enable_offload = false;
bool print_inst=false;
unsigned int iteration_no=0;

Trace_info* trace_info_array;
UINT64 g_inst_count =0;
vector<vector<std::string>> g_mem_set;
// vector<Inst_info> g_ins_buffer;
map<ADDRINT, Inst_info*> g_ins_buffer;

std::ofstream TraceFile;


#ifdef PINLINUX
uint32_t eflag_value=0;
#endif





/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "mem-dump.out", "specify trace file name");

KNOB<string> Knob_trace_name  (KNOB_MODE_WRITEONCE, "pintool", 
    "tracename", "trace",  "trace output filename");

KNOB<bool>   Knob_print_inst (KNOB_MODE_WRITEONCE, "pintool", 
    "print_inst", "0", "dumping trace in the stdout" );


KNOB<string> Knob_dump_file  (KNOB_MODE_WRITEONCE, "pintool", 
    "dump_file", "dump.txt", "");


KNOB<bool>   Knob_inst_dump  (KNOB_MODE_WRITEONCE, "pintool", 
    "dump", "1", "Dump t_info to the file");



/* ===================================================================== */



//----------------------------------
//Function declaration
//----------------------------------

void sanity_check(void);
VOID offload_funcAfter(ADDRINT ret);

void write_inst_debugfile(Trace_info* trace_info,Inst_info* i_info);
void write_inst_frm_ins_buffer (ADDRINT iaddr);
VOID EnterROI();
VOID ExitROI();
VOID offload_funcBefore();
VOID mem_func_ins_LD(UINT32 nodeid, UINT64 mem_addr);
VOID mem_func_ins_ST(UINT32 nodeid, UINT64 mem_addr);
void IInstruction(INS ins, void* v);
void IncrementNumInstruction();
void iinstrument(INS ins);

void get_target(ADDRINT target, bool taken);



/////////////////////////////////////////////////////////////////////////////////////////
// get an effetive load address : instrumentation function
/////////////////////////////////////////////////////////////////////////////////////////
void get_ld_ea(ADDRINT addr, UINT32 mem_read_size,
#ifndef PINLINUX
    UINT32 eflag_value,
#endif
    BOOL stackOp )
{

  Trace_info* trace_info = trace_info_array;
  trace_info->vaddr1        = addr;
  trace_info->mem_read_size = mem_read_size;
  trace_info->eflags        = eflag_value;
  trace_info->actually_taken = stackOp;
  //-----------------
  // if(!stackOp)
  // {
  //   check_n_write_marker(addr,trace_info);
  //
  // }
}


/////////////////////////////////////////////////////////////////////////////////////////
// Get effective load addresses : instrumentation function
// Insturction that has 2 load addresses
/////////////////////////////////////////////////////////////////////////////////////////
void get_ld_ea2( ADDRINT addr1, ADDRINT addr2,/* UINT32 mem_read_size,*/
#ifndef PINLINUX
    UINT32 eflag_value,
#endif
  UINT32 mem_read_size )
{

  Trace_info* trace_info = trace_info_array;
  trace_info->vaddr1        = addr1;
  trace_info->vaddr2        = addr2;
  trace_info->mem_read_size = mem_read_size;
  trace_info->eflags        = 0;
  //-----------------
  // Cannot be a stack Load
  // check_n_write_marker(addr1,trace_info);
  // check_n_write_marker(addr2,trace_info);
}



/////////////////////////////////////////////////////////////////////////////////////////
// Get store address : instrumentation function
/////////////////////////////////////////////////////////////////////////////////////////
void get_st_ea(ADDRINT addr, UINT32 mem_st_size,
#ifndef PINLINUX
    UINT32 eflag_value,
#endif

    BOOL stackOp)
{

  Trace_info* trace_info = trace_info_array;

  trace_info->st_vaddr       = addr;
  trace_info->mem_write_size = mem_st_size;
  trace_info->eflags         = 0;
  trace_info->actually_taken = stackOp;
  // if(!stackOp)
  //   check_n_write_marker(addr,trace_info);
}


/* ===================================================================== */
/* Analysis routines                                                     */
/* ===================================================================== */


 VOID offload_funcAfter(ADDRINT ret)
{

  // TraceFile << "---------------------------------------- " << endl;
  // TraceFile << "  returns " << ret << endl;
  // TraceFile << "---------------------------------------- " << endl;
  if(g_enable_instrument)
  {
    if(ret)
    {

      for (auto vec_set: g_mem_set)
      {
        for (const std::string& x: vec_set) TraceFile<< x <<",";
        TraceFile<<"\n";

      }

      //------------------------------------------------------------------------------------------ 
      //Insert a marker indicating begining of axc func -- which was a success
      // Create a NOP instr -- add it to write_inst 
      // also clear trace_info other params

      // Inst_info* m = (Inst_info*)memset((void *)new Inst_info, 0, sizeof(Inst_info));
      // m->ds_flag = true;
      // m->chunk_id = iteration_no;
      // m->is_chunk_begin = true;
      // m->num_read_regs = 0;
      // //TODO : To Modify for multi threaded
      // m->thread_id = 1;
      // m->opcode = TR_NOP; 
      //
      // // Trace_info* trace_info = trace_info_array;
      //
      // delete m;
      //------------------------------------------------------------------------------------------ 


      //flush instructions-- in offload func to tracefile
    }

    g_mem_set.clear();

    //  TraceFile<<"return value is: "<<ret<<endl;
    // TraceFile << "---------------------------------------- " << endl;
    // -- empty stream containing instructions from offload func
  }

  g_enable_offload = false;
}


VOID offload_funcBefore()
{
    // cout << "__ofloadfunc before" << endl;
  g_enable_offload =true;
  iteration_no++;
  // TraceFile<<"#######################"<< endl;
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


VOID mem_func_ins_LD(UINT32 nodeid, UINT64 mem_addr  )
{
  if(g_enable_instrument)
  {
    // TraceFile << "--LD Mem func -- " << endl; 

    vector<std::string> vec_set;
    vec_set.push_back(ToString(iteration_no));
    vec_set.push_back("Load");
    vec_set.push_back(ToString(nodeid));
    vec_set.push_back(ToString(mem_addr));
    g_mem_set.push_back(vec_set);
    
  }

}



VOID mem_func_ins_ST(UINT32 nodeid, UINT64 mem_addr  )
{
  
  if(g_enable_instrument)
  {

    // TraceFile << "--ST Mem func -- " << endl; 
    vector<std::string> vec_set;
    vec_set.push_back(ToString(iteration_no));
    vec_set.push_back("Store");
    vec_set.push_back(ToString(nodeid));
    vec_set.push_back(ToString(mem_addr));
    g_mem_set.push_back(vec_set);

  }
}



////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instruction - instrumentation
////////////////////////////////////////////////////////////////////////////////////////////////////////
void IInstruction(INS ins, void* v)
{
  // Count instructions
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IncrementNumInstruction, IARG_END);

  // Instrument instructions
  iinstrument(ins);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////
// Increment the number of instructions
////////////////////////////////////////////////////////////////////////////////////////////////////////
void IncrementNumInstruction()
{

  if(!g_enable_instrument)
    return;

  g_inst_count++;

}

/////////////////////////////////////////////////////////////////////////////////////////
// Main instrumentation routine
// Depending on the type of an instruction, call corresponding subroutines.
// Do not modify this routine!!
/////////////////////////////////////////////////////////////////////////////////////////


void iinstrument(INS ins)
{

  if (!g_enable_instrument)
     return;



  set<LEVEL_BASE::REG> src_regs;
  set<LEVEL_BASE::REG> dst_regs;
  const ADDRINT iaddr = INS_Address(ins);
  Inst_info *info = new Inst_info;
  memset(info,0,sizeof(*info));

  // cout<<"Initialize: num_read_regs: "<<(uint32_t) info->num_read_regs<<endl;
  src_regs.clear();
  dst_regs.clear();

  // ----------------------------------------
  // check all operands
  // ----------------------------------------
  for (UINT32 ii = 0; ii < INS_OperandCount(ins); ++ii) {
    LEVEL_BASE::REG reg;
    // ----------------------------------------
    // operand - Register
    // add source and destination registers
    // ----------------------------------------
    if (INS_OperandIsReg(ins, ii)) {
      reg = INS_OperandReg(ins, ii);

      if (INS_OperandRead(ins,ii)) {
        src_regs.insert(reg);
      }
      if (INS_OperandWritten(ins,ii)) {
        dst_regs.insert(reg);
      }
    }
    // ----------------------------------------
    // operand - Memory
    // ----------------------------------------
    else if (INS_OperandIsMemory(ins, ii) || INS_OperandIsAddressGenerator(ins, ii)) {
      // ----------------------------------------
      // Add base, index, or segment registers if exists
      // ----------------------------------------
      reg = INS_OperandMemoryBaseReg(ins, ii);
      if (reg) {
        src_regs.insert(reg);
      }

      reg = INS_OperandMemoryIndexReg(ins, ii);
      if (reg) {
        src_regs.insert(reg);
      }

      reg = INS_OperandMemorySegmentReg(ins, ii);
      if (reg) {
        src_regs.insert(reg);
      }
    }
  }

  // ----------------------------------------
  // handle source registers
  // ----------------------------------------
  if (!src_regs.empty()) {
    info->num_read_regs = src_regs.size();
    // cout<<"Storing: num_read_regs: "<<src_regs.size()<<endl;
    assert(info->num_read_regs < MAX_SRC_NUM);

    set<LEVEL_BASE::REG>::iterator begin(src_regs.begin()), end(src_regs.end());
    uint8_t *ptr = info->src;
    while (begin != end) {
      if (*begin >= LEVEL_BASE::REG_PIN_EDI && *begin <= LEVEL_BASE::REG_LAST) {
        cerr << "PIN LEVEL_BASE::REGISTER!! : " << *begin << endl;
        assert(0);
      }

      if (LEVEL_BASE::REG_is_fr(*begin))
        info->is_fp = TRUE;

      *ptr = REG_FullRegName(*begin);
      ++ptr;
      ++begin;
    }
  }


  // ----------------------------------------
  // destination registers
  // ----------------------------------------
  if (!dst_regs.empty()) {
    info->num_dest_regs=dst_regs.size();
    assert(info->num_dest_regs < MAX_DST_NUM);
    set<LEVEL_BASE::REG>::iterator begin(dst_regs.begin()), end(dst_regs.end());
    uint8_t *ptr = info->dst;
    while (begin != end) {
      if (*begin >= LEVEL_BASE::REG_PIN_EDI && *begin <= LEVEL_BASE::REG_LAST) {
        cerr << "PIN LEVEL_BASE::REGISTER!! : " << *begin << endl;
        assert(0);
      }

      if (*begin == LEVEL_BASE::REG_EFLAGS)
        info->write_flg = 1;

      if (LEVEL_BASE::REG_is_fr(*begin))
        info->is_fp = 1;

      *ptr = REG_FullRegName(*begin);
      ++ptr;
      ++begin;
    }
  }

  // ----------------------------------------
  // instruction size
  // ----------------------------------------
  info->size = INS_Size(ins);

  // ----------------------------------------
  // PC address
  // ----------------------------------------
  info->instruction_addr = iaddr;



  // ----------------------------------------
  // set the opcode
  // ----------------------------------------
  if (OPCODE_StringShort(INS_Opcode(ins)).find("NOP") != string::npos) {
    info->opcode = TR_NOP;
  }
  // ----------------------------------------
  // opcode : multiply
  // ----------------------------------------
  else if (OPCODE_StringShort(INS_Opcode(ins)).find("MUL") != string::npos) {
    if (INS_Opcode(ins) == XED_ICLASS_IMUL || INS_Opcode(ins) == XED_ICLASS_MUL) {
      info->opcode = TR_MUL;
    } else {
      info->opcode = TR_FMUL;
    }
  }
  // ----------------------------------------
  // opcode : multiply
  // ----------------------------------------
  else if (OPCODE_StringShort(INS_Opcode(ins)).find("DIV") != string::npos) {
    if (INS_Opcode(ins) == XED_ICLASS_DIV || INS_Opcode(ins) == XED_ICLASS_IDIV) {
      info->opcode = TR_DIV;
    } else {
      info->opcode = TR_FDIV;
    }
  }
  // ----------------------------------------
  // opcode : prefetch
  // ----------------------------------------
  else if (INS_Opcode(ins) == XED_ICLASS_PREFETCHNTA) {
    info->opcode = PREFETCH_NTA;
    // assert(0 && "PREFTCH_NTA");
  }
  else if (INS_Opcode(ins) == XED_ICLASS_PREFETCHT0) {
    info->opcode = PREFETCH_T0;
    // assert(0 && "PREFTCH_T0");
  }
  else if (INS_Opcode(ins) == XED_ICLASS_PREFETCHT1) {
    info->opcode = PREFETCH_T1;
    // assert(0 && "PREFTCH_T1");
  }
  else if (INS_Opcode(ins) == XED_ICLASS_PREFETCHT2) {
    info->opcode = PREFETCH_T2;
    // assert(0 && "PREFTCH_T2");
  }
  // ----------------------------------------
  // opcode : others
  // ----------------------------------------
  else {
    info->opcode = (uint8_t) (INS_Category(ins));
  }


  // ----------------------------------------
  // SSE, AVX (Vector) instruction
  // ----------------------------------------
  if (INS_Category(ins) == XED_CATEGORY_AVX ||
      INS_Category(ins) == XED_CATEGORY_FCMOV ||
      INS_Category(ins) == XED_CATEGORY_X87_ALU ||
      INS_Category(ins) == XED_CATEGORY_MMX ||
      INS_Category(ins) == XED_CATEGORY_SSE) {
    info->is_fp = 1;
  }


  // ----------------------------------------
  // Branch instruction - set branch type
  // ----------------------------------------
  if (INS_IsIndirectBranchOrCall(ins) && !INS_IsRet(ins) && !INS_IsInterrupt(ins)) {
    /**< Indirect branch */
    if (INS_Category(ins)  == XED_CATEGORY_UNCOND_BR)
      info->cf_type = CF_IBR;
    else if (INS_Category(ins)  == XED_CATEGORY_COND_BR)
      info->cf_type = CF_ICBR;
    else if (INS_Category(ins) == XED_CATEGORY_CALL)
      info->cf_type = CF_ICALL;
  }
  else if(INS_IsDirectBranchOrCall(ins) && !INS_IsInterrupt(ins)) {
    /**< Direct branch */
    if (INS_Category(ins) == XED_CATEGORY_UNCOND_BR)
      info->cf_type = CF_BR;
    else if (INS_Category(ins)  == XED_CATEGORY_COND_BR)
      info->cf_type = CF_CBR;
    else if (INS_Category(ins) == XED_CATEGORY_CALL)
      info->cf_type = CF_CALL;
    info->branch_target = INS_DirectBranchOrCallTargetAddress(ins);
  }
  else if (INS_IsRet(ins)) {
    info->cf_type = CF_RET;
  }
  else if (INS_IsInterrupt(ins)) {
    info->cf_type = CF_ICO;
  }
  else {
    info->cf_type = NOT_CF;
  }


  // ----------------------------------------
  // Load instruction
  // ----------------------------------------
  if (INS_IsMemoryRead(ins)) {
    bool stackOp = false;
    if(INS_IsStackRead(ins))
    {
      stackOp = true;
      // cout<<"stack read \n";
    }



    // 2 memory loads
    if (INS_HasMemoryRead2(ins)) {
      info->num_ld = 2;
      INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)get_ld_ea2, IARG_MEMORYREAD_EA, IARG_MEMORYREAD2_EA, 
#ifndef PINLINUX
          IARG_LEVEL_BASE::REG_VALUE, LEVEL_BASE::REG_EFLAGS,
#endif

  IARG_MEMORYREAD_SIZE, IARG_END);
      assert(!INS_IsStackRead(ins));
    }
    // 1 memory load
    else {
      info->num_ld = 1;
      INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)get_ld_ea, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE,
#ifndef PINLINUX
          IARG_LEVEL_BASE::REG_VALUE, LEVEL_BASE::REG_EFLAGS,
#endif

          IARG_BOOL,stackOp, IARG_END);
    }



  }


  // ----------------------------------------
  // Store instruction
  // ----------------------------------------
  if (INS_IsMemoryWrite(ins)) {
    bool stackOp = false;
    if(INS_IsStackWrite(ins))
    {
      stackOp = true;
      // cout<<"stack write \n";


    }



    info->has_st = 1;
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)get_st_ea, IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE,
#ifndef PINLINUX
        IARG_LEVEL_BASE::REG_VALUE, LEVEL_BASE::REG_EFLAGS,
#endif

        IARG_BOOL, stackOp, IARG_END);
  }

  if (g_ins_buffer.find(iaddr) == g_ins_buffer.end()) { 
      // g_ins_buffer[iaddr] = info;
      g_ins_buffer.insert(std::make_pair<ADDRINT,Inst_info*>(iaddr,info));
      // cout<<"info->num_read_regs: "<< (uint32_t) info->num_read_regs <<endl;
  }

  // ----------------------------------------
  // Branch instruction
  // ----------------------------------------
  if (info->cf_type) {
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)get_target, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
  }


  // ----------------------------------------
  // Write an instruction to buffer
  // nav
  // if -- offload_func is true write into BUFFER B, else WRITE in BUFFER A
  // ----------------------------------------
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)write_inst_frm_ins_buffer, IARG_INST_PTR, IARG_END);
  //
  //
  // ----------------------------------------
  // Dump out an instruction
  // ----------------------------------------
  // if (print_inst) {
  //   INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)dprint_inst, IARG_INST_PTR, IARG_PTR, new string(INS_Disassemble(ins)), IARG_END);
  // }

}

/////////////////////////////////////////////////////////////////////////////////////////
// 
// 
//
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// Get branch target address : instrumentation function
/////////////////////////////////////////////////////////////////////////////////////////
void get_target(ADDRINT target, bool taken)
{

  Trace_info* trace_info = trace_info_array;

  trace_info->target         = target;
  trace_info->actually_taken = taken;
}



/////////////////////////////////////////////////////////////////////////////////////////
// preprare instruction to write in trace
/////////////////////////////////////////////////////////////////////////////////////////


void prepare_inst(ADDRINT iaddr)
{




}


/////////////////////////////////////////////////////////////////////////////////////////
// Write an instruction to the buffer : instrumentation function
/////////////////////////////////////////////////////////////////////////////////////////
void write_inst_frm_ins_buffer(ADDRINT iaddr)
{
  // cout<<"WriteInst:\n";


  Trace_info* trace_info = trace_info_array;

  if(!g_enable_instrument)
  {
    // If instrument not enabled, clean up residual info and return
    trace_info->vaddr1         = 0;
    trace_info->vaddr2         = 0;
    trace_info->st_vaddr       = 0;
    trace_info->target         = 0;
    trace_info->actually_taken = 0;
    trace_info->mem_read_size  = 0;
    trace_info->mem_write_size = 0;
    trace_info->eflags         = 0;
    return;
  }



  if (g_ins_buffer.find(iaddr) == g_ins_buffer.end()) { 
    cout<<"--------------------- EROR ------------------------\n";
    assert(0);
  }

  Inst_info* static_inst = g_ins_buffer[iaddr];
  // cout<<"static_inst:num_read_regs:"<<dec <<(uint32_t) static_inst->num_read_regs<<endl;
 
    //can get rid of this memcpy by directly copying to buffer..
    memcpy(&trace_info->inst_info, static_inst, sizeof(Inst_info));
    //
    Inst_info *i_info = &trace_info->inst_info;

    i_info->ld_vaddr1      = trace_info->vaddr1;
    i_info->ld_vaddr2      = trace_info->vaddr2;
    i_info->st_vaddr       = trace_info->st_vaddr;
    i_info->branch_target  = trace_info->target;
    i_info->actually_taken = trace_info->actually_taken;
    i_info->mem_read_size  = trace_info->mem_read_size;
    i_info->mem_write_size = trace_info->mem_write_size;
    i_info->rep_dir        = (trace_info->eflags & (0x400)) >> 10;

    //nav
    //CPU Trace
    i_info->thread_id = 0;
    i_info->ds_flag   = false;

    

    //reset trace_info
    trace_info->vaddr1         = 0;
    trace_info->vaddr2         = 0;
    trace_info->st_vaddr       = 0;
    trace_info->target         = 0;
    trace_info->actually_taken = 0;
    trace_info->mem_read_size  = 0;
    trace_info->mem_write_size = 0;
    trace_info->eflags         = 0;

   
}


void write_inst_debugfile(Trace_info* trace_info,Inst_info* i_info)
{

  Inst_info *t_info = i_info;

  *(trace_info->debug_stream) << "\nInstruction count: " << g_inst_count << endl;
  *(trace_info->debug_stream) << "*** beginning of the data strcture *** " <<endl;

  *(trace_info->debug_stream) << "t_info->ds_flag " <<(bool) t_info->ds_flag  << endl;
  *(trace_info->debug_stream) << "t_info->chunk_id " <<(uint64_t) t_info->chunk_id  << endl;

  *(trace_info->debug_stream) << "t_info->uop_opcode no " <<(uint32_t) t_info->opcode << endl;
  // *(trace_info->debug_stream) << "t_info->uop_opcode " <<tr_opcode_names[(uint32_t) t_info->opcode]  << endl;
  *(trace_info->debug_stream) << "t_info->num_read_regs: " << (uint32_t) t_info->num_read_regs << endl;
  // *(trace_info->debug_stream) << "t_info->num_read_regs: " << hex <<  (uint8_t) t_info->num_read_regs << endl;
  *(trace_info->debug_stream) << "t_info->num_dest_regs: " << hex << (uint32_t) t_info->num_dest_regs << endl;
  for (UINT32 ii = 0; ii < 4; ++ii) {
    if (t_info->src[ii] != 0) {
      *(trace_info->debug_stream) << "t_info->src" << ii << ": " << hex << LEVEL_BASE::REG_StringShort(static_cast<LEVEL_BASE::REG>(t_info->src[ii])) << endl;
    }
  }

  for (UINT32 ii = 0; ii < 4; ++ii) {
    if (t_info->dst[ii] != 0) {
      *(trace_info->debug_stream) << "t_info->src" << ii << ": " << hex << LEVEL_BASE::REG_StringShort(static_cast<LEVEL_BASE::REG>(t_info->dst[ii])) << endl;
    }
  }

  *(trace_info->debug_stream) << "t_info->cf_type: " << hex << tr_cf_names[(uint32_t) t_info->cf_type] << endl;
  *(trace_info->debug_stream) << "t_info->has_immediate: " << hex << (uint32_t) t_info->has_immediate << endl;
  *(trace_info->debug_stream) << "t_info->r_dir:" << (uint32_t) t_info->rep_dir << endl;
  *(trace_info->debug_stream) << "t_info->has_st: " << hex << (uint32_t) t_info->has_st << endl;
  *(trace_info->debug_stream) << "t_info->num_ld: " << hex << (uint32_t) t_info->num_ld << endl;
  *(trace_info->debug_stream) << "t_info->mem_read_size: " << hex << (uint32_t) t_info->mem_read_size << endl;
  *(trace_info->debug_stream) << "t_info->mem_write_size: " << hex << (uint32_t) t_info->mem_write_size << endl;
  *(trace_info->debug_stream) << "t_info->is_fp: " << (uint32_t) t_info->is_fp << endl;
  *(trace_info->debug_stream) << "t_info->ld_vaddr1: " << hex << (uint64_t) t_info->ld_vaddr1 << endl;
  *(trace_info->debug_stream) << "t_info->ld_vaddr2: " << hex << (uint64_t) t_info->ld_vaddr2 << endl;
  *(trace_info->debug_stream) << "t_info->st_vaddr: " << hex << (uint64_t) t_info->st_vaddr << endl;
  *(trace_info->debug_stream) << "t_info->instruction_addr: " << hex << (uint64_t) t_info->instruction_addr << endl;
  *(trace_info->debug_stream) << "t_info->branch_target: " << hex << (uint64_t) t_info->branch_target << endl;
  *(trace_info->debug_stream) << "t_info->actually_taken: " << hex << (uint32_t) t_info->actually_taken << endl;
  *(trace_info->debug_stream) << "t_info->write_flg: " << hex << (uint32_t) t_info->write_flg << endl;
  *(trace_info->debug_stream) << "*** end of the data strcture *** " << endl;
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
    // std::cout<<"ROI FUNC CALLED \n ";
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

  //---------- Instrument Ld and St function 
  RTN Ld_funcRtn = RTN_FindByName(img, LDFUNC);
  if (RTN_Valid(Ld_funcRtn))
  {
    RTN_Open(Ld_funcRtn);

    RTN_InsertCall(Ld_funcRtn, IPOINT_BEFORE, (AFUNPTR)mem_func_ins_LD,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
        IARG_END);

    RTN_Close(Ld_funcRtn);
  }

  //---------- Instrument Ld and St function 
  RTN St_funcRtn = RTN_FindByName(img, STFUNC);
  if (RTN_Valid(St_funcRtn))
  {
    RTN_Open(St_funcRtn);

    RTN_InsertCall(St_funcRtn, IPOINT_BEFORE, (AFUNPTR)mem_func_ins_ST,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
        IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
        IARG_END);

    RTN_Close(St_funcRtn);
  }



  cout << "Image Instrumentation\n";



}

/* ===================================================================== */

VOID Fini(INT32 code, VOID *v)
{
  cout<<"Total Iterations:"<<iteration_no<<"\n";
  cout<<"Exiting ...\n";
  delete  trace_info_array;
  TraceFile.close();
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



/////////////////////////////////////////////////////////////////////////////////////////
/// Sanity check
/////////////////////////////////////////////////////////////////////////////////////////
void sanity_check(void)
{
  // ----------------------------------------
  // check whether there has been changes in the pin XED enumerators
  // ----------------------------------------
  for (int ii = 0; ii < XED_CATEGORY_LAST; ++ii) {
    if (tr_opcode_names[ii] != CATEGORY_StringShort(ii)) {
      cout << ii << " " << tr_opcode_names[ii] << " " << CATEGORY_StringShort(ii) << "\n";
      cout << "-> Changes in XED_CATEGORY!\n";
      exit(0);
    }
  }
}

void initialize(void)
{

  Trace_info* trace_info = new Trace_info;

  if (trace_info == NULL) {
    cerr << "could not allocate memory\n";
    return;
  }

  //initialize trace_info with all 0s
  memset(trace_info, 0, sizeof(Trace_info));

  // CPU Trace fields 
  stringstream sstream;
  sstream << Knob_trace_name.Value() << "_0.raw";
	string file_name;
	sstream  >> file_name;
	cout<<"print  tid : "<<file_name <<endl;

  std::ofstream outfile(file_name.c_str(), std::ios::out);

	if(!open(file_name.c_str(), std::fstream::out | std::fstream::app | std::fstream::trunc) )
	{
		perror("Error creating file for writing: ");
    exit(-1);
  }

  int trace_stream = -1;
  trace_stream = open(file_name.c_str(),  O_WRONLY );


  // DEBUG
  // char debug_file_name[256] = {'\0'};

  // ofstream* debug_stream = NULL;
  // sprintf(debug_file_name, "%s_0.dump", Knob_dump_file.Value().c_str());
  // debug_stream = new ofstream(debug_file_name);


  trace_info->trace_stream = trace_stream;
  trace_info->bytes_accumulated = 0;
  trace_info->inst_count = 0;

  // if (Knob_inst_dump.Value()) {
  //   trace_info->debug_stream = debug_stream;
  // }

  trace_info_array = trace_info;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char *argv[])
{

  //if( argc < 3 )
  //{
    //cerr << "Binary Arg1:Longest Function Arg2:Offloaded function" << endl;
    //exit(0);
  //}

  //f << argv[argc-2];
  //ROI_FUNC= ;
  ROI_FUNC = new char[STRING_SIZE];
  OFFLOAD_FUNC = new char[STRING_SIZE];
  
  //f << argv[argc-1];
  // ROI_FUNC = argv[argc-2];
  //OFFLOAD_FUNC = argv[argc-1];
  //cout<<"argc:"<<argc<<endl;
    std::ifstream temp_i_file("function.txt", ios::in);
    assert(temp_i_file.is_open() && "ERROR: WE COULDN'T FIND THE FILE!!!");

    int i = 0;
    for( std::string line; getline(temp_i_file, line); ){
        if(i == 0)
            line.copy(ROI_FUNC, line.size(), 0);
        else if (i == 1)
            line.copy(OFFLOAD_FUNC, line.size(), 0);
        else
            return 1;
        i++;
    }


  cout<<"ROI_FUNC:"<<ROI_FUNC<<"\t OFFLOAD_FUNC:"<<OFFLOAD_FUNC<<endl;


  // Initialize pin & symbol manager
  PIN_InitSymbols();
  if( PIN_Init(argc,argv) )
  {
    return Usage();
  }


  // Write to a file since cout and cerr maybe closed by the application
  TraceFile.open(KnobOutputFile.Value().c_str());
  TraceFile << hex;
  TraceFile.setf(ios::showbase);
  initialize();
  sanity_check();
  print_inst = Knob_print_inst.Value();
  // Register Image to be called to instrument functions.
  IMG_AddInstrumentFunction(Image, 0);
  PIN_AddFiniFunction(Fini, 0);

  INS_AddInstrumentFunction(IInstruction, 0);
  // Never returns
  PIN_StartProgram();

  return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
