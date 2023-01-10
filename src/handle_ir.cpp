/*
  - 从我们 Generate 的 IR 到 Load 成 Raw_Program 的 IR，也是被加了优化
  
  - 对 If-Else 指令做内存占用减少的优化？

*/

#include <string>
#include <memory>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "handle_ir.h"
#include "koopa.h"

using namespace std;

// --------------------------- MainTain the RISC-V Stack -----------------------
const unsigned int Mask_16 = (~0u) << 4;
unsigned int need_size_on_stack, stack_size, call_param_size;
inline void ClearStack(){ need_size_on_stack = 0;}
inline void GrowStack(const unsigned int x){ need_size_on_stack += x;}
inline unsigned int GetStackSize(){ return need_size_on_stack; }
inline void AlignStackSize(unsigned int &x){ x = (x + (~Mask_16)) & Mask_16;}
unordered_map<koopa_raw_binary_t* , unsigned int> mmp; // bin_instr -> offset
unordered_map<koopa_raw_load_t* , unsigned int> ldmmp; // load_instr (等号左边的百分号) -> offset
unordered_map<koopa_raw_value_t, unsigned int> allcmmp; // local alloc -> offset
unordered_map<koopa_raw_call_t*, unsigned int> callmmp; // call_instr -> offset
unordered_map<koopa_raw_get_elem_ptr_t*, unsigned int> get_ele_mmp; // get_ele_ptr -> offset
unordered_map<koopa_raw_get_ptr_t*, unsigned int> get_ptr_mmp; // get_ele_ptr -> offset
string now_func_name_risc;
int if_define_stage;
static string param_reg_name[8] = {
  "a0",
  "a1",
  "a2",
  "a3",
  "a4",
  "a5",
  "a6",
  "a7"
};

// 将栈空间某偏移量的数据 load 到指定寄存器中
inline void Offset2Register(unsigned int &OFFSET, string reg_name, string init_stack_p = "sp"){
  if (OFFSET < (1<<11)){
    cout << risc_lw(init_stack_p, reg_name, OFFSET);
  }
  else{
    cout << risc_li("t1", OFFSET);
    cout << risc_add("t1", "t1", init_stack_p);
    cout << risc_lw("t1", reg_name, 0);
  }
}

inline void PureOffSet2Register(unsigned int &OFFSET, string reg_name, string init_stack_p = "sp"){
  if (OFFSET < (1<<11)){
    cout << risc_addi(reg_name, init_stack_p, OFFSET);
  }
  else{
    cout << risc_li("t1", OFFSET);
    cout << risc_add(reg_name, "t1", init_stack_p);
  }
}

// 前身是 Binary2Register ： 将某条二进制指令的结果load到指定寄存器中
inline void Binary2Register(koopa_raw_binary_t* addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = mmp[addr];
  Offset2Register(offset, reg_name, init_stack_p);
}

inline void Load2Register(koopa_raw_load_t* addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = ldmmp[addr];
  Offset2Register(offset, reg_name, init_stack_p);
}

inline void Alloc2Register(koopa_raw_value_t addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = allcmmp[addr];
  Offset2Register(offset, reg_name, init_stack_p);
}

inline void AllocAddr2Register(koopa_raw_value_t addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = allcmmp[addr];
  PureOffSet2Register(offset, reg_name, init_stack_p);
}

inline void Call2Register(koopa_raw_call_t* addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = callmmp[addr];
  Offset2Register(offset, reg_name, init_stack_p);
}

inline void GetEle2Register(koopa_raw_get_elem_ptr_t* addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = get_ele_mmp[addr];
  Offset2Register(offset, reg_name, init_stack_p);
}

inline void GetPtr2Register(koopa_raw_get_ptr_t* addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = get_ptr_mmp[addr];
  Offset2Register(offset, reg_name, init_stack_p);
}

inline void GetEleREAL2Register(koopa_raw_get_elem_ptr_t* addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = get_ele_mmp[addr];
  Offset2Register(offset, "t1", init_stack_p);
  cout << risc_lw("t1", reg_name, 0);
}

inline void GetPtrREAL2Register(koopa_raw_get_ptr_t* addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = get_ptr_mmp[addr];
  Offset2Register(offset, "t1", init_stack_p);
  cout << risc_lw("t1", reg_name, 0);
}

// From an Instr to the Register
inline void Instr2Register(const koopa_raw_value_t& addr, string reg_name, string init_stack_p = "sp", int get_addr = 0){
  if (addr == NULL) // void
    return;

  int reg_rank;
  switch (addr->kind.tag) {
    case KOOPA_RVT_INTEGER :
      cout << risc_li(reg_name, addr->kind.data.integer.value) ;
      break;
    case KOOPA_RVT_BINARY :
      Binary2Register((koopa_raw_binary_t *)&(addr->kind.data.binary), reg_name, init_stack_p);
      break;
    case KOOPA_RVT_LOAD :
      // Load 的 result 放到 Reg 上
      Load2Register((koopa_raw_load_t *)&(addr->kind.data.load), reg_name, init_stack_p);
      break;
    case KOOPA_RVT_ALLOC :
      if (get_addr == 0)
        // 这个对应的局部变量的结果放到 Reg 上
        Alloc2Register((koopa_raw_value_t)(addr), reg_name, init_stack_p);
      else
        // 我们只想要这个局部变量的地址
        AllocAddr2Register((koopa_raw_value_t)(addr), reg_name, init_stack_p);
      break;
    case KOOPA_RVT_CALL:
      Call2Register((koopa_raw_call_t *)&(addr->kind.data.call), reg_name, init_stack_p);
      break;
    case KOOPA_RVT_GET_ELEM_PTR:
      if (get_addr == 0)
        GetEleREAL2Register((koopa_raw_get_elem_ptr_t *)&(addr->kind.data.get_elem_ptr), reg_name, init_stack_p);
      else
        GetEle2Register((koopa_raw_get_elem_ptr_t *)&(addr->kind.data.get_elem_ptr), reg_name, init_stack_p);
      break;
    case KOOPA_RVT_GET_PTR:
      if (get_addr == 0)
        GetPtrREAL2Register((koopa_raw_get_ptr_t *)&(addr->kind.data.get_ptr), reg_name, init_stack_p);
      else
        GetPtr2Register((koopa_raw_get_ptr_t *)&(addr->kind.data.get_ptr), reg_name, init_stack_p);
      break;
    case KOOPA_RVT_FUNC_ARG_REF:
      reg_rank = addr->kind.data.func_arg_ref.index;
      if (reg_rank < 8){
        cout << risc_mv(reg_name, param_reg_name[reg_rank]);
      }
      else{
        // use t4 to find out, where the param is
        int OffSet = 4 * (reg_rank - 8); // 别忘了每个单位是 4 bytes
        if (OffSet < (1<<11)){
          cout << risc_lw("t4", reg_name, OffSet);
        }
        else{
          cout << risc_li("t1", OffSet);
          cout << risc_add("t1", "t1", "t4");
          cout << risc_lw("t1", reg_name, 0);
        }
      }
      break;
    case KOOPA_RVT_GLOBAL_ALLOC :
      cout << risc_la("t1", addr->name + 1);
      if (get_addr == 0)
        cout << risc_lw("t1", reg_name, 0);
      else
        cout << risc_addi(reg_name, "t1", 0);
      break;
    default:
      assert(false);
  }
}

inline void Register2Stack(unsigned int &OFFSET, string reg_name, string init_stack_p = "sp"){
  // int IMM = GetStackSize();
  if (reg_name == "t1")
    exit(7);
  if (OFFSET < (1<<11)){
    cout << risc_sw(init_stack_p, reg_name, OFFSET);
  }
  else{
    cout << risc_li("t1", OFFSET);
    cout << risc_add("t1", "t1", init_stack_p);
    cout << risc_sw("t1", reg_name, 0);
  }
}

inline void Register2Binary(koopa_raw_binary_t* addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = mmp[addr];
  Register2Stack(offset, reg_name, init_stack_p);
}

inline void Register2Load(koopa_raw_load_t* addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = ldmmp[addr];
  Register2Stack(offset, reg_name, init_stack_p);
}

inline void Register2Alloc(koopa_raw_value_t addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = allcmmp[addr];
  Register2Stack(offset, reg_name, init_stack_p);
}

inline void Register2Call(koopa_raw_call_t* addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = callmmp[addr];
  Register2Stack(offset, reg_name, init_stack_p);
}

inline void Register2GetEle(koopa_raw_get_elem_ptr_t* addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = get_ele_mmp[addr];
  Register2Stack(offset, reg_name, init_stack_p);
}

inline void Register2GetPtr(koopa_raw_get_ptr_t* addr, string reg_name, string init_stack_p = "sp"){
  unsigned int offset = get_ptr_mmp[addr];
  Register2Stack(offset, reg_name, init_stack_p);
}


inline void Register2Instr(const koopa_raw_value_t& addr, string reg_name, string init_stack_p = "sp"){
  switch (addr->kind.tag) {
    case KOOPA_RVT_INTEGER :
      assert(false);
      break;
    case KOOPA_RVT_BINARY :
      assert(false);
      break;
    case KOOPA_RVT_LOAD :
      assert(false);
      break;
    case KOOPA_RVT_ALLOC :
      // 将 Reg 的值传到这个局部变量在栈上的位置
      Register2Alloc((koopa_raw_value_t)(addr), reg_name, init_stack_p);
      break;
    case KOOPA_RVT_GLOBAL_ALLOC :
      // 将 Reg 的值传到全局变量在DATA段的位置
      cout << risc_la("t1", addr->name + 1);
      cout << risc_sw("t1", reg_name, 0);
      break;
    default :
      Instr2Register((koopa_raw_value_t)(addr), "t1", "sp", 1);
      cout << risc_sw("t1", reg_name, 0);
      break;
    // default:
    //   assert(false);
  } 
}

inline void pre_func(){
  auto ret = BaseAST::glbsymbtl->GetItemByName(now_func_name_risc);
  if (ret->VarType() & 16){
    GrowStack(4);
    cout << risc_sw("sp","ra",-4);
  }

  /*
    注：由于目前没有需要保存的 Callee Reg，所以这一部分不需要保存和 (after_func) 恢复 Callee Reg
  */

  stack_size = need_size_on_stack; // 存在 stack_size 里
  AlignStackSize(stack_size);
  ClearStack();

  if (stack_size == 0) return;

  if (stack_size <= (1<<11)){
    cout << risc_addi("sp", "sp", -((int)stack_size));
  }
  else{
    cout << risc_li("t1", -((int)stack_size));
    cout << risc_add("sp", "t1", "sp");
  }
}

inline void after_func(){
  auto ret = BaseAST::glbsymbtl->GetItemByName(now_func_name_risc);
  if (stack_size == 0) return;

  if (stack_size < (1<<11)){
    cout << risc_addi("sp", "sp", stack_size);
  }
  else{
    cout << risc_li("t1", stack_size);
    cout << risc_add("sp", "t1", "sp");
  }

  if (ret->VarType() & 16) 
    cout << risc_lw("sp", "ra", -4);
}

// Basic Functions

// From Binary to Risc-V Instr, and the final-result is in 't2'
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
}

string Global_name;

int zero_num = 0;

void put_zero(){
  int temp;
  while(zero_num){
    temp = zero_num & -zero_num;
    cout << "   .zero " << (temp << 2) << endl;
    zero_num -= temp;
  }
}

void Solve_aggregate(const koopa_raw_aggregate_t &aggregate){
  const koopa_raw_slice_t &slice = aggregate.elems;
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    // cout << slice.kind << endl;
    auto value = reinterpret_cast<koopa_raw_value_t>(ptr);
    if (value->kind.tag == KOOPA_RVT_INTEGER){
      const int32_t &now_int = value->kind.data.integer.value;
      if (now_int == 0) zero_num ++;
      else{
        put_zero();
        cout << "   .word " << now_int << endl;
      }
    }
    else if (value->kind.tag == KOOPA_RVT_AGGREGATE)
      Solve_aggregate(value->kind.data.aggregate);
    else 
      exit(18);
  }
}

inline void Define(const koopa_raw_global_alloc_t &global_alloc){
  cout << "   .global " << Global_name << endl;
  cout << Global_name << ':' << endl;
  if (global_alloc.init->kind.tag == KOOPA_RVT_INTEGER)
    cout << "   .word " << global_alloc.init->kind.data.integer.value << endl;
  else if (global_alloc.init->kind.tag == KOOPA_RVT_ZERO_INIT)
    cout << "   .zero " << cal_size_(global_alloc.init->ty) << endl;
  else{
    // aggregate
    zero_num = 0; // We do a small optimization to the allocation
    Solve_aggregate(global_alloc.init->kind.data.aggregate);
    put_zero();
  }
}

int cal_size_(const koopa_raw_type_t &type){
  switch (type->tag) {
    case KOOPA_RTT_UNIT: 
      return 0;
      break;
    case KOOPA_RTT_ARRAY:
      return type->data.array.len * cal_size_(type->data.array.base);
      break;
    default:
      return 4;
      break;
  }
}


// ------------------------ Basic Visit ------------------------------------------------------

void Visit(const koopa_raw_program_t &program, const int mode){
  // 执行一些其他的必要操作
  // ...
  // 
  if_define_stage = 1;
  if (program.values.len > 0 && !(mode))
    printf("   .data\n");
  Visit(program.values, mode);

  cout << endl;

  // 访问所有函数
  if_define_stage = 0;
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
  now_func_name_risc = string(func->name + 1);

  if (now_func_name_risc == "getint" || now_func_name_risc == "getch" || now_func_name_risc == "getarray" || now_func_name_risc == "putint" ||now_func_name_risc == "putch" ||now_func_name_risc == "putarray" || now_func_name_risc == "starttime" ||now_func_name_risc == "stoptime")
    return;

  // 执行一些其他的必要操作
  if(func->name && !(mode)) 
    printf("%s:\n", func->name + 1);
  ClearStack();
  Visit(func->bbs, 1); // 预计算出需要的栈空间
          // 由于我的 implementation 是 只有局部数组需要占栈空间，所以
  pre_func();
  
  // 访问所有基本块
  Visit(func->bbs, mode);
  // Solved
  cout << "Func_" << now_func_name_risc << "_Ret:" << endl;
  after_func(); // can be right at least now
  cout << "   ret" <<endl;
}
// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb, const int mode) {
  // cout << string(bb->name + 1) << ' ' << bb->insts.len << endl;
  // 执行一些其他的必要操作
  // if(bb->name && !(mode))
  //   printf(" %s:\n", bb->name + 1);
  // 访问所有指令
  if(bb->name && !(mode))
    cout << now_func_name_risc << '_' << string(bb->name + 1) << ':' << endl;
  Visit(bb->insts, mode);
}

/* --------------------- next part is important-----------------------------
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
*/

// -----------------------------------访问指令-------------------------------------
void Visit(const koopa_raw_value_t &value, const int mode) {
  // 非多重递归假设：由于一条指令所用的量必然先前已经计算好了，
    // 所以我们进入这条指令之后没有任何必要再次递归到这个地方
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  string tttmmpp;
  SymbolTableItem* ret;
  int Size_alloc;

  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      if (!mode)
        Visit(kind.data.ret, mode);
      break;
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      if (!mode)
        Visit(kind.data.integer, mode);
      break;
    case KOOPA_RVT_BINARY:
      // 二进制操作指令
      if (!mode){
        // if (!mmp.count((koopa_raw_binary_t* )(&kind.data.binary)))
        Visit(kind.data.binary, mode); 
        // 一次性空间
      }
      else 
        GrowStack(4);
      
      // above : 如果没有访问过，就把这个 binary 指令 `映射` 到一个新的临时寄存器里
      // 之后如果要用这次 binary 指令的运算结果，直接到这个对应的 reg 里面找就行
      break;
    case KOOPA_RVT_ALLOC:
      Size_alloc = cal_size_(value->ty->data.pointer.base);
      // if (!mode){
      //   cout << Size_alloc << ' ' << value->ty->tag << ' ' << value->ty->data.pointer.base->tag << endl;
      // }
      if (!mode){
        allcmmp[(koopa_raw_value_t)(value)] = GetStackSize();
        // 局部变量不进行初始化
        GrowStack(Size_alloc);
      }
      else 
        GrowStack(Size_alloc);
      break;
    case KOOPA_RVT_LOAD:
      if (!mode){
        // if (!ldmmp.count((koopa_raw_load_t* )&(kind.data.load)))
        Visit(kind.data.load, mode);
        // cout << "there?" << endl;
        // 一次性空间
      }
      else 
        GrowStack(4);

      break;
    case KOOPA_RVT_STORE:
      if (!mode)
        Visit(kind.data.store, mode);
      break;
    case KOOPA_RVT_BRANCH:
      if (!mode)
        Visit(kind.data.branch, mode);
      break;
    case KOOPA_RVT_JUMP:
      if (!mode)
        Visit(kind.data.jump, mode);
      break;
    case KOOPA_RVT_CALL:
      tttmmpp = string(kind.data.call.callee->name + 1);
      ret = BaseAST::glbsymbtl->GetItemByName(tttmmpp);
      // 判断调用的函数是不是有返回值，决定要不要开一个 local var 存它
      if (ret->VarType() & 8){
        if (!mode)
          Visit(kind.data.call, 1);
        else 
          GrowStack(4);
      }
      else
        if (!mode)
          Visit(kind.data.call, 0);
      break;
    case KOOPA_RVT_FUNC_ARG_REF :
      // do nothing, we can tackle this in the section of 
      break;
    case KOOPA_RVT_GLOBAL_ALLOC:
      if (if_define_stage){
        Global_name = string(value->name + 1);
        Define(kind.data.global_alloc);
      }
      else{
        // Do Nothing
      }
      break;
    case KOOPA_RVT_GET_ELEM_PTR:
      if (!mode){
        Visit(kind.data.get_elem_ptr, mode);
      }
      else
        GrowStack(4);
      break;
    case KOOPA_RVT_GET_PTR:
      if (!mode){
        Visit(kind.data.get_ptr, mode);
      }
      else
        GrowStack(4);
      break;
    default:
      // break;
      // // 其他类型暂时遇不到
      cout << "Type = " << kind.tag << endl;
      assert(false);
  }
}

// ------------------------ `koopa_raw_value_t` Visit ------------------------------------------------------

void Visit(const koopa_raw_return_t &ret, const int mode){
  // cout << "here" << ' ' << ret.value->kind.tag << endl;
  Instr2Register(ret.value, "a0");
  cout << risc_j("Func_" + now_func_name_risc + "_Ret");
}

void Visit(const koopa_raw_integer_t &INT, const int mode){
  cout << INT.value << endl;
}

void Visit(const koopa_raw_binary_t &BinOP, const int mode){
  mmp[(koopa_raw_binary_t* )(&BinOP)] = GetStackSize();
  
  Instr2Register(BinOP.lhs, "t3");

  Instr2Register(BinOP.rhs, "t2");

  binary2risc(BinOP.op, "t3", "t2");
  // out_come is in 't2'
  Register2Binary((koopa_raw_binary_t* )(&BinOP), "t2");
  // put the result on the stack

  // 彻底翻译完了这条指令，__把我们的运算结果放在了 reg_name[now_reg]里__，我们才能翻篇
  GrowStack(4);
}

void Visit(const koopa_raw_load_t &LoadOP, const int mode){
  ldmmp[(koopa_raw_load_t* )(&LoadOP)] = GetStackSize();

  // cout << "Source = " << LoadOP.src->kind.tag << endl;
  // Source Always 6-> ALLOC
  Instr2Register(LoadOP.src, "t2");
    // 将目前，这个局部变量的值，根据它的相对地址(allcmmp[LoadOP.src])，搞到 't1' 里来
  Register2Load((koopa_raw_load_t *)(&LoadOP), "t2");  
    // 将这个值，存在这条 Load 指令对应的相对地址里 (stack) 上

  GrowStack(4);
}

void Visit(const koopa_raw_store_t &StoreOP, const int mode){
  Instr2Register(StoreOP.value, "t2");
  Register2Instr(StoreOP.dest, "t2");
}

void Visit(const koopa_raw_branch_t &Branch, const int mode){
  // Branch.cond;
  // cout << Branch.true_bb->name << endl;
  // ...
  Instr2Register(Branch.cond, "t2");
  cout << risc_bnez("t2", now_func_name_risc + string("_") + string(Branch.true_bb->name + 1));
  cout << risc_j(now_func_name_risc + string("_") + string(Branch.false_bb->name + 1));
}

void Visit(const koopa_raw_jump_t &Jump, const int mode){
  cout << risc_j(now_func_name_risc + string("_") + string(Jump.target->name + 1));
}

inline int down_8(int x){
  return x >= 8 ? x-8 : 0;
}

void Give_param2Callee(const koopa_raw_slice_t &slice){
  call_param_size = 4;

  cout << risc_sw("sp", "t4", -4); // t4 maintain the beginning of present more Param

  call_param_size += 4 * down_8(slice.len);
  AlignStackSize(call_param_size);

  if (call_param_size <= (1<<11)){
    cout << risc_addi("t4", "sp", -((int)call_param_size));
  }
  else{
    cout << risc_li("t1", -((int)call_param_size));
    cout << risc_add("t4", "t1", "sp");
  }

  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    auto now_value = reinterpret_cast<koopa_raw_value_t>(ptr);
    Instr2Register(now_value, "t2", "sp", 2);
    
    if (i <= 7){
      // 前面八个参数搞到 Reg 里
      cout << risc_mv(param_reg_name[i], "t2");
      continue;
    }

    unsigned int nn_offset = 4 * (i - 8);
    Register2Stack(nn_offset, "t2", "t4");
    // 于是我们存好了所有额外的参数
  }

  cout << risc_mv("sp", "t4");
}

void Visit(const koopa_raw_call_t &Call, const int mode){
  if (mode) 
    callmmp[(koopa_raw_call_t* )(&Call)] = GetStackSize();

  Give_param2Callee(Call.args);

  cout << risc_call(Call.callee->name + 1);

  if (call_param_size < (1<<11)){
    cout << risc_addi("sp", "sp", call_param_size);
  }
  else{
    cout << risc_li("t1", call_param_size);
    cout << risc_add("sp", "t1", "sp");
  }

  cout << risc_lw("sp", "t4", -4); // recover the t4 register
  
  if (mode){
    Register2Call((koopa_raw_call_t* )(&Call), "a0"); // 返回值在 a0 里，如果有的话
    GrowStack(4);
  }
}

void Visit(const koopa_raw_get_elem_ptr_t &get_ele, const int mode){
  // %ptr = getelemptr @arr, 1
  get_ele_mmp[(koopa_raw_get_elem_ptr_t* )(&get_ele)] = GetStackSize();

  assert(get_ele.src->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY);
  // 可以肯定用 getelemptr 的 pointer 一定指向了一个数组

  int stride = cal_size_(get_ele.src->ty->data.pointer.base) / get_ele.src->ty->data.pointer.base->data.array.len;
  Instr2Register(get_ele.src, "t3", "sp", 1);
  Instr2Register(get_ele.index, "t1");
  cout << risc_li("t2", stride);
  binary2risc(8, "t1", "t2"); //mul, result in "t2"
  cout << risc_add("t2", "t3", "t2"); // final address
  Register2GetEle((koopa_raw_get_elem_ptr_t *)(&get_ele), "t2");

  GrowStack(4);
}

void Visit(const koopa_raw_get_ptr_t &get_ptr, const int mode){
  // %0 = load %arr
  // %1 = getptr %0, 1
  get_ptr_mmp[(koopa_raw_get_ptr_t* )(&get_ptr)] = GetStackSize();
  
  int stride = cal_size_(get_ptr.src->ty->data.pointer.base);
  Instr2Register(get_ptr.src, "t3", "sp", 1);
  Instr2Register(get_ptr.index, "t1");
  cout << risc_li("t2", stride);
  binary2risc(8, "t1", "t2"); //mul, result in "t2"
  cout << risc_add("t2", "t3", "t2"); // final address
  Register2GetPtr((koopa_raw_get_ptr_t *)(&get_ptr), "t2");

  GrowStack(4);
}

int handle_str_ir(const char *str){
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(str, &program);
  exit(ret);
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