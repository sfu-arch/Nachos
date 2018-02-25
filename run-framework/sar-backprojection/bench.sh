SUITE=perfect
NAME=sar-backprojection
BC=${BITCODE_REPO}/${LLVM_VERSION}/${SUITE}/${NAME}.bc
CFLAGS=
FUNCTIONS=('sar_backprojection')
LDFLAGS=
RUNCMD="inout"
PRERUN="ln -s ${DATA}/perfect-suite-v1.0/suite/sar/inout"
