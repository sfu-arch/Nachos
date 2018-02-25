#!/bin/bash

source params.sh
source common.sh
source n1.common.sh
source apps.sh

DOTBASE=/p-ssd/nvedula/midas-isca/hotpaths/input/dotbase 
MAY_PROF_BIN=/home/nvedula/summer2016/mem-axc-64-build/may-prof-build/bin/may_prof 
TraceDir=/p-ssd/nvedula/midas-isca/hotpaths/input/offload-trace
top5_TraceDir=/p-ssd/nvedula/midas-isca/hotpaths/input/top5-offload-trace
EPPNAME=epp-sequences.txt
TEMPNAME=temp
CMP=/p-ssd/amoeba/shared/Pasha/run-framework/cmp.py
OTHER=/p-ssd/amoeba/shared/Pasha/run-framework-orig
ROOT=`pwd`
LDST_EDGES=/home/nvedula/summer2016/mem-axc-64/tools/python/create_ldst_edges
HOTPATH_ALIAS=/home/nvedula/summer2016/mem-axc-64/tools/python/hotpath-alias/hotpath-alias.py
LONGEST_PATH_CONST_LAT=/home/nvedula/summer2016/mem-axc-64/tools/python/longest_path/longest_path_constmem_lat.py
FANIN=/home/nvedula/summer2016/mem-axc-64/tools/python/fanins/fanins.py

# MEMTRACE=/p-ssd/nvedula/midas-isca/hotpaths-stack/mem-trace


# APPS=('histogram')
run_func() {
for x in ${APPS[@]}; do
  pushd ${x} > /dev/null
  source bench.sh          
  for f in ${FUNCTIONS[@]}; do
    # echo $x 

    # ${OPT} -dot-callgraph -q ${x}.bc > /dev/null 2> /dev/null
    pushd $f > /dev/null
    LOGFILE=${x}.${f}.log
    echo $PWD

    if [ "$1" == "microworkload_trace_nachos" ]; then
      echo "microworkload_trace_nachos called"
      $1 ${x}.bc ${f} &

    elif [ "$1" == "microworkload_trace_tartan" ]; then
      echo "microworkload_trace_tartan called"
      $1 ${x}.bc ${f} &

    elif [ "$2" == "seq" ]; then
      $1 ${x}  

    else
      $1 ${x} & 
    fi
    
    popd > /dev/null
  done
  popd > /dev/null
done

}


get_stats() {

  >&2 echo "get_stats"

  
  #---------------------------------------------------------------------------------------
  # run_func add_tartan_prefix_alias_files "seq"


  echo "diff nachos tartan aa.txt" > $ROOT/diff_alias.txt
  run_func verify_diff_tartan_nachos_alias "seq"
  #---------------------------------------------------------------------------------------
  # echo "-- $dir --">$ROOT/walltime.csv
  # run_func grep_walltime "seq"
  # ##------------------------------------
  #
  # # --- Verified
  # echo "app orig_may_and_must_edges added_may_and_must_edges" >${ROOT}/simplified.csv
  # echo "app naive_may naive_no naive_must"  > ${ROOT}/naiveAA.csv
  # echo "app total_naive_sum total_ipaa_may total_naive_must" > ${ROOT}/ipAA.csv
  #
  # >&2 echo "grep_all_AAstats"
  # run_func grep_all_AAstats  "seq"




  # # #------------------------------------
  # # XXX MAY-PROF IS DISCONTINUED -- AVOID USING THIS
  # # echo "app NO_conflict Yes_Conflict split">${ROOT}/oracle_may.csv
  # # >&2 echo "grep_may_prof_stats"
  # # run_func grep_may_prof_stats "seq"
  # #
  #
  # --- Verified
  # echo "app naive_total_aa_pairs naive_may_pairs naive_must_pairs IPAA+simplified_edges IPAA+simplified_may IPAA_simplified_must" >${ROOT}/selected_path_alias_info.csv
  # echo "app naive_total_aa_pairs naive_edges IPAA+simplified_edges naive_edges_may_contribution IPAA+simplified_may_edge_contribution" >${ROOT}/plot_selected_path_alias_info.csv
  # >&2 echo "get_alias_info_selected_path"
  # run_func get_alias_info_selected_path "seq"
  #
  #
  #
  # --- Verified
  # echo "app total_naive_sum simplified_may simplified_must total_simp_edges" >${ROOT}/top5_alias_info.csv
  # >&2 echo "get_alias_info_top5_path"
  # run_func get_alias_info_top5_path "seq"
  #
  #
  # #------------------------------------
  # --- Verified
  # echo "bench SS_no_conf SS_yes_conf LS_no_conf LS_yes_conf SL_no_conf SL_yes_conf LL_no_conf LL_yes_conf" > ${ROOT}/offload_stats5.csv
  #
  # echo "bench Total_no_AA Total_yes_AA" > $ROOT/oracle_conflict.csv
  # >&2 echo "grep_mem_dep_5"
  # run_func grep_mem_dep_5 "seq" 
  #
  #
  # --- Verified
  # # # #------------------------------------
  # echo "bench SS_no_conf SS_yes_conf LS_no_conf LS_yes_conf SL_no_conf SL_yes_conf LL_no_conf LL_yes_conf" > ${ROOT}/offload_stats1.csv
  # >&2 echo "grep_mem_dep_1"
  # run_func grep_mem_dep_1 "seq" 
  
  

  # echo "app dfg.crit.length mead.crit.length dfg.crit.latency  mead.crit.latency #cde" >${ROOT}/critical_length.csv
  # run_func grep_longest_path "seq"  

  
  # echo "FANIN SUMMARY">$ROOT/fanin_summary.txt
  # run_func grep_fanins "seq"

  # ###----------------------------------------------------------- 
  # ###  AUXILLARY STATS 
  # ###----------------------------------------------------------- 
  # #------------------------------------
  # >&2 echo "get_aew_summary"
  #
  # echo " AEW STATS " >$ROOT/aew.stats.txt
  # run_func get_aew_summary $x "seq"
  #
  # ##------------------------------------
  # echo "app Load Store Int FP Total" >$ROOT/ops_distribution.csv
  # run_func get_ops "seq"
  # #------------------------------------
  # echo "app rankid b1_edges mead_edges" >$ROOT/num_dataflow_edges.csv  
  # run_func get_dataflow_edges "seq"
  # ##------------------------------------
  # echo "app num-orig-edges num-added-edges num-dataflow-edges">$ROOT/selected_path_edges.csv 
  # run_func grep_edges
  

}


cp-files() {
  echo "Copying Files "

  pushd $DOTBASE >/dev/null
  ./cp-dots.sh
  popd > /dev/null
  pushd $top5_TraceDir > /dev/null
  ./cp-top5-trace.sh
  popd >/dev/null

  pushd $TraceDir > /dev/null
  ./cp-trace.sh
  popd > /dev/null

}


run_tools() {

  BEGIN_TIME=`date`
  echo "run_tools"
  # # #
  # run_func microworkload_trace_tartan
  # run_func microworkload_trace_nachos
  # wait
  # #------------------------------------
  # # generate dot graph with ordering edges
  # # to run baseline LSQ
  #
  # run_func run_ldst_edges
  # wait
  #
  # ##------------------------------------
  # ### DONT USE __ USE MIDAS INSTEAD
  # # # Get dot graph for MEAD
  # run_func run_mead
  # # wait
  #
  # ##------------------------------------
  # # # Get got graph for MEAD
  # run_func run_midas
  # wait
  #
  #
  #
  # ##------------------------------------
  # run_func run_longest_path
  # wait
  # ##------------------------------------
  # # Remember : to cp files to input folder 
  # # dotbase, offload_trace etc
  # cp-files
  # wait
  # ##------------------------------------
  #
  # # # Get oracle_may.csv
  # # run_func run_may_prof_selected
  # # wait
  # ##------------------------------------
  # # # Gets Store-Load, Load-Store, Store-Store dependencies in a dot graph
  # # # for top 5 paths
  # run_func get_offload_mem_dep_5  
  # wait
  # END_TIME=`date`
  # echo -e "Beg: ${BEGIN_TIME}\nEnd: ${END_TIME}" | mail -s "N5 runs complete" $(whoami)@sfu.ca
  # # run_func get_offload_mem_dep "seq"

  # ##------------------------------------
  # run_func run_fanins

}

# run_tools
get_stats
