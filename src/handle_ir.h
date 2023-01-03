/*
    Notes here:
        1. RISC-V templates below
        2. 目前处理 Binary Instr (of Koopa Format) -> RISC-V 的思路：
            - 每条指令对应一个 Reg，指令执行完存在对应的 Reg里
            - 这个映射关系用一个 map 维护
            - 如果 lhs or rhs 有用到寄存器，那么直接引用；否则把立即数 li 到 a1/a2 里
        
        
*/

#pragma once

#include "koopa.h"

// Basic Visit Function (Walk Through Koopa Tree in a recursion type)
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
// ------------------------ `koopa_raw_value_t` Visit ------------------------------------------------------
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &INT);
void Visit(const koopa_raw_binary_t &BinOP);

int handle_str_ir(const char *str);

// -------------------------Target Langudage Part (RISC-V)-------------------------------------

// RISC-V 加载和移动类
#define risc_mv(rd,rs) "   mv "<<rd<<", "<<rs<<endl // Reg[rd] = Reg[rs]
#define risc_la(rd,label) "   la "<<rd<<", "<<label<<endl // Reg[rd] = GetAddr(label)
#define risc_li(rd,imm) "   li "<<rd<<", "<<imm<<endl // Reg[rd] = imm

// RISC-V 运算类 下面的 imm 都是 imm12 ! 只有 12 bits 宽
#define risc_add(rd,rs1,rs2) "   add "<<rd<<", "<<rs1<<", "<<rs2<<endl // Reg[rd] = Reg[rs1] + Reg[rs2]
#define risc_addi(rd,rs1,imm) "   addi "<<rd<<", "<<rs1<<", "<<imm<<endl // Reg[rd] = Reg[rs1] + imm
#define risc_sub(rd,rs1,rs2) "   sub "<<rd<<", "<<rs1<<", "<<rs2<<endl // Reg[rd] = Reg[rs1] - Reg[rs2]
#define risc_slt(rd,rs1,rs2) "   slt "<<rd<<", "<<rs1<<", "<<rs2<<endl // Reg[rd] = Reg[rs1] < Reg[rs2]
#define risc_sgt(rd,rs1,rs2) "   sgt "<<rd<<", "<<rs1<<", "<<rs2<<endl // Reg[rd] = Reg[rs1] > Reg[rs2]
#define risc_seqz(rd,rs) "   seqz "<<rd<<", "<<rs<<endl // Reg[rd] = Reg[rs] == 0
#define risc_snez(rd,rs) "   snez "<<rd<<", "<<rs<<endl // Reg[rd] = Reg[rs] != 0
#define risc_xor(rd,rs1,rs2) "   xor "<<rd<<", "<<rs1<<", "<<rs2<<endl // Reg[rd] = Reg[rs1] ^ Reg[rs2]
#define risc_xori(rd,rs1,imm) "   xori "<<rd<<", "<<rs1<<", "<<imm<<endl // Reg[rd] = Reg[rs1] ^ imm
#define risc_or(rd,rs1,rs2) "   or "<<rd<<", "<<rs1<<", "<<rs2<<endl // Reg[rd] = Reg[rs1] | Reg[rs2]
#define risc_ori(rd,rs1,imm) "   ori "<<rd<<", "<<rs1<<", "<<imm<<endl // Reg[rd] = Reg[rs1] | imm
#define risc_and(rd,rs1,rs2) "   and "<<rd<<", "<<rs1<<", "<<rs2<<endl // Reg[rd] = Reg[rs1] & Reg[rs2]
#define risc_andi(rd,rs1,imm) "   andi "<<rd<<", "<<rs1<<", "<<imm<<endl // Reg[rd] = Reg[rs1] & imm
#define risc_sll(rd,rs1,rs2) "   sll "<<rd<<", "<<rs1<<", "<<rs2<<endl // Reg[rd] = Reg[rs1] << Reg[rs2]
#define risc_srl(rd,rs1,rs2) "   srl "<<rd<<", "<<rs1<<", "<<rs2<<endl // Reg[rd] = Reg[rs1] >> Reg[rs2] (Logical)
#define risc_sra(rd,rs1,rs2) "   sra "<<rd<<", "<<rs1<<", "<<rs2<<endl // Reg[rd] = Reg[rs1] >> Reg[rs2] (Arith)
#define risc_mul(rd,rs1,rs2) "   mul "<<rd<<", "<<rs1<<", "<<rs2<<endl // Reg[rd] = Reg[rs1] * Reg[rs2]
#define risc_div(rd,rs1,rs2) "   div "<<rd<<", "<<rs1<<", "<<rs2<<endl // Reg[rd] = Reg[rs1] / Reg[rs2]
#define risc_rem(rd,rs1,rs2) "   rem "<<rd<<", "<<rs1<<", "<<rs2<<endl // Reg[rd] = Reg[rs1] % Reg[rs2]

// RISC-V 访存类 下面的 imm 都是 imm12 ! 只有 12 bits 宽
#define risc_lw(rd,rs,imm) "   lw "<<rs<<", "<<imm<<'('<<rd<<')'<<endl // Reg[rs] = Mem[Reg[rd] + imm]
#define risc_sw(rd,rs,imm) "   sw "<<rs<<", "<<imm<<'('<<rd<<')'<<endl // Mem[Reg[rd] + imm] = Reg[rs]

// RISC-V 控制转移类
#define risc_beqz(rs,label) "   beqz "<<rs<<", "<<label<<endl // if(Reg[rs] == 0) jump-to label
#define risc_bnez(rs,label) "   bnez "<<rs<<", "<<label<<endl // if(Reg[rs] != 0) jump-to label
#define risc_j(label) "   j "<<label<<endl // jump-to label
#define risc_call(label) "   call "<<label<<endl // load addr of next-instr into REG[ra], and jump-to label
#define risc_ret "   ret"<<endl // jump-to REG[ra]



// 杂：Other Functions
// inline string Binary2Register(koopa_raw_binary_t* addr);  ------> 从Binary Instr Addr 到 Reg_Name 的映射
// inline void binary2risc(koopa_raw_binary_op_t optype, string o1, string o2);  ----> Binary Instr 到 riscv代码映射