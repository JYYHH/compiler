This is our original AST :

CompUnitAST {
  TotolComponentNum = 1
  FuncDefAST {
    int,
    FuncName = main,
    FuncParamNum = 0

    BlockAST {
      BlockNum = 2
      BlockItemAST {
        GLBIfAST {
          IfElseStmtAST {
            StmtAST {
              OptionalExpAST {

              }
            }
          }
        }
      }********** End of Child 0
      BlockItemAST {
        GLBIfAST {
          IfElseStmtAST {
            StmtAST {
              return
              OptionalExpAST {
                ExpAST {
                  LOrExpAST {
                    LAndExpAST {
                      EqExpAST {
                        RelExpAST {
                          AddExpAST {
                            MulExpAST {
                              UnaryExpAST {
                                PrimaryExpAST {
                                  Number = 6
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }********** End of Child 1

    }
  }********** End of Child 0

}
