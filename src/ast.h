#pragma once

/*
    Notes here:
        1. Dump() 输出基本结构
        2. IRDump() 将 koopa 输出到 stdout （然后重定向到output）里
        3. var_ins[] 的存在，优化了常数加载的指令 (省去所有常数加载的指令，大量减少指令数目)
          // is_01 = 0 -> binary ; 1 -> 0/1 ; 2 -> const
        4. BaseAST 中的 can_compute, val : 编译时预处理技术，提前将可以算的东西算出来, `可以与3.协同工作`
          详情见 `sysy.y` 中所有有 "// Pre-Compute Tech" tag 的部分
            由于我们在父节点只需要知道子节点是不是可以提前算，并且如果可以的话值是多少，所以 BaseAST 只需要
            多定义 can_compute, val 俩变量
          
          适用该技术的最高层为 `Exp`，也就是说再高了我们就 不需要 也 不能再用 该技术了
        5. 
*/


class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump(int sj) const = 0;
  virtual void IRDump() const = 0;
  inline void HandleSJ(int sj) const ;
  int can_compute, val;
};

class CompUnitAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_def;
  void Dump(int sj) const override;
  void IRDump() const override;
};

class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  void Dump(int sj) const override;
  void IRDump() const override;
};

// Lv1_Left
class FuncTypeAST : public BaseAST {
 public:
  std::string type;
  void Dump(int sj) const override;
  void IRDump() const override;
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;
  void Dump(int sj) const override;
  void IRDump() const override;
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  void Dump(int sj) const override;
  void IRDump() const override;
};


// --------------------------------- Lv_3  Operator-------------------------------------

// Useful Functions:
  // inline void out_IR(int fi, int se, string op, int is_01_fi = 0, int is_01_se = 0);
  // inline void bin201();
  // inline void alr_compute_procedure(int NUMb);

class ExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> lorexp;
  void Dump(int sj) const override;
  void IRDump() const override;
};

class LOrExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> lorexp, landexp;
  int sel;
  void Dump(int sj) const override;
  void IRDump() const override;
};

class LAndExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> landexp, eqexp;
  int sel;
  void Dump(int sj) const override;
  void IRDump() const override;
};

class EqExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> eqexp, relexp;
  std::string rel;
  int sel;
  void Dump(int sj) const override;
  void IRDump() const override;
};

class RelExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> relexp, addexp;
  std::string rel;
  int sel;
  void Dump(int sj) const override;
  void IRDump() const override;
};

class AddExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> addexp, mulexp;
  std::string opt; 
  int sel;
  void Dump(int sj) const override;
  void IRDump() const override;
};

class MulExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> mulexp, unaryexp;
  std::string opt; 
  int sel;
  void Dump(int sj) const override;
  void IRDump() const override;
};

// useless class
// class UnaryOpAST : public BaseAST {
//  public:
//   std::string opt;
//   void Dump(int sj) const override;
//   void IRDump() const override;
// };

class UnaryExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> unaryexp, pexp;
  std::string opt;
  int sel;
  void Dump(int sj) const override;
  void IRDump() const override;
};

class PrimaryExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  int number;
  int sel;
  void Dump(int sj) const override;
  void IRDump() const override;
};



// Number 在 "sysy.y" 中定义的类型直接是 int_val 而不是 ast_val，所以不需要语法树类型
// class Number : public BaseAST {
//  public:
//     std::int val;
// };