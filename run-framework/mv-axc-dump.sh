#!/bin/bash

DEST=/p-ssd/nvedula/midas-isca/hotpaths/offload-trace
source apps.sh

for x in ${APPS[@]}; do
  pushd ${x} > /dev/null
  source bench.sh          
  for f in ${FUNCTIONS[@]}; do
    echo $x

    pushd $f > /dev/null
    LOGFILE=${NAME}.${f}.log
    mkdir -p ${DEST}/${x}/
    mv axc-dump.out* ${DEST}/${x}/

    popd > /dev/null
  done
  popd > /dev/null
done

