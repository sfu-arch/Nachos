#!/bin/bash

source ../params.sh
source ../common.sh
source bench.sh

EPPNAME=epp-sequences.txt
TEMPNAME=temp

# Cleanup
for f in ${FUNCTIONS[@]}; do
    rm -rf $f
done 
rm -f ${NAME}.bc

if [ -f function_log.txt ]; then
    rm function_log.txt
fi

# link bitcode from repo
linkbc ${BC} ${NAME}


for f in ${FUNCTIONS[@]}
do
    mymkdir $f
    pushd $f > /dev/null
    mymkdir stats    
    LOGFILE=${NAME}.${f}.log
    echo "Start  : " `date` >> ${LOGFILE}
    copybc      ${NAME}
    errorcheck "Copy" $? $f ${LOGFILE}
    echo "Copy   : " `date` >> ${LOGFILE}

    llvmlinkmath ${NAME}.bc
    optimizebc ${NAME}.bc

    #profile ${NAME}.bc ${NAME}    

    epp         ${NAME}.bc ${f}
    errorcheck "EPP" $? $f ${LOGFILE}
    echo "EPP    : " `date` >> ${LOGFILE}

    execute     ${NAME}
    errorcheck "Exec" $? $f ${LOGFILE}
    echo "Exec   : " `date` >> ${LOGFILE}

    decode      ${NAME}.bc ${f}
    errorcheck "Decode" $? $f ${LOGFILE}
    echo "Decode : " `date` >> ${LOGFILE}

    popd > /dev/null
done


