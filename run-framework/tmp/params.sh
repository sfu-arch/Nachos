# Params
ARCH=x86_64 # can be x86_64 or i686
LLVM_VERSION=3.8
BITCODE_REPO=/p-ssd/nvedula/midas-isca/hotpaths/workloads-bitcode/${ARCH}
LLVM_OBJ=/p-ssd/modules/llvm-3.8.1/bin
PASHA_OBJ=/p-ssd/nvedula/midas-isca/hotpaths/build-pasha-release/bin
PASHA_LIB=/p-ssd/nvedula/midas-isca/hotpaths/build-pasha-release/lib 
HELPER_LIB=${PASHA_LIB}/helpers.bc
DATA=/p/amoeba/shared/bench/workloads-data/

