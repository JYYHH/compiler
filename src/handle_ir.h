#pragma once

#include "koopa.h"

// Basic
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
// ------------------------ `koopa_raw_value_t` Visit ------------------------------------------------------
void Visit(const koopa_raw_return_t &ret);
void Visit(const koopa_raw_integer_t &INT);
void Visit(const koopa_raw_binary_t &BinOP);

int handle_str_ir(const char *str);


