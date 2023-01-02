#include <string>
#include <memory>
#include <iostream>
#include "ast.h"

using namespace std;

string sj_map[30] = {
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
  "                                                        "
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
    stmt->Dump(sj + 1);
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}
void StmtAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "StmtAST {" <<endl;  
    HandleSJ(sj+1);
    std::cout << "return" << endl;
    exp->Dump(sj + 1);
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
    else{
        HandleSJ(sj + 1);
        std::cout << "Number = " << number;
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

// ------------------ Dump End ---------------------------------------------


// ---------------------Begin the Part of Generting the IR (koopa)---------------------------------

void CompUnitAST :: IRDump() const{
    func_def->IRDump();
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
    std::cout << " {" << endl;
    // list the basic block here further?
    std::cout << " %" << "entry: " << endl;
    stmt->IRDump();
    std::cout << endl << "}";
}

int var_num, is_01;

void StmtAST :: IRDump() const {
    var_num = 0;
    exp->IRDump();
    std::cout << "    ret %" << var_num - 1;
}

inline void out_IR(int fi, int se, string op){
    std::cout << "    %" << var_num << " = " << op << " %" << fi << ", %" << se << endl;
    var_num ++; // whenever var_num ++, we should check whether it's of 01 format, led by an instr after this func
}

    //---------------- Expression Part--------------------------------
    // Only after executing Logic and Comparing Operating, we can manipulate a value into 0/1

inline void bin201(){
    if(is_01 == 0){
        std::cout << "    %" << var_num << " = ne 0, %" << var_num - 1 << endl;
        var_num ++, is_01 = 1;
    }
}

void ExpAST :: IRDump() const {
    lorexp->IRDump();
}
void LOrExpAST :: IRDump() const {
    if (sel == 0)
        landexp->IRDump();
    else{
        lorexp->IRDump();
        bin201();
        int pre = var_num - 1;
        landexp->IRDump();
        bin201();
        out_IR(pre, var_num - 1, "or");
        is_01 = 1;
    }
}
void LAndExpAST :: IRDump() const {
    if (sel == 0)
        eqexp->IRDump();
    else{
        landexp->IRDump();
        bin201();
        int pre = var_num - 1;
        eqexp->IRDump();
        bin201();
        out_IR(pre, var_num - 1, "and");
        is_01 = 1;
    }
}
void EqExpAST :: IRDump() const {
    if (sel == 0)
        relexp->IRDump();
    else{
        eqexp->IRDump();
        int pre = var_num - 1;
        relexp->IRDump();
        if (rel[0] == '=')
            out_IR(pre, var_num - 1, "eq");
        else
            out_IR(pre, var_num - 1, "ne");
        is_01 = 1;
    }
}
void RelExpAST :: IRDump() const {
    if (sel == 0)
        addexp->IRDump();
    else{
        relexp->IRDump();
        int pre = var_num - 1;
        addexp->IRDump();
        if (rel[0] == '>')
            if (rel.length() == 1)
                out_IR(pre, var_num - 1, "gt");
            else
                out_IR(pre, var_num - 1, "ge");
        else
            if (rel.length() == 1)
                out_IR(pre, var_num - 1, "lt");
            else
                out_IR(pre, var_num - 1, "le");
        is_01 = 1;
    }
}
void AddExpAST :: IRDump() const {
    if (sel == 0)
        mulexp->IRDump();
    else{
        addexp->IRDump();
        int pre = var_num - 1;
        mulexp->IRDump();
        if(opt[0] == '+')
            out_IR(pre, var_num - 1, "add");
        else
            out_IR(pre, var_num - 1, "sub");
        is_01 = 0;
    }
}
void MulExpAST :: IRDump() const {
    if (sel == 0)
        unaryexp->IRDump();
    else{
        mulexp->IRDump();
        int pre = var_num - 1;
        unaryexp->IRDump();
        if(opt[0] == '*')
            out_IR(pre, var_num - 1, "mul");
        else if (opt[0] == '/')
            out_IR(pre, var_num - 1, "div");
        else
            out_IR(pre, var_num - 1, "mod");
        is_01 = 0;
    }
}
void UnaryExpAST :: IRDump() const {
    if (sel == 0)
        pexp->IRDump();
    else{
        unaryexp->IRDump();
        if(opt[0] == '-'){
            std::cout << "    %" << var_num << " = sub 0, %" << var_num - 1 << endl;
            var_num ++, is_01 = 0;
        }
        else if (opt[0] == '!'){
            std::cout << "    %" << var_num << " = eq 0, %" << var_num - 1 << endl;
            var_num ++, is_01 = 1;
        }
    }
}
void PrimaryExpAST :: IRDump() const {
    if (sel == 0)
        exp->IRDump();
    else{
        // std::cout << "    %" << var_num << " = " << number << endl;
        std::cout << "    %" << var_num << " = or 0, " << number << endl;
        var_num ++, is_01 = 0;
    }
}
