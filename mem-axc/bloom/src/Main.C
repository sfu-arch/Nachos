#include <iostream>
using namespace std;
#include "GenericBloomFilter_1.h" 
#include "Address.h"

int main()
{
  GenericBloomFilter_1 bf_lsb("LSB_Counting-512:128",false); 
  
  // for smt threads "NonCounting, Bulk  
  // GenericBloomFilter bf_MG("Multigrain-16384_16384");//first bloom size _ second bloom size 
  // GenericBloomFilter bf_MBS("MultiBitSel-16384_4_1_Regular"); 
  //filtersize_numhashes_skipbits_(Regular/Parallel)
  // head contains filter size, tail contains bit offset from block number

  // GenericBloomFilter bf_H3("H3-16384_4_Regular"); 

  //----------------------------------------------------------
  
  Address addr;
  addr.setAddress(123233231231);
  bf_lsb.increment(addr);
  bf_lsb.increment(addr);
  cout<<"getcount:"<<bf_lsb.getCount(addr)<<endl;
  bf_lsb.decrement(addr);
  cout<<"getcount:"<<bf_lsb.getCount(addr)<<endl;
  bf_lsb.clear();
  cout<<"Totalcount:"<<bf_lsb.getTotalCount()<<endl;
  return 0;
}
