/*
 * cpu_op.c
 *	The functions that do the instruction emulation
 * 
 * (c)2001 Mattias Wadman
 */

#include "cpu_op.h"
#include "cpu.h"
#include "memory.h"
#include "maggie.h"

// ADC A,$xx
void cpu_op_adc_reg_imm()
{
	cpu_op_t1 = FLAG_BIT_VALUE_C;
	cpu_reg[REG_F] = FLAG_SET_ADC_H(cpu_reg[REG_A],cpu_op_p2,FLAG_BIT_VALUE_C) | FLAG_SET_ADC_C(cpu_reg[REG_A],cpu_op_p2,FLAG_BIT_VALUE_C);
	cpu_reg[REG_A] += cpu_op_p2+cpu_op_t1;
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]);
}

// ADC A,(HL)
void cpu_op_adc_reg_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL);
	cpu_op_t2 = FLAG_BIT_VALUE_C;
	cpu_reg[REG_F] = FLAG_SET_ADC_H(cpu_reg[REG_A],cpu_op_t1,FLAG_BIT_VALUE_C) | FLAG_SET_ADC_C(cpu_reg[REG_A],cpu_op_t1,FLAG_BIT_VALUE_C);
	cpu_reg[REG_A] += cpu_op_t1+cpu_op_t2;
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]);
}

// ADC A,B  ADC A,C  ADC A,D  ADC A,E  ADC A,H  ADC A,L  ADC A,A
void cpu_op_adc_reg_reg()
{
	cpu_op_t1 = FLAG_BIT_VALUE_C;
	cpu_reg[REG_F] = FLAG_SET_ADC_H(cpu_reg[REG_A],cpu_reg[cpu_op_p2],FLAG_BIT_VALUE_C) | FLAG_SET_ADC_C(cpu_reg[REG_A],cpu_reg[cpu_op_p2],FLAG_BIT_VALUE_C);
	cpu_reg[REG_A] += cpu_reg[cpu_op_p2]+cpu_op_t1;
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]);
}

// ADD SP,$xx
void cpu_op_add_reg16_imm()
{
	cpu_reg[REG_F] = FLAG_SET_ADD_C_16(cpu_reg_sp,SIGNED_8(cpu_op_p2)) | FLAG_SET_ADD_H_16(cpu_reg_pc,SIGNED_8(cpu_op_p2));
	cpu_reg_sp += SIGNED_8(cpu_op_p2);
}

// ADD HL,BC  ADD HL,DE  ADD HL,HL  ADD HL,SP
void cpu_op_add_reg16_reg16()
{
	if(cpu_op_p2 == REG_SP) // NOTE: special case, REG_SP not in reg array
		cpu_op_t1_16 = cpu_reg_sp;
	else
		cpu_op_t1_16 = REG_GET_16(cpu_op_p2);
	cpu_reg[REG_F] = FLAG_SET_ADD_H_16(REG_GET_HL,cpu_op_t1_16) | FLAG_SET_ADD_C_16(REG_GET_HL,cpu_op_t1_16) | FLAG_VALUE_Z;
	cpu_op_t1_16 += REG_GET_HL;
	REG_SET_HL(cpu_op_t1_16);
}

// ADD A,$xx
void cpu_op_add_reg_imm()
{
	cpu_reg[REG_F] = FLAG_SET_ADD_H(cpu_reg[REG_A],cpu_op_p2) | FLAG_SET_ADD_C(cpu_reg[REG_A],cpu_op_p2);
	cpu_reg[REG_A] += cpu_op_p2;	
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]);
}

// ADD A,(HL)
void cpu_op_add_reg_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL);
	cpu_reg[REG_F] = FLAG_SET_ADD_H(cpu_reg[REG_A],cpu_op_t1) | FLAG_SET_ADD_C(cpu_reg[REG_A],cpu_op_t1);
	cpu_reg[REG_A] += cpu_op_t1;
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]);
}

// ADD A,B  ADD A,C  ADD A,D  ADD A,E  ADD A,H  ADD A,L  ADD A,A
void cpu_op_add_reg_reg()
{	
	cpu_reg[REG_F] = FLAG_SET_ADD_H(cpu_reg[REG_A],cpu_reg[cpu_op_p2]) | FLAG_SET_ADD_C(cpu_reg[REG_A],cpu_reg[cpu_op_p2]);
	cpu_reg[REG_A] += cpu_reg[cpu_op_p2];
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]);
}

// AND $xx
void cpu_op_and_imm()
{
	cpu_reg[REG_A] &= cpu_op_p1;
	cpu_reg[REG_F] = FLAG_BIT_H | FLAG_SET_Z(cpu_reg[REG_A]);
}

// AND (HL)
void cpu_op_and_indreg16()
{
	cpu_reg[REG_A] &= MEMORY_READ(REG_GET_HL);
	cpu_reg[REG_F] = FLAG_BIT_H | FLAG_SET_Z(cpu_reg[REG_A]);
}

// AND B  AND C  AND D  AND E  AND H  AND L  AND A
void cpu_op_and_reg()
{
	cpu_reg[REG_A] &= cpu_reg[cpu_op_p1];
	cpu_reg[REG_F] = FLAG_BIT_H | FLAG_SET_Z(cpu_reg[REG_A]);
}

// CALL NZ,$aabb  CALL Z,$aabb  CALL NC,$aabb  CALL C,$aabb
void cpu_op_call_cond_imm16()
{
	if(COND_CHECK(cpu_op_p1))
	{
		memory_stack_push(cpu_reg_pc);
		cpu_reg_pc = cpu_op_p2_16;
		cpu_op_cycles += 3; // 3/6
	}
}

// CALL $aabb
void cpu_op_call_imm16()
{
	memory_stack_push(cpu_reg_pc);
	cpu_reg_pc = cpu_op_p1_16;	
}

// CCF
void cpu_op_ccf()
{
	cpu_reg[REG_F] = (FLAG_VALUE_C^FLAG_BIT_C) | FLAG_VALUE_Z;
}

// CP $xx
void cpu_op_cp_imm()
{
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[REG_A]-cpu_op_p1) | FLAG_SET_SUB_H(cpu_reg[REG_A],cpu_op_p1) | FLAG_SET_SUB_C(cpu_reg[REG_A],cpu_op_p1) | FLAG_BIT_N;
}

// CP (HL)
void cpu_op_cp_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL);
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[REG_A]-cpu_op_t1) | FLAG_SET_SUB_H(cpu_reg[REG_A],cpu_op_t1) | FLAG_SET_SUB_C(cpu_reg[REG_A],cpu_op_t1) | FLAG_BIT_N;
}

// CP B  CP C  CP D  CP E  CP H  CP L  CP A
void cpu_op_cp_reg()
{
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[REG_A]-cpu_reg[cpu_op_p1]) | FLAG_SET_SUB_H(cpu_reg[REG_A],cpu_reg[cpu_op_p1]) | FLAG_SET_SUB_C(cpu_reg[REG_A],cpu_reg[cpu_op_p1]) | FLAG_BIT_N;
}

// CPL
void cpu_op_cpl()
{
	cpu_reg[REG_A] = ~cpu_reg[REG_A];
	cpu_reg[REG_F] |= FLAG_BIT_N | FLAG_BIT_H;
}

// DAA
void cpu_op_daa()
{
	// algorithm taken from GBE

	if((cpu_reg[REG_A]&0x0f) > 0x09)
		cpu_reg[REG_A] += (FLAG_VALUE_N?-0x06:0x06);
	if((cpu_reg[REG_A]&0xf0) > 0x90)
	{
		cpu_reg[REG_A] += (FLAG_VALUE_N?-0x60:0x60);
		cpu_reg[REG_F] = FLAG_BIT_C | FLAG_VALUE_N;
	}

	cpu_reg[REG_F] &= ~FLAG_BIT_Z; // if Z flag is set and it should be reset on the next line
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]);
}

// DEC (HL)
void cpu_op_dec_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL)-1;
	MEMORY_WRITE(REG_GET_HL,cpu_op_t1);		
	cpu_reg[REG_F] = FLAG_SET_SUB_H((cpu_op_t1+1),1) | FLAG_SET_Z(cpu_op_t1) | FLAG_BIT_N | FLAG_VALUE_C;
	
}

// DEC B  DEC C  DEC D  DEC E  DEC H  DEC L  DEC A
void cpu_op_dec_reg()
{
	cpu_reg[cpu_op_p1]--;
	cpu_reg[REG_F] = FLAG_SET_SUB_H((cpu_reg[cpu_op_p1]+1),1) | FLAG_SET_Z(cpu_reg[cpu_op_p1]) | FLAG_BIT_N | FLAG_VALUE_C;
}

// DEC BC  DEC DE  DEC HL  DEC SP
void cpu_op_dec_reg16()
{
	if(cpu_op_p1 == REG_SP)
		cpu_reg_sp--;
	else
	{
		cpu_op_t1_16 = REG_GET_16(cpu_op_p1)-1;
		REG_SET_16(cpu_op_p1,cpu_op_t1_16);
	}
}

// DI
void cpu_op_di()
{
	cpu_ima = FALSE;
}

// EI
void cpu_op_ei()
{
	cpu_ima = TRUE;
}

// HALT
void cpu_op_halt()
{
	if(cpu_ime)
		cpu_halt = TRUE;
}

// INC (HL)
void cpu_op_inc_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL)+1;
	MEMORY_WRITE(REG_GET_HL,cpu_op_t1);
	cpu_reg[REG_F] = FLAG_SET_ADD_H((cpu_op_t1-1),1) | FLAG_SET_Z(cpu_op_t1) | FLAG_VALUE_C;	
}

// INC B  INC C  INC D  INC E  INC H  INC L  INC A
void cpu_op_inc_reg()
{
	cpu_reg[cpu_op_p1]++;
	cpu_reg[REG_F] = FLAG_SET_ADD_H((cpu_reg[cpu_op_p1]-1),1) | FLAG_SET_Z(cpu_reg[cpu_op_p1]) | FLAG_VALUE_C;
}

// INC BC  INC DE  INC HL  INC SP
void cpu_op_inc_reg16()
{
	if(cpu_op_p1 == REG_SP)
		cpu_reg_sp++;
	else
	{
		cpu_op_t1_16 = REG_GET_16(cpu_op_p1)+1;
		REG_SET_16(cpu_op_p1,cpu_op_t1_16);
	}
}

// JP NZ,$aabb  JP Z,$aabb  JP NC,$aabb  JP C,$aabb
void cpu_op_jp_cond_imm16()
{
	if(COND_CHECK(cpu_op_p1))
	{
		cpu_reg_pc = cpu_op_p2_16;
		cpu_op_cycles++; // 3/4
	}
}

// JP $aabb
void cpu_op_jp_imm16()
{
	cpu_reg_pc = cpu_op_p1_16;
}

// JP HL
void cpu_op_jp_reg16()
{
	cpu_reg_pc = REG_GET_HL;
}

// JR NZ,$xx  JR Z,$xx  JR NC,$xx  JR C,$xx
void cpu_op_jr_cond_imm()
{
	if(COND_CHECK(cpu_op_p1))
	{
		cpu_reg_pc += SIGNED_8(cpu_op_p2);
		cpu_op_cycles++; // 2/3
	}
}

// JR $xx
void cpu_op_jr_imm()
{
	cpu_reg_pc += SIGNED_8(cpu_op_p1);
}

// LD ($aabb),A
void cpu_op_ld_indimm16_reg()
{
	MEMORY_WRITE(cpu_op_p1_16,cpu_reg[REG_A]);
}

// LD ($aabb),SP
void cpu_op_ld_indimm16_reg16()
{
	// FIXME: is this right? write two bytes to memory? it takes a lot of cycles so..
	MEMORY_WRITE(cpu_op_p1_16,cpu_reg_sp&0x00ff);
	MEMORY_WRITE(cpu_op_p1_16+1,cpu_reg_sp>>8);
}

// LD (HL),$xx
void cpu_op_ld_indreg16_imm()
{
	MEMORY_WRITE(REG_GET_HL,cpu_op_p2);
}

// LD (BC),A  LD (DE),A  LD (HL),B  LD (HL),C  LD (HL),D  LD (HL),E  LD (HL),H  LD (HL),L  LD (HL),A
void cpu_op_ld_indreg16_reg()
{
	MEMORY_WRITE(REG_GET_16(cpu_op_p1),cpu_reg[cpu_op_p2]);
}

// LD BC,$aabb  LD DE,$aabb  LD HL,$aabb  LD SP,$aabb
void cpu_op_ld_reg16_imm16()
{
	if(cpu_op_p1 == REG_SP)
		cpu_reg_sp = cpu_op_p2_16;
	else
		REG_SET_16(cpu_op_p1,cpu_op_p2_16);
}

// LD SP,HL
void cpu_op_ld_reg16_reg16()
{
	cpu_reg_sp = REG_GET_HL;
}

// LD B,$xx  LD C,$xx  LD D,$xx  LD E,$xx  LD H,$xx  LD L,$xx  LD A,$xx
void cpu_op_ld_reg_imm()
{
	cpu_reg[cpu_op_p1] = cpu_op_p2;
}

// LD A,($aabb)
void cpu_op_ld_reg_indimm16()
{
	cpu_reg[REG_A] = MEMORY_READ(cpu_op_p2_16);
}

// LD A,(BC)  LD A,(DE)  LD B,(HL)  LD C,(HL)  LD D,(HL)  LD E,(HL)  LD H,(HL)  LD L,(HL)  LD A,(HL)
void cpu_op_ld_reg_indreg16()
{
	cpu_reg[cpu_op_p1] = MEMORY_READ(REG_GET_16(cpu_op_p2));
}

// LD B,B  LD B,C  LD B,D  LD B,E  LD B,H  LD B,L  LD B,A  LD C,B  LD C,C  LD C,D  LD C,E  LD C,H  LD C,L  LD C,A  LD D,B  LD D,C  LD D,D  LD D,E  LD D,H  LD D,L  LD D,A  LD E,B  LD E,C  LD E,D  LD E,E  LD E,H  LD E,L  LD E,A  LD H,B  LD H,C  LD H,D  LD H,E  LD H,H  LD H,L  LD H,A  LD L,B  LD L,C  LD L,D  LD L,E  LD L,H  LD L,L  LD H,A  LD A,B  LD A,C  LD A,D  LD A,E  LD A,H  LD A,L  LD A,A
void cpu_op_ld_reg_reg()
{
	cpu_reg[cpu_op_p1] = cpu_reg[cpu_op_p2];
}

// LDD (HL),A
void cpu_op_ldd_indreg16_reg()
{
	cpu_op_t1_16 = REG_GET_HL;
	MEMORY_WRITE(cpu_op_t1_16,cpu_reg[REG_A]);
	cpu_op_t1_16--;
	REG_SET_HL(cpu_op_t1_16);
}

// LDD A,(HL)
void cpu_op_ldd_reg_indreg16()
{
	cpu_op_t1_16 = REG_GET_HL;
	cpu_reg[REG_A] = MEMORY_READ(cpu_op_t1_16);
	cpu_op_t1_16--;
	REG_SET_HL(cpu_op_t1_16);
}

// LDH ($xx),A
void cpu_op_ldh_indimm_reg()
{
	MEMORY_WRITE(0xff00+cpu_op_p1,cpu_reg[REG_A]);
}

// LDH (C),A
void cpu_op_ldh_indreg_reg()
{
	MEMORY_WRITE(0xff00+cpu_reg[REG_C],cpu_reg[REG_A]);
}

// LDH A,($xx)
void cpu_op_ldh_reg_indimm()
{
	cpu_reg[REG_A] = MEMORY_READ(0xff00+cpu_op_p2);
}

// LDH A,(C)
void cpu_op_ldh_reg_indreg()
{
	cpu_reg[REG_A] = MEMORY_READ(0xff00+cpu_reg[REG_C]);
}

// LDHL SP,$xx
void cpu_op_ldhl_reg16_imm()
{
	REG_SET_HL(cpu_reg_sp+SIGNED_8(cpu_op_p2));
	cpu_reg[REG_F] = FLAG_SET_ADD_H_16(cpu_reg_sp,SIGNED_8(cpu_op_p2)) | FLAG_SET_ADD_C_16(cpu_reg_sp,SIGNED_8(cpu_op_p2));
}

// LDI (HL),A
void cpu_op_ldi_indreg16_reg()
{
	cpu_op_t1_16 = REG_GET_HL;
	MEMORY_WRITE(cpu_op_t1_16,cpu_reg[REG_A]);
	cpu_op_t1_16++;
	REG_SET_HL(cpu_op_t1_16);
}

// LDI A,(HL)
void cpu_op_ldi_reg_indreg16()
{
	cpu_op_t1_16 = REG_GET_HL;
	cpu_reg[REG_A] = MEMORY_READ(cpu_op_t1_16);
	cpu_op_t1_16++;
	REG_SET_HL(cpu_op_t1_16);
}

// NOP
void cpu_op_nop()
{
}

// OR $xx
void cpu_op_or_imm()
{
	cpu_reg[REG_A] |= cpu_op_p1;
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[REG_A]);
}

// OR (HL)
void cpu_op_or_indreg16()
{
	cpu_reg[REG_A] |= MEMORY_READ(REG_GET_HL);
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[REG_A]);
}

// OR B  OR C  OR D  OR E  OR H  OR L  OR A
void cpu_op_or_reg()
{
	cpu_reg[REG_A] |= cpu_reg[cpu_op_p1];
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[REG_A]);
}

// POP BC  POP DE  POP HL  POP AF
void cpu_op_pop_reg16()
{
	// need a temp variable here because the macro REG_SET_16 would use memory_stack_pop twice (!)
	cpu_op_t1_16 = memory_stack_pop();
	REG_SET_16(cpu_op_p1,cpu_op_t1_16);
}

// PUSH BC  PUSH DE  PUSH HL  PUSH AF
void cpu_op_push_reg16()
{
	memory_stack_push(REG_GET_16(cpu_op_p1));
}

// RET
void cpu_op_ret()
{
	cpu_reg_pc = memory_stack_pop();
}

// RET NZ  RET Z  RET NC  RET C
void cpu_op_ret_cond()
{
	if(COND_CHECK(cpu_op_p1))
	{
		cpu_reg_pc = memory_stack_pop();
		cpu_op_cycles +=  3; // 2/5
	}
}

// RETI
void cpu_op_reti()
{
	cpu_ima = TRUE;
	cpu_reg_pc = memory_stack_pop();
}

// RLA
void cpu_op_rla()
{
	cpu_op_t1 = (cpu_reg[REG_A]&BIT_7?FLAG_BIT_C:0);
	cpu_reg[REG_A] = FLAG_BIT_VALUE_C|(cpu_reg[REG_A]<<1);
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[REG_A]) | cpu_op_t1;
}

// RLCA
void cpu_op_rlca()
{	
	cpu_reg[REG_F] = (cpu_reg[REG_A]&BIT_7?FLAG_BIT_C:0);
	cpu_reg[REG_A] = (cpu_reg[REG_A]>>7)|(cpu_reg[REG_A]<<1);
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]);
}

// RRA
void cpu_op_rra()
{
	cpu_op_t1 = (cpu_reg[REG_A]&BIT_0?FLAG_BIT_C:0);
	cpu_reg[REG_A] = (FLAG_BIT_VALUE_C?BIT_7:0)|(cpu_reg[REG_A]>>1);
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[REG_A]) | cpu_op_t1;
}

// RRCA
void cpu_op_rrca()
{
	cpu_reg[REG_F] = (cpu_reg[REG_A]&BIT_0?FLAG_BIT_C:0);
	cpu_reg[REG_A] = (cpu_reg[REG_A]<<7)|(cpu_reg[REG_A]>>1);
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]);
}

// RST $00  RST $08  RST $10  RST $18  RST $20  RST $28  RST $30  RST $38
void cpu_op_rst()
{
	memory_stack_push(cpu_reg_pc);
	cpu_reg_pc = cpu_op_p1;
}

// SBC A,$xx
void cpu_op_sbc_reg_imm()
{
	cpu_op_t1 = FLAG_BIT_VALUE_C;
	cpu_reg[REG_F] = FLAG_SET_SBC_H(cpu_reg[REG_A],cpu_op_p2,FLAG_BIT_VALUE_C) | FLAG_SET_SBC_C(cpu_reg[REG_A],cpu_op_p2,FLAG_BIT_VALUE_C);
	cpu_reg[REG_A] -= cpu_op_p2+cpu_op_t1;
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]) | FLAG_BIT_N;
}

// SBC A,(HL)
void cpu_op_sbc_reg_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL);
	cpu_op_t2 = FLAG_BIT_VALUE_C;
	cpu_reg[REG_F] = FLAG_SET_SBC_H(cpu_reg[REG_A],cpu_op_t1,FLAG_BIT_VALUE_C) | FLAG_SET_SBC_C(cpu_reg[REG_A],cpu_op_t1,FLAG_BIT_VALUE_C);
	cpu_reg[REG_A] -= cpu_op_t1+cpu_op_t2;
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]) | FLAG_BIT_N;
}

// SBC A,B  SBC A,C  SBC A,D  SBC A,E  SBC A,H  SBC A,L  SBC A,A
void cpu_op_sbc_reg_reg()
{
	cpu_op_t1 = FLAG_BIT_VALUE_C;
	cpu_reg[REG_F] = FLAG_SET_SBC_H(cpu_reg[REG_A],cpu_reg[cpu_op_p2],FLAG_BIT_VALUE_C) | FLAG_SET_SBC_C(cpu_reg[REG_A],cpu_reg[cpu_op_p2],FLAG_BIT_VALUE_C);
	cpu_reg[REG_A] -= cpu_reg[cpu_op_p2]+cpu_op_t1;
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]) | FLAG_BIT_N;
}

// SCF
void cpu_op_scf()
{
	cpu_reg[REG_F] = FLAG_BIT_C | FLAG_VALUE_Z;
}

// STOP
void cpu_op_stop()
{
	cpu_stop = TRUE;

	if(cpu_op_p1 != 0x00)
		maggie_error("WARNING: cpu_op_stop: unclean stop (0x%2x)\n",cpu_op_p1);
}

// SUB $xx
void cpu_op_sub_imm()
{
	cpu_reg[REG_F] = FLAG_SET_SUB_H(cpu_reg[REG_A],cpu_op_p1) | FLAG_SET_SUB_C(cpu_reg[REG_A],cpu_op_p1) | FLAG_BIT_N;
	cpu_reg[REG_A] -= cpu_op_p1;
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]);
}

// SUB (HL)
void cpu_op_sub_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL);
	cpu_reg[REG_F] = FLAG_SET_SUB_H(cpu_reg[REG_A],cpu_op_t1) | FLAG_SET_SUB_C(cpu_reg[REG_A],cpu_op_t1) | FLAG_BIT_N;
	cpu_reg[REG_A] -= cpu_op_t1;
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]);
}

// SUB B  SUB C  SUB D  SUB E  SUB H  SUB L  SUB A
void cpu_op_sub_reg()
{
	cpu_reg[REG_F] = FLAG_SET_SUB_H(cpu_reg[REG_A],cpu_reg[cpu_op_p1]) | FLAG_SET_SUB_C(cpu_reg[REG_A],cpu_reg[cpu_op_p1]) | FLAG_BIT_N;
	cpu_reg[REG_A] -= cpu_reg[cpu_op_p1];
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[REG_A]);
}

// XOR $xx
void cpu_op_xor_imm()
{
	cpu_reg[REG_A] ^= cpu_op_p1;
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[REG_A]);
}

// XOR (HL)
void cpu_op_xor_indreg16()
{
	cpu_reg[REG_A] ^= MEMORY_READ(REG_GET_HL);
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[REG_A]);
}

// XOR B  XOR C  XOR D  XOR E  XOR H  XOR L  XOR A
void cpu_op_xor_reg()
{
	cpu_reg[REG_A] ^= cpu_reg[cpu_op_p1];
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[REG_A]);
}


// 0xcb operations

// BIT 0,(HL)  BIT 1,(HL)  BIT 2,(HL)  BIT 3,(HL)  BIT 4,(HL)  BIT 5,(HL)  BIT 6,(HL)  BIT 7,(HL)
void cpu_op_bit_cons_indreg16()
{
	cpu_reg[REG_F] = FLAG_SET_Z(MEMORY_READ(REG_GET_HL)&(1<<cpu_op_p1)) | FLAG_BIT_H | FLAG_VALUE_C;
}

// BIT 0,B  BIT 0,C  BIT 0,D  BIT 0,E  BIT 0,H  BIT 0,L  BIT 0,A  BIT 1,B  BIT 1,C  BIT 1,D  BIT 1,E  BIT 1,H  BIT 1,L  BIT 1,A  BIT 2,B  BIT 2,C  BIT 2,D  BIT 2,E  BIT 2,H  BIT 2,L  BIT 2,A  BIT 3,B  BIT 3,C  BIT 3,D  BIT 3,E  BIT 3,H  BIT 3,L  BIT 3,A  BIT 4,B  BIT 4,C  BIT 4,D  BIT 4,E  BIT 4,H  BIT 4,L  BIT 4,A  BIT 5,B  BIT 5,C  BIT 5,D  BIT 5,E  BIT 5,H  BIT 5,L  BIT 5,A  BIT 6,B  BIT 6,C  BIT 6,D  BIT 6,E  BIT 6,H  BIT 6,L  BIT 6,A  BIT 7,B  BIT 7,C  BIT 7,D  BIT 7,E  BIT 7,H  BIT 7,A
void cpu_op_bit_cons_reg()
{
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[cpu_op_p2]&(1<<cpu_op_p1)) | FLAG_BIT_H | FLAG_VALUE_C;
}

// RES 0,(HL)  RES 1,(HL)  RES 2,(HL)  RES 3,(HL)  RES 4,(HL)  RES 5,(HL)  RES 6,(HL)  RES 7,(HL)
void cpu_op_res_cons_indreg16()
{
	MEMORY_WRITE(REG_GET_HL,MEMORY_READ(REG_GET_HL)&(~(1<<cpu_op_p1)));	
}

// RES 0,B  RES 0,C  RES 0,D  RES 0,E  RES 0,H  RES 0,L  RES 0,A  RES 1,B  RES 1,C  RES 1,D  RES 1,E  RES 1,H  RES 1,L  RES 1,A  RES 2,B  RES 2,C  RES 2,D  RES 2,E  RES 2,H  RES 2,L  RES 2,A  RES 3,B  RES 3,C  RES 3,D  RES 3,E  RES 3,H  RES 3,L  RES 3,A  RES 4,B  RES 4,C  RES 4,D  RES 4,E  RES 4,H  RES 4,L  RES 4,A  RES 5,B  RES 5,C  RES 5,D  RES 5,E  RES 5,H  RES 5,L  RES 5,A  RES 6,B  RES 6,C  RES 6,D  RES 6,E  RES 6,H  RES 6,L  RES 6,A  RES 7,B  RES 7,C  RES 7,D  RES 7,E  RES 7,H  RES 7,L  RES 7,A
void cpu_op_res_cons_reg()
{
	cpu_reg[cpu_op_p2] &= ~(1<<cpu_op_p1);
}

// RL (HL)
void cpu_op_rl_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL);
	cpu_op_t2 = (cpu_op_t1&BIT_7?FLAG_BIT_C:0);	
	cpu_op_t1 = FLAG_BIT_VALUE_C|(cpu_op_t1<<1);
	MEMORY_WRITE(REG_GET_HL,cpu_op_t1);
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_op_t1)|cpu_op_t2;
}

// RL B  RL C  RL D  RL E  RL H  RL L  RL A
void cpu_op_rl_reg()
{
	cpu_op_t1 = (cpu_reg[cpu_op_p1]&BIT_7?FLAG_BIT_C:0);
	cpu_reg[cpu_op_p1] = FLAG_BIT_VALUE_C|(cpu_reg[cpu_op_p1]<<1);
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[cpu_op_p1])|cpu_op_t1;
}

// RLC (HL)
void cpu_op_rlc_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL);
	cpu_reg[REG_F] = (cpu_op_t1&BIT_7?FLAG_BIT_C:0);
	cpu_op_t1 = (cpu_op_t1>>7)|(cpu_op_t1<<1);
	MEMORY_WRITE(REG_GET_HL,cpu_op_t1);
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_op_t1);
}

// RLC B  RLC C  RLC D  RLC E  RLC H  RLC L  RLC A
void cpu_op_rlc_reg()
{
	cpu_reg[REG_F] = (cpu_reg[cpu_op_p1]&BIT_7?FLAG_BIT_C:0);
	cpu_reg[cpu_op_p1] = (cpu_reg[cpu_op_p1]>>7)|(cpu_reg[cpu_op_p1]<<1);
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[cpu_op_p1]);
}

// RR (HL)
void cpu_op_rr_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL);
	cpu_op_t2 = (cpu_op_t1&BIT_0?FLAG_BIT_C:0);
	cpu_op_t1 = (FLAG_BIT_VALUE_C?BIT_7:0)|(cpu_op_t1>>1);
	MEMORY_WRITE(REG_GET_HL,cpu_op_t1);
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_op_t1)|cpu_op_t2;
}

// RR B  RR C  RR D  RR E  RR H  RR L  RR A
void cpu_op_rr_reg()
{
	cpu_op_t1 = (cpu_reg[cpu_op_p1]&BIT_0?FLAG_BIT_C:0);
	cpu_reg[cpu_op_p1] = (FLAG_BIT_VALUE_C?BIT_7:0)|(cpu_reg[cpu_op_p1]>>1);
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[cpu_op_p1])|cpu_op_t1;
}

// RRC (HL)
void cpu_op_rrc_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL);
	cpu_reg[REG_F] = (cpu_op_t1&BIT_0?FLAG_BIT_C:0);
	cpu_op_t1 = (cpu_op_t1&BIT_0?BIT_7:0)|(cpu_op_t1>>1);
	MEMORY_WRITE(REG_GET_HL,cpu_op_t1);
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_op_t1);
}

// RRC B  RRC C  RRC D  RRC E  RRC H  RRC L  RRC A
void cpu_op_rrc_reg()
{
	cpu_reg[REG_F] = (cpu_reg[cpu_op_p1]&BIT_0?FLAG_BIT_C:0);
	cpu_reg[cpu_op_p1] = (cpu_reg[cpu_op_p1]<<7)|(cpu_reg[cpu_op_p1]>>1);
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[cpu_op_p1]);
}

// SET 0,(HL)  SET 1,(HL)  SET 2,(HL)  SET 3,(HL)  SET 4,(HL)  SET 5,(HL)  SET 6,(HL)  SET 7,(HL)
void cpu_op_set_cons_indreg16()
{
	MEMORY_WRITE(REG_GET_HL,MEMORY_READ(REG_GET_HL)|(1<<cpu_op_p1));	
}

// SET 0,B  SET 0,C  SET 0,D  SET 0,E  SET 0,H  SET 0,L  SET 0,A  SET 1,B  SET 1,C  SET 1,D  SET 1,E  SET 1,H  SET 1,L  SET 1,A  SET 2,B  SET 2,C  SET 2,D  SET 2,E  SET 2,H  SET 2,L  SET 2,A  SET 3,B  SET 3,C  SET 3,D  SET 3,E  SET 3,H  SET 3,L  SET 3,A  SET 4,B  SET 4,C  SET 4,D  SET 4,E  SET 4,H  SET 4,L  SET 4,A  SET 5,B  SET 5,C  SET 5,D  SET 5,E  SET 5,H  SET 5,L  SET 5,A  SET 6,B  SET 6,C  SET 6,D  SET 6,E  SET 6,H  SET 6,L  SET 6,A  SET 7,B  SET 7,C  SET 7,D  SET 7,E  SET 7,H  SET 7,L  SET 7,A
void cpu_op_set_cons_reg()
{
	cpu_reg[cpu_op_p2] |= 1<<cpu_op_p1;
}

// SLA (HL)
void cpu_op_sla_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL);
	cpu_reg[REG_F] = (cpu_op_t1&BIT_7?FLAG_BIT_C:0);
	cpu_op_t1 <<= 1;
	MEMORY_WRITE(REG_GET_HL,cpu_op_t1);
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_op_t1);	
}

// SLA B  SLA C  SLA D  SLA E  SLA H  SLA L  SLA A
void cpu_op_sla_reg()
{
	cpu_reg[REG_F] = (cpu_reg[cpu_op_p1]&BIT_7?FLAG_BIT_C:0);
	cpu_reg[cpu_op_p1] <<= 1;
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[cpu_op_p1]);	
}

// SRA (HL)
void cpu_op_sra_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL);
	cpu_reg[REG_F] = (cpu_op_t1&BIT_0?FLAG_BIT_C:0);
	cpu_op_t1 = (cpu_op_t1&BIT_7)|(cpu_op_t1>>1);
	MEMORY_WRITE(REG_GET_HL,cpu_op_t1);
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_op_t1);	
}

// SRA B  SRA C  SRA D  SRA E  SRA H  SRA L  SRA A
void cpu_op_sra_reg()
{
	cpu_reg[REG_F] = (cpu_reg[cpu_op_p1]&BIT_0?FLAG_BIT_C:0);
	cpu_reg[cpu_op_p1] = (cpu_reg[cpu_op_p1]&BIT_7)|(cpu_reg[cpu_op_p1]>>1);
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[cpu_op_p1]);
}

// SRL (HL)
void cpu_op_srl_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL);
	cpu_reg[REG_F] = (cpu_op_t1&BIT_0?FLAG_BIT_C:0);
	cpu_op_t1 >>= 1;
	MEMORY_WRITE(REG_GET_HL,cpu_op_t1);
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_op_t1);	
}

// SRL B  SRL C  SRL D  SRL E  SRL H  SRL L  SRL A
void cpu_op_srl_reg()
{
	cpu_reg[REG_F] = (cpu_reg[cpu_op_p1]&BIT_0?FLAG_BIT_C:0);
	cpu_reg[cpu_op_p1] >>= 1;
	cpu_reg[REG_F] |= FLAG_SET_Z(cpu_reg[cpu_op_p1]);
}

// SWAP (HL)
void cpu_op_swap_indreg16()
{
	cpu_op_t1 = MEMORY_READ(REG_GET_HL);
	cpu_op_t1 = (cpu_op_t1>>4)|(cpu_op_t1<<4);
	MEMORY_WRITE(REG_GET_HL,cpu_op_t1);
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_op_t1);
}

// SWAP B  SWAP C  SWAP D  SWAP E  SWAP H  SWAP L  SWAP A
void cpu_op_swap_reg()
{	
	cpu_reg[cpu_op_p1] = (cpu_reg[cpu_op_p1]>>4)|(cpu_reg[cpu_op_p1]<<4);
	cpu_reg[REG_F] = FLAG_SET_Z(cpu_reg[cpu_op_p1]);
}

