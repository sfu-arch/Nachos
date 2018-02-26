# Dependencies
gcc 4.8+ 
cmake minimum VERSION 2.8
boost library - 1.61 and above
pin3.0 
Macsim - uses 64 bit shared library generated from pin
gems ruby memory model

# Directory Structure
* \[mem-axc-64\](./mem-axc-64): simulator for dataflow architecture
	* apps:
		* bloom.s: bloom filter extracted from GEMS simulator
	    * dafaxc.r.b.s: dataflow accelerator with ruby memory model from GEMS ( r) and bloom filter (b). It is a combined framework to model - load store queues, nachos-sw, and nachos
	    * macsim.r.d.b.s: macsim cpu simulator integrated with dataflow accelerator(d), ruby memory model(r) and bloom filter(b)
	   * test: small examples for testing each framework
   * tools: tools to calculate statistics
	   * c++/may-profile: runtime profiling of may alias pairs to know the ground truth
	   * pin-tools: tools to calculate dynamic and static statistics for hotpaths
	   * python: tools to calculate statistics such as fan-ins, and generate hotpaths for laod store queues, software approach, and nachos
	   
* needle:  llvm compiler framework for extracting hotpaths


l





# PITFALLS

RUBY : hardcoded CC= g++ ; need to manually change 
at $HOME/apps/macsim.r.d.b.s/macsim-mem-axc-64/gems-lib-ooo/common/Makefile.common



To get the stack addreses for pin tool
Commented from pasha/tools/mwe/main.cpp
// pm.add(new mwe::MicroWorkloadExtract(SeqFilePath, ExtractedModules));




# Installation

```bash
### THIS README IS INCOMPLETE, FOLLOW THE README IN apps/dafaxc.r.b.s to build that
### TO BUILD GEMS - FOLLOW THE README IN apps/macsim.r.d.b.s/macsim-mem-axc-64

mkdir build_repo
cd build_repo
mkdir build-mem-axc-64
cd build-mem-axc-64

#### Build  bloom filter 
mkdir bloom && cd bloom
cmake <path/to/bloom.s>
make -j 8
cd ../
#### build ruby memory model
cd $(HOME)/mem-axc-64/apps/macsim.r.d.b.s/macsim-mem-axc-64
TO BUILD & RUN 
1. make -C knobs/   # creates files all_knobs.* and stats.* 
2. make -C gems-lib-ooo/ruby_clean PROTOCOL=MESI_CMP_directory_m  NOTESTER=1 DEBUG=1 -j 8
3. mkdir build 
4\. cd build 
5\. cmake -CMAKE\_BUILD\_TYPE=Debug ../
6\. cd bin
7\. cp params.in trace\_file\_list to bin directory from ../../cp\_to\_bin_files/
8\. ./ooo-sim
### creates a libruby.so


#### building dataflow simulator 
#### Dependencies
#### libbloom.so: from bloom filter
#### libruby.so: from ruby memory model
mkdir dafaxc

### in the cmake file of dafaxc set the path to the generated libbloom.a in the previous folder
cmake -DBOOST_ROOT= <path/to/boost>  <path/to/dafaxc>
make -j 8
```bash



