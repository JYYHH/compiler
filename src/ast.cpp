#include <string>
#include <memory>
#include <iostream>
#include "ast.h"
// #define var_num present_tbl()->var_num
// #define is_01 present_tbl()->is_01
// #define var_ins present_tbl()->var_ins

using namespace std;

string now_btype;
int BLKID, ID_instr;

SymbolTable* present_tbl(){
    return (*(BaseAST::glbstst)).top();
}   

void push_into_tbl_stk(SymbolTable* item){
    item->father = present_tbl();
    (*(BaseAST::glbstst)).push(item);
}

void pop_tbl_stk(){
    (*(BaseAST::glbstst)).pop();
}

string sj_map[40] = {
  "",
  "  ",
  "    ",
  "      ",
  "        ",
  "          ",
  "            ",
  "              ",
  "                ",
  "                  ",
  "                    ",
  "                      ",
  "                        ",
  "                          ",
  "                            ",
  "                              ",
  "                                ",
  "                                  ",
  "                                    ",
  "                                      ",
  "                                        ",
  "                                          ",
  "                                            ",
  "                                              ",
  "                                                ",
  "                                                  ",
  "                                                    ",
  "                                                      ",
  "                                                        ",
  "                                                          ",
  "                                                            ",
  "                                                              ",
  "                                                                ",
  "                                                                  ",
  "                                                                    ",
  "                                                                      ",
  "                                                                        ",
  "                                                                          " 
};

// in the beginning of each line
inline void BaseAST :: HandleSJ(int sj) const{
    std::cout << sj_map[sj];
}

// ------------------ Dump Begin ---------------------------------------------

void CompUnitAST :: Dump(int sj) const{
    HandleSJ(sj);
    std::cout << "CompUnitAST {" << endl;
    func_def->Dump(sj+1);
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}
void FuncDefAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "FuncDefAST {" << endl;
    func_type->Dump(sj+1);
    std::cout << ',' << endl;
    HandleSJ(sj+1);
    std::cout << "FuncName = " << ident << ',' << endl;
    block->Dump(sj+1);
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}
void FuncTypeAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "FuncTypeAST { ";
    std::cout << type;
    std::cout << " }";
}
void BlockAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "BlockAST {" << endl;  
    std::vector< std::unique_ptr<BaseAST> > &now_vec = *blockitem; 
    HandleSJ(sj+1);
    std::cout << "ChildNum = " << child_num << endl;
    for (int i=0; i<child_num; i++){
        now_vec[i]->Dump(sj + 1);
        std::cout << "********** End of Child " << i <<endl;
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}
void StmtAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "StmtAST {" <<endl;
    if (sel == 1){
        HandleSJ(sj+1);
        std::cout << "return" << endl;
        exp->Dump(sj + 1);
    }
    else{
        HandleSJ(sj+1);
        std::cout << lval << endl;
        HandleSJ(sj+1);
        std::cout << '=' << endl;
        exp->Dump(sj + 1);        
    }  
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

void ExpAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "ExpAST {" <<endl; 
    lorexp->Dump(sj + 1);
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

void LOrExpAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "LOrExpAST {" <<endl; 
    if (sel == 0){
        landexp->Dump(sj + 1);
    }
    else{
        lorexp->Dump(sj + 1);
        std::cout << endl;
        HandleSJ(sj + 1);
        std::cout << "||" << endl;
        landexp->Dump(sj + 1);
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

void LAndExpAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "LAndExpAST {" <<endl; 
    if (sel == 0){
        eqexp->Dump(sj + 1);
    }
    else{
        landexp->Dump(sj + 1);
        std::cout << endl;
        HandleSJ(sj + 1);
        std::cout << "&&" << endl;
        eqexp->Dump(sj + 1);
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

void EqExpAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "EqExpAST {" <<endl; 
    if (sel == 0){
        relexp->Dump(sj + 1);
    }
    else{
        eqexp->Dump(sj + 1);
        std::cout << endl;
        HandleSJ(sj + 1);
        std::cout << rel << endl;
        relexp->Dump(sj + 1);
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

void RelExpAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "RelExpAST {" <<endl; 
    if (sel == 0){
        addexp->Dump(sj + 1);
    }
    else{
        relexp->Dump(sj + 1);
        std::cout << endl;
        HandleSJ(sj + 1);
        std::cout << rel << endl;
        addexp->Dump(sj + 1);
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

void AddExpAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "AddExpAST {" <<endl; 
    if (sel == 0){
        mulexp->Dump(sj + 1);
    }
    else{
        addexp->Dump(sj + 1);
        std::cout << endl;
        HandleSJ(sj + 1);
        std::cout << opt << endl;
        mulexp->Dump(sj + 1);
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

void MulExpAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "MulExpAST {" <<endl; 
    if (sel == 0){
        unaryexp->Dump(sj + 1);
    }
    else{
        mulexp->Dump(sj + 1);
        std::cout << endl;
        HandleSJ(sj + 1);
        std::cout << opt << endl;
        unaryexp->Dump(sj + 1);
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

void UnaryExpAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "UnaryExpAST {" <<endl; 
    if (sel == 0){
        pexp->Dump(sj + 1);
    }
    else{
        HandleSJ(sj + 1);
        std::cout << opt;
        std::cout << ',' << endl;
        unaryexp->Dump(sj + 1);
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

// void UnaryOpAST :: Dump(int sj) const {
//     HandleSJ(sj);
//     std::cout << opt;
// }

void PrimaryExpAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "PrimaryExpAST {" <<endl; 
    if (sel == 0){
        exp->Dump(sj + 1);
    }
    else if (sel == 1){
        HandleSJ(sj + 1);
        std::cout << "Number = " << number;
    }
    else{
        HandleSJ(sj + 1);
        std::cout << "VarName = " << lval;
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

void DeclAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "DeclAST {" <<endl; 

    if (sel == 0)
        constdecl->Dump(sj + 1);
    else
        vardecl->Dump(sj + 1);

    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

void BlockItemAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "BlockItemAST {" <<endl; 

    if (sel == 0)
        decl->Dump(sj + 1);
    else
        stmt->Dump(sj + 1);

    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
} 

void InitValAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "InitValAST {" <<endl; 

    exp->Dump(sj + 1);

    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
} 

void ConstExpAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "ConstExpAST {" <<endl; 

    exp->Dump(sj + 1);

    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
} 

void ConstInitValAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "ConstInitValAST {" <<endl; 

    constexp->Dump(sj + 1);

    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
} 

void ConstDefAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "ConstDefAST {" <<endl; 
    
    HandleSJ(sj + 1);
    std::cout << "(IDENT)" << ident << '=' << endl;
    constinitval->Dump(sj + 1);

    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
} 

void ConstDeclAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "ConstDeclAST {" << endl;  
    HandleSJ(sj + 1);
    std::cout << "const" << endl;
    HandleSJ(sj + 1);
    std::cout << btype << endl;
    std::cout << endl;

    std::vector< std::unique_ptr<BaseAST> > &now_vec = *constdef; 
    HandleSJ(sj+1);
    std::cout << "ChildNum = " << child_num << endl;
    for (int i=0; i<child_num; i++){
        now_vec[i]->Dump(sj + 1);
        std::cout << "********** End of Child " << i << endl;
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

void VarDefAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "VarDefAST {" <<endl; 
    
    if (sel == 1){
        HandleSJ(sj + 1);
        std::cout << "(IDENT)" << ident << '=' << endl;
        initval->Dump(sj + 1);
    }
    else{
        HandleSJ(sj + 1);
        std::cout << "(IDENT)" << ident;
    }

    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
} 

void VarDeclAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "VarDeclAST {" << endl;  
    HandleSJ(sj + 1);
    std::cout << btype << endl;
    std::cout << endl;

    std::vector< std::unique_ptr<BaseAST> > &now_vec = *vardef; 
    HandleSJ(sj+1);
    std::cout << "ChildNum = " << child_num << endl;
    for (int i=0; i<child_num; i++){
        now_vec[i]->Dump(sj + 1);
        std::cout << "********** End of Child " << i <<endl;
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}


// ------------------ Dump End ---------------------------------------------


// ---------------------Begin the Part of Generting the IR (koopa)---------------------------------

void CompUnitAST :: IRDump() const{
    push_into_tbl_stk(glbsymbtl);
    func_def->IRDump();
    pop_tbl_stk();
}
void FuncDefAST :: IRDump() const {
    std::cout << "fun @";
    std::cout << ident << "(): ";
    func_type->IRDump();
    block->IRDump();
}
void FuncTypeAST :: IRDump() const {
    if (type == "int")
        std::cout << "i32";
}
void BlockAST :: IRDump() const {
    push_into_tbl_stk(symbtl);

    std::cout << " {" << endl;
    // list the basic block here further?
    std::vector< std::unique_ptr<BaseAST> > &now_vec = *blockitem; 
    std::cout << " %" << "Block " << block_id << ": " << endl;
    for (int i=0;i<child_num;i++){
        BLKID = block_id;
        ID_instr = i;
        now_vec[i]->IRDump();
    }
    std::cout << "}" << endl;

    pop_tbl_stk();
}

void BlockItemAST :: IRDump() const {
    std::cout << " %" << "Instr_" << ID_instr << "FromBlock_" << BLKID << ": " << endl;
    if (sel == 0)
        decl->IRDump();
    else 
        stmt->IRDump();
}


int var_num, is_01; // is_01 = 0 -> binary ; 1 -> 0/1 ; 2 -> const
int var_ins[100005]; // an optimization on saved IR register --- save constant before

// we save space for useless `%xx` items in IR-code, since they're all constant 

void StmtAST :: IRDump() const {
    // var_num = 0;
    if (sel == 1){
        if (can_compute)
            std::cout << "    ret " << val << endl;
        else{
            exp->IRDump();
            if(is_01 >> 1)
                std::cout << "    ret " << var_ins[var_num - 1] << endl;
            else
                std::cout << "    ret %" << var_num - 1 << endl;
        }
    }
    else{
        if (can_compute)
            return;
        exp->IRDump();
        // store %1, @x
        std::cout << "    store %" << var_num - 1 << ", @" << present_tbl()->ST_name << '_' << lval << endl;
        // And you can consider why there's not other condition?
        // BBBBBecause, all the tree nodes' 'can_compute' are already determined, after `sysy.y`
            // scans the source code.
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

inline void alr_compute_procedure(int NUMb){
    var_ins[var_num ++] = NUMb, is_01 = 2;
}

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
    if (can_compute){
        alr_compute_procedure(val);
        return;
    }
    
    lorexp->IRDump();
}
void LOrExpAST :: IRDump() const {
    if (can_compute){
        alr_compute_procedure(val);
        return;
    }

    if (sel == 0)
        landexp->IRDump();
    else{
        lorexp->IRDump();
        bin201();
        int pre = var_num - 1;
        landexp->IRDump();
        bin201();
        out_binary_IR(pre, var_num - 1, "or");
        is_01 = 1;
    }
}
void LAndExpAST :: IRDump() const {
    if (can_compute){
        alr_compute_procedure(val);
        return;
    }

    if (sel == 0)
        eqexp->IRDump();
    else{
        landexp->IRDump();
        bin201();
        int pre = var_num - 1;
        eqexp->IRDump();
        bin201();
        out_binary_IR(pre, var_num - 1, "and");
        is_01 = 1;
    }
}
void EqExpAST :: IRDump() const {
    if (can_compute){
        alr_compute_procedure(val);
        return;
    }

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
    if (can_compute){
        alr_compute_procedure(val);
        return;
    }

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
    if (can_compute){
        alr_compute_procedure(val);
        return;
    }

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
    if (can_compute){
        alr_compute_procedure(val);
        return;
    }

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
    if (can_compute){
        alr_compute_procedure(val);
        return;
    }

    if (sel == 0)
        pexp->IRDump();
    else{
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
}
void PrimaryExpAST :: IRDump() const {
    if (can_compute){
        alr_compute_procedure(val);
        return;
    }

    if (sel == 0)
        exp->IRDump();
    else if (sel == 2){
        // %0 = load @x
        std::cout << "    %" << var_num << " = load @" << present_tbl()->ST_name << '_' << lval << endl;
        var_num ++, is_01 = 0;
    }
}


// --------------------------------- Lv_4  Const and Var-------------------------------------

void InitValAST :: IRDump() const {
    if (can_compute){
        alr_compute_procedure(val);
        return;
    }
    
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
        return "i32";
}

void VarDefAST :: IRDump() const {
    //@x = alloc i32
    std::cout << "    @" << present_tbl()->ST_name << '_' << ident << " = alloc " << btype_transfer(now_btype) << endl;
    
    if (can_compute)
        std::cout << "    store " << val << ", @" << present_tbl()->ST_name << '_' << ident << endl;
    else if (sel){
        initval->IRDump();
        // The same as case in StmtAST, we can make sure this can't be a constant
        std::cout << "    store %" << var_num - 1 << ", @" << present_tbl()->ST_name << '_' << ident << endl;
    }
}



// %0 = load @x
// std::cout << "    %" << var_num << " = load @" << present_tbl()->ST_name << '_' << lval << endl
// store %1, @x
// std::cout << "    store %" << var_num - 1 << ", @" << present_tbl()->ST_name << '_' << lval << endl;
// @x = alloc i32
// std::cout << "    @" << present_tbl()->ST_name << '_' << lval << " = alloc " << btype_transfer(BTYPE) << endl;