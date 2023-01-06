#include <string>
#include <memory>
#include <iostream>
#include "ast.h"

using namespace std;

int total_blk_num = 0;

inline void BaseAST :: PreComputeAssign(std::unique_ptr<BaseAST> &child){
    if (child->can_compute == 1){
        can_compute = 1;
        val = child->val;
    }
    else{
        can_compute = val = 0;
    }
}

void CompUnitAST :: PreCompute(){
    glbsymbtl->ST_name = "GLOBAL_Table";
    BaseAST::glbsymbtl->reach_st.push(1);
    push_into_tbl_stk(glbsymbtl, 0); // 全局变量进入
    
    func_def->PreCompute();

    pop_tbl_stk(); // 全局变量表退出
    BaseAST::glbsymbtl->reach_st.pop();
}

void FuncDefAST :: PreCompute(){
    func_type->PreCompute();
    block->PreCompute();
}

void FuncTypeAST :: PreCompute(){

}

void BlockAST :: PreCompute(){
    total_blk_num ++;
    block_id = total_blk_num;
    symbtl = new SymbolTable();
    symbtl->ST_name = "block_" + to_string(total_blk_num);
    symbtl->reach_st.push(1); // 每一块的入口，我们初始化为1，表示必然可达
    push_into_tbl_stk(symbtl, 1); // 这个 Block 的符号表进入

    std::vector< std::unique_ptr<BaseAST> > &now_vec = *blockitem; 
    for (int i=0;i<child_num;i++)
        now_vec[i]->PreCompute();
    
    pop_tbl_stk(); // 这个 Block 的符号表退出
    symbtl->reach_st.pop();
}  

void GLBIfAST :: PreCompute(){
    if (sel == 0)
        ifstmt->PreCompute();
    else 
        ifelsestmt->PreCompute();
}

void IfStmtAST :: PreCompute(){
    exp->PreCompute();
    // Pre-Compute
    PreComputeAssign(exp);

    if (sel == 0){
        present_tbl()->Push(!!exp->can_compute, !!exp->val);
        glbif->PreCompute();
        present_tbl()->Pop();
    }
    else{
        present_tbl()->Push(!!exp->can_compute, !!exp->val);
        ifelsestmt->PreCompute();
        present_tbl()->Pop();
        present_tbl()->Push(!!exp->can_compute, !exp->val);
        ifstmt->PreCompute();
        present_tbl()->Pop();
    }
}

void IfElseStmtAST :: PreCompute(){
    if (sel == 1){
        stmt->PreCompute();
        can_compute = 0; // meaningless here
        val = 666; // for fun
        return;
    }

    exp->PreCompute();
    // Pre-Compute
    PreComputeAssign(exp);

    present_tbl()->Push(!!exp->can_compute, !!exp->val);
    ifelsestmtl->PreCompute();
    present_tbl()->Pop();
    present_tbl()->Push(!!exp->can_compute, !exp->val);
    ifelsestmtr->PreCompute();
    present_tbl()->Pop();
}

void StmtAST :: PreCompute(){
    if (sel == 3){
        optionalexp->PreCompute();
        PreComputeAssign(optionalexp);
    }
    else if (sel == 1){
        // 不用递归，因为子树的结果我们完全用不到
        can_compute = 0; // useless
    }
    else if (sel == 2){
        block->PreCompute();
        can_compute = 0; // useless
    }
    else{
        exp->PreCompute();

        auto pres_symbtl = present_tbl();
        // Check whether something wrong?
        auto ret = pres_symbtl->GetItemByName(lval);
        lval_belong = pres_symbtl->present;

        if (ret == NULL)
            exit(4);
        if (!(ret->VarType() & 1))
            exit(6);

        int LOCAL = 1; // 计算，从这个Lval的定义到目前的可达状态
        for (auto now = pres_symbtl; now; now = now->father){
            LOCAL = NewState(now->reach_st.top(), LOCAL); // 注意先后顺序，是从前面的块到后面的块
            if (now == lval_belong)
                break;
        }

        // Changing the Mode of a Var, into unknown state
        // See comments in the top of `ast.h` for details
        if (exp->can_compute == 0 || LOCAL == 2){
            can_compute = 0;
            ret->BecomeUnknown();
            // waiting computing it on the stack
        }
        // Conditions below has 'exp' 's attribute `can_compute = 1`
        else{
            if (LOCAL){
                can_compute = 1;
                val = exp->val;
                ret->SetVal(val);
            }
            else{
                // We just do nothing,
                // First reason is that this AST element can't be accessed when generating IR,
                // So it doesn't make sense whether you do anything here.
                // Second is that, no consequence can be pushed onto the Symbol Table
            }
        }
    }
}

void ExpAST :: PreCompute(){
    lorexp->PreCompute();
    PreComputeAssign(lorexp);
}

void LOrExpAST :: PreCompute(){
    if (sel == 0){
        landexp->PreCompute();
        PreComputeAssign(landexp);
    }
    else{
        lorexp->PreCompute();
        landexp->PreCompute();

        can_compute = lorexp->can_compute && landexp->can_compute;
        if (can_compute)
            val = lorexp->val || landexp->val;
        else if (lorexp->can_compute && lorexp->val)
            can_compute = val = 1;
        else if (landexp->can_compute && landexp->val)
            can_compute = val = 1;
    }
}

void LAndExpAST :: PreCompute(){
    if (sel == 0){
        eqexp->PreCompute();
        PreComputeAssign(eqexp);
    }
    else{
        landexp->PreCompute();
        eqexp->PreCompute();

        can_compute = landexp->can_compute && eqexp->can_compute;
        if (can_compute)
            val = landexp->val && eqexp->val;
        else if (landexp->can_compute && !landexp->val)
            can_compute = 1, val = 0;
        else if (eqexp->can_compute && !eqexp->val)
            can_compute = 1, val = 0;
    }
}

void EqExpAST :: PreCompute(){
    if (sel == 0){
        relexp->PreCompute();
        PreComputeAssign(relexp);
    }
    else{
        eqexp->PreCompute();
        relexp->PreCompute();

        can_compute = eqexp->can_compute && relexp->can_compute;
        if (can_compute){
            if (rel[0] == '=')
                val = eqexp->val == relexp->val;
            else 
                val = eqexp->val != relexp->val;
        }
    }
}

void RelExpAST :: PreCompute(){
    if (sel == 0){
        addexp->PreCompute();
        PreComputeAssign(addexp);
    }
    else{
        relexp->PreCompute();
        addexp->PreCompute();

        can_compute = relexp->can_compute && addexp->can_compute;
        if (can_compute){
            if (rel[0] == '>'){
                if (rel.length() == 1)
                    val = relexp->val > addexp->val;
                else 
                    val = relexp->val >= addexp->val;
            }
            else{
                if (rel.length() == 1)
                    val = relexp->val < addexp->val;
                else 
                    val = relexp->val <= addexp->val;
            }
        }
    }
}

void AddExpAST :: PreCompute(){
    if (sel == 0){
        mulexp->PreCompute();
        PreComputeAssign(mulexp);
    }
    else{
        addexp->PreCompute();
        mulexp->PreCompute();

        can_compute = addexp->can_compute && mulexp->can_compute;
        if (can_compute){
            if (opt[0] == '+')
                val = addexp->val + mulexp->val;
            else if (opt[0] == '-')
                val = addexp->val - mulexp->val;
            else
                exit(2);
        }
    }
}

void MulExpAST :: PreCompute(){
    if (sel == 0){
        unaryexp->PreCompute();
        PreComputeAssign(unaryexp);
    }
    else{
        mulexp->PreCompute();
        unaryexp->PreCompute();

        can_compute = mulexp->can_compute && unaryexp->can_compute;
        if (can_compute){
            if (opt[0] == '*')
                val = mulexp->val * unaryexp->val;
            else if (opt[0] == '/')
                val = mulexp->val / unaryexp->val;
            else 
                val = mulexp->val % unaryexp->val;
        }
    }
}

void UnaryExpAST :: PreCompute(){
    if (sel == 0){
        pexp->PreCompute();
        PreComputeAssign(pexp);
    }
    else{
        unaryexp->PreCompute();

        can_compute = unaryexp->can_compute;
        if (can_compute){
            if (opt[0] == '-')
                val = -unaryexp->val;
            else if (opt[0] == '!')
                val = !unaryexp->val;
            else 
                val = unaryexp->val;
        }
    }
}

void PrimaryExpAST :: PreCompute(){
    if (sel == 0){
        exp->PreCompute();
        PreComputeAssign(exp);
    }
    else if (sel == 1){
        can_compute = 1;
        val = number;
    }
    else {
        // CAN'T PRECOMPUTE, if and only if its state now is known, not matter because
            // unknown value or unknown logic result
        auto pres_symbtl = present_tbl();
        auto ret = pres_symbtl->GetItemByName(lval);
        lval_belong = pres_symbtl->present;
        if (ret == NULL)
            exit(4);
        
        can_compute = ret->VarType() >> 1;
        val = ret->VarVal();
    }
}

void InitValAST :: PreCompute(){
    exp->PreCompute();
    PreComputeAssign(exp);
}

void ConstExpAST :: PreCompute(){
    exp->PreCompute();
    PreComputeAssign(exp);
}

void ConstInitValAST :: PreCompute(){
    constexp->PreCompute();
    PreComputeAssign(constexp);
}

void BlockItemAST :: PreCompute(){
    if (sel == 0)
        decl->PreCompute();
    else 
        glbif->PreCompute();
}

void DeclAST :: PreCompute(){
    if (sel == 0)
        constdecl->PreCompute();
    else 
        vardecl->PreCompute();  
}

void VarDefAST :: PreCompute(){
    if (sel == 0){
        // Insert a non-valued item into present symbal table
        auto pres_symbtl = present_tbl();
        auto new_symtbl_item = new SymbolTableItem(1);
        pres_symbtl->Insert(ident, *new_symtbl_item);
        can_compute = 0; // can't be decided in compiling time
    }
    else{
        initval->PreCompute();

        auto pres_symbtl = present_tbl();
        // Check Whether Already Computed
        if (initval->can_compute == 0){
            // Insert a non-valued item into present symbal table
            auto new_symtbl_item = new SymbolTableItem(1);
            pres_symbtl->Insert(ident, *new_symtbl_item);
            // Wait for putting its computing procedure onto the stack?
            can_compute = 0;
        }
        else{
        // Insert an already-valued item into present symbal table
            val = initval->val;
            auto new_symtbl_item = new SymbolTableItem(1, val);
            pres_symbtl->Insert(ident, *new_symtbl_item);
            can_compute = 1;
        }
    }
}

void VarDeclAST :: PreCompute(){
    std::vector< std::unique_ptr<BaseAST> > &now_vec = *vardef; 
    for (int i=0;i<child_num;i++)
        now_vec[i]->PreCompute();
}

void ConstDefAST :: PreCompute(){
    constinitval->PreCompute();

    if (constinitval->can_compute == 0)
      exit(5);
    // Because ConstDef must be computed in the compiling time, 
      // so we don't use can_compute, it must be 1

    // Insert a const item into present symbal table
    auto pres_symbtl = present_tbl();
    auto new_symtbl_item = new SymbolTableItem(0, constinitval->val);
    pres_symbtl->Insert(ident, *new_symtbl_item);
}

void ConstDeclAST :: PreCompute(){
    std::vector< std::unique_ptr<BaseAST> > &now_vec = *constdef; 
    for (int i=0;i<child_num;i++)
        now_vec[i]->PreCompute();
}

void OptionalExpAST :: PreCompute(){
    if (exp != NULL){
        exp->PreCompute();
        PreComputeAssign(exp);
    }
    else{
        can_compute = 1;
        val = 0;
        // Cheat Code
    }
}

