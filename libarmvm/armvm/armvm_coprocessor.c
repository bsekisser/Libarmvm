#include "armvm_coprocessor.h"
#include "armvm_core.h"
#include "armvm_action.h"
#include "armvm.h"

/* **** */

#include "armvm_coprocessor_glue.h"
#include "armvm_core_glue.h"

/* **** */

#include "libarm/include/arm_disasm.h"

#include "libbse/include/err_test.h"
#include "libbse/include/handle.h"
#include "libbse/include/log.h"
#include "libbse/include/mem_access.h"

/* **** */

typedef struct armvm_coprocessor_callback_t* armvm_coprocessor_callback_p;
typedef struct armvm_coprocessor_callback_t {
	armvm_coprocessor_callback_fn fn;
	void* param;
}armvm_coprocessor_callback_t;

typedef struct armvm_coprocessor_t {
	armvm_coprocessor_callback_t cp15[16][16][7][7];
	uint32_t cp15r[16][16][7][7];
//
	armvm_p armvm;
	armvm_core_p core;
//
	armvm_coprocessor_h h2cp;
}armvm_coprocessor_t;

/* **** */

static void __armvm_coprocessor_alloc_init(armvm_coprocessor_p const cp)
{
	if(cp->armvm->config.trace.alloc_init) LOG();

	cp->core = cp->armvm->core;
}

static void __armvm_coprocessor_exit(armvm_coprocessor_p const cp)
{
	if(cp->armvm->config.trace.exit) LOG();

	handle_free((void*)cp->h2cp);
}

/* **** */

static uint32_t* _armvm_coprocessor__cp15r_rmw(armvm_coprocessor_p const  cp,
	const uint32_t ir)
{
	uint32_t *const p2r = &cp->cp15r[ir_cp_crn(ir)][ir_cp_crm(ir)][ir_cp_op1(ir)][ir_cp_op2(ir)];
	return(p2r);
}

/* **** */

static armvm_coprocessor_callback_p _armvm_coprocessor_callback(armvm_coprocessor_p const  cp,
	const uint32_t ir)
{
	return(&cp->cp15[ir_cp_crn(ir)][ir_cp_crm(ir)][ir_cp_op1(ir)][ir_cp_op2(ir)]);
}

static uint32_t _armvm_coprocessor_cp15r(armvm_coprocessor_p const  cp,
	const uint32_t ir, uint32_t* write)
{
	return(mem_32_access(_armvm_coprocessor__cp15r_rmw(cp, ir), write));
}

/*static uint32_t _armvm_cp15_0_1_0_0_access(void *const param, uint32_t *const write)
{
	armvm_coprocessor_p cp = param;

	return(mem_32_access(&cp->armvm->cp15r1, write));
}*/

void armvm_coprocessor(armvm_coprocessor_p const cp, const unsigned action)
{
	ERR_NULL(cp);

	switch(action) {
		case ARMVM_ACTION_ALLOC_INIT: __armvm_coprocessor_alloc_init(cp); break;
	}
//
//
	switch(action) {
		case ARMVM_ACTION_EXIT: __armvm_coprocessor_exit(cp); break;
	}
}

uint32_t armvm_coprocessor_access(armvm_coprocessor_p const cp, uint32_t *const write)
{
	armvm_core_p const core = cp->core;

	if(15 == ir_cp_num(IR)) {
		armvm_coprocessor_callback_p const cb = _armvm_coprocessor_callback(cp, IR);

		if(cb->fn)
			return(cb->fn(cb->param, write));
		else
			return(_armvm_coprocessor_cp15r(cp, IR, write));
	}

	arm_disasm_arm(IP, IR);
	LOG_ACTION(exit(-1));
	return(-1);
}


armvm_coprocessor_p armvm_coprocessor_alloc(armvm_p const avm,
	armvm_coprocessor_h const h2cp)
{
	ERR_NULL(h2cp);
	ERR_NULL(avm);

	if(avm->config.trace.alloc) LOG();

	armvm_coprocessor_p const cp =
		handle_calloc((void*)h2cp, 1, sizeof(armvm_coprocessor_t));

	ERR_NULL(cp);

	/* **** */

	cp->armvm = avm;
	cp->h2cp = h2cp;

	/* **** */

//	armvm_coprocessor_register_callback(cp, cp15(0, 1, 0, 0),
//		_armvm_cp15_0_1_0_0_access, cp);

	return(cp);
}

uint32_t armvm_coprocessor_cp15r(armvm_coprocessor_p const cp, const uint32_t cpx, uint32_t *const write)
{ return(_armvm_coprocessor_cp15r(cp, cpx, write)); }

void armvm_coprocessor_cp15r_bmas(armvm_coprocessor_p const cp, const uint32_t cpx,
	const unsigned bit, const unsigned set)
{
	uint32_t *const p2v = _armvm_coprocessor__cp15r_rmw(cp, cpx);
	BMAS(*p2v, bit, set);
}

uint32_t* armvm_coprocessor_cp15r_rmw(armvm_coprocessor_p const cp, const uint32_t cpx)
{ return(_armvm_coprocessor__cp15r_rmw(cp, cpx)); }

void armvm_coprocessor_register_callback(armvm_coprocessor_p const cp,
	uint32_t cpx,
	armvm_coprocessor_callback_fn const fn, void *const param)
{
	if(15 == ir_cp_num(cpx)) {
		armvm_coprocessor_callback_p const cb = _armvm_coprocessor_callback(cp, cpx);

		cb->fn = fn;
		cb->param = param;
	}
}
