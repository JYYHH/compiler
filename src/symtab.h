#pragma once

/*
    Notes here:
        Record my Symbol Table Data Structure Here.
    
        
*/

#include <unordered_map>

class SymbolTableItem {
 public:
    int sel; 
    // Last Bit of 'sel' : 0 -> const / 1 -> var ; 
    // First Bit of 'sel' : 0 -> unvalued before / 1 -> has been valued 
    int val; // value
    
    SymbolTableItem(int SEL) : sel(SEL),val(1 << 31){}
    SymbolTableItem(int SEL, int VAL) : sel(SEL|2),val(VAL){}
    int VarType();
    int VarVal();
    void SetVal(const int &new_val);
};

class SymbolTable {
 public:
    std::unordered_map < std::string, SymbolTableItem* > ST;
    std::string ST_name;
    SymbolTable *father;
    SymbolTable *present;

    void Insert(std::string &name, SymbolTableItem &STitem);
    SymbolTableItem* GetItemByName(std::string &name);
};