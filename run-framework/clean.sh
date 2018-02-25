#!/bin/bash

source apps.sh

for x in ${APPS[@]}; do
  pushd ${x} > /dev/null
  source bench.sh          
  for f in ${FUNCTIONS[@]}; do
    echo $x

    pushd $f > /dev/null
    rm aew.*.stats.txt
    rm dfg.*
    rm mead.*
    rm __offload_func*
    rm tartan.__offload_func*
    rm axc-dump.out*
    rm ldst.dfg.* 
    rm may-prof-*
    rm offload_stats*.csv
    LOGFILE=${NAME}.${f}.log
    rm axc-dump.out

    popd > /dev/null
  done
  popd > /dev/null
done

