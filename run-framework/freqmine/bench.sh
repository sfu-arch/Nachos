SUITE=parsec
NAME=freqmine
BC=${BITCODE_REPO}/${LLVM_VERSION}/${SUITE}/${NAME}.bc
CFLAGS=
FUNCTIONS=('_Z32FPArray_conditional_pattern_baseIhEiP7FP_treeiiT_')
LDFLAGS=
RUNCMD="${DATA}/${NAME}/kosarak_990k.dat 790"
PRERUN=
