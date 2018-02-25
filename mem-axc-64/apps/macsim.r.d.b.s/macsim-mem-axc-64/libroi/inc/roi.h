/*******
Author: ska124
*******/

#ifndef _ROI_H
#define  _ROI_H

#pragma once 
#include <stdio.h>
#include <stdint.h>

/* General Markers */

void __attribute__ ((noinline)) __app_roi_begin()
{
    asm("");
}

void __attribute__ ((noinline)) __app_roi_end()
{
    asm("");
}
//
// void __attribute__ ((noinline)) __app_roi_end_detach()
// {
//     asm("");
// }
//
//
// /* General Data Structure Markers */
//
// void __attribute__ ((noinline)) __ds_ignore_begin()
// {
//     asm("");
// }
//
// void __attribute__ ((noinline)) __ds_ignore_end()
// {
//     asm("");
// }
//
// void __attribute__ ((noinline)) __pref_iter_complete()
// {
//     asm("");
// }
//
// void __attribute__ ((noinline)) __inst_window_begin()
// {
//     asm("");
// }
//
// void __attribute__ ((noinline)) __inst_window_end()
// {
//     asm("");
// }
//
// /* Vector Markers
//    Assume at max 8 PEs, so that means we can do 8
//    vectors at a time.
//
//    compute_slice_latency is the max number of cycles of profiled section.
//    compute_slice_energy is the average number of cycles executed in the profiled section.
//    compute_k is the extra op latency for loop related overhead to simulate.
//    stack_accesses are the number of stack accesses converted to regfile reads
//  */
//
//
// void __attribute__ ((noinline)) __vec_iter(uint32_t num_iter,
//                 void* load_base_addrs[],        /* v.data() */
//                 uint32_t node_size[],               /* Single vector for load and store */
//                 uint32_t load_index_offsets[],
//                 uint32_t static_ins_count,
//                 uint32_t compute_slice_latency,
//                 uint32_t compute_int_count,
//                 uint32_t compute_fp_count,
//                 uint32_t compute_k,
//                 uint32_t stack_accesses,
//                 void* store_base_addrs[],
//                 uint32_t store_index_offsets[],
//                 uint32_t vec_len[])
// {
//     asm ("");
// }
//
//
// void __attribute__ ((noinline)) __vec_sort(void* base_addr,
//                 uint32_t node_size,
//                 uint32_t start_index,
//                 uint32_t end_index,
//                 uint32_t comp_func_latency)
// {
//     asm ("");
// }
//
//
// /*
//   For a __vec_store use __vec_iter:
//   -- num_iter = 1
//   -- store_base_addr + store_index_offsets to specify locations [max 8]
//   -- compute_slice = 1
//   -- The simulator crashes for this - Avoid
//
//   For a __vec__find use __vec_iter:
//   -- num_iter = vec.size() if looking for all matches,
//               = iter where match was found if looking for first instance
//   -- load_base_addr[0] for the vector being searched
//
//   For a __vec_sort add a __vec_iter after the marker with
//   -- num_iter = vec.size()
//   -- load_base_addr[0] for the vector being sorted
//   -- all offsets set to 0
// */
//
// /* Hash table markers */
//
// // This is actually a linear search through a bucket of values using our wider
// // memory interface and the processing power of N PEs
// void __attribute__ ((noinline)) __hash_search(void *md5_key, int num_keys, void *base_addr)
// {
//     asm ("");
// }
//
//
// /* BTree Marker */
//
// void __attribute__ ((noinline)) __btree_traverse(void)
// {
//     asm ("");
// }
//
//
// /* Prefetch Markers */
// void __attribute__ ((noinline)) __pref_vector(void* load_base_addrs[],
//                    uint32_t node_size[],
//                    uint32_t vec_len[],
//                    uint32_t delay)
// {
//     asm ("");
// }
//
//


void __attribute__ ((noinline)) __chunk_begin(
                uint32_t chunk_id,
                uint32_t tid,
                uint32_t core_id)
{
    asm ("");
    printf("chunk_begin \n");
}

void __attribute__ ((noinline)) __chunk_end(
                uint32_t chunk_id,
                uint32_t tid,
                uint32_t core_id)
{
    asm ("");
    printf("chunk_end \n");
}

#endif
