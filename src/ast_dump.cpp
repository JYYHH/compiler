#include <string>
#include <memory>
#include <iostream>
#include "ast.h"

using namespace std;

string sj_map[305];


inline void BaseAST :: HandleSJ(int sj) const{
    std::cout << sj_map[sj];
}

// ------------------ Dump Begin ---------------------------------------------

void CompUnitAST :: Dump(int sj) const{
    sj_map[0] = "";
    for(int i=1;i<=300;i++) sj_map[i] = sj_map[i-1] + "  ";

    HandleSJ(sj);
    std::cout << "CompUnitAST {" << endl;  
    std::vector< std::unique_ptr<BaseAST> > &now_vec = *func_def; 
    HandleSJ(sj+1);
    std::cout << "TotolComponentNum = " << child_num << endl;
    for (int i=0; i<child_num; i++){
        now_vec[i]->Dump(sj + 1);
        std::cout << "********** End of Child " << i <<endl;
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}
void FuncDefAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "FuncDefAST {" << endl;
    // func_type->Dump(sj+1);
    HandleSJ(sj + 1);
    std::cout << func_type;
    std::cout << ',' << endl;
    HandleSJ(sj+1);
    std::cout << "FuncName = " << ident << ',' << endl;
    std::vector< std::unique_ptr<BaseAST> > &now_vec = *funcfparam; 
    HandleSJ(sj+1);
    std::cout << "FuncParamNum = " << child_num << endl;
    for (int i=0; i<child_num; i++){
        now_vec[i]->Dump(sj + 1);
        std::cout << "********** End of Child " << i <<endl;
    }
    std::cout << endl;
    block->Dump(sj+1);
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}
void FuncFParamAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "FuncFParamAST { ";
    std::cout << "Type: " << btype << ", Name: " << ident;
    std::cout << " }";
}
void BlockAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "BlockAST {" << endl;  
    std::vector< std::unique_ptr<BaseAST> > &now_vec = *blockitem; 
    HandleSJ(sj+1);
    std::cout << "BlockNum = " << child_num << endl;
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
    if (sel == 3){
        HandleSJ(sj+1);
        std::cout << "return" << endl;
        if (optionalexp != NULL)
            optionalexp->Dump(sj + 1);
    }
    else if (sel == 2){
        block->Dump(sj + 1);
    }
    else if (sel == 1){
        if (optionalexp != NULL)
            optionalexp->Dump(sj + 1);
    }
    else if (!sel){
        HandleSJ(sj+1);
        std::cout << lval << endl;
        HandleSJ(sj+1);
        std::cout << '=' << endl;
        exp->Dump(sj + 1);        
    }
    else if (sel == 4){
        exp->Dump(sj + 1);
        glbif->Dump(sj + 1);
    }
    else if (sel == 5){
        HandleSJ(sj+1);
        std::cout << "CONTINUE" << endl;
    }
    else{
        HandleSJ(sj+1);
        std::cout << "BREAK" << endl;
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
    else if (sel == 1){
        HandleSJ(sj + 1);
        std::cout << opt;
        std::cout << ',' << endl;
        unaryexp->Dump(sj + 1);
    }
    else{
        HandleSJ(sj + 1);
        std::cout << "FuncName = " << ident << std::endl;

        std::vector< std::unique_ptr<BaseAST> > &now_vec = *funcrparam; 
        HandleSJ(sj+1);
        std::cout << "Para_Call = " << child_num << endl;
        for (int i=0; i<child_num; i++){
            now_vec[i]->Dump(sj + 1);
            std::cout << "********** End of Child " << i <<endl;
        }
        std::cout << endl;
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
        glbif->Dump(sj + 1);

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
    std::cout << "ConstDeclNum = " << child_num << endl;
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
    std::cout << "VarDeclNum = " << child_num << endl;
    for (int i=0; i<child_num; i++){
        now_vec[i]->Dump(sj + 1);
        std::cout << "********** End of Child " << i <<endl;
    }
    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

void OptionalExpAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "OptionalExpAST {" <<endl; 

    if (exp != NULL)
        exp->Dump(sj + 1);

    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}";
}

void GLBIfAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "GLBIfAST {" <<endl;

    if (sel == 0)
        ifstmt->Dump(sj + 1);
    else 
        ifelsestmt->Dump(sj + 1);

    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}"; 
}

void IfStmtAST :: Dump(int sj) const{
    HandleSJ(sj);
    std::cout << "IfStmtAST {" <<endl;
    
    if (sel == 0){
        exp->Dump(sj + 1);
        std::cout << ',' << std::endl;
        glbif->Dump(sj + 1);
    }
    else{
        exp->Dump(sj + 1);
        std::cout << ',' << std::endl;
        ifelsestmt->Dump(sj + 1);
        std::cout << ',' << std::endl;
        ifstmt->Dump(sj + 1);
    }

    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}"; 
}

void IfElseStmtAST :: Dump(int sj) const {
    HandleSJ(sj);
    std::cout << "IfElseStmtAST {" <<endl; 
    
    if (sel == 0){
        exp->Dump(sj + 1);
        std::cout << ',' << std::endl;
        ifelsestmtl->Dump(sj + 1);
        std::cout << ',' << std::endl;
        ifelsestmtr->Dump(sj + 1);
    }
    else{
        stmt->Dump(sj + 1);
    }

    std::cout << endl;
    HandleSJ(sj);
    std::cout << "}"; 
}


// ------------------ Dump End ---------------------------------------------
