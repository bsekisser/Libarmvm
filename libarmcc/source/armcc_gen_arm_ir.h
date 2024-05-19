#pragma once

/* **** */

#include "armcc_t.h"

/* **** */

extern "C" {
	#include "libbse/include/bitfield.h"
	#include "libbse/include/log.h"
}

/* **** */

#include <assert.h>
#include <stdint.h>

/* **** */

static uint32_t _gen_arm_ir__cc(const arm_condition_t cc)
{
	assert(0 == (cc & ~15));
	return(pbBFMOV(cc, 0, 4, 28));
}

static uint32_t _gen_arm_ir__dp_opcode(const arm_dp_opcode_t opcode, const unsigned s)
{
	assert(0 == (opcode & ~15));
	return(pbBFMOV(opcode, 0, 4, 21) | (s << 20));
}

static uint32_t _gen_arm_ir__group(const unsigned group)
{
	assert(0 == (group & ~7));
	return(pbBFMOV(group, 0, 3, 25));
}

static uint32_t _gen_arm_ir__rd(const arm_reg_t rd)
{
	assert(0 == (rd & ~15));
	return(pbBFMOV(rd, 0, 4, 12));
}

static uint32_t _gen_arm_ir__rn(const arm_reg_t rn)
{
	assert(0 == (rn & ~15));
	return(pbBFMOV(rn, 0, 4, 16));
}

static uint32_t _gen_arm_ir__rm(const arm_reg_t rm)
{
	assert(0 == (rm & ~15));
	return(rm & 15);
}

/* **** */

static uint32_t gen_arm_ir__rd_rn(const arm_reg_t rd, const arm_reg_t rn)
{ return(_gen_arm_ir__rd(rd) | _gen_arm_ir__rn(rn)); }

static uint32_t gen_arm_ir__rd_rn_rm(const arm_reg_t rd, const arm_reg_t rn,
	const arm_reg_t rm)
{ return(gen_arm_ir__rd_rn(rd, rn) | _gen_arm_ir__rm(rm)); }

/* **** */

static uint32_t gen_arm_ir__cc_group_x(const arm_condition_t cc,
	const unsigned group, const uint32_t x)
{ return(_gen_arm_ir__cc(cc) | _gen_arm_ir__group(group) | x); }

static uint32_t gen_arm_ir__group_x(const unsigned group, const uint32_t x)
{ return(gen_arm_ir__cc_group_x(CC_AL, group, x)); }
