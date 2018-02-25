#!/bin/bash

source apps.sh

# max=4
# for ((i = 0 ; i < max ; i++ )); do echo "$i"; done

optimizebc()
{
    out=`echo ${1} | xargs basename | sed 's/\.bc//'`
    if [ "${out}" == "450.soplex" ]; then
        UNROLL="-unroll-count=8 -unroll-allow-partial -unroll-threshold 800"

    elif [ "${out}" == "bodytrack" ]; then
        UNROLL="-unroll-count=8 -unroll-allow-partial -unroll-threshold 800"

    elif [ "${out}" == "ferret" ]; then
        UNROLL="-unroll-count=8 -unroll-allow-partial -unroll-threshold 800"

    elif [ "${out}" == "fft-2d" ]; then
        UNROLL="-unroll-count=8 -unroll-allow-partial -unroll-threshold 800"

    elif [ "${out}" == "fluidanimate" ]; then
        UNROLL="-unroll-count=8 -unroll-allow-partial -unroll-threshold 800"

    elif [ "${out}" == "freqmine" ]; then
        UNROLL="-unroll-count=8 -unroll-allow-partial -unroll-threshold 800"

    elif [ "${out}" == "streamcluster" ]; then
        UNROLL="-unroll-count=8 -unroll-allow-partial -unroll-threshold 800"

    else
        UNROLL="-unroll-count=4 -unroll-allow-partial -unroll-threshold 400"
    fi

    echo  ${1}  $UNROLL
}

for x in ${APPS[@]}; do

  optimizebc $x.bc
done

 
