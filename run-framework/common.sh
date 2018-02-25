#!/bin/bash
# Common variables across benchmarks
##


OPT=${LLVM_OBJ}/opt
LLC=${LLVM_OBJ}/llc
LINK=${LLVM_OBJ}/llvm-link
LLVMDIS=${LLVM_OBJ}/llvm-dis
CLANG=${LLVM_OBJ}/clang++

PASHA=${PASHA_OBJ}/pasha
EPP=${PASHA_OBJ}/epp
SBE=${PASHA_OBJ}/sbe
MWE=${PASHA_OBJ}/mwe
IFCONV=${PASHA_OBJ}/ifconv


# make dir, remove if exists
mymkdir()
{
    if [ -d "$1" ]; then
        rm -rf $1
    fi
    mkdir $1
}

# Compile the bitcode with optimizations and 
# google performance tools profiler
# Args bitcode name 
profile()
{
    ${CLANG} -fno-inline -fno-vectorize -fno-slp-vectorize  -O2 -L${GPERFT}/lib ${CFLAGS} ${1} -o ${2}-prof -lprofiler ${LIBS}
    eval $PRERUN 
    eval "env LD_LIBRARY_PATH=${GPERFT}/lib:/usr/local/lib64 CPUPROFILE=./${NAME}.prof CPUPROFILE_FREQUENCY=1000 ./${2}-prof ${RUNCMD} 2>&1 > run.out"
    ${GPERFT}/bin/pprof --text ./${NAME}-prof ${NAME}.prof > ${NAME}.prof.text
}


# optimize bitcode
# Args : bitcode 
# optimizebc()
# {
#     out=`echo ${1} | xargs basename | sed 's/\.bc//'`
#     if [ "${out}" == "450.soplex" ]; then
#         UNROLL="-unroll-count=8 -unroll-allow-partial -unroll-threshold 800"
#     else
#         UNROLL="-unroll-count=4 -unroll-allow-partial -unroll-threshold 400"
#     fi
#     ${OPT} -O2 -disable-loop-vectorization -disable-slp-vectorization ${UNROLL} ${1} > ${out}-opt.bc
#     cp ${out}-opt.bc ${out}.bc
# }

optimizebc()
{
    out=`echo ${1} | xargs basename | sed 's/\.bc//'`
    if [ "${out}" == "450.soplex" ]; then
        UNROLL="-unroll-count=8 -unroll-allow-partial -unroll-threshold 800"

    elif [ "${out}" == "bodytrack" ]; then
        UNROLL="-unroll-count=8 -unroll-allow-partial -unroll-threshold 800"


    elif [ "${out}" == "fft-2d" ]; then
        UNROLL="-unroll-count=8 -unroll-allow-partial -unroll-threshold 800"

    elif [ "${out}" == "fluidanimate" ]; then
        UNROLL="-unroll-count=8 -unroll-allow-partial -unroll-threshold 800"

    elif [ "${out}" == "freqmine" ]; then
        UNROLL="-unroll-count=8 -unroll-allow-partial -unroll-threshold 800"

    else
        UNROLL="-unroll-count=4 -unroll-allow-partial -unroll-threshold 400"
    fi
  
    ${OPT} -O2 -disable-loop-vectorization -disable-slp-vectorization ${UNROLL} ${1} > ${out}-opt.bc
    cp ${out}-opt.bc ${out}.bc

    echo  ${1}  $UNROLL
}


# add epp instrumentation
# Args : bitcode, function-name
epp()
{
    out=`echo ${1} | xargs basename | sed 's/\.bc//'`
    env PATH=${LLVM_OBJ}:$PATH ${EPP} ${LDFLAGS} -L${PASHA_LIB} -epp-fn=${2} ${1} -o ${out}-epp ${LIBS} 2> epp.log

}

# execute benchmark
# Args : name
execute()
{
    eval $PRERUN 
    eval "env LD_LIBRARY_PATH=${PASHA_LIB}:/usr/local/lib64 ./${1}-epp ${RUNCMD} 2>&1 > run.out"
}

# decode
# Args : bitcode, function-name 
decode()
{
    ${EPP} -epp-fn=${2} ${1} -p=path-profile-results.txt 2> decode.log
}

superblock()
{
    mymkdir stats
    ${SBE} -fn=${2} ${1}
    # ${PASHA} -fn=${2} -seq=superblocks.txt -num=1000 -trace=dynamic ${1} 2>> superblock.log 
    python ${ROOT}/sb-finder.py epp-sequences.txt superblocks.txt > sbfind.txt
}


# linkbc
# Args : bitcode, name
linkbc()
{
    if [[ -f "${2}.bc" ]]; then
        rm ${2}.bc
    fi
    ln -s ${1} ${2}.bc 
}

# copybc
# Args : name
copybc()
{ 
    cp ../${NAME}.bc . 
}

# errorcheck
# Args : function's name, error code, function name, logfile
errorcheck()
{
    #error checking
    if [[ ${2} != 0 ]]; then
        echo "${1} failed  : " `date` >> ${4}   
        echo "error code :" ${2} >> ${4}

        local currentpath="$PWD" 
        local parrentpath="$(dirname "$currentpath")"
        pushd ${parrentpath} > /dev/null
        echo "Function ${3} in step  ${1} failed" >> function_log.txt
        echo
        popd > /dev/null

        #exit ${2}
    fi

}

# collecting stats
# Args: apps_name
collectstats(){
    local currentpath="$PWD"
    local parrentpath="$(dirname "$currentpath")"
    pushd ${parrentpath}/collectstats > /dev/null
    rm -rf ${1}
    mkdir ${1}
    popd > /dev/null
    find  ${1} -name "*.json" -exec sed -i 's/nan/1/g' {} \;
    find ${1} -name "*.json" -exec python stats.py {} \; > ../collectstats/${1}/${1}-grok.csv
}

# Trace stats
# Args: apps_name
tracectstats(){
    local currentpath="$PWD"
    local parrentpath="$(dirname "$currentpath")"
    pushd ${parrentpath}/collectstats > /dev/null
    rm -rf ${1}
    mkdir ${1}
    popd > /dev/null
    find . -name "*.json" -exec sed -i 's/nan/1/g' {} \;
    find . -name "*.json" -exec python stats.py {} \; > ../collectstats/${1}/${1}-trace.csv
}


# generate success log
# Args: success
success(){
    local currentpath="$PWD" 
    local parrentpath="$(dirname "$currentpath")"
    pushd ${parrentpath} > /dev/null
    echo "Function: ${1} sucess" >> function_log.txt
    echo
    popd > /dev/null

}

# link the math library bitcode
# should be invoked before the inline pass 
# Args: bitcode_file 
llvmlinkmath() {
    ${LINK} ${1} ${BITCODE_REPO}/3.8/lib/m.bc -o m.${1}
    mv m.${1} ${1}
}


ifconv()
{
    for x in `ls ifc.*.dot`; do
        name=`echo ${x} | cut -d'.' -f2`
        ${IFCONV} ${x} > ${name}.preds
    done
}

microworkload_trace_nachos()
{

  python ${ROOT}/path.py epp-sequences.txt > paths.stats.txt

  # for n in {0..0}; do 
  for n in {0..4}; do 

    # Add -simdfg flag for labeling dfg, dumping single.ll, instrumenting load table, printing dfg.
    if [ ! -f "path-seq-${n}.txt"  ]; then
      continue
    fi

    echo > mwe.$n.nachos.log
    # env PATH=${LLVM_OBJ}:$PATH ${MWE} -fn=${2} -ExtractType::trace -simdfg -num=1 -seq=path-seq-${n}.txt ${LIBS} -u=${HELPER_LIB} ${1} -o ${NAME}-mwe-${n} 2>> mwe.log
    env PATH=${LLVM_OBJ}:$PATH ${MWE} -fn=${2} -ExtractType::trace -simdfg -aa-edges -time-passes -seq=path-seq-${n}.txt ${LIBS} -u=${HELPER_LIB} ${1} -o ${NAME}-mwe-${n} 2>> mwe.$n.nachos.log

    # RUN PIN TOOL -- Can run only once to generate addresses
    # if [ "$?" -ne 0 ]; then
    #   touch dun.goofed
    # else
    #
    #   ID=`head -n 1 path-seq-${n}.txt | cut -d' ' -f1`
    #   echo ${f} > function.txt
    #   echo "__offload_func_${ID}" >> function.txt
    #   PIN_INVOKE="/p-ssd/modules/pin-3.0-76991-gcc-linux/pin -t /home/nvedula/summer2016/mem-axc-64/tools/pin-tools/mem-addr-offload-only/obj-intel64/Offload.so -- "
    #   eval ${PIN_INVOKE}  "./${NAME}-mwe-${n} ${RUNCMD}"  #>pin.${n}.log 2>pin.${n}.err.log
    #   gzip -c axc-dump.out > axc-dump.out.${n}.gz
    #   mv undo.stats.txt undo.${n}.stats.txt
    #   mv mwe.stats.txt mwe.${n}.stats.txt
    #   mv aew.stats.txt aew.${n}.stats.txt
    #   mv function.stats.txt function.${n}.stats.txt
    # fi
  done
}

microworkload_trace_tartan()
{

  python ${ROOT}/path.py epp-sequences.txt > paths.stats.txt

  # Note: in a dynamic run tartan will genearte a different dot graph and addresses, but the dot graphs should be 
  # structurally same for tartan and nachos, with tartan having a few edges extra
  # Thus, for simulation purposes we are using the same addresses from nachos
  # Only the dot graphs will be different and they should differ only in # of edges

  # for n in {0..0}; do 
  for n in {0..4}; do 
    # Add -simdfg flag for labeling dfg, dumping single.ll, instrumenting load table, printing dfg.
    if [ ! -f "path-seq-${n}.txt"  ]; then
      continue
    fi

    echo > mwe.$n.tartan.log
    ## Add -only-basic-aa to use only basic alias analysis for tartan in stage 1 of alias analysis  
    env PATH=${LLVM_OBJ}:$PATH ${MWE} -fn=${2} -ExtractType::trace -simdfg -only-basic-aa -aa-edges -time-passes -seq=path-seq-${n}.txt ${LIBS} -u=${HELPER_LIB} ${1} -o ${NAME}-mwe-${n} 2>> mwe.$n.tartan.log

  done
}

mementropy()
{
  rm -f mementropy.txt
  head -n 5 epp-sequences.txt > epp-10.txt 
  while IFS='' read -r line || [[ -n "$line" ]]; do
    ID=`echo ${line} | cut -d' ' -f1`
    e=`zcat ${ID}.memtrace | head -n 10000000 | /home/ska124/Working/mem-entropy/entropy`
    echo "${ID},${e}" >> mementropy.txt
  done < epp-10.txt
}

opcodecount()
{
    rm -f *.opcode.txt 
    head -n 5 epp-sequences.txt > epp-10.txt 
    while IFS='' read -r line || [[ -n "$line" ]]; do
        ID=`echo ${line} | cut -d' ' -f1`
        echo -n "${NAME}," >> ~/opcode.txt
        ${LLVM_OBJ}/opt -load ${PASHA_LIB}/libcount.so -pasha-count-ins ${ID}.ll > /dev/null 2> ${ID}.opcode.txt
        add=`grep "^Add" ${ID}.opcode.txt | awk '{print $2}'`
        fadd=`grep FAdd ${ID}.opcode.txt | awk '{print $2}'`
        gep=`grep GetElementPtr ${ID}.opcode.txt |awk '{print $2}' `
        ld=`grep Load ${ID}.opcode.txt |awk '{print $2}' `
        echo -n "${add},${fadd},${gep},${ld}" >> ~/opcode.txt
        echo >> ~/opcode.txt
    done < epp-10.txt
}

