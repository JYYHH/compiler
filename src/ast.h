#pragma once

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump() const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;
  void Dump() const override;
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  void Dump() const override;
};

// My Part
class FuncTypeAST : public BaseAST {
 public:
    std::string type;
    void Dump() const override;
};

class BlockAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> stmt;
    void Dump() const override;
};

class StmtAST : public BaseAST {
 public:
    int number;
    void Dump() const override;
};

// Number 在 "sysy.y" 中定义的类型直接是 int_val 而不是 ast_val，所以不需要语法树类型
// class Number : public BaseAST {
//  public:
//     std::int val;
// };