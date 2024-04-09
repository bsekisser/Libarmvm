#include "armvm_mmu.h"

/* **** */

#include "armvm_action.h"
#include "armvm_coprocessor_cp15.h"
#include "armvm_core.h"
#include "armvm_core_exception.h"
#include "armvm_mem.h"
#include "armvm_tlb.h"
#include "armvm.h"

/* **** */

#include "libbse/include/handle.h"
#include "libbse/include/log.h"
#include "libbse/include/page.h"

/* **** */

typedef struct armvm_mmu_t {
	armvm_mem_callback_p l1[PAGE_SIZE];
//
	armvm_p armvm;
	armvm_core_p core;
#define pARMVM_CORE mmu->core

	armvm_mmu_h h2mmu;
	armvm_mem_p mem;
	armvm_tlb_p tlb;
}armvm_mmu_t;

/* **** */

static void __armvm_mmu_alloc_init(armvm_mmu_p mmu)
{
	mmu->core = mmu->armvm->core;
	mmu->mem = mmu->armvm->mem;
}

static void __armvm_mmu_exit(armvm_mmu_p mmu)
{
	if(mmu->armvm->config.trace.exit) LOG();

	handle_free((void*)mmu->h2mmu);
}

/* **** */

void armvm_mmu(unsigned action, armvm_mmu_p const mmu)
{
	switch(action) {
		case ARMVM_ACTION_ALLOC_INIT: __armvm_mmu_alloc_init(mmu); break;
	}

	armvm_tlb(action, mmu->tlb);

	switch(action) {
		case ARMVM_ACTION_EXIT: __armvm_mmu_exit(mmu); break;
	}
}

static int armvm_mmu__vpa2ppa(uint32_t* ppa, armvm_mmu_p mmu)
{ LOG_ACTION(exit(-1)); } // TODO

armvm_mmu_p armvm_mmu_alloc(armvm_mmu_h const h2mmu, armvm_p const avm)
{
	ERR_NULL(h2mmu);
	ERR_NULL(avm);

	if(avm->config.trace.alloc) LOG();

	armvm_mmu_p mmu = handle_calloc((void*)h2mmu, 1, sizeof(armvm_mmu_t));

	mmu->armvm = avm;
	mmu->h2mmu = h2mmu;

	/* **** */

	ERR_NULL(armvm_tlb_alloc(&mmu->tlb, mmu, avm));

	/* **** */

	return(mmu);
}

int armvm_mmu_ifetch(uint32_t *const ir, uint32_t va, size_t size, armvm_mmu_p const mmu)
{
	uint32_t ppa = va;
	armvm_mem_callback_p src = 0;
	int tlb = 0;
	armvm_tlbe_p tlbe = 0;

	if(CP15_REG1_BIT(M)) {
		src = armvm_tlb_ifetch(va, &tlbe, mmu->tlb);

		if(src) {
			*ir = armvm_mem_callback_io(va, size, 0, src);
			return(1);
		}

		tlb = armvm_mmu__vpa2ppa(&ppa, mmu);
	}

	*ir = armvm_mem_access_read(ppa, size, &src, mmu->mem);
	if(!src)
		return(armvm_core_exception_prefetch_abort(mmu->core));

	if(tlb && src)
		armvm_tlb_fill_instruction_tlbe(va, src, tlbe);

	return(1);
}

int armvm_mmu_read(uint32_t *const read, uint32_t va, size_t size, armvm_mmu_p const mmu)
{
	uint32_t ppa = va;
	armvm_mem_callback_p src = 0;
	int tlb = 0;
	armvm_tlbe_p tlbe = 0;

	if(CP15_REG1_BIT(M)) {
		src = armvm_tlb_read(va, &tlbe, mmu->tlb);

		if(src) {
			*read = armvm_mem_callback_io(va, size, 0, src);
			return(1);
		}

		tlb = armvm_mmu__vpa2ppa(&ppa, mmu);
	}

	*read = armvm_mem_access_read(ppa, size, &src, mmu->mem);
	if(!src)
		return(armvm_core_exception_data_abort(mmu->core));

	if(tlb && src)
		armvm_tlb_fill_data_tlbe(va, src, tlbe);

	return(1);
}

int armvm_mmu_write(const uint32_t va, const size_t size, uint32_t write, armvm_mmu_p const mmu)
{
	uint32_t ppa = va;
	armvm_mem_callback_p dst = 0;
	int tlb = 0;
	armvm_tlbe_p tlbe = 0;

	if(CP15_REG1_BIT(M)) {
		dst = armvm_tlb_write(va, &tlbe, mmu->tlb);

		if(dst) {
			armvm_mem_callback_io(va, size, &write, dst);
			return(1);
		}

		tlb = armvm_mmu__vpa2ppa(&ppa, mmu);
	}

	dst = armvm_mem_access_write(ppa, size, write, mmu->mem);
	if(!dst)
		return(armvm_core_exception_data_abort(mmu->core));

	if(tlb && dst)
		armvm_tlb_fill_data_tlbe(va, dst, tlbe);

	return(1);
}
