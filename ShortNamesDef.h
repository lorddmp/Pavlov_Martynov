#pragma once

#include "NCC.h"

#define ROOT		(const node_data_t){.type = TP_ROOT, .val.num = 0}
#define OP_SEQ		(const node_data_t){.type = TP_OP_SEQ, .val.num = 0}
//#define OP_SEQ		(const node_data_t){.type = TP_OP_SEQ, .val.num = 0}
//#define EXPR		(const node_data_t){.type = TP_EXPR, .val.num = 0}

#define OPN_BRC		(const node_data_t){.type = TP_SYMB, .val.symb = SYM_OPN_BRC}
#define CLS_BRC 	(const node_data_t){.type = TP_SYMB, .val.symb = SYM_CLS_BRC}
#define OPN_PAR 	(const node_data_t){.type = TP_SYMB, .val.symb = SYM_OPN_PAR}
#define CLS_PAR 	(const node_data_t){.type = TP_SYMB, .val.symb = SYM_CLS_PAR}
#define SEMICOLON 	(const node_data_t){.type = TP_SYMB, .val.symb = SYM_SEMICOL}
#define COMMA 		(const node_data_t){.type = TP_SYMB, .val.symb = SYM_COMMA}
#define EQ 			(const node_data_t){.type = TP_OP, .val.op = OP_EQ}
#define ASSIGN 		(const node_data_t){.type = TP_OP, .val.op = OP_ASSIGN}
#define ADD 		(const node_data_t){.type = TP_OP, .val.op = OP_ADD}
#define SUB 		(const node_data_t){.type = TP_OP, .val.op = OP_SUB}
#define GREATER 	(const node_data_t){.type = TP_OP, .val.op = OP_GREATER}
#define LESS 		(const node_data_t){.type = TP_OP, .val.op = OP_LESS}
#define MUL 		(const node_data_t){.type = TP_OP, .val.op = OP_MUL}
#define DIV 		(const node_data_t){.type = TP_OP, .val.op = OP_DIV}
#define OR 			(const node_data_t){.type = TP_OP, .val.op = OP_OR}
#define AND 		(const node_data_t){.type = TP_OP, .val.op = OP_AND}
#define IF 			(const node_data_t){.type = TP_KWORD, .val.kword = KW_IF}
#define ELSE 		(const node_data_t){.type = TP_KWORD, .val.kword = KW_ELSE}
#define WHILE 		(const node_data_t){.type = TP_KWORD, .val.kword = KW_WHILE}
#define FOR 		(const node_data_t){.type = TP_KWORD, .val.kword = KW_FOR}
#define ASM 		(const node_data_t){.type = TP_KWORD, .val.kword = KW_ASM}



#define IS_(short_name, ptr_to_arr) (!memcmp(&(short_name), *(ptr_to_arr), sizeof(node_data_t)))

