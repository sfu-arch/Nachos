# Dependencies
gcc 4.8+ 
cmake minimum VERSION 2.8+
boost library - 1.61 and above
pin3.0 

Macsim - uses 64 bit shared library generated from pin and needs zlib,perl
gems ruby memory model needs flex and bison and zlib

``` bash
sudo apt-get update 
sudo apt-get upgrade 
sudo apt-get install flex bison
sudo apt-get install zlib1g-dev
sudo apt-get install cmake

sudo apt-get install perl

# boost needs to be installed from source: 
# http://www.boost.org/doc/libs/1_65_0/more/getting_started/unix-variants.html

# To install Pin
# https://software.intel.com/en-us/articles/pin-a-binary-instrumentation-tool-downloads


```




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







# PITFALLS

To get the stack addreses for pin tool
Commented from pasha/tools/mwe/main.cpp
// pm.add(new mwe::MicroWorkloadExtract(SeqFilePath, ExtractedModules));




# Installation

* Building GEMS, DAFXC, BLOOM, and MACSIM 

```bash
./install.sh <path/to/boost/root>
```
* To run dafaxc
```bash
cd $NACHOSROOT/build_repo/dafaxc/bin
#set NACHOSROOT in run-dafaxc.sh
./run-dafaxc.sh
# The script calls DAFAXCBIN=$NACHOSROOT/build_repo/dafaxc/bin/dafaxc
# use $DAFAXCBIN --help for help options
```


# Installing NEEDLE
 Please follow the readme file from here to build needle:
 https://github.com/sfu-arch/needle
 We are using an older version of needle and have added all our alias analysis mentioned in the paper




