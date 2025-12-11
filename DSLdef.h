#pragma once
#include "NCC.h"

#define NUM(n)				NewNode((const node_data_t){.type = T_NUM,   .val.num = n}, NULL, NULL)
#define VAR(v)				NewNode((const node_data_t){.type = T_VAR, .val.var = v}, NULL, NULL)

#define ADD_(expr1, expr2)	NewNode((const node_data_t){.type = T_OP, .val.arifm = AR_ADD}, expr1, expr2)
#define SUB_(expr1, expr2)	NewNode((const node_data_t){.type = T_OP, .val.arifm = AR_SUB}, expr1, expr2)
#define MUL_(expr1, expr2)	NewNode((const node_data_t){.type = T_OP, .val.arifm = AR_MUL}, expr1, expr2)
#define DIV_(expr1, expr2)	NewNode((const node_data_t){.type = T_OP, .val.arifm = AR_DIV}, expr1, expr2)
#define POW_(expr1, expr2)	NewNode((const node_data_t){.type = T_OP, .val.arifm = AR_POW}, expr1, expr2)

// #define LN_(expr)			NewNode((const node_data_t){.type = OP_ELFUNC, .val.elfunc = F_LN}, NULL, expr)
// #define SIN_(expr)			NewNode((const node_data_t){.type = OP_ELFUNC, .val.elfunc = F_SIN}, NULL, expr)
// #define COS_(expr)			NewNode((const node_data_t){.type = OP_ELFUNC, .val.elfunc = F_COS}, NULL, expr)

// #define SINH_(expr)			NewNode((const node_data_t){.type = OP_ELFUNC, .val.elfunc = F_SINH}, NULL, expr)
// #define COSH_(expr)			NewNode((const node_data_t){.type = OP_ELFUNC, .val.elfunc = F_COSH}, NULL, expr)

// #define dL		TakeDeriv(tree->left, d_var)
// #define dR		TakeDeriv(tree->right, d_var)

// #define cL		TreeCopy(tree->left)
// #define cR		TreeCopy(tree->right)
// #define cC		TreeCopy(tree)
