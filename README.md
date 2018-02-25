# Notice
NEEDLE and runframework are borrowed from https://www.github.com/sfu-arch
and modified for this projcet


# Dependencies
gcc 4.8+ 
cmake minimum VERSION 2.8
boost library - 1.61 used
pin3.0 
Macsim - uses 64 bit shared library generated from pin
gems ruby 



# Directory Structure
You will see files with following extensions:
macsim.r.d.b.s 
r= ruby
d = dafaxc
b = bloom filter 
s = standalone

It just means that it includes those modules

Toplevel Directory contains:
  apps  tools 

  
apps-
bloom.s  dafaxc.r.b.s  macsim.r.d.b.s

bloom.s = standalone module for various types of bloom filters borrowed from gems 

dafaxc = dataflow accelerator, which reads dot graphs (dependency : boost)
  r. = ruby
  b. = bloom filter 

tools-
c++/may-profile

longest_path  may-profile  mead  pin-tools

longest_path = is a python script that reads in the hottest path in the form of dotgraph and finds the longest_chain
mead = is a python script which reads a dot graph and finds original edges, added edges by compiler and other info







# PITFALLS

RUBY : hardcoded CC= g++ ; need to manually change 
at $HOME/apps/macsim.r.d.b.s/macsim-mem-axc-64/gems-lib-ooo/common/Makefile.common



To get the stack addreses for pin tool
Commented from pasha/tools/mwe/main.cpp
// pm.add(new mwe::MicroWorkloadExtract(SeqFilePath, ExtractedModules));




# Installation

```bash
mkdir build_repo
cd build_repo
mkdir build-mem-axc-64
cd build-mem-axc-64


```bash



