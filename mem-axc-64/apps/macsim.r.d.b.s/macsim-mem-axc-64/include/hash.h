#ifndef HASH_H
#define HASH_H

#include <cmath>
#include <assert.h>

int hash_func(int tid, int core_id);
int get_tid_frm_hash_value(int hash_value);
int get_core_id_frm_hash_value(int hash_value);

#endif
