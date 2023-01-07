#include <string>
#include <memory>
#include <iostream>
#include <cstdio>
#include "ast.h"

/*
    Note for IR Generating:
        1.. 用了一个比较精巧的结构 (int var_num, is_01; int var_ins[300005];)
            -> 同时存 Koopa 中出现的常量 和 变量
    
    Note for DS using:
        1. BaseAST 中的 :
            static std::stack< SymbolTable* > *glbstst; -> 维护 parsing 以及 IR_generating 中需要的
              符号表栈
                - 对应函数是：
                    1. present_tbl()
                    2. push_into_tbl_stk()
                    3. pop_tbl_stk()
            static SymbolTable *glbsymbtl; -> 全局符号表?
        3. stack< Pr > blk_info;
            IR_generating 时块信息的管理，主要用来管理当前 Block 和 Instr在这个Block中的ID
        4.  int var_num, is_01;
            int var_ins[300005];
           这些是用来维护，IR_generating过程中，所有 `%number` 项目的信息
        6.
          
*/

using namespace std;

// Block Info & Func Ret
stack< Pr > blk_info;
#define PUSH(x,y) blk_info.push(Pr(x,y))
#define POP blk_info.pop()
#define TOP blk_info.top()

string now_btype;
string now_type;
string funcparam_name_ir;
string funcparam_btype;
int BLKID, ID_instr, Func_Ret;
int If_num = 0, While_NUM = 0;
int OR_num = 0, AND_num = 0;
int Continue_num = 0, Break_num = 0;
stack <int> while_stack;

inline void UPDATE(){
    Pr &u = TOP;
    BLKID = u.first;
    ID_instr = u.second;
}

// Page_Table Part------------------------------------------------------
SymbolTable* present_tbl(){
    return (*(BaseAST::glbstst)).top();
}   

void push_into_tbl_stk(SymbolTable* item, int has_fa){
    if (has_fa)
        item->father = present_tbl();
    else 
        item->father = NULL;
    (*(BaseAST::glbstst)).push(item);
}

void pop_tbl_stk(){
    (*(BaseAST::glbstst)).pop();
}

// Page Table Part ----------------------------------------------------------
// Some Useful function

int var_num, is_01; // is_01 = 0 -> binary ; 1 -> 0/1 ; 2 -> const
int var_ins[300005]; // an optimization on saved IR register --- save constant before
// we save space for useless `%xx` items in IR-code, since they're all constant 


inline void alr_compute_procedure(int NUMb){
    var_ins[var_num ++] = NUMb, is_01 = 2;
}

inline int BaseAST :: PreComputeProcedure() const{
    if (can_compute == MODE){ // in this condition, we handle this instr already in the parsing time.
        alr_compute_procedure(val);
        return 1;
    }
    return 0;
}

// ---------------------Begin the Part of Generting the IR (koopa)---------------------------------

void CompUnitAST :: IRDump() const{
    printf("decl @getint(): i32\ndecl @getch(): i32\ndecl @getarray(*i32): i32\ndecl @putint(i32)\ndecl @putch(i32)\ndecl @putarray(i32, *i32)\ndecl @starttime()\ndecl @stoptime()\n\n\n");


    push_into_tbl_stk(glbsymbtl, 0);
    std::vector< std::unique_ptr<BaseAST> > &now_vec = *func_def;
    for (int i=0; i<child_num; i++)
        now_vec[i]->IRDump();
    pop_tbl_stk();
}
void FuncDefAST :: IRDump() const {
    var_num = 0; // 局部临时变量可以重用

    std::cout << "fun @";
    std::cout << ident << '(';

    std::vector< std::unique_ptr<BaseAST> > &now_vec = *funcfparam; 
    for (int i=0; i<child_num; i++){
        if (i) std::cout << ", ";
        now_vec[i]->IRDump();
        std::cout << '@' << funcparam_name_ir << ": " << funcparam_btype;
    }

    std::cout << ')';

    if (func_type == "int")
        std::cout << ": i32 ", now_type = "int";
    else 
        std::cout << ' ', now_type = "void";

    std::cout << "{" << endl;
    std::cout << " %" << "entry:" << endl;
    
    for (int i=0; i<child_num; i++){
        now_vec[i]->IRDump();
        cout << "    %" << funcparam_name_ir << " = alloc i32" << endl;
        cout << "    store @" << funcparam_name_ir << ", %" << funcparam_name_ir << endl;
        // 但如果后续在函数中修改了函数 Param 可能会寄掉
    }

    block->IRDump();
    if (now_type == "int")
        std::cout << "    ret 0" << endl; // 愿称你为托底天王
    else if (now_type == "void")
        std::cout << "    ret" << endl; // 愿称你为托底天王
    std::cout << "}" << endl;
}
void FuncFParamAST :: IRDump() const {
    funcparam_name_ir = ident;
    if (btype == "int")
        funcparam_btype = "i32";
    else 
        funcparam_btype = "Please Check your Func Param Btype";
}
void BlockAST :: IRDump() const {
    push_into_tbl_stk(symbtl, 1);
    present_tbl()->reach_st.push(1);

    // list the basic block here further?
    std::vector< std::unique_ptr<BaseAST> > &now_vec = *blockitem; 

    
    // std::cout << " %" << "Block_" << block_id << ": " << endl;
    // Can't be this format?

    for (int i=0;i<child_num;i++){
        PUSH(block_id, i);
        now_vec[i]->IRDump();
        POP;
    }

    present_tbl()->reach_st.pop();
    pop_tbl_stk();
}

void BlockItemAST :: IRDump() const {
    // std::cout << " %" << "Instr_" << ID_instr << "_FromBlock_" << BLKID << ": " << endl;
    // Can't be this format?

    if (sel == 0)
        decl->IRDump();
    else 
        glbif->IRDump();
}

void StmtAST :: IRDump() const {
    // var_num = 0;
    if (sel == 3){
        Func_Ret ++;
        // if (Func_Ret > 1) return;

        if (now_type == "int"){
            if (optionalexp == NULL)
                exit(9);
            if (can_compute == MODE)
                std::cout << "    ret " << val << endl;
            else{
                optionalexp->IRDump();
                if(is_01 >> 1)
                    std::cout << "    ret " << var_ins[var_num - 1] << endl;
                else
                    std::cout << "    ret %" << var_num - 1 << endl;
            }
        }
        else{
            // if (optionalexp != NULL)
            //     exit(10);
            std::cout << "    ret" << endl;
        }

        std::cout << " %" << "after_ret_" << Func_Ret << ':' << endl;
    }
    else if (sel == 0){
        if (can_compute == MODE){
            string alter_one = lval;
            auto ret = lval_belong->GetItemByName(alter_one);
            if (ret->VarType() & 64)
                std::cout << "    store " << val << ", %" << lval << endl;
            else
                std::cout << "    store " << val << ", @" << lval_belong->ST_name << '_' << lval << endl;
            return;
        }

        exp->IRDump();

        string alter_one = lval;
        auto ret = lval_belong->GetItemByName(alter_one);

        if(is_01 >> 1)
            if (ret->VarType() & 64)
                std::cout << "    store " << var_ins[var_num - 1] << ", %" << lval << endl;
            else
                std::cout << "    store " << var_ins[var_num - 1] << ", @" << lval_belong->ST_name << '_' << lval << endl;
        else
            if (ret->VarType() & 64)
                std::cout << "    store %" << var_num - 1 << ", %" << lval << endl;
            else
                std::cout << "    store %" << var_num - 1 << ", @" << lval_belong->ST_name << '_' << lval << endl;
    }
    else if (sel == 1){
        if (can_compute == 0)
            optionalexp->IRDump();
    }
    else if (sel == 2)
        block->IRDump();
    else if (sel == 4){ // while
        int WHILENUM = ++While_NUM;
        while_stack.push(WHILENUM);

        // jump %WHILE_BEGIN_<num>
        std::cout << "    jump %WHILE_BEGIN_" << WHILENUM << endl;
        // %WHILE_BEGIN_<num>:
        std::cout << " %" << "WHILE_BEGIN_" << WHILENUM << ':' << endl;

        exp->IRDump();

        if (is_01 >> 1)
            std::cout << "    br " << var_ins[var_num - 1] << ", %" << "WHILE_THEN_" << WHILENUM << ", %" << "WHILE_END_" << WHILENUM << endl;
        else
            std::cout << "    br %" << var_num - 1 << ", %" << "WHILE_THEN_" << WHILENUM << ", %" << "WHILE_END_" << WHILENUM << endl;
        
        // %WHILE_THEN_<num>:
        std::cout << " %" << "WHILE_THEN_" << WHILENUM << ':' << endl;
        glbif->IRDump();
        // jump %WHILE_BEGIN_<num>
        std::cout << "    jump %WHILE_BEGIN_" << WHILENUM << endl;

        // %WHILE_END_<num>:
        std::cout << " %" << "WHILE_END_" << WHILENUM << ':' << endl;
        while_stack.pop();
    }
    else if (sel == 5){ // continue
        if (while_stack.size() == 0)
            exit(8);
        Continue_num ++;
        std::cout << "    jump %WHILE_BEGIN_" << while_stack.top() << endl;
        std::cout << " %" << "AFTER_CONTINUE_" << Continue_num << ':' << endl;
    }
    else if (sel == 6){ // break
        if (while_stack.size() == 0)
            exit(8);
        Break_num ++;
        std::cout << "    jump %WHILE_END_" << while_stack.top() << endl;
        std::cout << " %" << "AFTER_BREAK_" << Break_num << ':' << endl;
    }
}

inline void out_binary_IR(int fi, int se, string op, int is_01_fi = 0, int is_01_se = 0){
    if (!is_01_fi && !is_01_se)
        std::cout << "    %" << var_num << " = " << op << " %" << fi << ", %" << se << endl;
    else if(is_01_fi && is_01_se)
        std::cout << "    %" << var_num << " = " << op << " " << var_ins[fi] << ", " << var_ins[se] << endl;
    else if(is_01_fi)
        std::cout << "    %" << var_num << " = " << op << " " << var_ins[fi] << ", %" << se << endl;
    else
        std::cout << "    %" << var_num << " = " << op << " %" << fi << ", " << var_ins[se] << endl;
    var_num ++; // whenever var_num ++, we should check whether it's of 01 format, led by an instr after this func
}

    //---------------- Expression Part--------------------------------
    // Only after executing Logic and Comparing Operating, we can manipulate a value into 0/1

inline void bin201(){
    if (is_01 == 0){
        std::cout << "    %" << var_num << " = ne 0, %" << var_num - 1 << endl;
        var_num ++, is_01 = 1;
    }
    else if (is_01 == 2){
        std::cout << "    %" << var_num << " = ne 0, " << var_ins[var_num - 1] << endl;
        var_num ++, is_01 = 1;
    }
}

void ExpAST :: IRDump() const {
    if (PreComputeProcedure()) return;
    
    lorexp->IRDump();
}
void LOrExpAST :: IRDump() const {
    if (PreComputeProcedure()) return;

    if (sel == 0)
        landexp->IRDump();
    else{
        int ORNUM = ++OR_num;
        // @x = alloc i32
        std::cout << "    @" << "LOR" << '_' << ORNUM << " = alloc i32" << endl;
        // store 1, @x
        std::cout << "    store 1, @LOR_" << ORNUM << endl;

        lorexp->IRDump();
        bin201();

        if (is_01 >> 1)
            std::cout << "    br " << var_ins[var_num - 1] << ", %" << "LOREND" << '_' << ORNUM << ", %" << "LORTHEN" << '_' << ORNUM << endl;
        else
            std::cout << "    br %" << var_num - 1 << ", %" << "LOREND" << '_' << ORNUM << ", %" << "LORTHEN" << '_' << ORNUM << endl;
        
        std::cout << " %" << "LORTHEN" << '_' << ORNUM << ':' << endl;
        landexp->IRDump();
        bin201();
        // store %<new>, @x
        std::cout << "    store %" << var_num - 1 << ", @LOR_" << ORNUM << endl;
        std::cout << "    jump %" << "LOREND" << '_' << ORNUM << endl;

        std::cout << " %" << "LOREND" << '_' << ORNUM << ':' << endl;
        // %<new> = load @x
        std::cout << "    %" << var_num << " = load @LOR_" << ORNUM << endl;
        var_num ++, is_01 = 1;
    }
}
void LAndExpAST :: IRDump() const {
    if (PreComputeProcedure()) return;

    if (sel == 0)
        eqexp->IRDump();
    else{
        int ANDNUM = ++AND_num;
        // @x = alloc i32
        std::cout << "    @" << "LAND" << '_' << ANDNUM << " = alloc i32" << endl;
        // store 1, @x
        std::cout << "    store 0, @LAND_" << ANDNUM << endl;

        landexp->IRDump();
        bin201();

        if (is_01 >> 1)
            std::cout << "    br " << var_ins[var_num - 1] << ", %" << "LANDTHEN" << '_' << ANDNUM << ", %" << "LANDEND" << '_' << ANDNUM << endl;
        else
            std::cout << "    br %" << var_num - 1 << ", %" << "LANDTHEN" << '_' << ANDNUM << ", %" << "LANDEND" << '_' << ANDNUM << endl;
        
        std::cout << " %" << "LANDTHEN" << '_' << ANDNUM << ':' << endl;
        eqexp->IRDump();
        bin201();
        // store %<new>, @x
        std::cout << "    store %" << var_num - 1 << ", @LAND_" << ANDNUM << endl;
        std::cout << "    jump %" << "LANDEND" << '_' << ANDNUM << endl;

        std::cout << " %" << "LANDEND" << '_' << ANDNUM << ':' << endl;
        // %<new> = load @x
        std::cout << "    %" << var_num << " = load @LAND_" << ANDNUM << endl;
        var_num ++, is_01 = 1;
    }
}
void EqExpAST :: IRDump() const {
    if (PreComputeProcedure()) return;

    if (sel == 0)
        relexp->IRDump();
    else{
        eqexp->IRDump();
        int pre = var_num - 1, pre_is = is_01;
        relexp->IRDump();
        if (rel[0] == '=')
            out_binary_IR(pre, var_num - 1, "eq", pre_is >> 1, is_01 >> 1);
        else
            out_binary_IR(pre, var_num - 1, "ne", pre_is >> 1, is_01 >> 1);
        is_01 = 1;
    }
}
void RelExpAST :: IRDump() const {
    if (PreComputeProcedure()) return;

    if (sel == 0)
        addexp->IRDump();
    else{
        relexp->IRDump();
        int pre = var_num - 1, pre_is = is_01;
        addexp->IRDump();
        if (rel[0] == '>')
            if (rel.length() == 1)
                out_binary_IR(pre, var_num - 1, "gt", pre_is >> 1, is_01 >> 1);
            else
                out_binary_IR(pre, var_num - 1, "ge", pre_is >> 1, is_01 >> 1);
        else
            if (rel.length() == 1)
                out_binary_IR(pre, var_num - 1, "lt", pre_is >> 1, is_01 >> 1);
            else
                out_binary_IR(pre, var_num - 1, "le", pre_is >> 1, is_01 >> 1);
        is_01 = 1;
    }
}
void AddExpAST :: IRDump() const {
    if (PreComputeProcedure()) return;

    if (sel == 0)
        mulexp->IRDump();
    else{
        addexp->IRDump();
        int pre = var_num - 1, pre_is = is_01;
        mulexp->IRDump();
        if(opt[0] == '+')
            out_binary_IR(pre, var_num - 1, "add", pre_is >> 1, is_01 >> 1);
        else
            out_binary_IR(pre, var_num - 1, "sub", pre_is >> 1, is_01 >> 1);
        is_01 = 0;
    }
}
void MulExpAST :: IRDump() const {
    if (PreComputeProcedure()) return;

    if (sel == 0)
        unaryexp->IRDump();
    else{
        mulexp->IRDump();
        int pre = var_num - 1, pre_is = is_01;
        unaryexp->IRDump();
        if(opt[0] == '*')
            out_binary_IR(pre, var_num - 1, "mul", pre_is >> 1, is_01 >> 1);
        else if (opt[0] == '/')
            out_binary_IR(pre, var_num - 1, "div", pre_is >> 1, is_01 >> 1);
        else
            out_binary_IR(pre, var_num - 1, "mod", pre_is >> 1, is_01 >> 1);
        is_01 = 0;
    }
}
void UnaryExpAST :: IRDump() const {
    if (PreComputeProcedure()) return;

    if (sel == 0)
        pexp->IRDump();
    else if (sel == 1) {
        unaryexp->IRDump();
        if(opt[0] == '-'){
            if(is_01 >> 1)
                std::cout << "    %" << var_num << " = sub 0, " << var_ins[var_num - 1] << endl;
            else
                std::cout << "    %" << var_num << " = sub 0, %" << var_num - 1 << endl;
            var_num ++, is_01 = 0;
        }
        else if (opt[0] == '!'){
            if(is_01 >> 1)
                std::cout << "    %" << var_num << " = eq 0, " << var_ins[var_num - 1] << endl;
            else
                std::cout << "    %" << var_num << " = eq 0, %" << var_num - 1 << endl;
            var_num ++, is_01 = 1;
        }
    }
    else{
        // 我们进行函数调用
        std::vector< std::unique_ptr<BaseAST> > &now_vec = *funcrparam; 
        std::vector< Pr > *now_exp = new std::vector< Pr >;

        for (int i=0; i<child_num; i++){
            now_vec[i]->IRDump();
            (*now_exp).push_back(Pr(var_num - 1, is_01));
            // cout << var_num - 1 << ' ' << is_01 << ' ' << var_ins[var_num - 1] << endl;
        }

        string tttemp = ident;
        auto ret = BaseAST::glbsymbtl->GetItemByName(tttemp);
        if (ret == NULL)
            exit(11);
        if (ret->VarType() & 8){
            cout << "    %" << var_num << " = call @" << ident << '(';
            var_num ++, is_01 = 0;
        }
        else
            cout << "    call @" << ident << '(';
        
        for (int i=0; i<child_num; i++){
            if (i)
                cout << ", ";
            Pr &now_pr = (*now_exp)[i];
            if (now_pr.second >> 1)
                cout << var_ins[now_pr.first];
            else
                cout << '%' << now_pr.first;
        }

        cout << ')' << endl;
    }   
}
void PrimaryExpAST :: IRDump() const {
    if (PreComputeProcedure()) return;

    if (sel == 0)
        exp->IRDump();
    else if (sel == 2){
        // %0 = load @x
        string alter_one = lval;
        auto ret = lval_belong->GetItemByName(alter_one);
        // cout << "PrimaryExpAST Belong to Block: " << lval_belong->ST_name << endl;
        if (!(ret->VarType() & 1)){
            alr_compute_procedure(ret->VarVal());
            return;
        }
        if (ret->VarType() & 64)
            std::cout << "    %" << var_num << " = load %" << lval << endl;
        else
            std::cout << "    %" << var_num << " = load @" << lval_belong->ST_name << '_' << lval << endl;
        var_num ++, is_01 = 0;
    }
    else 
        alr_compute_procedure(val);
}


// --------------------------------- Lv_4  Const and Var-------------------------------------

void InitValAST :: IRDump() const {
    if (PreComputeProcedure()) return;
    
    exp->IRDump();
}

void ConstInitValAST :: IRDump() const {
    // do nothing, for const
}

void ConstExpAST :: IRDump() const {
    // do nothing, for const
}

void DeclAST :: IRDump() const {
    if (sel == 0)
        constdecl->IRDump();
    else 
        vardecl->IRDump();
}

void ConstDeclAST :: IRDump() const {
    // WE USE CONST VAR ONLY IN SYMBOL TABLE, not here
    // SO we don't need to parse IR, for the computing the constant value

    // do nothing, for const has already been computed before
}

void VarDeclAST :: IRDump() const {
    std::vector< std::unique_ptr<BaseAST> > &now_vec = *vardef;
    for(int i=0;i<child_num;i++){
        now_btype = btype;
        now_vec[i]->IRDump();
    }
}

void ConstDefAST :: IRDump() const {
    // do nothing, for const has already been computed before
}

std::string btype_transfer(std::string &BTYPE){
    if (BTYPE == "int") 
        return "i32";
    else
        return BTYPE;
}

void VarDefAST :: IRDump() const {
    if (present_tbl() == glbsymbtl){
        // 全局的变量
        if (can_compute) // 这个不管选不选优化都不能关掉，因为全局变量的赋值必须当场解决
            std::cout << "global @" << present_tbl()->ST_name << '_' << ident << " = alloc " << btype_transfer(now_btype) << ", " << val << endl;
        else if (sel == 0)
            std::cout << "global @" << present_tbl()->ST_name << '_' << ident << " = alloc " << btype_transfer(now_btype) << ", zeroinit" << endl;
        else 
            exit(12);
    }
    else{
        //        函数内的变量
        //@x = alloc i32
        std::cout << "    @" << present_tbl()->ST_name << '_' << ident << " = alloc " << btype_transfer(now_btype) << endl;
        // 保证 ident 在当前 symbol table 里
        if (can_compute == MODE)
            // Although Already in SymTb, we should also make sure it's stored
            std::cout << "    store " << val << ", @" << present_tbl()->ST_name << '_' << ident << endl;
        else if (sel){
            initval->IRDump();
            // The same as case in StmtAST, we can make sure this can't be a constant
            // Also finally implemented such a f**k if, for debugging
            if(is_01 >> 1)
                std::cout << "    store " << var_ins[var_num - 1] << ", @" << present_tbl()->ST_name << '_' << ident << endl;
            else
                std::cout << "    store %" << var_num - 1 << ", @" << present_tbl()->ST_name << '_' << ident << endl;
        }
    }
}


// ------------------------------------ Lv 5 Block -------------------

void OptionalExpAST :: IRDump() const {
    if (PreComputeProcedure()) return;
    exp->IRDump();
}

// ------------------------------------ Lv 5 Conditional -------------------

void GLBIfAST :: IRDump() const {
    if (sel == 0)
        ifstmt->IRDump();
    else
        ifelsestmt->IRDump();
}

void IfStmtAST :: IRDump() const {
    if (sel == 1){
        // IF '(' Exp ') IfElseStmt ELSE IfStmt

        // pre_compute tech
        if (can_compute == MODE){
            if (!val)
                ifstmt->IRDump(); // 预计算值为0执行后面的语句
            else
                ifelsestmt->IRDump(); // 否则执行前面的语句
        }
        else{
            If_num ++;
            int IFNUM = If_num;
            
            // First Br
            exp->IRDump();
            UPDATE();
            if (is_01>>1)
                std::cout << "    br " << var_ins[var_num - 1] << ", %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "then" << ", %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "else"  << endl; 
            else
                std::cout << "    br %" << var_num - 1 << ", %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "then" << ", %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "else"  << endl; 

            // Then Then
            std::cout << " %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "then:" << endl;
            ifelsestmt->IRDump();
            UPDATE();
            std::cout << "    jump %" <<  "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "end" << endl; 

            // Then Else
            std::cout << " %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "else:" << endl;
            ifstmt->IRDump();
            UPDATE();
            std::cout << "    jump %" <<  "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "end" << endl; 

            // Finally End
            std::cout << " %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "end:" << endl;
        }    
    }
    else{
        // IF '(' Exp ')' GLBIf
        
        // pre_compute tech
        if (can_compute == MODE){
            if (!val) return; // 预计算值为0，相当于这条语句必不执行，可以直接返回
            glbif->IRDump();
        }
        else{
            If_num ++;
            int IFNUM = If_num;

            // First Br
            exp->IRDump();
            UPDATE();
            if (is_01>>1)
                std::cout << "    br " << var_ins[var_num - 1] << ", %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "then" << ", %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "end"  << endl; 
            else
                std::cout << "    br %" << var_num - 1 << ", %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "then" << ", %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "end"  << endl; 

            // Then Then
            std::cout << " %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "then:" << endl;
            glbif->IRDump();
            UPDATE();
            std::cout << "    jump %" <<  "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "end" << endl; 

            // Finally End
            std::cout << " %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "end:" << endl;
        }
    }
}

void IfElseStmtAST :: IRDump() const {
    if (sel == 1){
        stmt->IRDump();
        return;
    }

    // pre_compute tech
    if (can_compute == MODE){
        if (!val)
            ifelsestmtr->IRDump(); // 预计算值为0执行后面的语句
        else
            ifelsestmtl->IRDump(); // 否则执行前面的语句
    }

    else{
        If_num ++;
        int IFNUM = If_num;

        // First Br
        exp->IRDump();
        UPDATE();
        if (is_01>>1)
            std::cout << "    br " << var_ins[var_num - 1] << ", %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "then" << ", %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "else"  << endl; 
        else
            std::cout << "    br %" << var_num - 1 << ", %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "then" << ", %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "else"  << endl; 

        // Then Then
        std::cout << " %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "then:" << endl;
        ifelsestmtl->IRDump();
        UPDATE();
        std::cout << "    jump %" <<  "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "end" << endl; 

        // Then Else
        std::cout << " %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "else:" << endl;
        ifelsestmtr->IRDump();
        UPDATE();
        std::cout << "    jump %" <<  "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_' << "end" << endl; 

        // Finally End
        std::cout << " %" << "br_" << BLKID << '_' << ID_instr << '_' << IFNUM << '_'  << "end:" << endl;
    }
}

// %0 = load @x
// std::cout << "    %" << var_num << " = load @" << present_tbl()->ST_name << '_' << lval << endl
// store %1, @x
// std::cout << "    store %" << var_num - 1 << ", @" << present_tbl()->ST_name << '_' << lval << endl;
// @x = alloc i32
// std::cout << "    @" << present_tbl()->ST_name << '_' << lval << " = alloc " << btype_transfer(BTYPE) << endl;