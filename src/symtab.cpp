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


void SymbolTable :: Insert(std::string &name, SymbolTableItem &STitem){
    if (ST.count(name))
        exit(3);
    ST[name] = &STitem;
}

SymbolTableItem* SymbolTable :: GetItemByName(std::string &name){
    for(SymbolTable* now = this; now; now = now->father)
        if (now->ST.count(name))
            now->ST[name];
    return NULL;
}