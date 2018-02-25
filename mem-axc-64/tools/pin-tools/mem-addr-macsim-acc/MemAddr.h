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


//Nagesh

#ifndef INDI_H
#define INDI_H

#ifdef _MSC_VER
typedef unsigned __int8 uint8_t;
typedef unsigned __int32 uint32_t;
#else
#include <inttypes.h>
#endif
#include <stdio.h>
/* added on jan-28 by nagesh */
#include <zlib.h>
#include <ostream>
/* added on jan-28 by nagesh */
#include "xed-category-enum.h"
#include "pin.H"
#include <vector>




#define MAX_SRC_NUM 9
#define MAX_DST_NUM 6




/* added on jan-28 by nagesh */

using namespace std;


/**
 * Instruction Information
 */
struct Inst_info {
    uint8_t num_read_regs;      // 3-bits
    uint8_t num_dest_regs;      // 3-bits
    uint8_t src[MAX_SRC_NUM]; // 6-bits * 4
    uint8_t dst[MAX_DST_NUM]; // 6-bits * 4
    uint8_t cf_type;  // 4 bits
    bool has_immediate;  // 1bits
    uint8_t opcode; // 6 bits
    bool  has_st;  // 1 bit
    bool is_fp;    // 1bit
    bool write_flg;   // 1bit
    uint8_t num_ld;  // 2bit
    uint8_t size; // 5 bit
    // **** dynamic ****
    uint64_t ld_vaddr1; // 4 bytes
    uint64_t ld_vaddr2; // 4 bytes
    uint64_t st_vaddr;   // 4 bytes
    uint64_t instruction_addr; // 4 bytes
    uint64_t branch_target; // not the dynamic info. static info  // 4 bytes
    uint8_t mem_read_size; // 8 bit
    uint8_t mem_write_size;  // 8 bit
    bool rep_dir;  // 1 bit
    bool actually_taken; // 1 bit

#ifndef INCLUDE_DS_FIELDS
#include "./common-trace-fields.cpp"
#endif
    Inst_info(): num_read_regs(0), num_dest_regs(0), 
    cf_type(0),has_immediate(false),opcode(0), has_st(false),
    is_fp(false),write_flg(false), num_ld(0), size(0),
    ld_vaddr1(0), ld_vaddr2(0),st_vaddr(0),instruction_addr(0),
    branch_target(0),mem_read_size(0), mem_write_size(0),
    rep_dir(false),actually_taken(false)
  {


// #ifndef INCLUDE_DS_FIELDS
    chunk_id=0;thread_id=0;core_id=0;ds_flag=false;is_chunk_begin=false;is_sp_full=false;
// #endif
    for ( uint8_t &entry: src){ entry=0;}
    for ( uint8_t &entry: dst){ entry=0;}

  }
};

#define BUF_SIZE (1 * sizeof(struct Inst_info))

struct Trace_info {
  int trace_stream;
  char trace_buf[BUF_SIZE];
  int bytes_accumulated;
  Inst_info inst_info;
  uint64_t inst_count;
  uint32_t vaddr1;
  uint32_t vaddr2;
  uint32_t st_vaddr;
  uint32_t target;
  uint32_t actually_taken;
  uint32_t mem_read_size;
  uint32_t mem_write_size;
  uint32_t eflags;
  ofstream* debug_stream;
  std::vector<Inst_info> offload_ins_vec;
};



enum TR_OPCODE_enum {
  TR_MUL = XED_CATEGORY_LAST ,
  TR_DIV,
  TR_FMUL,
  TR_FDIV,
  TR_NOP,
  PREFETCH_NTA,
  PREFETCH_T0,
  PREFETCH_T1,
  PREFETCH_T2,
}TR_OPCODE;


enum CF_TYPE_enum {
  NOT_CF,           // not a control flow instruction
  CF_BR,            // an unconditional branch
  CF_CBR,           // a conditional branch
  CF_CALL,          // a call
  // below this point are indirect cfs
  CF_IBR,           // an indirect branch
  CF_ICALL,         // an indirect call
  CF_ICO,           // an indirect jump to co-routine
  CF_RET,           // a return
  CF_SYS,
  CF_ICBR                     // an indirect conditional branch
}CF_TYPE;

string tr_cf_names[15] = {
  "NOT_CF",         // not a control flow instruction
  "CF_BR",          // an unconditional branch
  "CF_CBR",         // a conditional branch
  "CF_CALL",            // a call
  "CF_IBR",         // an indirect branch
  "CF_ICALL",           // an indirect call
  "CF_ICO",         // an indirect jump to co-routine
  "CF_RET",         // a return
  "CF_SYS",
  "CF_ICBR"
};


//Pin 3.0
string tr_opcode_names[75] = {
  "INVALID",
  "3DNOW",
  "AES",
  "AVX",
  "AVX2",
  "AVX2GATHER",
  "AVX512",
  "AVX512VBMI",
  "BDW",
  "BINARY",
  "BITBYTE",
  "BLEND",
  "BMI1",
  "BMI2",
  "BROADCAST",
  "CALL",
  "CLFLUSHOPT",
  "CLWB",
  "CMOV",
  "COMPRESS",
  "COND_BR",
  "CONFLICT",
  "CONVERT",
  "DATAXFER",
  "DECIMAL",
  "EXPAND",
  "FCMOV",
  "FLAGOP",
  "FMA4",
  "GATHER",
  "IFMA",
  "INTERRUPT",
  "IO",
  "IOSTRINGOP",
  "KMASK",
  "LOGICAL",
  "LOGICAL_FP",
  "LZCNT",
  "MISC",
  "MMX",
  "MPX",
  "NOP",
  "PCLMULQDQ",
  "PCOMMIT",
  "POP",
  "PREFETCH",
  "PUSH",
  "RDRAND",
  "RDSEED",
  "RDWRFSGS",
  "RET",
  "ROTATE",
  "SCATTER",
  "SEGOP",
  "SEMAPHORE",
  "SETCC",
  "SGX",
  "SHA",
  "SHIFT",
  "SMAP",
  "SSE",
  "STRINGOP",
  "STTNI",
  "SYSCALL",
  "SYSRET",
  "SYSTEM",
  "TBM",
  "UNCOND_BR",
  "VFMA",
  "VTX",
  "WIDENOP",
  "X87_ALU",
  "XOP",
  "XSAVE",
  "XSAVEOPT"
  };


/////////////////////////////////////////////////////////////////////////////////////////
/// Function forward declaration
// void write_inst_to_file(ofstream*, Inst_info*);
// VOID write_inst_to_file(ofstream* file, Inst_info *t_info);
// void dprint_inst(ADDRINT, string*, THREADID);
// void finish(void);
// void thread_end(void);
// void thread_end(THREADID threadid);
// void write_memtrace(ADDRINT addr, bool isRead, ADDRINT ipointer);

/////////////////////////////////////////////////////////////////////////////////////////
//// Data Structure Counters

// UINT64 ds_curr_iter_inst_count;
// UINT64 ds_iter_count;
// bool  ds_in_roi;
// UINT32 ds_marker_count;
// UINT64 ds_max_inst_per_roi;
// UINT64 ds_all_inst_count;
//


#endif //INDI_H

