/*
    待实现优化：
        1. while 内的优化 (maybe no solution)
        2. 全局变量的优化 (maybe no solution)
        3. 函数参数调用的优化 (maybe no solution)
        4. 赋值 和 引用语句 的, LCA的优化

    可能存在的隐患：
        1. 不确定如果在函数内部对 Param 进行修改会有什么后效性 (虽然确实可以这样做)

*/

#include <string>
#include <memory>
#include <iostream>
#include "ast.h"

using namespace std;

int total_blk_num = 0;
int bin_precompute_for_var = 0; // 用来在 while(exp) 中关闭 exp 对于变量Var 依赖的提前计算功能
// 用来预处理函数的第一个 Block 开头需要的信息
string now_func_name;
string funcparam_name; // 全局的temp变量，用来维护扫描到的 Param 的名字
std::vector< std::unique_ptr<BaseAST> > *glb_funcfparam;
SymbolTableItem* const_need_valued = NULL;

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
    BaseAST::glbsymbtl->ST_name = "GLOBAL_Table";
    BaseAST::glbsymbtl->reach_st.push(1); // 全局 Block, 我们也假设必然可达
    push_into_tbl_stk(glbsymbtl, 0); // 全局变量进入
    
    std::vector< std::unique_ptr<BaseAST> > &now_vec = *func_def; 
    for (int i=0; i<child_num; i++)
        now_vec[i]->PreCompute();

    pop_tbl_stk(); // 全局变量表退出
    BaseAST::glbsymbtl->reach_st.pop();
}

void FuncDefAST :: PreCompute(){
    now_func_name = ident;
    glb_funcfparam = funcfparam;
    block->PreCompute();

    // check whether a function is leaf function
    // 16 -> non-leaf
    // 0 -> leaf

    // auto ret = BaseAST::glbsymbtl->GetItemByName(ident);
    // cout << ident << " = " << (ret->VarType() & 16) << endl;
}

void FuncFParamAST :: PreCompute(){
    funcparam_name = ident;
    if (sel){
        for(int i=0;i<child_num;i++)
            (*arrconst)[i]->PreCompute();
    }
}

void BlockAST :: PreCompute(){
    total_blk_num ++;
    block_id = total_blk_num;
    symbtl = new SymbolTable();
    symbtl->ST_name = "block_" + to_string(total_blk_num);
    symbtl->reach_st.push(1); // 每一块的入口，我们初始化为1，表示必然可达
    push_into_tbl_stk(symbtl, 1); // 这个 Block 的符号表进入

    if (glb_funcfparam != NULL){
        std::vector< std::unique_ptr<BaseAST> > &now_vec = *glb_funcfparam; 
        int BOUND = now_vec.size();
        for (int i=0; i<BOUND; i++){
            now_vec[i]->PreCompute();
            auto FUNCPARAM = new SymbolTableItem((1 << 6) | 1);
            // cout << i << ' ' << FUNCPARAM->VarType() << endl;
            FUNCPARAM->dimension = ((FuncFParamAST *)(now_vec[i].get()))->child_num + 1; 
            // 注意上面是由于我们数组参数是省略的第一维
            present_tbl()->Insert(funcparam_name, *FUNCPARAM);
        }

        glb_funcfparam = NULL;
    }

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
    if (sel == 3 || sel == 1){
        if (optionalexp->can_compute != 99){ // == 99 -> NULL
            optionalexp->PreCompute();
            PreComputeAssign(optionalexp);
        }
        else can_compute = 1;
    }
    else if (sel == 2){
        block->PreCompute();
        can_compute = 0; // useless
    }
    else if (sel == 0){
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
    else if (sel == 4){ 
        // we meet a while, but can do nothing for it 
        present_tbl()->Push(0, 0); 
        // must push element 2 (unknown times) into the stack, even before the exp
        bin_precompute_for_var ++;
        exp->PreCompute();
        glbif->PreCompute();
        bin_precompute_for_var --;
        present_tbl()->Pop();
    }
    else if (sel == 5){
        // just do nothing
    }
    else if (sel == 6){
        // just do nothing
    }
    else if (sel == 7){
        exp->PreCompute();

        auto pres_symbtl = present_tbl();
        auto ret = pres_symbtl->GetItemByName(lval);
        lval_belong = pres_symbtl->present;

        if (ret == NULL)
            exit(4);
        if (!(ret->VarType() & 1))
            exit(6);

        std::vector< std::unique_ptr<BaseAST> > &now_vec = *lval_ref; 
        for(int i=0;i<child_num;i++) 
            now_vec[i]->PreCompute();
        can_compute = 0;
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
        // 模拟生成 IR 时的短路：(见 ast.h )
        else if (lorexp->can_compute && lorexp->val)
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
    else if (sel == 1){
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
    else{
        std::vector< std::unique_ptr<BaseAST> > &now_vec = *funcrparam; 
        for (int i=0;i<child_num;i++)
            now_vec[i]->PreCompute();
        
        auto ret = BaseAST::glbsymbtl->GetItemByName(now_func_name);
        if (ret == NULL)
            exit(13);

        ret->BecomeUnLeaf(); // 由于调用了其他函数，所以当前的函数需要变成非叶子节点

        can_compute = 0;
        val = 1 << 31;
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
    else if (sel == 2){
        // CAN'T PRECOMPUTE, if and only if its state now is known, not matter because
            // unknown value or unknown logic result
        auto pres_symbtl = present_tbl();
        auto ret = pres_symbtl->GetItemByName(lval);
        lval_belong = pres_symbtl->present;
        if (ret == NULL)
            exit(4);

        if (ret->VarType() & 128){ // 遇到了把数组做参数传进去的，一律返回
            can_compute = 0;
            return;
        }
        
        can_compute = (ret->VarType() >> 1) & (1 ^ ((bin_precompute_for_var != 0) & ret->VarType())) & (lval_belong != BaseAST::glbsymbtl);
        // 如果我们暂时关了 precompute功能 (主要可能是因为While循环) ，且要用的是 Var，那么这个 PrimaryExp 也是不能优化的
        // 同时 全局变量 (在最后一个判断里) && 函数参数(VarType = (1 << 6) | 1) 也不能优化
        if (!(ret->VarType() & 1))
            can_compute = 1;
        // 但是全局常量还是能用的
        val = ret->VarVal();
    }
    else{
        can_compute = 0;
        // array
        auto pres_symbtl = present_tbl();
        auto ret = pres_symbtl->GetItemByName(lval);
        lval_belong = pres_symbtl->present;
        if (ret == NULL)
            exit(4);

        std::vector< std::unique_ptr<BaseAST> > &now_vec = *lval_ref; 
        for(int i=0;i<child_num;i++) 
            now_vec[i]->PreCompute();
    }
}

void InitValAST :: Solve_array_assign(int lev){
    if (sel == 0){
        exp->PreCompute();
        if (!exp->can_compute)
            exit(12);
        const_need_valued->PushBack(exp->val);
    }
    else{
        if (lev >= const_need_valued->dimension)
            exit(16);

        std::vector< std::unique_ptr<BaseAST> > &now_vec = *initvals; 
        for(int i=0;i<child_num;i++)
            ((InitValAST *)(now_vec[i].get()))->Solve_array_assign(const_need_valued->check_lev());
        
        if (const_need_valued->filled > const_need_valued->mul_dim)
            exit(17);

        int align_size = const_need_valued->lev_size(lev);
        while(const_need_valued->filled == 0 || const_need_valued->filled % align_size)
            const_need_valued->PushBack(0);
    }
}

int walking_mode = 0;
void InitValAST :: PreCompute(){
    if (sel == 0){
        if (const_need_valued)
            exit(14);
        exp->PreCompute();
        PreComputeAssign(exp);
    }
    else if (walking_mode){
        for (int i=0;i<child_num;i++)
            (*(initvals))[i]->PreCompute();
    }
    else {
        Solve_array_assign(0);
        const_need_valued = NULL;
    }
}

void ConstExpAST :: PreCompute(){
    exp->PreCompute();
    PreComputeAssign(exp);
    if (exp->can_compute == 0)
        exit(5);
}

void ConstInitValAST :: Solve_array_assign(int lev){
    if (sel == 0){
        constexp->PreCompute();
        const_need_valued->PushBack(constexp->val);
    }
    else{
        if (lev >= const_need_valued->dimension)
            exit(16);

        std::vector< std::unique_ptr<BaseAST> > &now_vec = *constinitvals; 
        for(int i=0;i<child_num;i++)
            ((ConstInitValAST *)(now_vec[i].get()))->Solve_array_assign(const_need_valued->check_lev());
        
        if (const_need_valued->filled > const_need_valued->mul_dim)
            exit(17);

        int align_size = const_need_valued->lev_size(lev);
        while(const_need_valued->filled == 0 || const_need_valued->filled % align_size)
            const_need_valued->PushBack(0);
    }
}

void ConstInitValAST :: PreCompute(){
    if (sel == 0){
        if (const_need_valued)
            exit(14);
        constexp->PreCompute();
        PreComputeAssign(constexp);
    }
    else{
        Solve_array_assign(-1);
        const_need_valued = NULL;
    }
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
    auto pres_symbtl = present_tbl();
    if (sel == 0){
        // Insert a non-valued item into present symbal table
        auto new_symtbl_item = new SymbolTableItem(1);
        pres_symbtl->Insert(ident, *new_symtbl_item);
        can_compute = 0; // can't be decided in compiling time
    }
    else if (sel == 1){
        initval->PreCompute();
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
    else {
        // 统一处理，右边exp可计算与不可计算的 Var Array Define 问题
        // 只有 if (sel == 3 && pres_symbtl == BaseAST::glbsymbtl)
            // 这种情况，需要把我们的初始值算出来，并且如果算不出来就是语义错误
        auto new_symtbl_item = new SymbolTableItem((1 << 7) | 1);
        new_symtbl_item->dimension = child_num;
        int DIM = 1;
        new_symtbl_item->dimension_item = new std::vector<int>;
        std::vector< std::unique_ptr<BaseAST> > &now_vec = *constexp; 
        for (int i=0;i<child_num;i++){
            now_vec[i]->PreCompute();
            DIM *= now_vec[i]->val; // constexp must be computed
            (*(new_symtbl_item->dimension_item)).push_back(now_vec[i]->val);
        }
        new_symtbl_item->mul_dim = DIM;
        new_symtbl_item->arr = new int[DIM + 3];
        new_symtbl_item->filled = 0; // 插入 filled = 0 的元素就表明是未初始化的

        if (sel == 3 && pres_symbtl == BaseAST::glbsymbtl){
            // 全局且已赋值的Var需要先算出来
            const_need_valued = new_symtbl_item;
            if (((InitValAST *)initval.get())->sel == 0)
                exit(15);
            initval->PreCompute();
        }
        else if(sel == 3) {
            walking_mode = 1;
            if (((InitValAST *)initval.get())->sel == 0)
                exit(15);
            initval->PreCompute();
            walking_mode = 0;
        }

        pres_symbtl->Insert(ident, *new_symtbl_item);
        can_compute = 0;
    }
}

void VarDeclAST :: PreCompute(){
    std::vector< std::unique_ptr<BaseAST> > &now_vec = *vardef; 
    for (int i=0;i<child_num;i++)
        now_vec[i]->PreCompute();
}

void ConstDefAST :: PreCompute(){
    auto pres_symbtl = present_tbl();
    SymbolTableItem* new_symtbl_item;
     
    if (sel){
        if (((ConstInitValAST *)constinitval.get())->sel == 0)
            exit(15);

        // const array
        new_symtbl_item = new SymbolTableItem(1 << 7);
        new_symtbl_item->dimension = child_num;
        int DIM = 1;
        new_symtbl_item->dimension_item = new std::vector<int>;
        std::vector< std::unique_ptr<BaseAST> > &now_vec = *constexp; 
        for (int i=0;i<child_num;i++){
            now_vec[i]->PreCompute();
            DIM *= now_vec[i]->val; // constexp must be computed
            (*(new_symtbl_item->dimension_item)).push_back(now_vec[i]->val);
        }
        new_symtbl_item->mul_dim = DIM;
        new_symtbl_item->arr = new int[DIM + 3];
        new_symtbl_item->filled = 0;

        const_need_valued = new_symtbl_item;
        constinitval->PreCompute();

        // 搞个大新闻，放到全局变量里
        std::cout << "global @" << pres_symbtl->ST_name << '_' << ident << " = alloc ";
        IR_alloc_code_gen_const(0, (const ConstDefAST *)this);
        cout << ", ";
        IR_alloc_code_gen2(0, new_symtbl_item, new_symtbl_item->mul_dim / (*(new_symtbl_item->dimension_item))[0], 0);
        cout << endl;
    }
    else{
        // const int
        constinitval->PreCompute();
        new_symtbl_item = new SymbolTableItem(0, constinitval->val);
    }
    pres_symbtl->Insert(ident, *new_symtbl_item);
    can_compute = 1;
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
        can_compute = 99;
        val = 0;
        // Cheat Code
    }
}

