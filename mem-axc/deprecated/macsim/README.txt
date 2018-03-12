CXX=g++-5 cmake -DCMAKE_BUILD_TYPE=Debug -DBOOST_ROOT=/home/nvedula/software/boost_1_61_0-install  ../


TO BUILD & RUN 
1. make -C knobs/   # creates files all_knobs.* and stats.* 
2. BUILD : RUBY - in gems-lib-ooo : USED GEMS-LIB: from Hydra/macsim-ooo : gems-lib-ooo
    make -C gems-lib-ooo/ruby_clean PROTOCOL=MESI_CMP_directory_m  NOTESTER=1 DEBUG=1 -j 8
3. mkdir - build 
4. cd build 
5. cmake -CMAKE_BUILD_TYPE=Debug ../
6. cd bin
7. cp params.in trace_file_list to bin directory from ../../cp_to_bin_files/
8. ./ooo-sim









Things to keep in mind

for cache miss - Checking tryacccache method for L1 aS WELL AS L2 - 
though caches are inclusive - but the above method returns false for transient states of L1 and L2
hence need to check both L1 and L2


Sometimes there can be cache misses but the instruction scheduled is so back in the rob
that by the time it reaches retire stage the value has arived from the memory


branch predictor is off currently


DSTrace.so for multithreaded version comes from macsim-gpp in sqrl whereas 
ooo version DSTrace.so comes from sqrl ooo
and all traces are done in amoeba head using pin 2.10  version
pin -t DSTrace.so  -thread 3 --  ./blackscholes-gpp 2  ./input.txt output.txt

DO NOT FOrget to modify trace.txt after trace generation
Sample trace.txt for 8 threads 
9 x86
0 0
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0


chunk_ids wrong from single thread bayes
