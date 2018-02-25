#!/bin/bash

source params.sh
source common.sh
source n1.common.sh
source apps.sh

DOTBASE=/p-ssd/nvedula/midas-isca/input/dotbase
MAY_PROF_BIN=/home/nvedula/summer2016/mem-axc-64-build/may-prof-build/bin/may_prof 
TraceDir=/p-ssd/nvedula/midas-isca/input/offload-trace
top5_TraceDir=/p-ssd/nvedula/midas-isca/input/top5-offload-trace
EPPNAME=epp-sequences.txt
TEMPNAME=temp
CMP=/p-ssd/amoeba/shared/Pasha/run-framework/cmp.py
OTHER=/p-ssd/amoeba/shared/Pasha/run-framework-orig
ROOT=`pwd`
LDST_EDGES=/home/nvedula/summer2016/mem-axc-64/tools/python/create_ldst_edges
HOTPATH_ALIAS=/home/nvedula/summer2016/mem-axc-64/tools/python/hotpath-alias/hotpath-alias.py

# MEMTRACE=/p-ssd/nvedula/midas-isca/hotpaths-stack/mem-trace
echo "app orig_may_and_must_edges added_may_and_must_edges" >${ROOT}/simplified.csv
echo "app naive_may naive_no naive_must"  > ${ROOT}/naiveAA.csv
echo "app ipa_may_may ipa_may_no naive_may_from_total" > ${ROOT}/ipAA.csv
echo "app NO_conflict Yes_Conflict split">${ROOT}/oracle_may.csv

echo "bench SS_no_conf SS_yes_conf LS_no_conf LS_yes_conf SL_no_conf SL_yes_conf" > ${ROOT}/offload_stats5.csv
echo "bench SS_no_conf SS_yes_conf LS_no_conf LS_yes_conf SL_no_conf SL_yes_conf" > ${ROOT}/offload_stats.csv
for x in ${APPS[@]}; do
  pushd ${x} > /dev/null
  source bench.sh          
  for f in ${FUNCTIONS[@]}; do
    echo $x

    # ${OPT} -dot-callgraph -q ${x}.bc > /dev/null 2> /dev/null
    pushd $f > /dev/null
    LOGFILE=${NAME}.${f}.log
    


    #-----------------------
    # 1. 
    # microworkload_trace ${NAME}.bc ${f} &
    #-----------------------
    # 2. 
    # run_ldst_edges
    # run_mead 

    #-----------------------
    # 2.5
    # run_longest_path
    #-----------------------
    # # Get Alias Stats 
    # 3. 
    # grep_all_AAstats ${x} 
    # run_may_prof_selected ${x}
    # grep_all_AA1_stats ${x} # takes all 5 paths stats separately 
    #-----------------------

    #-----------------------
    # 4. 
    # # Gets Store-Load, Load-Store, Store-Store dependencies in a dot graph
    # get_offload_mem_dep_5 ${x}
    grep_mem_dep_5 ${x}

    #-----------------------

    popd > /dev/null
  done
  popd > /dev/null
done

