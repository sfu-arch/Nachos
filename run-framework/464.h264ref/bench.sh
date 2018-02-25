SUITE=spec2006
NAME=464.h264ref
BC=${BITCODE_REPO}/${LLVM_VERSION}/${SUITE}/${NAME}.bc
CFLAGS=
LDFLAGS=
FUNCTIONS=('dct_luma_16x16')
 
RUNCMD="-d ${DATA}/${NAME}/foreman_ref_encoder_baseline.cfg"
TESTCMD="-d ${DATA}/${NAME}/foreman_test_encoder_baseline.cfg"
PRERUN=
