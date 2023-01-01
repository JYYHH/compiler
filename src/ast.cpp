#include <string>
#include <memory>
#include <iostream>
#include "ast.h"

using namespace std;

string sj_map[20] = {
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
  "                                      "
};

void CompUnitAST :: Dump() const{
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << " }";
}
void FuncDefAST :: Dump() const {
    std::cout << "FuncDefAST { ";
    func_type->Dump();
    std::cout << ", " << ident << ", () ";
    block->Dump();
    std::cout << " }";
}
void FuncTypeAST :: Dump() const {
    std::cout << "FuncTypeAST { ";
    std::cout << type;
    std::cout << " }";
}
void BlockAST :: Dump() const {
    std::cout << "BlockAST { ";  
    stmt->Dump();
    std::cout << " }";
}
void StmtAST :: Dump() const {
    std::cout << "StmtAST { ";  
    std::cout << "return " << number << " ;";
    std::cout << " }";
}

// in the beginning of each line
inline void BaseAST :: HandleSJ(int sj) const{
    std::cout << sj_map[sj];
}

void CompUnitAST :: IRDump(int sj) const{
    func_def->IRDump(sj);
}
void FuncDefAST :: IRDump(int sj) const {
    HandleSJ(sj);
    std::cout << "fun @";
    std::cout << ident << "(): ";
    func_type->IRDump(sj);
    block->IRDump(sj);
}
void FuncTypeAST :: IRDump(int sj) const {
    if (type == "int")
        std::cout << "i32";
}
void BlockAST :: IRDump(int sj) const {
    std::cout << " {" << endl;
    // list the basic block here further?
    HandleSJ(sj);
    std::cout << " %" << "entry: " << endl;
    stmt->IRDump(sj + 1); // 基本块应该有缩进
    std::cout << endl << "}";
}
void StmtAST :: IRDump(int sj) const {
    HandleSJ(sj);
    std::cout << "ret " << number;
}