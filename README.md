# 一、编译器概述
## 1. 基本功能 (可以翻译的语句)
### 1.0 (源语言SysY)语法(文法)结构
```python
CompUnit      ::= [CompUnit] (Decl | FuncDef);

Decl          ::= ConstDecl | VarDecl;
ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";";
BType         ::= "int";
ConstDef      ::= IDENT {"[" ConstExp "]"} "=" ConstInitVal;
ConstInitVal  ::= ConstExp | "{" [ConstInitVal {"," ConstInitVal}] "}";
VarDecl       ::= BType VarDef {"," VarDef} ";";
VarDef        ::= IDENT {"[" ConstExp "]"}
                | IDENT {"[" ConstExp "]"} "=" InitVal;
InitVal       ::= Exp | "{" [InitVal {"," InitVal}] "}";

FuncDef       ::= FuncType IDENT "(" [FuncFParams] ")" Block;
FuncType      ::= "void" | "int";
FuncFParams   ::= FuncFParam {"," FuncFParam};
FuncFParam    ::= BType IDENT ["[" "]" {"[" ConstExp "]"}];

Block         ::= "{" {BlockItem} "}";
BlockItem     ::= Decl | Stmt;
Stmt          ::= LVal "=" Exp ";"
                | [Exp] ";"
                | Block
                | "if" "(" Exp ")" Stmt ["else" Stmt]
                | "while" "(" Exp ")" Stmt
                | "break" ";"
                | "continue" ";"
                | "return" [Exp] ";";

Exp           ::= LOrExp;
LVal          ::= IDENT {"[" Exp "]"};
PrimaryExp    ::= "(" Exp ")" | LVal | Number;
Number        ::= INT_CONST;
UnaryExp      ::= PrimaryExp | IDENT "(" [FuncRParams] ")" | UnaryOp UnaryExp;
UnaryOp       ::= "+" | "-" | "!";
FuncRParams   ::= Exp {"," Exp};
MulExp        ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
AddExp        ::= MulExp | AddExp ("+" | "-") MulExp;
RelExp        ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
EqExp         ::= RelExp | EqExp ("==" | "!=") RelExp;
LAndExp       ::= EqExp | LAndExp "&&" EqExp;
LOrExp        ::= LAndExp | LOrExp "||" LAndExp;
ConstExp      ::= Exp;
```
### 1.1 Main 函数
```cpp
int main() {
  // 注释也应该被删掉哦
  return 0;
}
```
### 1.2 对于(Logic & Arithmetic)表达式的支持
```cpp
int main() {
  return (1 + 2 * -3) * 5 / 3 - 2 && 1 + 2 || 3;
}
```
### 1.3 对于常量和变量定义和使用的支持
```cpp
int main() {
  const int x = 233 * 4;
  int y = 10;
  y = y + x / 2;
  return y;
}
```
### 1.4 对于语句块和作用域的支持
```cpp
int main() {
  int a = 1, b = 2;
  {
    int a = 2;
    b = b + a;
  }
  return b;
}
// answer is 4
```
### 1.5 对于 if-else (or 单if) 语句的支持
```cpp
int main() {
  int a = 1;
  if (a == 2 || a == 3) {
    return 0;
  } else {
    if (a > 4) return a + 1;
  }
  return 0;
}
```
### 1.6 对于 while 语句 (以及对应的break/continue) 的支持
```cpp
int main() {
  int i = 0, pow = 1;
  while (i < 7) {
    if (i < 2) continue;
    if (i > 5) break;
    pow = pow * 2;
    i = i + 1;
  }
  return pow;
}
```
### 1.7 对于函数和全局变量的支持
```cpp
const int ten = 10;
int var;

int func(int x) {
  var = var + x;
  return var;
}

int main() {
  // putint 和 putch 都是 SysY 库函数
  // SysY 要求库函数不声明就可以使用
  putint(func(1));
  var = var * ten;
  putint(func(2));
  putch(10);
  return var;
}
```
### 1.8 对于数组 (一维数组、多维数组、数组参数) 的支持
```cpp
int init = 0;

void init1d(int n, int arr[]) {
  int i = 0;
  while (i < n) {
    arr[i] = init;
    init = init + 1;
    i = i + 1;
  }
}

void init2d(int n, int arr[][10]) {
  int i = 0;
  while (i < n) {
    init1d(10, arr[i]);
    i = i + 1;
  }
}

void init3d(int n, int arr[][10][10]) {
  int i = 0;
  while (i < n) {
    init2d(10, arr[i]);
    i = i + 1;
  }
}

int sum1d(int n, int arr[]) {
  int i = 0, sum = 0;
  while (i < n) {
    sum = sum + arr[i];
    i = i + 1;
  }
  return sum;
}

int sum2d(int n, int arr[][10]) {
  int i = 0, sum = 0;
  while (i < n) {
    sum = sum + sum1d(10, arr[i]);
    i = i + 1;
  }
  return sum;
}

int sum3d(int n, int arr[][10][10]) {
  int i = 0, sum = 0;
  while (i < n) {
    sum = sum + sum2d(10, arr[i]);
    i = i + 1;
  }
  return sum;
}

int main() {
  int arr[10][10][10];
  init3d(10, arr);
  int sum = sum3d(10, arr);
  sum = sum + sum2d(10, arr[1]);
  sum = sum + sum1d(10, arr[2][3]);
  putint(sum);
  putch(10);
  return sum;
}
```

### 1.9 Conclusion
- 所以综上所述，这个编译器实现了编译lab要求的所有语法&所有target language (IR / RISC-V) 的翻译，并在最后的数据评测中得到了 `97.27/100.00` (IR) && `96.36/100.0` (RISC-V) 的分数

<br/><br/><br/><br/>

-----------------

## 2. 主要特点
- 除了上述，我在没有参考任何材料的前提下为我的编译器开发了一个 `PreCompute` Module，详情见 `src/pre_compute.cpp` （以及这篇总结的后续Section）:
    - 它的主要特点是：
        1. 可以提前在编译时预处理出来的信息 (而不是需要运行时传入的parameter) 不会被写成IR，而是储存在 pre_compute.cpp 中的一些数据结构 (主要是各种AST的 `can_compute/val` 属性 && `Symbol Table`) 中
        2. 这样我们在翻译 IR 时，就可以省略一些不必要的 load / store 以及 branch
        3. ___由于时间原因目前这个 Module 只支持到 if-else 及其之前的部分，而对这之后的部分(几乎可以看成)关闭了优化。之后如果 upgrade 了会在相应的 Github Repo 中 push___
        4. 注意这个 `Module` 只是对 `IR generating` 这个过程进行一些优化，而不会对后面的任何一个过程优化

- 具体的例子请参考《三、编译器实现》

<br/><br/><br/><br/>

-----------------

# 二、编译器设计
## 1. 主要模块组成
1. 我们的 `Parser` 主要由 `sysy.l && sysy.y` 构成，它们的作用主要是解析读入的源代码文本，并构造出一个 AST (语法树)返回.
2. 而 `Parser` 生成的 AST 并不会直接交给 `IR-generator` 去生成 `IR`；而是先交给 `PreCompute` Module (`pre_compute.cpp`) 去进行一些优化，以及符号表等预处理工作，得到较为成熟的 AST'
3. 这个 AST' 则会被送到 `IR-generator` (对应 `ast.cpp`) 去生成 `IR`
4. 最后，我们的目标代码生成模块 (主要是 `handle_ir.cpp && handle_ir.h`) 会将上一步生成的 IR 进一步生成 RISC-V，从而完成了编译的全过程

- 除此之外，`symtab.cpp / symtab.h` 对应符号表；`ast_dump.cpp` 则是将 AST 遍历输出其结构 (方便 debug)；`main.cpp` 是实现我们上述形容的流程的主函数；`ast.h` 则是统筹掌控所有生成 IR 这个过程的所有 Data Structure && function

## 2. 主要数据结构
1. AST (ast.h) : 用来维护我们的语法树信息，其中绝大部分都是综合属性
2. Symbol Table (symtab.h) : 用来维护各个符号表的符号 (构成了所有符号的一个划分，每个符号的全名是 `<symtbl_name>_<string_name>`，这一点也体现在了我的 IR 生成中)
    - `symtab.h` 里面还详细定义了每一种符号类型对应的位图
3. Visit (function in `handle_ir.h`) : 用来生成 RISC-V 的树遍历函数，这个不能算 DS，但考虑到它的重要性还是要把它加上

- 剩下的一些数据结构(主要是一些用来存处理信息的栈 && Counter) && function 比较杂碎，这里不单独列出，如有疑问可以单独解释一下（

## 3. 算法设计考虑
1. 最重要的是周全性：
    - 由于我们设计的编译器，要有能翻译出所有符合文法的源代码的能力，所以这就需要我们格外小心注意严谨，一定要在设计算法的时候尽量考虑到所有情况，特别是一些特殊情况
2. 其次是模块性：
    - 对于目标代码的生成是很有规律的，所以我们可以尽量各种情况都要共用的功能写成一个又一个函数:
    - 这点我在 IR->RISC-V 的时候做的很好，在 AST'->IR 的过程做的一般
3. 最后是守规矩：
    - 由于我们只是一个翻译官，并不是所有事情都可以乱来。生成 RISC-V 的过程就需要不仅仅遵守 RISC-V 语句的规范，还要考虑到 RISC-V stack abstraction / register using principle 之类的事情

<br/><br/><br/><br/>

-----------------
# 三、编译器实现

## 1. 对所涉工具软件的介绍
### 1.1 cpp
- 对我来说，cpp是一个既贴近system但又不失时髦的编程语言
- 你在使用 cpp 的时候不仅可以继承 C 对于 system 强力支持的特性；同时也可以享受 OOP / auto 等现代编程属性的便利

### 1.2 Flex && Bison
- 这是编译帮助文档提供的，选用原因也很简单：没有必要重复造轮子
- 虽然课上已经学过了各种自动机，以及 LL / LR / SLR / LALR 等文法及其解析方法，但有现成的软件包，我觉得还是没有必要自己再实现一遍
- 同时我们通过使用这些软件包也可以进一步加深我们对于各种文法的理解：比如我一开始对着文档直接实现的文法就有 reduce-reduce conflict 和 shift-reduce conflict，之后通过自己的演算设计出了合法的文法才得以正确解析源代码

## 2. 各个阶段编码细节
### 2.1 Parsing
- 这个部分并没有太多细节，主要就是写好了词法和文法然后交给 Flex & Bison 去 Parse就可以了
- 为数不多有技术含量的就是解决 conflict：
    - reduce-reduce conflict 和 shift-reduce conflict 很常见的一种情况就是同时存在 `A -> BCD` && `A -> BCE` 这两条规则，其中大写字母都是非终结符。这种情况下我们应该提取一下左公因子 `O->BC`，然后把原文法改成：`A -> OD` && `A -> OE` 这两条规则。产生冲突的原因是，Parser reduce 出B之后就不知道该放哪边了，因为就算 look ahead 的话这两条规则后面都是 C 也是分辨不出来的 

### 2.2 PreComputing
- 这个是 Optional, 开关在 `ast.h` 的 `#define MODE` 这一句
- 这一个Module主要是实现在编译时就把能算的东西都算出来，而避免这一部分生成IR影响最后性能。主要规则如下：
    1. can_compute && val 是一对所有 AST 类型都具有的属性。前者用来维护一个语法单元(AST中的节点)是否是可以在运行前就已知结果的，后者是维护这个运行结果的值。
    2. 主要针对的语句是广大的各种 Exp 类，以及 Stmt 中的部分语句 (If-Else、赋值语句等)
    3. 一个 AST 的 can_compute 是综合属性，一般情况下需要所有所有儿子节点 （进而可以拓展到子树）的 can_compute 都为 1 才能为 1. 以下是例外情况：
        - 是叶子，且就是一个INT_CONST
        - 是叶子，且引用的符号是 `const` 或者 `在目前位置必然可计算`，后者定义比较复杂，后面会深入讨论
        - && 或者 || 语句，且前面的表达式已经是 0 或者 1 了，根据短路的逻辑我们在运行时也不会执行后面的语句（所以即使有后效性也无所谓了）
        - if-else 语句，且 if 中的 exp 是 can_compute 的

- 对于 `在目前位置必然可计算` 的定义：
    - 从这个语句引用的符号lval定义的位置，执行到现在的位置，__途中没有经过任何一条我们不知道是否执行的对lval进行修改的赋值语句__
    - 这个定义有一点抽象，不过有兴趣的话可以参考 `symtbl.h` 中 `SymbolTable.reach_st` 这个栈的应用、`symtbl.h` 最下方的 Table （我们定义 1 为必然可达状态 (类比成1)、0为必然不可达状态(类比成0)、2为我们也不知道可不可达或者可达多少次的状态(类比成 $\mathbb{N}$)），以及 `pre_compute.cpp` 中 进入 Block 和 IfElse 语句对于这个栈的修改：这是我对这个概念的具体实现，而且实验显示这种实现是对的 （因为从 Lv_4 开始，对于开不开这个 Module 的代码我都会测一次，每次分都是一样的）。

- 对于上述定义可以有一个抽象：我们每次赋值都会给一个变量带来一个作用range
- 如果这次赋值之于这个变量的定义语句是必然可达的，那么作用的range就是这个语句到下一个对这个变量赋值语句的前一句
- 否则，这个range是空的，且我们要在 Symble Table里把这个Symble置为 `unknown`

#### `Precompute Example`
- First is cpp
```cpp
int main(){
  int x = 2;
  if (x <= 5){
    int y = 8 * 9 * 2 + 889 || 2;
    x = x * y + y;
    x = x * x + 1;
    if (x >= 10)
      return x;  // 这句被直接 return 了
    else
      return 10 - x;
  }
  else{
    x = x + 6;
    if (x > 3)
      x = x * 8;
    else
      x = 9 - x;
  }
  return 0;
}
```

- And here is IR
```python
fun @main(): i32 {
 %entry:
    @block_1_x = alloc i32
    store 2, @block_1_x
    @block_2_y = alloc i32
    store 1, @block_2_y
    store 3, @block_1_x
    store 10, @block_1_x
    ret 10
 %after_ret_1:
    ret 0
 %after_ret_2:
    ret 0
}
```


### 2.3 IR-generator
- 这个阶段是上个阶段的延申：对于可以被 PreCompute 的 AST 单元，我们可以直接在这个节点把我们 PreCompute 得到的结果返回 (到 IR 的一个临时变量里)，而且可以设置为常数变量（这样就不用print IR code），直接返回就行：这样就实现了不生产任何多余的 IR instruction 的理念
- 对于其他AST单元，我们循规蹈矩地递归翻译就行，限于篇幅这里不具体展开 (Exp 对应各种算数逻辑表达式；"return" 就是 ret 后面的exp的结果；if-else 就是进行 branch；while的branch相比于 if-else 也就进行了微调；变量同意是 load/store；最后函数调用直接 call 就可以了)

#### `重难点：数组相关`
1. 数组初始化
    1. `pre_compute.cpp && ast.cpp` 中出现的 `Solve_array_assign` && `Solve_array_assign2` 函数，是递归解决数组初值问题的函数
    2. 见`ast.cpp` 的 `IR_alloc_code_gen_REALVAR` 这个函数，是在我们把数组初值存在这个数组名对应的符号表项对应位置之后，从这个位置把数据读出来挨个生成 IR code 来把这些运算结果搬进数组的对应位置的
    3. 对于 const 的 local array，我直接把它放到全局变量处理了，这么做的好处是显然的：它们是静态的，而且在运行前就可以算出来，那么为什么不放到内存的.data区呢？这样对堆栈都是友好的，也利于我们写代码

2. 数组寻址
    1. `ast.cpp` 中的 `load_matrix_pointer` 函数，实现了从数组索引offset的vector 到 数组元素load到对应 IR instruction 的 mapping。需要注意的是这个函数还提供了索引的数组是不是参数数组的这一选项。
    2. 同样的，如果我们 `load_matrix_pointer` 完了之后发现提供的数组索引offset的vector的长度没有这个数组的维度长，那么说明我们要用的是一个 pointer，所以要judge一下这种情况不能直接load，而要把load改成getelemtpr

### 2.4 RISC-V 生成
- 这一部分相对来说没有那么难写，可能有两个原因：
    1. 我这一部分模块化做的还不错，对于 RISC-V 指令的 output 和一些共性操作 （从内存拿数到register上；以及反向操作）都写好了宏 or 函数 （`handle_ir.cpp` 前 431 行的函数都是类似）
    2. Koopa IR 的数据结构整理的很整齐，一层一层的：Function/Basic Block/Value。所以写代码可以很轻易地 Handle这种规整的情况
- 我们基本上也是循规蹈矩地从每一条 IR 指令生成相应的 RISC-V 指令。并且由于所要用到的信息基本上用 Koopa IR 提供的内存模型的指针就可以找到（这就需要我们把 `koopa.h` 读明白），在利用前面说的我写的那些function把要用到的值在 Stack 和 CPU（Register）上来回倒一下就ok

#### `重难点：函数调用`
- 这一部分比较low level，容易写错。 首先要确认一条准则是：`由于你是在栈上操作，所以你每条指令对应的还原指令也是要按照栈序列排列的`。举个例子就是：如果你是在 改变 `sp` 值之前存的 `ra`，那么你也要再恢复 `sp`值之后再恢复 `ra`；以此类推。。。
- 然后除了 `sp` 你还需要一个 Register 存多余的 parameter，我用的是 临时变量`t4`，虽然好像有专门一个变量用来干这件事情。。。
- 这样你就有了一个 callee Reg (ra) 和 caller Reg (t4)，你需要在对应位置保存和恢复它们 （自己衡量）
- 同样的，你从 Stack 调数据 和 存数据到 Stack 上就有两个 Base Register : sp 和 t4
- 主要参考，`handle_ir.cpp` 中的 `void Visit(const koopa_raw_call_t &Call, const int mode)` 和 `void Visit(const koopa_raw_function_t &func, const int mode)`

<br/><br/><br/><br/>

-----------------

