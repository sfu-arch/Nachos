#!/bin/bash


# # Dependencies
# gcc 4.8+ 
# cmake minimum VERSION 2.8+
# boost library - 1.61 and above
# pin3.0 
#
# Macsim - uses 64 bit shared library generated from pin and needs zlib
# gems ruby memory model needs flex and bison and zlib
# sudo apt-get update 
# sudo apt-get upgrade 
# sudo apt-get install flex bison
# sudo apt-get install zlib1g-dev

# For macsim: perl, pin3.0, zlib
# sudo apt-get install perl

NACHOSROOT=$PWD
export $NACHOSROOT
echo "NACHOSROOT = $NACHOSROOT"

# echo "Building GEMS memory model. Creates a library libroi.so"
# pushd mem-axc-64/apps/macsim.r.d.b.s/macsim-mem-axc-64
# make -C knobs/ 
# make -C gems-lib-ooo/ruby_clean PROTOCOL=MESI_CMP_directory_m  NOTESTER=1 DEBUG=1 -j 8
# popd


echo "creating build_dir"
mkdir -p build_repo
pushd build_repo

# echo "Building  bloom filter creates a static library libbloom.a"
# mkdir bloom
# pushd bloom
# cmake $NACHOSROOT/mem-axc-64/apps/bloom.s
# make -j 8
# popd bloom


echo " building dataflow simulator" 
echo " Dependencies: libbloom.so: from bloom filter and libruby.so: from ruby memory model"
mkdir dafaxc
pushd dafaxc

### in the cmake file of dafaxc set the path to the generated libbloom.a in the previous folder
cmake -DBOOST_ROOT=$1 -DBLOOM_PATH=/media/vnaveen0/Windows/Users/naveen/wind_drive/sfu/nachos/Nachos/build_repo/bloom  $NACHOSROOT/mem-axc-64/apps/dafaxc.r.b.s 
make -j 8
popd

# # Building Macsim which executes CPU code.
# MACSIMSRC=$NACHOSROOT/mem-axc-64/apps/macsim.r.d.b.s/macsim-mem-axc-64
# pushd $MACSIMSRC 
# make -C knobs/
# popd 
#
# mkdir -p macsim-mem-axc
# pushd macsim-mem-axc
# cmake $MACSIMSRC  
# cd bin
# cp $MACSIMSRC/cp_to_bin_files/* .
# # Need to set trace_file_list path where the input trace files are located 
# RUBY=$MACSIMSRC/gems-lib-ooo/ruby_clean
# ln -s ${RUBY}/DRAM
# ln -s ${RUBY}/network
# ./ooo-sim



# #### build ruby memory model
# cd $(HOME)/mem-axc-64/apps/macsim.r.d.b.s/macsim-mem-axc-64
# TO BUILD & RUN 
# 1. make -C knobs/   # creates files all_knobs.* and stats.* 
# 2. make -C gems-lib-ooo/ruby_clean PROTOCOL=MESI_CMP_directory_m  NOTESTER=1 DEBUG=1 -j 8
# 3. mkdir build 
# 4\. cd build 
# 5\. cmake -CMAKE\_BUILD\_TYPE=Debug ../
# 6\. cd bin
# 7\. cp params.in trace\_file\_list to bin directory from ../../cp\_to\_bin_files/
# 8\. ./ooo-sim
# ### creates a libruby.so



* Installing NEEDLE
# Please follow the readme file from here:
# https://github.com/sfu-arch/needle
# We are using an older version of needle which is not compatible with the one mentioned in the link
