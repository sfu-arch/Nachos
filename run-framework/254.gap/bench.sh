SUITE=spec2000
NAME=254.gap
BC=${BITCODE_REPO}/${LLVM_VERSION}/${SUITE}/${NAME}.bc
CFLAGS="-m32"
FUNCTIONS=('longest_match') 
LDFLAGS=
RUNCMD="-l ${DATA}/${NAME}/glibs -q -m 192M < ${DATA}/${NAME}/ref.in "
PRERUN=


# Note : The entire toolchains needs to support 32 bit.
# a. math lib linking
# b. profilers
# c. epp-runtime
