SUITE=spec2006
NAME=403.gcc
BC=${BITCODE_REPO}/${LLVM_VERSION}/${SUITE}/${NAME}.bc
CFLAGS=
FUNCTIONS=('bitmap_operation')
LDFLAGS=
RUNCMD="${DATA}/${NAME}/200.i -o 200.s"
TESTCMD="${DATA}/${NAME}/cccp.i -o cccp.s"
PRERUN=
