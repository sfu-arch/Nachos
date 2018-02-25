/*
   Copyright (c) <2012>, <Georgia Institute of Technology> All rights reserved.

   Redistribution and use in source and binary forms, with or without modification, are permitted
   provided that the following conditions are met:

   Redistributions of source code must retain the above copyright notice, this list of conditions
   and the following disclaimer.

   Redistributions in binary form must reproduce the above copyright notice, this list of
   conditions and the following disclaimer in the documentation and/or other materials provided
   with the distribution.

   Neither the name of the <Georgia Institue of Technology> nor the names of its contributors
   may be used to endorse or promote products derived from this software without specific prior
   written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   */

//TODO : unless an empty trace_0.raw etc is created -- instructions are not written into the file -- need to modify

#include <iostream>
#include <map>
#include <set>
#include <assert.h>
#include <zlib.h>
#include "DSTrace.h"
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <cmath>
#include <queue>
#include <list>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <set>
#include <cinttypes>
/*
 * 1. all function of which name start with a capatal letter is used by PIN
 */

#include "pin.H"
#include "control_manager.H"
#include "portability.H"
#include <iomanip>

using namespace std;
using namespace CONTROLLER;


#if defined(TARGET_LINUX)
#define PINLINUX
#define WRITEM "w"
#else
#define WRITEM "wb"
#endif


#define addr_t ADDRINT
#define ctr_t UINT64
#define uns UINT32

#define DUMMY_THREAD 100000

#define THREAD_ENABLE_CHECK(tid) \
  if ((tid) == DUMMY_THREAD) \
return ; \
\
if (!g_enable_thread_instrument[(tid)]) \
return ;

//# of entries for a 32 bit address to fill a 8KB scratchpad
#define SP_ENTRIES  2048
////////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory Instruction Data type
////////////////////////////////////////////////////////////////////////////////////////////////////////

struct inst_t {
  addr_t pc;
  REG base;
  REG index;
  REG dest;
  INT64 displacement;
  UINT32 scale;
  UINT32 size;
  string assem;
};

map<ADDRINT, inst_t *> g_inst_map;


/////////////////////////////////////////////////////////////////////////////////////////
/// global variables
/////////////////////////////////////////////////////////////////////////////////////////

set<uint32_t> unique_addr_set;
Trace_info* trace_info_array[MAX_THREADS];
// map<ADDRINT, Inst_info*> g_inst_storage[MAX_THREADS];
PIN_LOCK g_lock;
UINT64 g_inst_count[MAX_THREADS]={0};
INT64 g_start_inst_count[MAX_THREADS] = {-1};
map<THREADID, THREADID> threadMap;
THREADID main_thread_id;
Thread_info thread_info[MAX_THREADS];
UINT64 ds_inst_count = 0;
UINT64 detach_inst = 0;
bool print_inst=false;
UINT32 thread_count = 0;
UINT32 ThreadArrivalCount = 0;
unsigned int g_inst_print_count[MAX_THREADS];
unsigned int func_count = 0;
CONTROL_MANAGER control;
bool g_enable_thread_instrument[MAX_THREADS];
bool g_enable_chunk[MAX_THREADS];
bool g_enable_instrument = false;
ofstream ds_data_file;
ofstream memtrace;
UINT32  inst_between_markers [MAX_THREADS];
//REMEMBER: numTraces is not including main thread
int numTraces = 0;
/////////////////////////////////////////////////////////////////////////////////////////
// knob variables (Knob_xxx_yyy)
/////////////////////////////////////////////////////////////////////////////////////////
KNOB<string> Knob_trace_name        (KNOB_MODE_WRITEONCE, "pintool", "tracename", "trace",  "trace output filename");
KNOB<bool>   Knob_print_inst        (KNOB_MODE_WRITEONCE, "pintool", "print_inst", "0", "dumping trace in the stdout" );
KNOB<UINT64> Knob_detach_inst       (KNOB_MODE_WRITEONCE, "pintool", "detach", "0", "detach pintool after n instructions");
KNOB<bool>   Knob_inst_dump         (KNOB_MODE_WRITEONCE, "pintool", "dump", "1", "Dump t_info to the file");
KNOB<UINT32> Knob_dump_max          (KNOB_MODE_WRITEONCE, "pintool", "dump_max", "1100", "Number of t_info to dump");
KNOB<string> Knob_dump_file         (KNOB_MODE_WRITEONCE, "pintool", "dump_file", "dump.txt", "");
KNOB<UINT32> Knob_num_thread        (KNOB_MODE_WRITEONCE, "pintool", "thread", "1", "Total number of threads to gather information");
KNOB<string> Knob_compiler          (KNOB_MODE_WRITEONCE, "pintool", "compiler", "gcc", "Which compiler was used?");
KNOB<string> Knob_pl                (KNOB_MODE_WRITEONCE, "pintool", "pl", "normal", "Programming Language");
KNOB<bool>   Knob_mem_access_trace  (KNOB_MODE_WRITEONCE, "pintool", "memtrace", "0", "Log all mem accesses");


////////////////////////////////////////////////////////////////////////////////////////////////////////
// function declaration
////////////////////////////////////////////////////////////////////////////////////////////////////////

void check_n_write_marker(uint32_t addr, Trace_info* trace_info);
void Finish(void);
void Initialize(void);
void sanity_check(void);
VOID ImageLoad(IMG img,  VOID *v);
VOID EnterROI();
VOID ExitROI();
VOID ExitROIDetach();

VOID Chunk_Begin(UINT32 chunk_id,
    UINT32 app_tid,
    UINT32 core_id,
    THREADID thread_id);

VOID Chunk_End(UINT32 chunk_id,
    UINT32 app_tid,
    UINT32 core_id,
    THREADID thread_id);


void dump_to_file(ofstream* file, Inst_info *t_info);
void  write_chunk_marker(THREADID tid, Inst_info* m, bool is_begin);







////////////////////////////////////////////////////////////////////////////////////////////////////////
// control handler for pinpoint (simpoint)
////////////////////////////////////////////////////////////////////////////////////////////////////////

// void Handler(CONTROL_EVENT ev, void * v, CONTEXT * ctxt, void * ip, THREADID tid)
LOCALFUN VOID Handler(EVENT_TYPE ev, void * v, CONTEXT * ctxt, void * ip, THREADID tid, bool bcast)
{

  switch (ev) {
    case EVENT_START: {
                          cerr << "-> CONTROL START " << g_inst_count[tid] << endl;
                          break;
                        }
    case EVENT_STOP: {
                         cerr << "-> Trace Generation Done at icount " << g_inst_count[tid] << endl;
                         g_enable_instrument = false;
                         PIN_Detach();
                         break;
                       }
    default: {
               ASSERTX(false);
               break;
             }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Insert if unique Ld/St address and flush if size is threshold 
////////////////////////////////////////////////////////////////////////////////////////////////////////
void check_n_write_marker(uint32_t addr, Trace_info* trace_info)
{
  uint32_t line_addr = (addr/100)*100;
  // line_addr<<2;
  auto p = unique_addr_set.insert(line_addr);
  //it was a unique value
  if(p.second)
  {
    if(unique_addr_set.size() > SP_ENTRIES) 
    {
      unique_addr_set.clear();
      //insert a marker here
      cout<<"clear set and insert marker \n";
      unique_addr_set.insert(line_addr);

      THREADID tid = threadMap[PIN_ThreadId()];
      Inst_info* m = (Inst_info*)memset((void *)new Inst_info, 0, sizeof(Inst_info));

      m->thread_id = tid + numTraces;
      m->opcode = TR_NOP; 
      //signifies that scratchpad 8k is full and need to refill it -- it will be added in the acc trace
      m->ds_flag = false;
      m->is_chunk_begin = true;
      
      delete m;

    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Increment the number of instructions
////////////////////////////////////////////////////////////////////////////////////////////////////////
void IncrementNumInstruction(THREADID threadid)
{

  THREADID tid = threadMap[PIN_ThreadId()];

  if (tid == MAX_THREADS)
    return ;


  g_inst_count[tid]++;

  if (!g_enable_thread_instrument[tid])
    return ;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////
// Detach function
////////////////////////////////////////////////////////////////////////////////////////////////////////
void Detach(void *v)
{
  cout << "Detaching..." << endl;
  thread_end(0);
  finish();
}


#ifdef PINLINUX
uint32_t eflag_value=0;
#endif


/////////////////////////////////////////////////////////////////////////////////////////
// get an effetive load address : instrumentation function
/////////////////////////////////////////////////////////////////////////////////////////
void get_ld_ea(ADDRINT addr, UINT32 mem_read_size,
#ifndef PINLINUX
    UINT32 eflag_value,
#endif
    THREADID threadid, BOOL stackOp)
{
  THREADID tid = threadMap[threadid];
  THREAD_ENABLE_CHECK(tid);

  Trace_info* trace_info = trace_info_array[tid];
  trace_info->vaddr1        = addr;
  trace_info->mem_read_size = mem_read_size;
  trace_info->eflags        = eflag_value;
  trace_info->actually_taken = stackOp;
  //-----------------
  if(!stackOp)
  {
    check_n_write_marker(addr,trace_info);

  }
}


/////////////////////////////////////////////////////////////////////////////////////////
// Get effective load addresses : instrumentation function
// Insturction that has 2 load addresses
/////////////////////////////////////////////////////////////////////////////////////////
void get_ld_ea2(ADDRINT addr1, ADDRINT addr2, UINT32 mem_read_size,
#ifndef PINLINUX
    UINT32 eflag_value,
#endif
    THREADID threadid)
{
  THREADID tid = threadMap[threadid];
  THREAD_ENABLE_CHECK(tid);

  Trace_info* trace_info = trace_info_array[tid];
  trace_info->vaddr1        = addr1;
  trace_info->vaddr2        = addr2;
  trace_info->mem_read_size = mem_read_size;
  trace_info->eflags        = eflag_value;
  //-----------------
  // Cannot be a stack Load
  check_n_write_marker(addr1,trace_info);
  check_n_write_marker(addr2,trace_info);
}



/////////////////////////////////////////////////////////////////////////////////////////
// Get store address : instrumentation function
/////////////////////////////////////////////////////////////////////////////////////////
void get_st_ea(ADDRINT addr, UINT32 mem_st_size,
#ifndef PINLINUX
    UINT32 eflag_value,
#endif
    THREADID threadid, BOOL stackOp)
{
  THREADID tid = threadMap[threadid];
  THREAD_ENABLE_CHECK(tid);

  Trace_info* trace_info = trace_info_array[tid];

  trace_info->st_vaddr       = addr;
  trace_info->mem_write_size = mem_st_size;
  trace_info->eflags         = eflag_value;
  trace_info->actually_taken = stackOp;
  if(!stackOp)
    check_n_write_marker(addr,trace_info);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Get branch target address : instrumentation function
/////////////////////////////////////////////////////////////////////////////////////////
void get_target(ADDRINT target, bool taken, THREADID threadid)
{
  THREADID tid = threadMap[threadid];
  THREAD_ENABLE_CHECK(tid);

  Trace_info* trace_info = trace_info_array[tid];

  trace_info->target         = target;
  trace_info->actually_taken = taken;
}



/////////////////////////////////////////////////////////////////////////////////////////
// Write an instruction to the buffer : instrumentation function
/////////////////////////////////////////////////////////////////////////////////////////
void write_inst(ADDRINT iaddr, THREADID threadid)
{
  // cout<<"WriteInst:\n";

  THREADID tid = threadMap[threadid];
  THREAD_ENABLE_CHECK(tid);

  Trace_info* trace_info = trace_info_array[tid];

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

  // Inst_info* static_inst = g_inst_storage[tid][iaddr];

  //can get rid of this memcpy by directly copying to buffer..
  // memcpy(&trace_info->inst_info, static_inst, sizeof(Inst_info));
  //
  Inst_info *t_info = &trace_info->inst_info;

  t_info->ld_vaddr1      = trace_info->vaddr1;
  t_info->ld_vaddr2      = trace_info->vaddr2;
  t_info->st_vaddr       = trace_info->st_vaddr;
  t_info->branch_target  = trace_info->target;
  t_info->actually_taken = trace_info->actually_taken;
  t_info->mem_read_size  = trace_info->mem_read_size;
  t_info->mem_write_size = trace_info->mem_write_size;
  t_info->rep_dir        = (trace_info->eflags & (0x400)) >> 10;

  trace_info->vaddr1         = 0;
  trace_info->vaddr2         = 0;
  trace_info->st_vaddr       = 0;
  trace_info->target         = 0;
  trace_info->actually_taken = 0;
  trace_info->mem_read_size  = 0;
  trace_info->mem_write_size = 0;
  trace_info->eflags         = 0;


  //nav
  //CPU Trace
  t_info->thread_id = tid;
  t_info->ds_flag   = false;

//FIXME: ERROR
  memcpy(trace_info->trace_buf + trace_info->bytes_accumulated, t_info, sizeof(Inst_info));
  trace_info->bytes_accumulated += sizeof(Inst_info);


//---------------------------------------
  // // FIXME Add a NOP instruction - just once in the acc trace corresponding to main thread 
  // //- since there will be nothing written in it and an empty trace is an issue
  // //Currently it will print a lot of nops -- need to change it to just 1 nop
  // if(!g_enable_chunk[tid] && tid == 0 ) {
  //   Inst_info* m = (Inst_info*)memset((void *)new Inst_info, 0, sizeof(Inst_info));
  //
  //   m->thread_id = tid + numTraces;
  //   m->opcode = TR_NOP; 
  //   int write_size = write(trace_info->trace_stream_acc, m, BUF_SIZE);
  //
  //   if (write_size != cout.good()) {
  //     cerr << "-> TID - " << tid+numTraces << " Error when writing instruction " << trace_info->inst_count << " write_size:" << write_size << " " << BUF_SIZE << endl;
  //     perror("Write failed: ");
  //     return;
  //   }
  // }



  //---------------------------------------

  // ----------------------------------------
  // once accumulated instructions exceed the buffer size, write instructions to the file
  // ----------------------------------------

  if (trace_info->bytes_accumulated == BUF_SIZE) {

    // int write_size = 0;

    //Write in CPU Trace
    if(!g_enable_chunk[tid] ) {

      cout<<"Writing in buffer\n";

      if(!write(trace_info->trace_stream, trace_info->trace_buf, BUF_SIZE))
      {
        cerr << "CPU THREAD: -> TID: " << tid 
        << " Error when writing instruction " << trace_info->inst_count 
        // << " write_size: "              << write_size 
        << "BUF_SIZE:     "             << BUF_SIZE 
        <<"SIZE of trace_stream:   "    << sizeof(trace_info->trace_stream)
        // <<"SIZE of trace_stream_acc:  " << sizeof(trace_info->trace_stream_acc)
        << endl;
      perror("Write failed: ");
      return;
      }
    }

    inst_between_markers[tid]++;


    trace_info->bytes_accumulated = 0;
  }

  // ----------------------------------------
  // dump out an instruction to the text file
  // ----------------------------------------
  if (Knob_inst_dump.Value()) {
    //To Verify: if chunk_enable is false it should not contain any marker 
    if(!g_enable_chunk[tid]  && t_info->ds_flag) {
      dump_to_file(trace_info->debug_stream, t_info);
      cout<<"enable chunk true -> tid:"<<tid<<" \n" <<t_info;
      }
  }

  // PIN_GetLock(&g_lock, threadid);
  trace_info->inst_count++;
  // PIN_ReleaseLock(&g_lock);
  // ----------------------------------------
  // detach : instruction counts exceed the limit
  // can we replace g_inst_count with trace_info->inst_count??
  // ----------------------------------------
  // if (detach_inst != 0 ) {
  //   cout << "Detaching after " << g_inst_count[tid] << " instructions." << endl;
  //   PIN_Detach();
  // }
}



/////////////////////////////////////////////////////////////////////////////////////////
// Main instrumentation routine
// Depending on the type of an instruction, call corresponding subroutines.
// Do not modify this routine!!
/////////////////////////////////////////////////////////////////////////////////////////
void iinstrument(INS ins)
{
  THREADID tid = threadMap[PIN_ThreadId()];
  // if (tid == 100000)
  if(tid== MAX_THREADS)
    return ;

  if (!g_enable_thread_instrument[tid])
    return;



  set<LEVEL_BASE::REG> src_regs;
  set<LEVEL_BASE::REG> dst_regs;
  const ADDRINT iaddr = INS_Address(ins);
  Inst_info *info = new Inst_info;
  memset(info,0,sizeof(*info));

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
      INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)get_ld_ea2, IARG_MEMORYREAD_EA, IARG_MEMORYREAD2_EA, IARG_MEMORYREAD_SIZE,
#ifndef PINLINUX
          IARG_LEVEL_BASE::REG_VALUE, LEVEL_BASE::REG_EFLAGS,
#endif
          IARG_THREAD_ID, IARG_END);
      assert(!INS_IsStackRead(ins));
    }
    // 1 memory load
    else {
      info->num_ld = 1;
      INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)get_ld_ea, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE,
#ifndef PINLINUX
          IARG_LEVEL_BASE::REG_VALUE, LEVEL_BASE::REG_EFLAGS,
#endif
          IARG_THREAD_ID, IARG_BOOL,stackOp, IARG_END);
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
        IARG_THREAD_ID, IARG_BOOL, stackOp, IARG_END);
  }


  //if(iaddr == 0x80484bc)
  //{
  //cerr << "OpCode: " << (uint32_t) info->opcode << endl;
  //}

  // ----------------------------------------
  // add a static instruction (per thread id)
  // ----------------------------------------
  // if (g_inst_storage[tid].find(iaddr) == g_inst_storage[tid].end()) {
  //   for (UINT32 ii = 0; ii < Knob_num_thread.Value(); ++ii) {
  //     g_inst_storage[ii][iaddr] = info;
  //   }
  // }


  // ----------------------------------------
  // Branch instruction
  // ----------------------------------------
  if (info->cf_type) {
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)get_target, IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_THREAD_ID, IARG_END);
  }

  // ----------------------------------------
  // Write an instruction to buffer
  // nav
  // ----------------------------------------
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)write_inst, IARG_INST_PTR, IARG_THREAD_ID, IARG_END);


  // ----------------------------------------
  // Dump out an instruction
  // ----------------------------------------
  if (print_inst) {
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)dprint_inst, IARG_INST_PTR, IARG_PTR, new string(INS_Disassemble(ins)), IARG_THREAD_ID, IARG_END);
  }
}

void write_memtrace(ADDRINT addr, bool isRead, ADDRINT ipointer)
{
  memtrace << "0x" << hex << addr << dec << "\t" << (isRead ? "R\t" : "W\t") << "0x" << hex << ipointer << dec << endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// Instruction - instrumentation
////////////////////////////////////////////////////////////////////////////////////////////////////////
void IInstruction(INS ins, void* v)
{
  // Count instructions
  INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IncrementNumInstruction, IARG_THREAD_ID, IARG_END);

  // Instrument instructions
  iinstrument(ins);
}



/////////////////////////////////////////////////////////////////////////////////////////
/// Thread Start Function
/// 1. Set trace file for each thread
/// 2. For multi-threaded application, there is a different thread mapping for each compiler. (gcc and icc)
/// 3. Set debug (dump) files for each thread
/////////////////////////////////////////////////////////////////////////////////////////
void ThreadStart(THREADID tid, CONTEXT *ctxt, INT32 flags, void *v)
{

  cout << "-> Thread[" << tid << "->" << threadMap[tid] << "] begins."<<endl;

  THREADID threadid = threadMap[tid];


  struct Trace_info* trace_info = NULL;

  trace_info = new Trace_info;
  if (trace_info == NULL) {
    cerr << "could not allocate memory\n";
    return;
  }
  memset(trace_info, 0, sizeof(Trace_info));

  // CPU Trace fields 
  stringstream sstream;
  sstream << Knob_trace_name.Value() << "_" << threadid << ".raw";
  string file_name;
  sstream >> file_name;
  cout<<"print  tid : "<<file_name <<endl;

  int trace_stream = -1;

  std::ofstream outfile(file_name.c_str(), std::ios::out); //creates a file
  if(!open(file_name.c_str(),std::fstream::in | std::fstream::out | std::fstream::app) )
  {
    perror("Error creating file for writing: ");
    exit(-1);
  }

  trace_stream = open(file_name.c_str(),  O_WRONLY );
  // DEBUG
  char debug_file_name[256] = {'\0'};

  ofstream* debug_stream = NULL;
  sprintf(debug_file_name, "%s_%d.dump", Knob_dump_file.Value().c_str(), threadid);
  debug_stream = new ofstream(debug_file_name);

  //-- acc trace fields 
  // stringstream sstream_acc;
  // int var_threadid_acc = threadid + numTraces ;
  // cout<<"var_thread_id_acc:" <<var_threadid_acc<<"\n";
  // THREADID threadid_acc = threadMap[var_threadid_acc] ;
  // sstream_acc << Knob_trace_name.Value() << "_" << threadid_acc  << ".raw";
  // string file_name_acc;
  // sstream_acc >> file_name_acc;
  // cout<<"print acc tid : "<<file_name_acc <<endl;

  // int trace_stream_acc = -1;

  // if(!open(file_name_acc.c_str(),std::fstream::in | std::fstream::out | std::fstream::app) )
  // {
  //   perror("Error creating acc file for writing: ");
  //   exit(-1);
  // }

  // trace_stream_acc = open(file_name_acc.c_str(),  O_WRONLY );

  //  acc DEBUG
  // char debug_file_name_acc[256] = {'\0'};

  // ofstream* debug_stream_acc = NULL;
  // sprintf(debug_file_name_acc, "%s_%d.dump", Knob_dump_file.Value().c_str(), threadid_acc);
  // debug_stream_acc = new ofstream(debug_file_name_acc);



    trace_info->trace_stream = trace_stream;
    // trace_info->trace_stream_acc = trace_stream_acc;
    trace_info->trace_stream_acc = 0;
    trace_info->bytes_accumulated = 0;
    trace_info->inst_count = 0;

    if (Knob_inst_dump.Value()) {
      trace_info->debug_stream = debug_stream;
      // trace_info->debug_stream_acc = debug_stream_acc;
      trace_info->debug_stream_acc = 0;
    }

    //remember numTraces does not count main thread which is pin TOOL itself
    //since threadid+numtraces = acc threadid :so actual #of threads created = 2*Threadid
    // assert(threadid/2 < MAX_THREADS);
    assert(threadid < MAX_THREADS);
    // PIN_GetLock(&g_lock, threadid+1);
    PIN_GetLock(&g_lock, threadid);
    cerr<<"Threadid inside Lock:"<<threadid<<"\n";

    // if (thread_count == 0)
    //   main_thread_id = threadid;

    thread_count++;
    PIN_ReleaseLock(&g_lock);

    trace_info_array[threadid] = trace_info;
    thread_info[threadid].thread_id = threadid;
    // thread_info[threadid].inst_count = trace_info_array[main_thread_id]->inst_count;


}



/////////////////////////////////////////////////////////////////////////////////////////
/// Thread end function : instrumentation
/////////////////////////////////////////////////////////////////////////////////////////
void ThreadEnd(THREADID threadid, const CONTEXT *ctxt, INT32 flags, void *v)
{
  thread_end();
}




/////////////////////////////////////////////////////////////////////////////////////////
/// Thread End Function
/// 1. This function can be called in
///   1) ThreadFini Function
///   2) Other conditions to terminate trace generation
/////////////////////////////////////////////////////////////////////////////////////////
void thread_end(void)
{
  thread_end(threadMap[PIN_ThreadId()]);
}



/////////////////////////////////////////////////////////////////////////////////////////
/// Thread end function
/////////////////////////////////////////////////////////////////////////////////////////
void thread_end(THREADID threadid)
{
  // THREADID threadid = threadMap[PIN_ThreadId()];
  if (threadid == 100000)
    return ;


  Trace_info* trace_info = NULL;
  trace_info = trace_info_array[threadid];

  /**< dump out instructions that are not written yet when a thread is terminated */
  if (trace_info->bytes_accumulated > 0)
  {

    if (trace_info->bytes_accumulated != write(trace_info->trace_stream,
          trace_info->trace_buf,
          trace_info->bytes_accumulated))
    {
      cerr << "TID " << threadid
        << " Error when writting instruction "
        << trace_info->inst_count << endl;
    }
    else 
      inst_between_markers[threadid]++;
  }
  cout<<"Thread END \n";

  /**< close trace file */
  close(trace_info->trace_stream);
  // close(trace_info->trace_stream_acc);

  /**< close dump file */
  if (Knob_inst_dump.Value()) {
    trace_info->debug_stream->close();
    // trace_info->debug_stream_acc->close();
  }

  /**< delete thread trace info */
  delete trace_info;



}



/////////////////////////////////////////////////////////////////////////////////////////
/// Fini function : instrumentation
/////////////////////////////////////////////////////////////////////////////////////////
void Fini(INT32 code, void *v)
{
  finish();
}



/////////////////////////////////////////////////////////////////////////////////////////
/// Finalization
/// Create configuration file
/// Final print to standard output
/////////////////////////////////////////////////////////////////////////////////////////
void finish(void)
{
  // thread_end();
  cerr<<"FINISH func called\n";

  /**< Create configuration file */
  string config_file_name = Knob_trace_name.Value() + ".txt";
  ofstream configFile;

  configFile.open(config_file_name.c_str());
  configFile << thread_count << " x86" << endl;
  for (unsigned int ii = 0; ii < thread_count; ++ii) {
    // thread_id thread_inst_start_count (relative to main thread)
    configFile << thread_info[ii].thread_id << " " << thread_info[ii].inst_count << endl;
  }
  configFile.close();

  /**< Final print to standard output */
  for (unsigned ii = 0; ii < thread_count; ++ii) {
    cout << "-> tid " << thread_info[ii].thread_id << " inst count " << g_inst_count[ii]
      << " (from " << thread_info[ii].inst_count << ")\n";
  }


  // std::cout << "Write Count: " << write_count << endl;

  if(Knob_mem_access_trace.Value())
  {
    memtrace.close();
  }

  cout << "-> Exiting..." << endl;
}


/////////////////////////////////////////////////////////////////////////////////////////
/// Initialization
/////////////////////////////////////////////////////////////////////////////////////////
void initialize(void)
{
  cerr << "Size of Inst_info: " << sizeof(Inst_info) << endl;
  // if(sizeof(Inst_info) > 52)
  //   cerr << "Tracing OOO with DS Fields enabled. Should only be done for the LLC contention traces." << endl;


  // Initialize Lock
  PIN_InitLock(&g_lock);

  // Enable Analysis Routine
  for (int ii = 0; ii < MAX_THREADS; ++ii) {
    g_enable_thread_instrument[ii] = true;
    g_enable_chunk[ii] = false;
    g_inst_print_count[ii] = 0;
    threadMap[ii] = ii;
  }


  g_enable_instrument = false;

  if(Knob_mem_access_trace.Value())
  {
    memtrace.open("memtrace.txt",ios::out);
  }
  // Initialize Data structure vars

  // ds_max_inst_per_roi = Knob_max_ds_inst.Value() / Knob_max_ds_markers.Value();

  // ds_data_file.open("ds_data");
  // ds_data_file << "DSMaxInstPerROI: " << ds_max_inst_per_roi << endl;
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


  // compiler must be either icc or gcc
  if (Knob_compiler.Value() != "icc" && Knob_compiler.Value() != "gcc") {
    cerr << "-> Please check -compiler value. Unknown compiler option.\n";
  }

  // programming language must be either c (normal) or c++
  if (Knob_pl.Value() != "normal" && Knob_pl.Value() != "c++") {
    cerr << "-> Please check -pl value. Unknown programming language option.\n";
  }
}


/////////////////////////////////////////////////////////////////////////////////////////
// Dump an instruction to a text file
/////////////////////////////////////////////////////////////////////////////////////////
void dump_to_file(ofstream* file, Inst_info *t_info)
{
  THREADID tid = threadMap[PIN_ThreadId()];

  g_inst_print_count[tid]++;
  if (tid == 100000 || !g_enable_thread_instrument[tid] || g_inst_print_count[tid] > Knob_dump_max.Value()) {
    if(t_info->ds_flag) cout<<"break 0 :"<< g_enable_thread_instrument[tid] <<"\t"  << g_inst_print_count[tid]  <<"\n";
    return ;
  }

  if(!g_enable_instrument) {

    if(t_info->ds_flag)  cout<<"break 1 \n";
    return;
  }

  // if(t_info->ds_flag)
  //   cout<<"Writing in file :"<<t_info->ds_flag <<"\n";



  (*file) << "*** begin of the data strcture *** " <<endl;
  (*file) << "t_info->uop_opcode " << (uint32_t) t_info->opcode << endl; // tr_opcode_names[(uint32_t) t_info->opcode]  << endl;
  (*file) << "t_info->num_read_regs: " << hex <<  (uint32_t) t_info->num_read_regs << endl;
  (*file) << "t_info->num_dest_regs: " << hex << (uint32_t) t_info->num_dest_regs << endl;
  for (UINT32 ii = 0; ii < 4; ++ii) {
    if (t_info->src[ii] != 0) {
      (*file) << "t_info->src" << ii << ": " << LEVEL_BASE::REG_StringShort(static_cast<LEVEL_BASE::REG>(t_info->src[ii])) << endl;
    }
  }
  for (UINT32 ii = 0; ii < 4; ++ii) {
    if (t_info->dst[ii] != 0) {
      (*file) << "t_info->dst" << ii << ": " << hex << LEVEL_BASE::REG_StringShort(static_cast<LEVEL_BASE::REG>(t_info->dst[ii])) << endl;
    }
  }
  (*file) << "t_info->cf_type: " << hex << tr_cf_names[(uint32_t) t_info->cf_type] << endl;
  (*file) << "t_info->has_immediate: " << hex << (uint32_t) t_info->has_immediate << endl;
  (*file) << "t_info->r_dir:" << (uint32_t) t_info->rep_dir << endl;
  (*file) << "t_info->has_st: " << hex << (uint32_t) t_info->has_st << endl;
  (*file) << "t_info->num_ld: " << hex << (uint32_t) t_info->num_ld << endl;
  (*file) << "t_info->mem_read_size: " << hex << (uint32_t) t_info->mem_read_size << endl;
  (*file) << "t_info->mem_write_size: " << hex << (uint32_t) t_info->mem_write_size << endl;
  (*file) << "t_info->is_fp: " << (uint32_t) t_info->is_fp << endl;
  (*file) << "t_info->ld_vaddr1: " << hex << (uint32_t) t_info->ld_vaddr1 << endl;
  (*file) << "t_info->ld_vaddr2: " << hex << (uint32_t) t_info->ld_vaddr2 << endl;
  (*file) << "t_info->st_vaddr: " << hex << (uint32_t) t_info->st_vaddr << endl;
  (*file) << "t_info->instruction_addr: " << hex << (uint32_t) t_info->instruction_addr << endl;
  (*file) << "t_info->branch_target: " << hex << (uint32_t) t_info->branch_target << endl;
  (*file) << "t_info->actually_taken: " << hex << (uint32_t) t_info->actually_taken << endl;
  (*file) << "t_info->write_flg: " << hex << (uint32_t) t_info->write_flg << endl;
  (*file) << "t_info->ds_flag: " << hex << (uint32_t) t_info->ds_flag << endl;
  (*file) << "t_info->is_chunk_begin: " << hex << (uint32_t) t_info->is_chunk_begin<< endl;
  (*file) << "t_info->thread_id: " << hex << (uint32_t) t_info->thread_id<< endl;
  (*file) << "t_info->chunk_id: " << hex << (uint32_t) t_info->chunk_id<< endl;

  (*file) << "*** end of the data strcture *** " << dec << endl << endl;
}


/////////////////////////////////////////////////////////////////////////////////////////
/// Dump an instruction to standard output
/////////////////////////////////////////////////////////////////////////////////////////
void dprint_inst(ADDRINT iaddr, string *disassemble_info, THREADID threadid)
{
  THREADID tid = threadMap[threadid];
  if (tid == 100000 || !g_enable_thread_instrument[tid])
    return ;

  // Inst_info *t_info = NULL;
  // // t_info = g_inst_storage[tid][iaddr];
  //
  // cout << "\nInstruction count: " << g_inst_count[tid] << endl;
  // cout << "*** beginning of the data strcture *** " <<endl;
  // cout << "t_info->uop_opcode " <<tr_opcode_names[(uint32_t) t_info->opcode]  << endl;
  // cout << "t_info->num_read_regs: " << hex <<  (uint32_t) t_info->num_read_regs << endl;
  // cout << "t_info->num_dest_regs: " << hex << (uint32_t) t_info->num_dest_regs << endl;
  // for (UINT32 ii = 0; ii < 4; ++ii) {
  //   if (t_info->src[ii] != 0) {
  //     cout << "t_info->src" << ii << ": " << hex << LEVEL_BASE::REG_StringShort(static_cast<LEVEL_BASE::REG>(t_info->src[ii])) << endl;
  //   }
  // }
  //
  // for (UINT32 ii = 0; ii < 4; ++ii) {
  //   if (t_info->dst[ii] != 0) {
  //     cout << "t_info->src" << ii << ": " << hex << LEVEL_BASE::REG_StringShort(static_cast<LEVEL_BASE::REG>(t_info->dst[ii])) << endl;
  //   }
  // }
  //
  // cout << "t_info->cf_type: " << hex << tr_cf_names[(uint32_t) t_info->cf_type] << endl;
  // cout << "t_info->has_immediate: " << hex << (uint32_t) t_info->has_immediate << endl;
  // cout << "t_info->r_dir:" << (uint32_t) t_info->rep_dir << endl;
  // cout << "t_info->has_st: " << hex << (uint32_t) t_info->has_st << endl;
  // cout << "t_info->num_ld: " << hex << (uint32_t) t_info->num_ld << endl;
  // cout << "t_info->mem_read_size: " << hex << (uint32_t) t_info->mem_read_size << endl;
  // cout << "t_info->mem_write_size: " << hex << (uint32_t) t_info->mem_write_size << endl;
  // cout << "t_info->is_fp: " << (uint32_t) t_info->is_fp << endl;
  // cout << "t_info->ld_vaddr1: " << hex << (uint32_t) t_info->ld_vaddr1 << endl;
  // cout << "t_info->ld_vaddr2: " << hex << (uint32_t) t_info->ld_vaddr2 << endl;
  // cout << "t_info->st_vaddr: " << hex << (uint32_t) t_info->st_vaddr << endl;
  // cout << "t_info->instruction_addr: " << hex << (uint32_t) t_info->instruction_addr << endl;
  // cout << "t_info->branch_target: " << hex << (uint32_t) t_info->branch_target << endl;
  // cout << "t_info->actually_taken: " << hex << (uint32_t) t_info->actually_taken << endl;
  // cout << "t_info->write_flg: " << hex << (uint32_t) t_info->write_flg << endl;
  // cout << "*** end of the data strcture *** " << endl;
}

VOID ImageLoad(IMG img,  VOID *v)
{
  for( SEC sec= IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec) )
  {
    for( RTN rtn= SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn) )
    {
      string rtnName =  PIN_UndecorateSymbolName(RTN_Name(rtn), UNDECORATION_NAME_ONLY);

      /*** Control Markers ***/

      if (rtnName == string("__app_roi_begin"))
      {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR) EnterROI, IARG_END);
        RTN_Close(rtn);
      }

      else if (rtnName == string("__app_roi_end"))
      {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR) ExitROI, IARG_END);
        RTN_Close(rtn);
      }

      else if (rtnName == string("__app_roi_end_detach"))
      {
        RTN_Open(rtn);
        RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR) ExitROIDetach, IARG_END);
        RTN_Close(rtn);
      }
      else if (rtnName == string("__chunk_begin"))
      {

        RTN_Open(rtn);

        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)Chunk_Begin,
            IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
            IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
            IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
            IARG_THREAD_ID,
            IARG_END);

        RTN_Close(rtn);
      }
      else if (rtnName == string("__chunk_end"))
      {

        RTN_Open(rtn);

        RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)Chunk_End,
            IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
            IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
            IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
            IARG_THREAD_ID,
            IARG_END);

        RTN_Close(rtn);
      }
    }
  }
}


/////////////////////////////////////////////////////////////////////////////////////////
/// main function
/////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
  //-----------------------------------------
  // ifstream traceTxt("numthreads.txt",ios::in);
  // if(!traceTxt.is_open())
  // {
  //   cerr << "Could not open trace.txt\n";
  //   return 0;
  // }
  // traceTxt >> numTraces;
  // cout<<"num of traces "<<numTraces<<"\n";
  // traceTxt.close();
  //----------------------------------------
  // for(int i = 2; i < numTraces  ; i++)
  // {
  //   string OrigFilename = string("trace_") + to_string(i) + string(".raw");
  //   OrigTrace = gzopen(OrigFilename.c_str(), "rb");
  //
  //
  //   if(!OrigTrace )
  //   {
  //     cerr << "Could not open trace files  " <<OrigFilename <<endl;
  //     return 0;
  //   }
  //   //unsigned long long  total_load_size =0;
  //   //cout<< "Total load + Store size for i=" <<i<<" is "<<processTrace()<<endl;
  //
  //   cout<<"Trace No :"<<i<<endl;
  //   processTrace();
  //   //storeTrace(i);
  //
  //   gzclose(OrigTrace);
  //
  // }
  // return 0;



  //------------------------------------------
  PIN_InitSymbols();
  if (PIN_Init(argc,argv)) {
    cerr << "This pin tool generates traces\n\n";
    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
  }

  numTraces = Knob_num_thread.Value() ;
  numTraces +=1;
  cout<<"num of traces ----- "<<numTraces<<"\n";


  initialize();
  sanity_check();

  detach_inst = Knob_detach_inst.Value();
  print_inst = Knob_print_inst.Value();

  IMG_AddInstrumentFunction ( &ImageLoad, NULL );
  PIN_AddThreadStartFunction(ThreadStart, 0);
  PIN_AddThreadFiniFunction(ThreadEnd, 0);

  // instrumentation option
  INS_AddInstrumentFunction(IInstruction, 0);
  // control.CheckKnobs(Handler, 0);
  control.RegisterHandler(Handler, 0, FALSE);

  PIN_AddDetachFunction(Detach, 0);
  PIN_AddFiniFunction(Fini, 0);
  PIN_StartProgram();

  return 0;
}


VOID EnterROI()
{
  g_enable_instrument = true;
  std::cout << "App Enter ROI at Inst#: " << g_inst_count[0] << endl;
}

VOID ExitROI()
{
  g_enable_instrument = false;
  std::cout << "Exit ROI at: " << g_inst_count[0] << endl;
}

VOID ExitROIDetach()
{
  g_enable_instrument = false;
  PIN_Detach();
  std::cout << "Exit ROI at: " << g_inst_count[0] << endl;
}


/////////////////////////////////////////////////////////////////////////////////////////
// 
/////////////////////////////////////////////////////////////////////////////////////////

VOID Chunk_Begin(UINT32 chunk_id,
    UINT32 app_tid,
    UINT32 core_id, 
    THREADID thread_id)
{
  // THREADID tid = threadMap[thread_id];
  THREADID tid = threadMap[PIN_ThreadId()];
  g_enable_chunk[tid] = true;
  THREAD_ENABLE_CHECK(tid);
  std::cout<<"Found Chunk Begin Iter : tid:"<<tid  << " chunk_id : "<< chunk_id <<std::endl;

  //dummy nop instruction before every marker
  Inst_info* t = (Inst_info*)memset((void *)new Inst_info, 0, sizeof(Inst_info));
  t->opcode = TR_NOP;
  t->thread_id = tid + numTraces;
  //will add NOP only in acc trace
  write_chunk_marker(tid,t,false);
  delete t;


  Inst_info* m = (Inst_info*)memset((void *)new Inst_info, 0, sizeof(Inst_info));
  m->ds_flag = true;
  // m->chunk_id = chunk_id++;
  m->chunk_id = chunk_id;
  m->is_chunk_begin = true;
  cout<<"inst_between_markers :"<<inst_between_markers[tid] <<" tid:"<<tid <<" BEGIN \n";
  inst_between_markers[tid]=0;

  m->thread_id = tid+ numTraces;
  m->opcode = TR_NOP; 
  
  write_chunk_marker(tid,m,true);
  delete m;
}

VOID Chunk_End(UINT32 chunk_id,
    UINT32 app_tid,
    UINT32 core_id ,
    THREADID thread_id)
{
  THREADID tid = threadMap[PIN_ThreadId()];
  g_enable_chunk[tid] = false;
  // THREADID tid = threadMap[thread_id];
  THREAD_ENABLE_CHECK(tid);

  std::cout<<"Found Chunk End Iter : tid:"<<tid  << " chunk_id : "<< chunk_id <<std::endl;
  Inst_info* m = (Inst_info*)memset((void *)new Inst_info, 0, sizeof(Inst_info));

  m->ds_flag = true;
  // m->chunk_id = chunk_id++;
  m->chunk_id = chunk_id;
  m->is_chunk_begin = false;

  cout<<"inst_between_markers :"<<inst_between_markers[tid] <<" tid:"<<tid <<"   END \n";
  inst_between_markers[tid]=0;


  m->thread_id = tid + numTraces;
  m->opcode = TR_NOP; 

  write_chunk_marker(tid,m,false);
  delete m;
}








void  write_chunk_marker(THREADID tid, Inst_info* m, bool is_begin)
{
  Trace_info* trace_info = trace_info_array[tid];

  // int write_size;
  
  // write_size = write(trace_info->trace_stream_acc, m, BUF_SIZE);
  //begin marker - write in cpu as well as acc
  if(is_begin) {

    // cout<<"marker begin acc tid: "<<m->thread_id<<" chunk_id:"<<m->chunk_id <<endl;
    //Since the tid info is different for CPU trace and Acc Trace
    m->thread_id = m->thread_id - numTraces;

    cout<<"marker begin cpu tid: "<<m->thread_id<<" chunk_id:"<<m->chunk_id <<endl;
    // write_size = write(trace_info->trace_stream, m, BUF_SIZE);
    if(! write(trace_info->trace_stream, m, BUF_SIZE))
    {

      // if (write_size != cout.good()) {
      cerr << "-> TID : " << tid << " Error when writing instruction " << trace_info->inst_count 
        // << " write_size:" << write_size 
        << " " << BUF_SIZE << endl;
      perror("Write failed: ");
      return;
    }

    trace_info->bytes_accumulated = 0;
    }
    //end marker - write in acc
  // else { 
  //   // write_size = write(trace_info->trace_stream_acc, m, BUF_SIZE);
  //   cout<<"marker end acc tid: "<<m->thread_id<<endl;
  // }

  if (Knob_inst_dump.Value()) {
    // cout<<"Writing Chunk Iter to file :"<<m->ds_flag <<"\n";
    //FIXME the output of m->thread_id will be wring in case of dump acc - but it is correct for trace.raw
    if(m->ds_flag) {

      if(is_begin) {
        dump_to_file(trace_info->debug_stream, m);
        // dump_to_file(trace_info->debug_stream_acc, m);
      }
      // else 
      //   dump_to_file(trace_info->debug_stream_acc, m);
    }
  }


  trace_info->inst_count++;

  // ----------------------------------------
  // detach : instruction counts exceed the limit
  // can we replace g_inst_count with trace_info->inst_count??
  // ----------------------------------------
  if (detach_inst != 0 && g_inst_count[tid] == detach_inst) {
    cout << "Detaching after " << g_inst_count[tid] << " instructions." << endl;
    PIN_Detach();
  }
}


