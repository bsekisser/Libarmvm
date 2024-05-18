#pragma once

/* **** */

enum arm_dp_opcode_t {
	ARM_AND,
	ARM_EOR,
	ARM_SUB,
	ARM_RSB,
//
	ARM_ADD,
	ARM_ADC,
	ARM_SBC,
	ARM_RSC,
//
	ARM_TST,
	ARM_TEQ,
	ARM_CMP,
	ARM_CMN,
//
	ARM_ORR,
	ARM_MOV,
	ARM_BIC,
	ARM_MVN,
// TUUMB EXTENSIONS
	ARM_ASR,
	ARM_LSL,
	ARM_LSR,
	ARM_ROR,
//
	ARM_MUL,
	ARM_NEG,
};
