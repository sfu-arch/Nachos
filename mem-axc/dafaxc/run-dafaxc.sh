#!/bin/bash

#SET this variable
# SET NACHOSROOT to NACHOS home folder= /home/vnaveen0/wind_drive/sfu/nachos/Nachos
NACHOSROOT=/home/vnaveen0/wind_drive/sfu/nachos/Nachos


if [ -z ${NACHOSROOT+x} ]; 
then 
  echo "NACHOSROOT is unset"; 
  NACHOSROOT=$1
else 
  echo "NACHOSROOT is set to '$NACHOSROOT'"; 
fi

TESTDIR=$NACHOSROOT/mem-axc/test
GEMSDIR=$NACHOSROOT/mem-axc/gems-lib-ooo/ruby_clean 
DAFAXCBIN=$NACHOSROOT/build_repo/dafaxc/bin/dafaxc

dotpath=$TESTDIR/test1.dot
ldstpath=$TESTDIR/test1.gz

#bloom filter size
bfp_fs=4 
ISHASH2=false
ln -s ${GEMSDIR}/DRAM
ln -s ${GEMSDIR}/network



# ${DAFAXCBIN} -i  ${dotpath} --ldst-file ${ldstpath} -s true -f $bfp_fs -h $ISHASH2 > ${bfp_fs}_${ISHASH2}.out 2>${bfp_fs}_${ISHASH2}.err & 
${DAFAXCBIN} -i  ${dotpath} --ldst-file ${ldstpath} -s true -f $bfp_fs -h $ISHASH2 > ${bfp_fs}_${ISHASH2}.out 2>${bfp_fs}_${ISHASH2}.err 


