CXX=g++-5 cmake  -DCMAKE_BUILD_TYPE=Debug -DBOOST_ROOT=/home/nvedula/software/boost_1_61_0-install/  ../
./may_prof -i /p-ssd/nvedula/mem-axc-64/dotbase/453.povray -m /p-ssd/nvedula/mem-axc-64/offload-trace/453.povray/axc-dump.out.gz >stdout 2>stderr
