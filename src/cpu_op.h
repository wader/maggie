/*
 * cpu_op.h
 * 	Function prototypes for all the functions that do the instruction emulation.
 * 	Defines cpu_op structure used in cpu.c and cpu_op_table.h.
 * 	Variables used when passing paramters to instructions.
 *
 * (c)2001 Mattias Wadman
 */

#ifndef __CPU_OP_H__
#define __CPU_OP_H__

struct cpu_op
{
	void (*function)(void);
	int parm1_size;
	int parm1_data;
	int parm2_size;
	int parm2_data;
	int cycles;
};

struct cpu_op *cpu_op_current;

// used to pass paramters
unsigned short cpu_op_p1_16;
unsigned short cpu_op_p2_16;
unsigned char cpu_op_p1;
unsigned char cpu_op_p2;

// used as temporary variables
unsigned short cpu_op_t1_16;
unsigned char cpu_op_t1;
unsigned char cpu_op_t2;

void cpu_op_adc_reg_imm();
void cpu_op_adc_reg_indreg16();
void cpu_op_adc_reg_reg();
void cpu_op_add_reg16_imm();
void cpu_op_add_reg16_reg16();
void cpu_op_add_reg_imm();
void cpu_op_add_reg_indreg16();
void cpu_op_add_reg_reg();
void cpu_op_and_imm();
void cpu_op_and_indreg16();
void cpu_op_and_reg();
void cpu_op_call_cond_imm16();
void cpu_op_call_imm16();
void cpu_op_ccf();
void cpu_op_cp_imm();
void cpu_op_cp_indreg16();
void cpu_op_cp_reg();
void cpu_op_cpl();
void cpu_op_daa();
void cpu_op_dec_indreg16();
void cpu_op_dec_reg();
void cpu_op_dec_reg16();
void cpu_op_di();
void cpu_op_ei();
void cpu_op_halt();
void cpu_op_inc_indreg16();
void cpu_op_inc_reg();
void cpu_op_inc_reg16();
void cpu_op_jp_cond_imm16();
void cpu_op_jp_imm16();
void cpu_op_jp_reg16();
void cpu_op_jr_cond_imm();
void cpu_op_jr_imm();
void cpu_op_ld_indimm16_reg();
void cpu_op_ld_indimm16_reg16();
void cpu_op_ld_indreg16_imm();
void cpu_op_ld_indreg16_reg();
void cpu_op_ld_reg16_imm16();
void cpu_op_ld_reg16_reg16();
void cpu_op_ld_reg_imm();
void cpu_op_ld_reg_indimm16();
void cpu_op_ld_reg_indreg16();
void cpu_op_ld_reg_reg();
void cpu_op_ldd_indreg16_reg();
void cpu_op_ldd_reg_indreg16();
void cpu_op_ldh_indimm_reg();
void cpu_op_ldh_indreg_reg();
void cpu_op_ldh_reg_indimm();
void cpu_op_ldh_reg_indreg();
void cpu_op_ldhl_reg16_imm();
void cpu_op_ldi_indreg16_reg();
void cpu_op_ldi_reg_indreg16();
void cpu_op_nop();
void cpu_op_or_imm();
void cpu_op_or_indreg16();
void cpu_op_or_reg();
void cpu_op_pop_reg16();
void cpu_op_push_reg16();
void cpu_op_ret();
void cpu_op_ret_cond();
void cpu_op_reti();
void cpu_op_rla();
void cpu_op_rlca();
void cpu_op_rra();
void cpu_op_rrca();
void cpu_op_rst();
void cpu_op_sbc_reg_imm();
void cpu_op_sbc_reg_indreg16();
void cpu_op_sbc_reg_reg();
void cpu_op_scf();
void cpu_op_stop();
void cpu_op_sub_imm();
void cpu_op_sub_indreg16();
void cpu_op_sub_reg();
void cpu_op_xor_imm();
void cpu_op_xor_indreg16();
void cpu_op_xor_reg();

// 0xcb prefixed opcodes

void cpu_op_bit_cons_indreg16();
void cpu_op_bit_cons_reg();
void cpu_op_res_cons_indreg16();
void cpu_op_res_cons_reg();
void cpu_op_rl_indreg16();
void cpu_op_rl_reg();
void cpu_op_rlc_indreg16();
void cpu_op_rlc_reg();
void cpu_op_rr_indreg16();
void cpu_op_rr_reg();
void cpu_op_rrc_indreg16();
void cpu_op_rrc_reg();
void cpu_op_set_cons_indreg16();
void cpu_op_set_cons_reg();
void cpu_op_sla_indreg16();
void cpu_op_sla_reg();
void cpu_op_sra_indreg16();
void cpu_op_sra_reg();
void cpu_op_srl_indreg16();
void cpu_op_srl_reg();
void cpu_op_swap_indreg16();
void cpu_op_swap_reg();

#endif
