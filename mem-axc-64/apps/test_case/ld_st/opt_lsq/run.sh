#!/bin/bash
GEMSDIR=/home/nvedula/summer2016/mem-axc-64/apps/macsim.r.d.b.s/macsim-mem-axc-64/gems-lib-ooo/ruby_clean
ln -s ${GEMSDIR}/DRAM
ln -s ${GEMSDIR}/network
../../../dafaxc.r.b.s-build/bin/dafaxc -i ../dfg.__offload_func_10.dot  --ldst-file ../axc-dump.out.0.gz -r 48 >b1.out 2>b1.err &
