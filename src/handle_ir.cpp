#include <string>
#include <memory>
#include <cassert>
#include <iostream>
#include <map>
#include "handle_ir.h"
#include "koopa.h"

using namespace std;

// ------------------------ Basic Visit ------------------------------------------------------

void Visit(const koopa_raw_program_t &program){
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  if(program.values.len > 0)
    printf("   .data\n");
  Visit(program.values);
  // 访问所有函数
  printf("   .text\n   .globl main\n");
  Visit(program.funcs);
}

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void Visit(const koopa_raw_function_t &func) {
  // 执行一些其他的必要操作
  if(func->name) 
    printf("%s:\n", func->name + 1);
  // 访问所有基本块
  Visit(func->bbs);
}

// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  if(bb->name)
    printf(" %s:\n", bb->name + 1);
  // 访问所有指令
  Visit(bb->insts);
}

// --------------------- next part is important-----------------------------
string reg_name[13] = {
  "a0",
  "t1",
  "t2",
  "t3",
  "t4",
  "t5",
  "t6",
  "a3",
  "a4",
  "a5",
  "a6",
  "a7",
  "t0"
};
int now_reg = 0, reg_bound = 13;

map<koopa_raw_binary_t* , int> mmp;

inline string Binary2Register(koopa_raw_binary_t* addr){
  return reg_name[mmp[addr]];
}

// -----------------------------------访问指令-------------------------------------
void Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      Visit(kind.data.integer);
      break;
    case KOOPA_RVT_BINARY:
      // 二进制操作指令
      if (!mmp.count((koopa_raw_binary_t* )(&kind.data.binary)))
        Visit(kind.data.binary); 
      // above : 如果没有访问过，就把这个 binary 指令 `映射` 到一个新的临时寄存器里
      // 之后如果要用这次 binary 指令的运算结果，直接到这个对应的 reg 里面找就行
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

// ------------------------ `koopa_raw_value_t` Visit ------------------------------------------------------

void Visit(const koopa_raw_return_t &ret){
  if (ret.value->kind.tag == KOOPA_RVT_INTEGER) // 直接ret整数
    cout << "    li a0, ",Visit(ret.value);
  else if (ret.value->kind.tag == KOOPA_RVT_BINARY) // ret 寄存器中的值
    cout<< risc_addi("a0",Binary2Register((koopa_raw_binary_t* )&(ret.value->kind.data.binary)),0);
  cout << "   ret" <<endl;

  now_reg = 0; // 清空已用的寄存器数量
}

void Visit(const koopa_raw_integer_t &INT){
  cout << INT.value << endl;
}

inline void binary2risc(koopa_raw_binary_op_t optype, string o1, string o2){
  // 暂时不优化成 addi 等指令
  // mapping list 在 koopa.h 搜索 "// map = {" 处
  string &des_reg = reg_name[now_reg];
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
}

// visit this instr for the first time, return the reg_id
// And each Binary code will be visit only once
void Visit(const koopa_raw_binary_t &BinOP){
  // lhs and rhs can only be 0(imm) or 12(ref of result of before instr)
  // test the kind of lhs and rhs
      // cout << "test time : " << BinOP.lhs->kind.tag << ' ' << BinOP.rhs->kind.tag << endl;
      // if (BinOP.lhs->kind.tag == 12)
      //   cout << "lhs uses Register : " << Binary2Register((koopa_raw_binary_t* )&(BinOP.lhs->kind.data.binary)) <<endl;
      // if (BinOP.rhs->kind.tag == 12)
      //   cout << "rhs uses Register : " << Binary2Register((koopa_raw_binary_t* )&(BinOP.rhs->kind.data.binary)) <<endl;  
      // cout << "This Binary uses Register : " << reg_name[now_reg] << endl;
  // above is Register Alloc Test, and this Program has passed it.


  mmp[(koopa_raw_binary_t* )(&BinOP)] = now_reg;
  // And we promise, when you come into this function, 
    // the child of potential Binary Type has been already computed.
    // SO Which occurred above WILL NOT CAUSE ANY BAD Effect !!!
  
  int l_p = (BinOP.lhs->kind.tag == 12), r_p = (BinOP.rhs->kind.tag == 12);
  if (l_p && r_p)
    binary2risc(
      BinOP.op, 
      Binary2Register((koopa_raw_binary_t* )&(BinOP.lhs->kind.data.binary)), 
      Binary2Register((koopa_raw_binary_t* )&(BinOP.rhs->kind.data.binary))
    );
  else if (!l_p && !r_p){
    cout << risc_li("a1",BinOP.lhs->kind.data.integer.value);
    cout << risc_li("a2",BinOP.rhs->kind.data.integer.value);
    binary2risc(BinOP.op, "a1", "a2");
  }
  else if (l_p){
    cout << risc_li("a2",BinOP.rhs->kind.data.integer.value);
    binary2risc(
      BinOP.op, 
      Binary2Register((koopa_raw_binary_t* )&(BinOP.lhs->kind.data.binary)), 
      "a2"
    );
  }
  else{
    cout << risc_li("a1",BinOP.lhs->kind.data.integer.value);
    binary2risc(
      BinOP.op, 
      "a1",
      Binary2Register((koopa_raw_binary_t* )&(BinOP.rhs->kind.data.binary))
    );
  }

  // 彻底翻译完了这条指令，__把我们的运算结果放在了 reg_name[now_reg]里__，我们才能翻篇
  now_reg ++; 
}


int handle_str_ir(const char *str){
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(str, &program);
  assert(ret == KOOPA_EC_SUCCESS);
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  koopa_delete_program(program);

  // ---------------------------Handling Raw Function-------------------------------
  Visit(raw);
  // ---------------------------End of Handling-------------------------------------

  // builder 中 含有 raw_program 用到的所有内存，所以不要在处理好 raw_program 之前释放它
  koopa_delete_raw_program_builder(builder);

  return 0;
}