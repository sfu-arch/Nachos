SUITE=synthetic
NAME=histogram
BC=${BITCODE_REPO}/${LLVM_VERSION}/${SUITE}/${NAME}.bc
CFLAGS=
FUNCTIONS=('rgb2hsl')
LDFLAGS=
RUNCMD=" ${DATA}/histogram/in.ppm"
PRERUN=
