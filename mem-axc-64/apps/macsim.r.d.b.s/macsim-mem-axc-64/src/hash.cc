#include "hash.h"

int hash_func(int tid, int core_id)
{

  assert( (core_id < pow(2,16) && tid < pow(2,16) ) && "core_id & tid  should be less than 2^16");
  int hash_value = core_id <<16;
  hash_value +=tid; 
  return hash_value;

}

int get_tid_frm_hash_value(int hash_value)
{
  int temp = pow(2,16) -1 ;
  return hash_value & temp; 
}

int get_core_id_frm_hash_value(int hash_value)
{
  int temp = pow(2,16) -1 ;
  return  ( hash_value - (hash_value & temp) )>>16 ; 

}


