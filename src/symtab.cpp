#include <string>
#include "symtab.h"

using namespace std;

int SymbolTableItem :: VarType(){
    return sel;
}

int SymbolTableItem :: VarVal(){
    return val;
}

void SymbolTableItem :: SetVal(const int &new_val){
    sel |= 2;
    val = new_val;
}

void SymbolTableItem :: BecomeUnknown(){
    sel &= 1;
}

void SymbolTableItem :: BecomeUnLeaf(){
    sel |= (1 << 4);
}

int SymbolTableItem :: check_lev(){
    int ret = dimension, remain = filled;
    while (ret && remain % (*dimension_item)[ret - 1] == 0){
        remain /= (*dimension_item)[ret - 1];
        ret --;
    }
    return max(1, ret);
}

void SymbolTableItem :: PushBack(const int x){
    arr[filled] = x;
    filled ++;
}

int SymbolTableItem :: lev_size(const int lev){
    int ret = mul_dim;
    for(int i=0;i<lev;i++)
        ret /= (*dimension_item)[i];
    return ret;
}

void SymbolTable :: Insert(std::string &name, SymbolTableItem &STitem){
    if (ST.count(name))
        exit(3);
    ST[name] = &STitem;
}

SymbolTableItem* SymbolTable :: GetItemByName(std::string &name){
    for(SymbolTable* now = this; now; now = now->father)
        if (now->ST.count(name)){
            this->present = now;
            return now->ST[name];
        }
    return NULL;
}

void SymbolTable :: Push(int CANN, int VALL){
    reach_st.push(NewState(reach_st.top(), NewElement(CANN, VALL)));
}

void SymbolTable :: Pop(){
    reach_st.pop();
}