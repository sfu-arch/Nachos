SUITE=parsec
NAME=ferret
BC=${BITCODE_REPO}/${LLVM_VERSION}/${SUITE}/${NAME}.bc
CFLAGS=
FUNCTIONS=('image_segment') 
LDFLAGS=
LIBS="-lpthread"
RUNCMD="${DATA}/${NAME}/corel lsh ${DATA}/${NAME}/queries 10 20 1 output.txt"
PRERUN=
