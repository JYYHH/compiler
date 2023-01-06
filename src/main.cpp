#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include "ast.h"
#include "handle_ir.h"

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);

char IR[333333];

int main(int argc, const char *argv[]) { // compiler 模式 输入文件 -o 输出文件
  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  assert(yyin);

  // Get 语法树 AST
  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);
  ast->PreCompute();
  cout << "This is our original AST :" << endl << endl;
  ast->Dump(0);
  cout << endl;

  // 生成中间代码
  auto old_stdout = dup(1);
  FILE *IRfile = freopen(output, "w", stdout);
  ast->IRDump();
  cout << endl;
  fflush(IRfile);
  dup2(old_stdout, 1); // 恢复 stdout

  if (mode[1] == 'r'){ // -riscv
    // 生成目标代码
    ifstream IRstream(output);
    char ch;
    int lenIR = 0;

    while(IRstream.get(ch))
      if(ch != '\n')
        IR[lenIR++] = ch;
    IR[lenIR] = '\0';

    IRstream.close(); // 关闭对IR的读入流

    printf("\nIntermedia-AST and IR :\n\n%s\n", IR);

    freopen(output, "w", stdout);
    handle_str_ir(IR);
  }

  return 0;
}
