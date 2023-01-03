%code requires {
  #include <memory>
  #include <string>
  #include "ast.h"
}

%{


#include <iostream>
#include <memory>
#include <string>
#include "ast.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast } // 这里改成 <BaseAST> 类型，因为我们返回的已经不是毫无组织的字符串了！

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val; // 用来返回每个非终结符对应的子语法树
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN
%token <str_val> IDENT UOP MULOPT RELOPT EQOPT ANDOPT OROPT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt
// 表达式定义
%type <ast_val> Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
// %type <str_val> UnaryOp
%type <int_val> Number

%%


CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;


FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;


FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->type = "int";
    $$ = ast;
  }
  ;

Block
  : '{' Stmt '}' {
    auto ast = new BlockAST();
    ast->stmt = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Stmt
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;

Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->lorexp = unique_ptr<BaseAST>($1);

    // Pre-Compute Tech
    ast->can_compute = ast->lorexp->can_compute;
    if (ast->can_compute)
      ast->val = ast->lorexp->val;
    //

    $$ = ast;
  }
  ;

LOrExp
  : LAndExp {
    auto ast = new LOrExpAST();
    ast->sel = 0;
    ast->landexp = unique_ptr<BaseAST>($1);

    // Pre-Compute Tech
    ast->can_compute = ast->landexp->can_compute;
    if (ast->can_compute)
      ast->val = ast->landexp->val;
    //

    $$ = ast;
  }
  | LOrExp OROPT LAndExp {
    auto ast = new LOrExpAST();
    ast->sel = 1;
    ast->lorexp = unique_ptr<BaseAST>($1);
    ast->landexp = unique_ptr<BaseAST>($3);

    // Pre-Compute Tech
    ast->can_compute = ast->lorexp->can_compute && ast->landexp->can_compute;
    if (ast->can_compute)
        ast->val = ast->lorexp->val || ast->landexp->val;
    //

    $$ = ast;
  }
  ;

LAndExp
  : EqExp {
    auto ast = new LAndExpAST();
    ast->sel = 0;
    ast->eqexp = unique_ptr<BaseAST>($1);

    // Pre-Compute Tech
    ast->can_compute = ast->eqexp->can_compute;
    if (ast->can_compute)
      ast->val = ast->eqexp->val;
    //

    $$ = ast;
  }
  | LAndExp ANDOPT EqExp {
    auto ast = new LAndExpAST();
    ast->sel = 1;
    ast->landexp = unique_ptr<BaseAST>($1);
    ast->eqexp = unique_ptr<BaseAST>($3);

    // Pre-Compute Tech
    ast->can_compute = ast->landexp->can_compute && ast->eqexp->can_compute;
    if (ast->can_compute)
        ast->val = ast->landexp->val && ast->eqexp->val;
    //

    $$ = ast;
  }
  ;

EqExp
  : RelExp {
    auto ast = new EqExpAST();
    ast->sel = 0;
    ast->relexp = unique_ptr<BaseAST>($1);

    // Pre-Compute Tech
    ast->can_compute = ast->relexp->can_compute;
    if (ast->can_compute)
      ast->val = ast->relexp->val;
    //

    $$ = ast;
  }
  | EqExp EQOPT RelExp {
    auto ast = new EqExpAST();
    ast->sel = 1;
    ast->eqexp = unique_ptr<BaseAST>($1);
    ast->rel = *unique_ptr<string>($2);
    ast->relexp = unique_ptr<BaseAST>($3);

    // Pre-Compute Tech
    ast->can_compute = ast->eqexp->can_compute && ast->relexp->can_compute;
    if (ast->can_compute){
      if (ast->rel[0] == '=')
        ast->val = ast->eqexp->val == ast->relexp->val;
      else 
        ast->val = ast->eqexp->val != ast->relexp->val;
    }
    //

    $$ = ast;
  }
  ;

RelExp
  : AddExp {
    auto ast = new RelExpAST();
    ast->sel = 0;
    ast->addexp = unique_ptr<BaseAST>($1);

    // Pre-Compute Tech
    ast->can_compute = ast->addexp->can_compute;
    if (ast->can_compute)
      ast->val = ast->addexp->val;
    //

    $$ = ast;
  }
  | RelExp RELOPT AddExp {
    auto ast = new RelExpAST();
    ast->sel = 1;
    ast->relexp = unique_ptr<BaseAST>($1);
    ast->rel = *unique_ptr<string>($2);
    ast->addexp = unique_ptr<BaseAST>($3);

    // Pre-Compute Tech
    ast->can_compute = ast->relexp->can_compute && ast->addexp->can_compute;
    if (ast->can_compute){
      if (ast->rel[0] == '>'){
        if (ast->rel.length() == 1)
          ast->val = ast->relexp->val > ast->addexp->val;
        else 
          ast->val = ast->relexp->val >= ast->addexp->val;
      }
      else{
        if (ast->rel.length() == 1)
          ast->val = ast->relexp->val < ast->addexp->val;
        else 
          ast->val = ast->relexp->val <= ast->addexp->val;
      }
    }
    //

    $$ = ast;
  }
  ;

AddExp
  : MulExp {
    auto ast = new AddExpAST();
    ast->sel = 0;
    ast->mulexp = unique_ptr<BaseAST>($1);

    // Pre-Compute Tech
    ast->can_compute = ast->mulexp->can_compute;
    if (ast->can_compute)
      ast->val = ast->mulexp->val;
    //

    $$ = ast;
  }
  | AddExp UOP MulExp {
    auto ast = new AddExpAST();
    ast->sel = 1;
    ast->addexp = unique_ptr<BaseAST>($1);
    ast->opt = *unique_ptr<string>($2);
    ast->mulexp = unique_ptr<BaseAST>($3);

    // Pre-Compute Tech
    ast->can_compute = ast->addexp->can_compute && ast->mulexp->can_compute;
    if (ast->can_compute){
      if (ast->opt[0] == '+')
        ast->val = ast->addexp->val + ast->mulexp->val;
      else if (ast->opt[0] == '-')
        ast->val = ast->addexp->val - ast->mulexp->val;
      else
        exit(2);
    }
    //

    $$ = ast;
  }
  ;

MulExp
  : UnaryExp {
    auto ast = new MulExpAST();
    ast->sel = 0;
    ast->unaryexp = unique_ptr<BaseAST>($1);

    // Pre-Compute Tech
    ast->can_compute = ast->unaryexp->can_compute;
    if (ast->can_compute)
      ast->val = ast->unaryexp->val;
    //

    $$ = ast;
  }
  | MulExp MULOPT UnaryExp {
    auto ast = new MulExpAST();
    ast->sel = 1;
    ast->mulexp = unique_ptr<BaseAST>($1);
    ast->opt = *unique_ptr<string>($2);
    ast->unaryexp = unique_ptr<BaseAST>($3);

    // Pre-Compute Tech
    ast->can_compute = ast->mulexp->can_compute && ast->unaryexp->can_compute;
    if (ast->can_compute){
      if (ast->opt[0] == '*')
        ast->val = ast->mulexp->val * ast->unaryexp->val;
      else if (ast->opt[0] == '/')
        ast->val = ast->mulexp->val / ast->unaryexp->val;
      else 
        ast->val = ast->mulexp->val % ast->unaryexp->val;
    }
    //

    $$ = ast;
  }
  ;

UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->sel = 0;
    ast->pexp = unique_ptr<BaseAST>($1);

    // Pre-Compute Tech
    ast->can_compute = ast->pexp->can_compute;
    if (ast->can_compute)
      ast->val = ast->pexp->val;
    //

    $$ = ast;
  }
  | UOP UnaryExp {
    auto ast = new UnaryExpAST();
    ast->sel = 1;
    ast->opt = *unique_ptr<string>($1);
    ast->unaryexp = unique_ptr<BaseAST>($2);

    // Pre-Compute Tech
    ast->can_compute = ast->unaryexp->can_compute;
    if (ast->can_compute){
      if (ast->opt[0] == '-')
        ast->val = -ast->unaryexp->val;
      else if (ast->opt[0] == '!')
        ast->val = !ast->unaryexp->val;
      else 
        ast->val = ast->unaryexp->val;
    }
    //

    $$ = ast;
  }
  ;

// UnaryOp 
//   : UOP {
//     // auto ast = new UnaryOpAST();
//     // ast->opt = *unique_ptr<string>($1);
//     $$ = $1;
//   }
//   ;

PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->sel = 0;
    ast->exp = unique_ptr<BaseAST>($2);
    
    // Pre-Compute Tech
    ast->can_compute = ast->exp->can_compute;
    if (ast->can_compute)
      ast->val = ast->exp->val;
    //
    
    $$ = ast;
  }
  | Number {
    auto ast = new PrimaryExpAST();
    ast->sel = 1;
    ast->number = ($1);

    // Pre-Compute Tech
    ast->can_compute = 1;
    ast->val = ast->number;
    //

    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    $$ = ($1);
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
