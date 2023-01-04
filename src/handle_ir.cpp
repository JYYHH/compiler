#include <string>
#include <memory>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "handle_ir.h"
#include "koopa.h"

using namespace std;

// --------------------------- MainTain the RISC-V Stack -----------------------
const unsigned int Mask_16 = (~0u) << 16;
unsigned int need_size_on_stack, stack_size;
inline void ClearStack(){ need_size_on_stack = 0;}
inline void GrowStack(const unsigned int x){ need_size_on_stack += x;}
inline unsigned int GetStackSize(){ return need_size_on_stack; }
inline void AlignStackSize(unsigned int &x){ x = (x + (~Mask_16)) & Mask_16;}
unordered_map<koopa_raw_binary_t* , unsigned int> mmp; // instr -> offset

// 将栈空间某偏移量的数据 load 到指定寄存器中
inline void Offset2Register(unsigned int &OFFSET, string reg_name){
  if (OFFSET < (1<<11)){
    cout << risc_lw("sp", reg_name, OFFSET);
  }
  else{
    cout << risc_li("t1", OFFSET);
    cout << risc_add("t1", "t1", "sp");
    cout << risc_lw("t1", reg_name, 0);
  }
}
// 前身是 Binary2Register ： 将某条二进制指令的结果load到指定寄存器中
inline void Binary2Register(koopa_raw_binary_t* addr, string reg_name){
  unsigned int offset = mmp[addr];
  Offset2Register(offset, reg_name);
}


inline void pre_func(){
  if (stack_size <= (1<<11)){
    cout << risc_addi("sp", "sp", -(int)stack_size);
  }
  else{
    cout << risc_li("t1", -(int)stack_size);
    cout << risc_add("sp", "t1", "sp");
  }
}

inline void after_func(){
  if (stack_size < (1<<11)){
    cout << risc_addi("sp", "sp", stack_size);
  }
  else{
    cout << risc_li("t1", stack_size);
    cout << risc_add("sp", "t1", "sp");
  }
}


// ------------------------ Basic Visit ------------------------------------------------------

void Visit(const koopa_raw_program_t &program, const int mode){
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  if (program.values.len > 0 && !(mode))
    printf("   .data\n");
  Visit(program.values, mode);
  // 访问所有函数
  if (!mode)
    printf("   .text\n   .globl main\n");
  Visit(program.funcs, mode);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice, const int mode) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr), mode);
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr), mode);
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr), mode);
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func, const int mode) {
  // 执行一些其他的必要操作
  ClearStack();
  Visit(func->bbs, 1); // 预计算出需要的栈空间
  stack_size = need_size_on_stack; // 存在 stack_size 里
  AlignStackSize(stack_size);
  ClearStack();

  pre_func();

  if(func->name && !(mode)) 
    printf("%s:\n", func->name + 1);
  // 访问所有基本块
  Visit(func->bbs, mode);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb, const int mode) {
  // 执行一些其他的必要操作
  if(bb->name && !(mode))
    printf(" %s:\n", bb->name + 1);
  // 访问所有指令
  Visit(bb->insts, mode);
}

// --------------------- next part is important-----------------------------
// string reg_name[13] = {
//   "a0",
//   "t1", // 还有你
//   "t2", // 钦点你
//   "t3",
//   "t4",
//   "t5",
//   "t6",
//   "a3",
//   "a4",
//   "a5",
//   "a6",
//   "a7",
//   "t0"
// };

// -----------------------------------访问指令-------------------------------------
void Visit(const koopa_raw_value_t &value, const int mode) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret, mode);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      Visit(kind.data.integer, mode);
      break;
    case KOOPA_RVT_BINARY:
      // 二进制操作指令
      if (!mmp.count((koopa_raw_binary_t* )(&kind.data.binary)))
        Visit(kind.data.binary, mode); 
      // above : 如果没有访问过，就把这个 binary 指令 `映射` 到一个新的临时寄存器里
      // 之后如果要用这次 binary 指令的运算结果，直接到这个对应的 reg 里面找就行
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

// ------------------------ `koopa_raw_value_t` Visit ------------------------------------------------------

void Visit(const koopa_raw_return_t &ret, const int mode){
  if (mode) return;

  if (ret.value->kind.tag == KOOPA_RVT_INTEGER) // 直接ret整数
    cout << "    li a0, ",Visit(ret.value, mode);
  else if (ret.value->kind.tag == KOOPA_RVT_BINARY) // ret 寄存器中的值
    Binary2Register((koopa_raw_binary_t* )&(ret.value->kind.data.binary), "a0");

  after_func();
  cout << "   ret" <<endl;
}

void Visit(const koopa_raw_integer_t &INT, const int mode){
  if (mode) return;
  cout << INT.value << endl;
}

inline void binary2risc(koopa_raw_binary_op_t optype, string o1, string o2){
  // 暂时不优化成 addi 等指令
  // mapping list 在 koopa.h 搜索 "// map = {" 处
  string des_reg = std::string("t2");
  switch (optype){
    case 0: // neq
      cout << risc_sub(o1, o1, o2);
      cout << risc_snez(des_reg, o1);
      break;
    case 1: // eq
      cout << risc_sub(o1, o1, o2);
      cout << risc_seqz(des_reg, o1);
      break;
    case 2: // gt
      cout << risc_sgt(des_reg, o1, o2);
      break;
    case 3: // lt
      cout << risc_slt(des_reg, o1, o2);
      break;
    case 4: // ge (反的lt)
      cout << risc_slt(o1, o1, o2);
      cout << risc_seqz(des_reg, o1);
      break;
    case 5: // le (反的gt)
      cout << risc_sgt(o1, o1, o2);
      cout << risc_seqz(des_reg, o1);
      break;
    case 6: // add
      cout << risc_add(des_reg, o1, o2);
      break;
    case 7: // sub
      cout << risc_sub(des_reg, o1, o2);
      break;
    case 8: // mul
      cout << risc_mul(des_reg, o1, o2);
      break;
    case 9: // div
      cout << risc_div(des_reg, o1, o2);
      break;
    case 10: // mod
      cout << risc_rem(des_reg, o1, o2);
      break;
    case 11: // and
      cout << risc_and(des_reg, o1, o2);
      break;
    case 12: // or
      cout << risc_or(des_reg, o1, o2);
      break;
    case 13: // xor 
      cout << risc_xor(des_reg, o1, o2);
      break;
    case 14: // shl
      cout << risc_sll(des_reg, o1, o2);
      break;
    case 15: // shr
      cout << risc_srl(des_reg, o1, o2);
      break;
    case 16: // sar
      cout << risc_sra(des_reg, o1, o2);
      break;
    default:
      // 未知的binary 操作码
      assert(false);
  }

  int IMM = GetStackSize();
  if (IMM < (1<<11)){
    cout << risc_sw("sp", "t2", IMM);
  }
  else{
    cout << risc_li("t1", IMM);
    cout << risc_add("t1", "t1", "sp");
    cout << risc_sw("t1", "t2", 0);
  }

  // put the result on the stack
  
}

// visit this instr for the first time, return the reg_id
// And each Binary code will be visit only once
void Visit(const koopa_raw_binary_t &BinOP, const int mode){
  // lhs and rhs can only be 0(imm) or 12(ref of result of before instr)
  // test the kind of lhs and rhs
      // cout << "test time : " << BinOP.lhs->kind.tag << ' ' << BinOP.rhs->kind.tag << endl;
      // if (BinOP.lhs->kind.tag == 12)
      //   cout << "lhs uses Register : " << Binary2Register((koopa_raw_binary_t* )&(BinOP.lhs->kind.data.binary)) <<endl;
      // if (BinOP.rhs->kind.tag == 12)
      //   cout << "rhs uses Register : " << Binary2Register((koopa_raw_binary_t* )&(BinOP.rhs->kind.data.binary)) <<endl;  
      // cout << "This Binary uses Register : " << reg_name[now_reg] << endl;
  // above is Register Alloc Test, and this Program has passed it.
  if (mode){
    GrowStack(4);
    return;
  }

  mmp[(koopa_raw_binary_t* )(&BinOP)] = GetStackSize();

  // And we promise, when you come into this function, 
    // the child of potential Binary Type has been already computed.
    // SO Which occurred above WILL NOT CAUSE ANY BAD Effect !!!
  
  if (BinOP.lhs->kind.tag == 12)
    Binary2Register((koopa_raw_binary_t* )&(BinOP.lhs->kind.data.binary), "t1");
  else
    cout << risc_li("t1",BinOP.lhs->kind.data.integer.value);
  if (BinOP.rhs->kind.tag == 12)
    Binary2Register((koopa_raw_binary_t* )&(BinOP.rhs->kind.data.binary), "t2");
  else
    cout << risc_li("t2",BinOP.rhs->kind.data.integer.value);

  binary2risc(BinOP.op, "t1", "t2");

  // 彻底翻译完了这条指令，__把我们的运算结果放在了 reg_name[now_reg]里__，我们才能翻篇
    GrowStack(4);
}


int handle_str_ir(const char *str){
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(str, &program);
  assert(ret == KOOPA_EC_SUCCESS);
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  koopa_delete_program(program);

  // ---------------------------Handling Raw Function-------------------------------
  Visit(raw, 0);
  // ---------------------------End of Handling-------------------------------------

  // builder 中 含有 raw_program 用到的所有内存，所以不要在处理好 raw_program 之前释放它
  koopa_delete_raw_program_builder(builder);

  return 0;
}