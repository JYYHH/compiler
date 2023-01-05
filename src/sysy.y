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

int total_blk_num = 0;
stack< BlockAST* > blk_st; // 用来实现块管理
std::stack< SymbolTable* > * BaseAST::glbstst = new std::stack< SymbolTable* >;
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

%token INT RETURN CONST IF ELSE
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

%type <str_val> LVal BType
%type <int_val> Number

%%


CompUnit
  : { 
    BaseAST::glbsymbtl->ST_name = "GLOBAL_Table";
    push_into_tbl_stk( BaseAST::glbsymbtl, 0); // 全局变量表进入
  }
   FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($2);

    pop_tbl_stk(); // 全局变量表退出
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
  : {
    total_blk_num ++;
    auto BLKast = new BlockAST();
    BLKast->block_id = total_blk_num;
    BLKast->symbtl = new SymbolTable();
    BLKast->symbtl->ST_name = "block_" + std::to_string(total_blk_num);
    blk_st.push(BLKast);
    push_into_tbl_stk(BLKast->symbtl, 1); // 这个 Block 的符号表进入
  }
   '{' Block_inter '}' {
    auto BLKast = blk_st.top();
    BLKast->blockitem = ($3);
    BLKast->child_num = (int)(*(BLKast->blockitem)).size();
    pop_tbl_stk(); // 这个 Block 的符号表退出
    blk_st.pop();
    $$ = BLKast;
  }
  ;

Stmt
  : 
  // {
  //   // Check Something
  //   std::cout << "Down here?" << endl;
  // }
  RETURN OptionalExp ';' {
    auto ast = new StmtAST();
    ast->optionalexp = unique_ptr<BaseAST>($2);
    ast->sel = 3;

    ast->can_compute = ast->optionalexp->can_compute;
    if (ast->can_compute)
      ast->val = ast->optionalexp->val;

    $$ = ast;
  }
  | LVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->lval = *unique_ptr<string>($1);
    ast->sel = 0;

    auto pres_symbtl = present_tbl();
    // Check whether something wrong?
    auto ret = pres_symbtl->GetItemByName(ast->lval);
    ast->lval_belong = pres_symbtl->present;

    if (ret == NULL)
      exit(4);
    if (!(ret->VarType() & 1))
      exit(6);

    // Changing the Mode of a Var
    if (ast->exp->can_compute == 0){
      ast->can_compute = 0;
      ret->BecomeUnknown();
      // waiting computing it on the stack
    }
    else{
      ast->can_compute = 1;
      ast->val = ast->exp->val;
      ret->SetVal(ast->val);
    }
    $$ = ast;
  }
  | OptionalExp ';' {
    auto ast = new StmtAST();
    ast->optionalexp = unique_ptr<BaseAST>($1);
    ast->sel = 1;

    ast->can_compute = ast->optionalexp->can_compute;
    if (ast->can_compute)
      ast->val = ast->optionalexp->val;

    $$ = ast;
  }
  | Block {
    auto ast = new StmtAST();
    ast->sel = 2;
    ast->block = unique_ptr<BaseAST>($1);

    ast->can_compute = 0;
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
  | LVal {
    // std::cout << (*(BaseAST::glbstst)).size() << std::endl;
    auto ast = new PrimaryExpAST();
    ast->sel = 2;
    ast->lval = *unique_ptr<string>($1);

    // CAN'T PRECOMPUTE, if and only if it hasn't been computed
    auto pres_symbtl = present_tbl();
    auto ret = pres_symbtl->GetItemByName(ast->lval);
    ast->lval_belong = pres_symbtl->present;
    if (ret == NULL)
      exit(4);
    
    ast->can_compute = ret->VarType() >> 1;
    ast->val = ret->VarVal();

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
    // Pre-Compute Tech
    ast->can_compute = ast->exp->can_compute;
    if (ast->can_compute)
      ast->val = ast->exp->val;
    //
    $$ = ast;
  }
  ;

ConstExp
  : Exp {
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<BaseAST>($1);
    // Pre-Compute Tech
    ast->can_compute = ast->exp->can_compute;
    if (ast->can_compute)
      ast->val = ast->exp->val;
    //
    $$ = ast;
  }
  ;

ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
    ast->constexp = unique_ptr<BaseAST>($1);
    // Pre-Compute Tech
    ast->can_compute = ast->constexp->can_compute;
    if (ast->can_compute)
      ast->val = ast->constexp->val;
    //
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
  | Stmt {
    auto ast = new BlockItemAST();
    ast->stmt = unique_ptr<BaseAST>($1);
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

    // Insert a non-valued item into present symbal table
    auto pres_symbtl = present_tbl();
    auto new_symtbl_item = new SymbolTableItem(1);
    pres_symbtl->Insert(ast->ident, *new_symtbl_item);
    ast->can_compute = 0; // can't be decided in compiling time

    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->initval = unique_ptr<BaseAST>($3);
    ast->sel = 1;

    auto pres_symbtl = present_tbl();
    // Check Whether Already Computed
    if (ast->initval->can_compute == 0){
      // Insert a non-valued item into present symbal table
      auto new_symtbl_item = new SymbolTableItem(1);
      pres_symbtl->Insert(ast->ident, *new_symtbl_item);
      // Wait for putting its computing procedure onto the stack?
      ast->can_compute = 0;
    }
    else{
    // Insert an already-valued item into present symbal table
      ast->can_compute = 1;
      ast->val = ast->initval->val;
      auto new_symtbl_item = new SymbolTableItem(1, ast->val);
      pres_symbtl->Insert(ast->ident, *new_symtbl_item);
    }

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

    // Check Whether Already Computed
    if (ast->constinitval->can_compute == 0)
      exit(5);
    // Because ConstDef must be computed in the compiling time, 
      // so we don't use ast->can_compute, it must be 1

    // Insert a const item into present symbal table
    
    auto pres_symbtl = present_tbl();
    auto new_symtbl_item = new SymbolTableItem(0, ast->constinitval->val);
    pres_symbtl->Insert(ast->ident, *new_symtbl_item);

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
    // Pre-Compute Tech
    ast->can_compute = ast->exp->can_compute;
    if (ast->can_compute)
      ast->val = ast->exp->val;
    //
    $$ = ast;
  }
  | {
    auto ast = new OptionalExpAST();
    ast->exp = NULL;
    ast->can_compute = 1;
    ast->val = 0;
    // Cheat Code

    $$ = ast;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
