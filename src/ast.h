#pragma once

// 注意，我们print出来的AST和IR不仅正确性✔很重要，可读性也很重要（方便debug）

class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump(int sj) const = 0;
  virtual void IRDump() const = 0;
  inline void HandleSJ(int sj) const ;
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