#pragma once

/* **** */

typedef struct armvm_tlb_t** armvm_tlb_h;
typedef struct armvm_tlb_t* armvm_tlb_p;

#include "armvm.h"

/* **** */

armvm_tlb_p armvm_tlb_alloc(armvm_tlb_h h2tlb, armvm_p avm);
void armvm_tlb(unsigned action, armvm_tlb_p tlb);
