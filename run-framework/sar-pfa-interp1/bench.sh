SUITE=perfect
NAME=sar-pfa-interp1
BC=${BITCODE_REPO}/${LLVM_VERSION}/${SUITE}/${NAME}.bc
CFLAGS=
FUNCTIONS=('sar_interp1')
LDFLAGS=
RUNCMD="inout"
PRERUN="ln -s ${DATA}/perfect-suite-v1.0/suite/sar/inout"
