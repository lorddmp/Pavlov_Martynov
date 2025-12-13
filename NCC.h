#pragma once

#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include "Colors.h"



typedef enum node_type_t
{
	TP_NUM,
	TP_OP,
	TP_VAR,
	TP_KWORD,
	TP_SYMB,
	TP_FUNC,

} node_type_t;
static const char *NODE_TYPE_NAME[] =
	{"number", "operation", "variable", "keyword", "symbol", "function"};


typedef enum op_t
{
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_GREATER,
	OP_LESS,
	OP_ASSIGN,
	OP_EQ,
	
	OP_OR,
	OP_AND,

} op_t;
static const char *OP_NAME[] =
	{"+", "-", "*", "/", ">", "<", "=", "==", "or", "and"};

typedef enum kword_t
{
	KW_IF,
	
	KW_ELSE,
	
	KW_WHILE,
	KW_FOR,
	KW_ASM,

} kword_t;
static const char *KWORD_NAME[] =
	{"if", "else", "while", "for", "asm"};

typedef enum symb_t
{
	SYM_BRC_OPN,
	SYM_BRC_CLS,
	SYM_PAR_OPN,
	SYM_PAR_CLS,
	SYM_SEMICOL,
	SYM_COMMA,

} symb_t;
static const char *SYMB_NAME[] =
	{"{", "}", "(", ")", ";", ","};

typedef union node_val_t
{
	char *name;
	size_t var;
	long num;
	op_t op;
	symb_t symb;
	kword_t kword;

} node_val_t;

typedef struct node_data_t
{
	node_type_t type;
	node_val_t val;
	
} node_data_t;

typedef struct node_t
{
	node_data_t data;
	struct node_t *parent;
	struct child_t *child;

} node_t;

typedef struct child_t
{
	node_t *node;
	struct child_t *prev, *next;
	
} child_t;

typedef struct lex_t
{
	const char *name;
	const node_data_t data;
} lex_t;

typedef struct toks_t
{
	node_data_t *data;
	size_t size;
	size_t cap;

} toks_t;

#include "DSLdef.h"

static const lex_t LEXS[] =
	{
		{"{", {.type = TP_SYMB, .val.symb = SYM_BRC_OPN}},
		{"}", {.type = TP_SYMB, .val.symb = SYM_BRC_CLS}},
		{"(", {.type = TP_SYMB, .val.symb = SYM_PAR_OPN}},
		{")", {.type = TP_SYMB, .val.symb = SYM_PAR_CLS}},
		{";", {.type = TP_SYMB, .val.symb = SYM_SEMICOL}},
		{",", {.type = TP_SYMB, .val.symb = SYM_COMMA}},
		
		{"==", {.type = TP_OP, .val.op = OP_EQ}},
		{"=", {.type = TP_OP, .val.op = OP_ASSIGN}},
		{"+", {.type = TP_OP, .val.op = OP_ADD}},
		{"-", {.type = TP_OP, .val.op = OP_SUB}},
		{">", {.type = TP_OP, .val.op = OP_GREATER}},
		{"<", {.type = TP_OP, .val.op = OP_GREATER}},
		{"*", {.type = TP_OP, .val.op = OP_MUL}},
		{"/", {.type = TP_OP, .val.op = OP_DIV}},
		{"or", {.type = TP_OP, .val.op = OP_OR}},
		{"and", {.type = TP_OP, .val.op = OP_AND}},
		
		{"if", {.type = TP_KWORD, .val.kword = KW_IF}},
		{"else", {.type = TP_KWORD, .val.kword = KW_ELSE}},
		{"while", {.type = TP_KWORD, .val.kword = KW_WHILE}},
		{"for", {.type = TP_KWORD, .val.kword = KW_FOR}},
		{"asm", {.type = TP_KWORD, .val.kword = KW_ASM}}
};

typedef enum tree_err_t
{
	TR_NO_ERR,
	TR_EXT_ERR,
	TR_NULLPTR,
	TR_OVERFLOW,

} tree_err_t;

#include "DSLundef.h"

static const size_t N_LEXS = sizeof(LEXS) / sizeof(lex_t);
static const size_t MAX_REC_DEPTH = 100;

/*--------------------------------------*/

long ReadFileToBuf(const char *file_path, char **buf);
//
//node_t *NewNode(const node_data_t data, node_t *left, node_t *right);
//node_t *TreeCopy(const node_t *tree);
//node_t *FindNode(node_t *tree, const node_data_t data);

toks_t *Tokenize(const char *s);

void TokensDestroy(toks_t *toks);

//tree_err_t TreeDumpHTML(const node_t *tree, const char *dot_file_path, const char *img_dir_path, const char *html_file_path, const char *caption);

void PrintToks(toks_t *toks, FILE *dump_file);