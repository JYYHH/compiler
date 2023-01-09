#pragma once

/*
    Notes here:
        Record my Symbol Table Data Structure Here.
    
        
*/

#include <unordered_map>
#include <stack>
#include <vector>

class SymbolTableItem {
 public:
    int sel; 
    // Last Bit of 'sel' : 0 -> const / 1 -> var (或者其他能变的东西) ; 
    // First Bit of 'sel' : 0 -> unvalued before / 1 -> has been valued 
    // special sel : 
        // 32 -> func without return value (leaf func)
        // 40 -> func with return value (leaf func)
        // 48 -> func without return value (non-leaf func)
        // 56 -> func with return value (non-leaf func)
        // 65 -> func_param
        // 128 -> const array
        // 129 -> var array

    int val; // value
    int dimension;
    std::vector<int> *dimension_item;

    int mul_dim, filled;
    int *arr;
    
    SymbolTableItem(int SEL) : sel(SEL),val(1 << 31), dimension(0){}
    SymbolTableItem(int SEL, int VAL) : sel(SEL|2),val(VAL), dimension(0){}
    int VarType();
    int VarVal();
    void SetVal(const int &new_val); 
    void BecomeUnknown();
    void BecomeUnLeaf();
    int check_lev(); // return correspond level
    void PushBack(const int x);
    int lev_size(const int lev); // return correspond subarray size
};

class SymbolTable {
 public:
    std::unordered_map < std::string, SymbolTableItem* > ST; // 管理 Lval
    std::stack <int> reach_st; // 用来实现可达管理
                            // 1 -> 必可达(且到达次数严格为 1) / 0 -> 必不可达 / 2 -> 完全不确定可到达的次数
                            // 在初始进入一个 Block，或者有 If 语句时，处理这个Stack
    std::string ST_name;
    SymbolTable *father;
    SymbolTable *present; // 返回 GetItemByName 对应的 SymbolTable*

    void Insert(std::string &name, SymbolTableItem &STitem);
    SymbolTableItem* GetItemByName(std::string &name);
    void Push(int CANN, int VALL);
    void Pop();
};

#define NewState(pre,now) ((1&pre&now)|(2&pre&now)|(2&pre&(now<<1))|(2&(pre<<1)&now))
/*  
    Table :
    pre\now | 0 | 1 | 2 |
        0   | 0 | 0 | 0 |
        1   | 0 | 1 | 2 |
        2   | 0 | 2 | 2 |
*/
#define NewElement(can_,val_) ((can_&val_)|((can_^1)<<1))
/*  
    Table :
  can_\now_ | 0 | 1 |
        0   | 2 | 2 |
        1   | 0 | 1 |
*/