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

stack< SymbolTable* > * BaseAST::glbstst = new stack< SymbolTable* >;
SymbolTable * BaseAST::glbsymbtl = new SymbolTable();

%}


%parse-param { std::unique_ptr<BaseAST> &ast } 
// 这里改成 <BaseAST> 类型，因为我们返回的已经不是毫无组织的字符串了



%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val; // 用来返回每个非终结符对应的子语法树
  std::vector< std::unique_ptr<BaseAST> > *ast_list; // 用来返回vector指针，vector用来存所有子节点
}
%define parse.error verbose
%token INT RETURN CONST IF ELSE WHILE BREAK CONTINUE
%token <str_val> IDENT UOP MULOPT RELOPT EQOPT ANDOPT OROPT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt
%type <ast_list> Block_inter VarDecl_inter ConstDecl_inter
// 表达式定义
%type <ast_val> Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
// 常量和变量定义
%type <ast_val> Decl BlockItem InitVal ConstExp VarDef VarDecl 
%type <ast_val> ConstInitVal ConstDef ConstDecl
// Lv_5 块儿定义
%type <ast_val> OptionalExp
// Lv_6 If-else
%type <ast_val> IfStmt IfElseStmt GLBIf
// Lv_7 While 直接加在了 Stmt 里

%type <str_val> LVal BType
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
  : '{' Block_inter '}' {
    auto ast = new BlockAST();
    ast->blockitem = ($2);
    ast->child_num = (int)(*(ast->blockitem)).size();
    $$ = ast;
  }
  ;

// subroutine:
//   %empty {
//     exp_state.push(NOW_exp);
//     present_tbl()->Push(!!NOW_exp.first, !!NOW_exp.second);
//   }
// ;

// subroutine2:
//   %empty {
//     NOW_exp = exp_state.top();
//     exp_state.pop();
//     present_tbl()->Pop();

//     exp_state.push(Pr(NOW_exp.first, !NOW_exp.second));
//     present_tbl()->Push(NOW_exp.first, !NOW_exp.second);
//   }
// ;

GLBIf 
  : IfStmt {
    auto ast = new GLBIfAST();
    ast->sel = 0;
    ast->ifstmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | IfElseStmt {
    auto ast = new GLBIfAST();
    ast->sel = 1;
    ast->ifelsestmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

IfStmt
  : IF '(' Exp ')' GLBIf {
    auto ast = new IfStmtAST();
    ast->sel = 0;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->glbif = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IF '(' Exp ')' IfElseStmt ELSE IfStmt {
    auto ast = new IfStmtAST();
    ast->sel = 1;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->ifelsestmt = unique_ptr<BaseAST>($5);
    ast->ifstmt = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  ;



IfElseStmt
  : IF '(' Exp ')' IfElseStmt ELSE IfElseStmt {
    auto ast = new IfElseStmtAST();
    ast->sel = 0;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->ifelsestmtl = unique_ptr<BaseAST>($5);
    ast->ifelsestmtr = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  | Stmt {
    auto ast = new IfElseStmtAST();
    ast->sel = 1;
    ast->stmt = unique_ptr<BaseAST>($1);
    ast->can_compute = 1;
    $$ = ast;
  }
  ;


Stmt
  : RETURN OptionalExp ';' {
    auto ast = new StmtAST();
    ast->optionalexp = unique_ptr<BaseAST>($2);
    ast->sel = 3;
    $$ = ast;
  }
  | LVal '=' Exp ';' { // 赋值语句是最麻烦的
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->lval = *unique_ptr<string>($1);
    ast->sel = 0;
    $$ = ast;
  }
  | OptionalExp ';' {
    auto ast = new StmtAST();
    ast->optionalexp = unique_ptr<BaseAST>($1);
    ast->sel = 1;
    $$ = ast;
  }
  | Block {
    auto ast = new StmtAST();
    ast->sel = 2;
    ast->block = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | WHILE '(' Exp ')' GLBIf {
    auto ast = new StmtAST();
    ast->sel = 4;
    ast->exp = unique_ptr<BaseAST>($3);
    ast->glbif = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | CONTINUE ';' {
    auto ast = new StmtAST();
    ast->sel = 5;
    $$ = ast;
  }
  | BREAK ';' {
    auto ast = new StmtAST();
    ast->sel = 6;
    $$ = ast;
  } 
  ;

Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->lorexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

LOrExp
  : LAndExp {
    auto ast = new LOrExpAST();
    ast->sel = 0;
    ast->landexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LOrExp OROPT LAndExp {
    auto ast = new LOrExpAST();
    ast->sel = 1;
    ast->lorexp = unique_ptr<BaseAST>($1);
    ast->landexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

LAndExp
  : EqExp {
    auto ast = new LAndExpAST();
    ast->sel = 0;
    ast->eqexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LAndExp ANDOPT EqExp {
    auto ast = new LAndExpAST();
    ast->sel = 1;
    ast->landexp = unique_ptr<BaseAST>($1);
    ast->eqexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

EqExp
  : RelExp {
    auto ast = new EqExpAST();
    ast->sel = 0;
    ast->relexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | EqExp EQOPT RelExp {
    auto ast = new EqExpAST();
    ast->sel = 1;
    ast->eqexp = unique_ptr<BaseAST>($1);
    ast->rel = *unique_ptr<string>($2);
    ast->relexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

RelExp
  : AddExp {
    auto ast = new RelExpAST();
    ast->sel = 0;
    ast->addexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | RelExp RELOPT AddExp {
    auto ast = new RelExpAST();
    ast->sel = 1;
    ast->relexp = unique_ptr<BaseAST>($1);
    ast->rel = *unique_ptr<string>($2);
    ast->addexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

AddExp
  : MulExp {
    auto ast = new AddExpAST();
    ast->sel = 0;
    ast->mulexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | AddExp UOP MulExp {
    auto ast = new AddExpAST();
    ast->sel = 1;
    ast->addexp = unique_ptr<BaseAST>($1);
    ast->opt = *unique_ptr<string>($2);
    ast->mulexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

MulExp
  : UnaryExp {
    auto ast = new MulExpAST();
    ast->sel = 0;
    ast->unaryexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | MulExp MULOPT UnaryExp {
    auto ast = new MulExpAST();
    ast->sel = 1;
    ast->mulexp = unique_ptr<BaseAST>($1);
    ast->opt = *unique_ptr<string>($2);
    ast->unaryexp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST();
    ast->sel = 0;
    ast->pexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | UOP UnaryExp {
    auto ast = new UnaryExpAST();
    ast->sel = 1;
    ast->opt = *unique_ptr<string>($1);
    ast->unaryexp = unique_ptr<BaseAST>($2);
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
    $$ = ast;
  }
  | Number {
    auto ast = new PrimaryExpAST();
    ast->sel = 1;
    ast->number = ($1);
    $$ = ast;
  }
  | LVal {
    // std::cout << (*(BaseAST::glbstst)).size() << std::endl;
    auto ast = new PrimaryExpAST();
    ast->sel = 2;
    ast->lval = *unique_ptr<string>($1);
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    $$ = ($1);
  }
  ;

//--------------------------------- Lv 4 ----------------------------------
  // easy part first
LVal
  : IDENT {
    $$ = ($1);
  }
  ;

InitVal
  : Exp {
    auto ast = new InitValAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

ConstExp
  : Exp {
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
    ast->constexp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

BType
  : INT {
    auto nnnn_str = new std::string("int");
    $$ = nnnn_str;
  }
  ;

  // then complicated

Block_inter // 这个类型可以由指向若干个BlockItem的pointer的Vector组成，当然这个类型返回的本身也是指针
  : {
    $$ = new std::vector< std::unique_ptr<BaseAST> >;
  }
  | Block_inter BlockItem{
    auto ret = ($1);
    (*ret).push_back( unique_ptr<BaseAST>($2) );
    $$ = ret;
  }
  ;

BlockItem
  : Decl {
    auto ast = new BlockItemAST();
    ast->decl = unique_ptr<BaseAST>($1);
    ast->sel = 0;
    $$ = ast;
  }
  | GLBIf {
    auto ast = new BlockItemAST();
    ast->glbif = unique_ptr<BaseAST>($1);
    ast->sel = 1;
    $$ = ast;
  }
  ;

Decl
  : ConstDecl {
    auto ast = new DeclAST();
    ast->constdecl = unique_ptr<BaseAST>($1);
    ast->sel = 0;
    $$ = ast;
  }
  | VarDecl {
    auto ast = new DeclAST();
    ast->vardecl = unique_ptr<BaseAST>($1);
    ast->sel = 1;
    $$ = ast;
  }
  ;

VarDef
  : IDENT {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->sel = 0;
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->initval = unique_ptr<BaseAST>($3);
    ast->sel = 1;
    $$ = ast;
  }
  ;

VarDecl
  : BType VarDecl_inter ';' {
    auto ast = new VarDeclAST();
    ast->btype = *unique_ptr<std::string>($1);
    ast->vardef = ($2);
    ast->child_num = (int)(*(ast->vardef)).size();
    $$ = ast;
  }
  ; 

VarDecl_inter
  : VarDef {
    auto ret = new std::vector< std::unique_ptr<BaseAST> >;
    (*ret).push_back( unique_ptr<BaseAST>($1) );
    $$ = ret;
  }
  | VarDecl_inter ',' VarDef {
    auto ret = ($1);
    (*ret).push_back( unique_ptr<BaseAST>($3) );
    $$ = ret;
  }
  ;

ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->constinitval = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

ConstDecl
  : CONST BType ConstDecl_inter ';' {
    auto ast = new ConstDeclAST();
    ast->btype = *unique_ptr<std::string>($2);
    ast->constdef = ($3);
    ast->child_num = (int)(*(ast->constdef)).size();
    $$ = ast;
  }
  ; 

ConstDecl_inter
  : ConstDef {
    auto ret = new std::vector< std::unique_ptr<BaseAST> >;
    (*ret).push_back( unique_ptr<BaseAST>($1) );
    $$ = ret;
  }
  | ConstDecl_inter ',' ConstDef {
    auto ret = ($1);
    (*ret).push_back( unique_ptr<BaseAST>($3) );
    $$ = ret;
  }
  ;

//--------------------------------- Lv 5 ----------------------------------

OptionalExp
  : Exp {
    auto ast = new OptionalExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | {
    auto ast = new OptionalExpAST();
    ast->exp = NULL;
    $$ = ast;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
