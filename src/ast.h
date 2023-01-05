#pragma once

#include "symtab.h"
#include <stack>
#include <vector>
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

        5. 我们对每个 Block 维护一个 Symtab （通过在 BlockAST 中加入一个指针变量，见 BlockAST内），
          
          5.5 同时，我们还需要注意肯定会有 Block 嵌套的情况，所以我们在基类 BaseAST 中开了一个全局栈
              来保存最近用的是哪个 Symbol Table；
              这个栈需要在 CompUnit (sysy.y) 中被初始化（记录全局变量）
          5.9 我们在Parsing 的时候就可以确定每一个 Lval 属于哪个 SymTbl，
            可以直接在此时记住，之后好处理

        6. 注意 ConstDef (sysy.y) 中变量赋值必须是已经计算好的，这就需要我们check一下can_compute
          其实相当于也是 4. 的延申

        *7. 这里借机讨论一下，我的代码和文法实现编译时常量预计算的不同：）
            1. 首先肯定两者都是对的，不会有运行时错误
            2. std文法是通过实现 Const 和 Var 量定义的分开，来实现这个功能；并且要求
              Const 在定义时就必须计算出来。
              ;---------------> 这是一种自顶向下的方法
                这当然是好的，这样我们在向下递归的过程中，根据已经过的路径就已经知道之后的量可不可以被算出来了；
                缺点见 7.5
            3. 而我的预编译方法是，自底向上地看每个Type是否可以在编译时就确定结果，
                (当然如果定义成const的Type不行，那么我们就会Throw一个error，这是语义错误)
              这样的最大优点 --------------> 所有 编译时就可以确定结果的Type确实都可以被总结出来
              ------------> 如果不能，那么一定是需要 outer_param的，那么我们直接去stack里找就可以了！
          ***** 同样的，由于 ConstDef 必须有返回值，所以我们对于 VarDef的限制是比较少的，这之后可能还会有其他
            错误，比如右侧的 Exp 即使用了栈里的变量也算不出来 *** 但这是语义错误和我有什么关系呢。。。之后管！
            4. 实现中采用的神奇的地方：
                1. 查注释 'Changing the Mode of a Var' -> 不管之后生成 IR 的时候咋样，
                  只要我在 parse的时候能算的东西我一定会算！
                2. 符号表的Var的值，其实 parsing 之后就完全不会管了，没有用
                  !!!!!!!!!!!!!! 也不能管 !!!!!!!!!!!!!!!!!
                  !!!!!!!!!!!!!! 也不能管 !!!!!!!!!!!!!!!!!
                  !!!!!!!!!!!!!! 也不能管 !!!!!!!!!!!!!!!!!
                  !!!!!!!!!!!!!! 也不能管 !!!!!!!!!!!!!!!!!

                3. 'void StmtAST :: IRDump() const {' 中，我们如果发现给 Var 赋值是 can_compute，
                  可以直接 return 而不用在解析 IR 时将这个赋值给放到 @Var 里
                    可以思考一下Why?
                    The Answer is that : 因为这个赋值在 parsing 的时候就已知了，（于是Var在Symtbl中被置成已知的）
                      所以在用到这个赋值的所有position，这个赋值都已经被 (at least) 转交给了它上层的 PrimaryEXP ！
                

        *7.5 一份有例子的详细比较：
            1: 对于 const 定义和利用的能力：一致，因为我也实现了符号表 (哪怕关掉我的优化mode，优化能力也至少和std一致)
            2: 多次对一个变量修改，但如果每次用这个变量都是它被修改成可在 parsing时计算 (can_compute == 1)
              那么我的方法更优
            3: 常量表达式：显然也是我优，因为std可没有对这个优化；(int x = 123 * (98 - 7 % 16); )
            4: 对于3.拓展的更广阔的情况：
              - 我们显然不可以保证一个柿子中所有部分都可以 can_compute == 1，这样就太trivial，
                但仔细观察我的实现可以发现，可以被归结出的节点（子树全为 can_compute ）都被归结出来了，
                这就大大节约了 Generate IR 时要做的工作 (both for compiler and IR)
              - 后面会加上逻辑表达式的短路求值


        8. 我们假定，Lv_3 中 binary operator 的返回值都是在 形如 `%num` 这样的变量里的
        9. 我们假定，IR code里，变量的名称总是 @<ST_name>_<var_name>

        10. `Can't be this format?` 记录了在 IR 中打tag的失败尝试

        11. 实现了优化模式，原理是 4. 开始的常量优化，可以通过控制 `MODE=?` 来开关
            
        12. Stmt 中的赋值语句千万不能在 sysy.y 中处理，不然就寄了！( 因为赋值是要随着程序的进行而进行的 )
          但等式右边是常数的倒是可以先处理? (√), 

        13. 对于 OptionalExp 的处理，请参考 `sysy.y` 中 // Cheat Code Again
          总的来说我们就是，因为这个语句没有任何后效性，所以我们直接当 can_compute == 1 的来处理
          
        14. Lv_5 中对 SymbleTable 的一点改进：
          - 原先没有考虑到一个 Block 中定义和使用顺序先后的问题，例子：
                                          {
                                            int x = 1;
                                            {
                                              cout << x = x + 2 << endl;
                                              int x = 6;
                                              cout << x << endl;
                                            }
                                          }
          - 这种情况是没有问题的，于是我们还是需要在parsing的过程中，预处理出每一个Lval所属的符号表
            （这个是不难的），到时候直接从那里面取就可以了（
        
        15. How to Handle `if-else`
        

    
    To-Do :
      1. 将 Note $4 拓展到 Lv4上 (done)
      2. 

*/

#define MODE 1 // 2 为关掉优化的模式，1为优化模式

class BaseAST {
 public:
  static std::stack< SymbolTable* > *glbstst;
  static SymbolTable *glbsymbtl;

  virtual ~BaseAST() = default;
  virtual void Dump(int sj) const = 0;
  virtual void IRDump() const = 0;
  inline void HandleSJ(int sj) const ;
  int can_compute /* ___can be computed in compiling time___ ? for types below */, val;
  inline int PreComputeProcedure() const;
  /*
  Already can_compute implemented can_compute TYPE:
    Stmt
    Exp
    LOrExp ~ PrimaryExp
    InitVal
    ConstExp
    ConstInitVal
    VarDef

  When 'can_compute == 0', means:
    1. lack of stack var using (good)
    2. using un-valued var (bad in syntax)
  */
};

// Useful functions
SymbolTable* present_tbl();
void push_into_tbl_stk(SymbolTable* item, int has_fa);
void pop_tbl_stk();
std::string btype_transfer(std::string &BTYPE);

// Subclass Definition

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
  std::vector< std::unique_ptr<BaseAST> > *blockitem;
  SymbolTable *symbtl; // 每个block 有自己对应的符号表
  int child_num;
  int block_id;
  void Dump(int sj) const override;
  void IRDump() const override;
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp, optionalexp, block;
  std::string lval;
  SymbolTable *lval_belong;
  int sel;
  void Dump(int sj) const override;
  void IRDump() const override;
};


// --------------------------------- Lv_3  Operator-------------------------------------

// Useful Functions:
  // inline void out_binary_IR(int fi, int se, string op, int is_01_fi = 0, int is_01_se = 0);
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
  std::string lval;
  SymbolTable *lval_belong;
  int number;
  int sel;
  void Dump(int sj) const override;
  void IRDump() const override;
};

// --------------------------------- Lv_4  Const and Var-------------------------------------
// Useful Functions:
  // inline SymbolTable* present_tbl();
  // 

class DeclAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> constdecl, vardecl;
  int sel;

  void Dump(int sj) const override;
  void IRDump() const override;
};

class BlockItemAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> decl, stmt;
  int sel;

  void Dump(int sj) const override;
  void IRDump() const override;
};

class InitValAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;

  void Dump(int sj) const override;
  void IRDump() const override;
};

class ConstExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;

  void Dump(int sj) const override;
  void IRDump() const override;
};

class ConstInitValAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> constexp;

  void Dump(int sj) const override;
  void IRDump() const override;
};

class ConstDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> constinitval;
  std::string ident;

  void Dump(int sj) const override;
  void IRDump() const override;
};

class ConstDeclAST : public BaseAST {
 public:
  std::vector< std::unique_ptr<BaseAST> > *constdef;
  std::string btype;
  int child_num;
  void Dump(int sj) const override;
  void IRDump() const override;
};

class VarDefAST : public BaseAST {
 public:
  std::string ident;
  std::unique_ptr<BaseAST> initval;
  int sel;

  void Dump(int sj) const override;
  void IRDump() const override;  
};

// class BTypeAST : public BaseAST {
//  public:
//   std::string type;
//   void Dump(int sj) const override;
//   void IRDump() const override;
// };

class VarDeclAST : public BaseAST {
 public:
  std::vector< std::unique_ptr<BaseAST> > *vardef;
  std::string btype;
  int child_num;
  void Dump(int sj) const override;
  void IRDump() const override;
};



//  ------------------------ Lv_5 Blocks -----------------------

class OptionalExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  void Dump(int sj) const override;
  void IRDump() const override;
};


// Number 在 "sysy.y" 中定义的类型直接是 int_val 而不是 ast_val，所以不需要语法树类型
// class Number : public BaseAST {
//  public:
//     std::int val;
// };