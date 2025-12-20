#include "NCC.h"

static int COMPILE_STATUS = 0;
static size_t LBL_CNT = 0;
static FILE *ASM_OUT = NULL;

/*


    RAX (Accumulator): для арифметических операций

    RCX (Counter): для хранения счетчика цикла

    RDX (Data): для арифметических операций и операций ввода-вывода

    RBX (Base): указатель на данные

    RSP (Stack pointer): указатель на верхушку стека

    RBP (Base pointer): указатель на базу стека внутри функции

    RSI (Source index): указатель на источник при операциях с массивом

    RDI (Destination index): указатель на место назначения в операциях с массивами

    RIP: указатель адреса следующей инструкции для выполнения

    RFLAGS: регистр флагов, содержит биты состояния процессора

*/

/*---------------------------------------------*/
static void GnrtDeclFunc(const node_t *tree);
static void GnrtAsm(const node_t *tree);
static void GnrtArifm(const node_t *tree, const size_t n_var);

static void GnrtExpr(const node_t *tree, const size_t n_var);
static void GnrtOr(const node_t *tree, const size_t n_var);
static void GnrtAnd(const node_t *tree, const size_t n_var);
static void GnrtComp(const node_t *tree, const size_t n_var, const op_t gle); /* Greater Less Equal*/

static void GnrtIf(const node_t *tree, const size_t n_var);
static void GnrtOp(const node_t *tree, const size_t n_var);
static void GnrtAssign(const node_t *tree, const size_t n_var);
static void GnrtWhile(const node_t *tree, const size_t n_var);
static void GnrtCallFunc(const node_t *tree, const size_t n_var);
static void GnrtReturn(const node_t *tree, const size_t n_var);

static size_t GetMaxID(const node_t *tree);
/*---------------------------------------------*/

int CompileTree(const node_t *tree, FILE *asm_out)
{
	if(tree == NULL || asm_out == NULL)
	{
		print_err_msg("nullptr passed as arg(s)");
		return 1;
	}
	if(!IS_(ROOT, &(tree->data)))
	{
		print_err_msg("I wanna be a root");
		return 1;
	}

	ASM_OUT = asm_out;

	child_t *decl = tree->child;
	while(decl)
	{
		if(decl->node == NULL)
		{
			print_err_msg("null node");
			return 1;
		}
		if(COMPILE_STATUS)
		{
			print_err_msg("compilation failed");
			return 1;
		}

		GnrtDeclFunc(decl->node);
		decl = decl->next;
	}

	return COMPILE_STATUS;
}

static void GnrtOp(const node_t *tree, const size_t n_var)
{
	assert(tree);
	assert(ASM_OUT);
	LEAVE_IF_ERR;

	switch(tree->data.type)
	{
	case TP_CALL_FUNC:
		GnrtCallFunc(tree, n_var);
		return;
	case TP_KWORD:
		switch(tree->data.val.kword)
		{
		case KW_ASM:
			GnrtAsm(tree);
			return;
		case KW_IF:
			GnrtIf(tree, n_var);
			return;
		case KW_WHILE:
			GnrtWhile(tree, n_var);
			return;
		case KW_RETURN:
			GnrtReturn(tree, n_var);
			return;
		case KW_ELSE:
		case KW_FOR:
		case KW_CONTINUE:		/* aren't implemented */
		case KW_BREAK:
		case KW_FUNC:
		default:
			err_exit_void("invalid keyword");
		}
		return;
	case TP_OP:
		switch(tree->data.val.op)
		{
		case OP_ASSIGN:
			GnrtAssign(tree, n_var);
			return;
		case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
		case OP_GREATER:		/* implemented in GnrtArifm */
		case OP_LESS:
		case OP_EQ:
		case OP_OR:
		case OP_AND:
		default:
			err_exit_void("invalid operation");
		}
		return;
	case TP_EOF:
	case TP_ROOT:
	case TP_NUM:
	case TP_OP_SEQ:
	case TP_IDENT:
	case TP_PARAM:				/* cannot be operation */
	case TP_VAR:
	case TP_SYMB:
	case TP_DECL_FUNC:
	case TP_LITERAL:
	default:
		err_exit_void("invalid type");
		return;
	}
}

static void GnrtOpSeq(const node_t *tree, const size_t n_var)
{
	assert(tree);
	assert(ASM_OUT);
	LEAVE_IF_ERR;
	if(tree->data.type != TP_OP_SEQ)
		err_exit_void("node is not an op. sequence");

	child_t *op = tree->child;
	while(op)
	{
		if(op->node == NULL)
			err_exit_void("child->node == NULL");
		GnrtOp(op->node, n_var);
		op = op->next;
	}
}

static void GnrtArifm(const node_t *tree, const size_t n_var)
{
	assert(tree);
	assert(ASM_OUT);
	LEAVE_IF_ERR;

	switch(tree->data.type)
	{
	case TP_NUM:
		print_asm("push %ld ;push num\n\n", tree->data.val.num);
		return;
	case TP_VAR:
		print_asm("mov rbp, rsp\n"
				  "add rbp, %lu ;calculate var pos in stack\n"
				  "push [rbp] ;push var\n\n",
				  tree->data.val.id);
		return;
	case TP_OP:
		if(!IS_BINNODE(tree))
			err_exit_void("invalid node");

		GnrtExpr(LEFT(tree), n_var);
		GnrtExpr(RIGHT(tree), n_var);

		switch(tree->data.val.op)
		{
		case OP_ADD:
			print_asm("add\n");
			return;
		case OP_SUB:
			print_asm("sub\n");
			return;
		case OP_MUL:
			print_asm("mul\n");
			return;
		case OP_DIV:
			print_asm("div\n");
			return;
		case OP_GREATER:
		case OP_LESS:
		case OP_ASSIGN:
		case OP_EQ:			/* implemented in GnrtComp, GnrtAnd, GnrtOr */
		case OP_OR:
		case OP_AND:
		default:
			err_exit_void("invalid operation");
		}
		return;
	case TP_EOF:
	case TP_ROOT:
	case TP_OP_SEQ:
	case TP_IDENT:
	case TP_PARAM:
	case TP_KWORD:
	case TP_SYMB:
	case TP_DECL_FUNC:
	case TP_CALL_FUNC:
	case TP_LITERAL:
	default:
		err_exit_void("invalid type");
	}
}

static void GnrtDeclFunc(const node_t *tree)
{
	LEAVE_IF_ERR;
	assert(tree);
	assert(ASM_OUT);
	if(tree->data.type != TP_DECL_FUNC)
		err_exit_void("node is not a function declaration");
	if(!IS_BINNODE(tree))
		err_exit_void("invalid node");

	print_asm("%s:\n", tree->data.val.name);

	GnrtOpSeq(RIGHT(tree), GetMaxID(tree));
	
	print_asm("ret\n\n"); /* dubiously */
}

static void GnrtAsm(const node_t *tree)
{
	assert(tree);
	assert(ASM_OUT);
	LEAVE_IF_ERR;
	if(!IS_(ASM, &(tree->data)))
		err_exit_void("is not 'asm'");

	print_asm(";asm insertion\n"
		  "%s\n\n",
		  tree->data.val.name);
}

static void GnrtOr(const node_t *tree, const size_t n_var)
{
	LEAVE_IF_ERR;
	assert(tree);
	assert(ASM_OUT);
	if(!IS_(OR, &(tree->data)))
		err_exit_void("is not 'or'");
	if(!IS_BINNODE(tree))
		err_exit_void("not binary 'or'");

	GnrtExpr(RIGHT(tree), n_var);
	GnrtExpr(LEFT(tree), n_var);

	print_asm(";or\n"
		  "pop rax ;lvalue\n"
		  "cmp rax, 0\n"
		  "jne .L%lu\n"
		  "pop rax ;rvalue\n"
		  "cmp rax, 0\n"
		  "jne .L%lu\n"
		  ";result:\n"
		  "push 0\n"
		  "jmp .L%lu\n"
		  ".L%lu:\n"
		  "push 1\n"
		  ".L%lu:\n\n",
		  LBL_CNT,
		  LBL_CNT,
		  LBL_CNT + 1,
		  LBL_CNT,
		  LBL_CNT + 1);
	LBL_CNT += 2;
}

static void GnrtAnd(const node_t *tree, const size_t n_var)
{
	LEAVE_IF_ERR;
	assert(tree);
	assert(ASM_OUT);
	if(!IS_(AND, &(tree->data)))
		err_exit_void("is not 'and'");
	if(!IS_BINNODE(tree))
		err_exit_void("not binary 'and'");

	GnrtExpr(RIGHT(tree), n_var);
	GnrtExpr(LEFT(tree), n_var);

	print_asm(";and\n"
			  "pop rax ;lvalue\n"
			  "cmp rax, 0\n"
			  "je .L%lu\n"
			  "pop rax ;rvalue\n"
			  "cmp rax, 0\n"
			  "je .L%lu\n"
			  ";result:\n"
			  "push 1\n"
			  "jmp .L%lu\n"
			  ".L%lu:\n"
			  "push 0\n"
			  ".L%lu:\n\n",
			  LBL_CNT,
			  LBL_CNT,
			  LBL_CNT + 1,
			  LBL_CNT,
			  LBL_CNT + 1);
	LBL_CNT += 2;
}

static void GnrtComp(const node_t *tree, const size_t n_var, const op_t gle)
{
	LEAVE_IF_ERR;
	assert(tree);
	assert(ASM_OUT);
	
	if(!IS_BINNODE(tree))
		err_exit_void("node is not binary");
	
	GnrtExpr(RIGHT(tree), n_var);
	GnrtExpr(LEFT(tree), n_var);

	const char *jmp_type = NULL;
	switch(gle)
	{
	case OP_GREATER:
		jmp_type = "ja";
		break;
	case OP_LESS:
		jmp_type = "jb";
		break;
	case OP_EQ:
		jmp_type = "je";
		break;
	case OP_ADD:
	case OP_SUB:
	case OP_MUL:
	case OP_DIV:
	case OP_ASSIGN:
	case OP_OR:
	case OP_AND:
	default:
		err_exit_void("jmptype out of range");
	}

	print_asm(";compare (..%s..)\n"
			  "pop rax ;lvalue\n"
			  "pop rdx ;rvalue\n"
			  "cmp rax, rdx\n"
			  "%s .L%lu\n"
			  "push 0\n"
			  "jmp .L%lu\n"
			  ".L%lu:\n"
			  "push 1\n"
			  ".L%lu:\n\n",
			  OP_NAME[(int)gle],
			  jmp_type, LBL_CNT,
			  LBL_CNT + 1,
			  LBL_CNT,
			  LBL_CNT + 1);
	LBL_CNT += 2;
}

static void GnrtExpr(const node_t *tree, const size_t n_var)
{
	LEAVE_IF_ERR;
	assert(tree);
	assert(ASM_OUT);
		
	switch(tree->data.type)
	{
	case TP_NUM:
	case TP_VAR:
		GnrtArifm(tree, n_var);
		break;
	case TP_CALL_FUNC:
		GnrtCallFunc(tree, n_var);
		break;
	case TP_OP:
		switch(tree->data.val.op)
		{
		case OP_ADD:
		case OP_SUB:
		case OP_MUL:
		case OP_DIV:
			GnrtArifm(tree, n_var);
			break;
		case OP_AND:
			GnrtAnd(tree, n_var);
			break;
		case OP_OR:
			GnrtOr(tree, n_var);
			break;
		case OP_EQ:
			GnrtComp(tree, n_var, OP_EQ);
			break;
		case OP_GREATER:
			GnrtComp(tree, n_var, OP_GREATER);
			break;
		case OP_LESS:
			GnrtComp(tree, n_var, OP_LESS);
			break;
		case OP_ASSIGN:
		default:
			err_exit_void("invalid operation");
		}
		break;
	case TP_EOF:
	case TP_ROOT:
	case TP_OP_SEQ:
	case TP_IDENT:
	case TP_PARAM:			/* cannot be expression */
	case TP_KWORD:
	case TP_SYMB:
	case TP_DECL_FUNC:
	case TP_LITERAL:
	default:
		err_exit_void("type out of range");
	}
}

static void GnrtIf(const node_t *tree, const size_t n_var)
{
	LEAVE_IF_ERR;
	assert(tree);
	assert(ASM_OUT);
	if(!IS_(IF, &(tree->data)))
		err_exit_void("is not 'if'");
	if(!IS_BINNODE(tree))
		err_exit_void("is not binary");

	GnrtExpr(LEFT(tree), n_var), n_var;
	size_t if_lbl = LBL_CNT++;
	print_asm(";if condition\n"
			  "pop rax\n"
			  "cmp rax, 0\n"
			  "je .L%lu\n"
			  ";if body\n",
			  if_lbl);

	GnrtOpSeq(RIGHT(tree), n_var);

	print_asm(";end if\n"
		  ".L%lu:\n\n",
		  if_lbl);
}

static void GnrtWhile(const node_t *tree, const size_t n_var)
{
	LEAVE_IF_ERR;
	assert(tree);
	assert(ASM_OUT);
	if(!IS_(WHILE, &(tree->data)))
		err_exit_void("is not 'while'");
	if(!IS_BINNODE(tree))
		err_exit_void("is not binary");

	GnrtExpr(LEFT(tree), n_var);

	size_t cond_lbl = LBL_CNT++, end_lbl = LBL_CNT++;
	print_asm(";while condition\n"
			  ".L%lu:\n"
			  "pop rax\n"
			  "cmp rax, 0\n"
			  "je .L%lu\n"
			  ";while body\n",
			  cond_lbl, end_lbl);

	GnrtOpSeq(RIGHT(tree), n_var);

	print_asm("jmp .L%lu\n"
			  ";end while\n"
			  ".L%lu:\n\n",
			  cond_lbl, end_lbl);
}

static void GnrtAssign(const node_t *tree, const size_t n_var)
{
	LEAVE_IF_ERR;
	assert(tree);
	assert(ASM_OUT);
	if(!IS_(ASSIGN, &(tree->data)))
		err_exit_void("is not 'assignment'");
	if(!IS_BINNODE(tree))
		err_exit_void("is not binary");
	if(LEFT(tree)->data.type != TP_VAR)
		err_exit_void("lvalue must be variable");

	GnrtExpr(RIGHT(tree), n_var);

	print_asm("mov rpb, rsp\n"
			  "add rbp, %lu ;calculate var pos in stack\n"
			  "pop [rpb] ;assign var a value\n\n",
			  LEFT(tree)->data.val.id);
}

static void GnrtCallFunc(const node_t *tree, const size_t n_var)
{
	LEAVE_IF_ERR;
	assert(tree);
	assert(ASM_OUT);
	if(tree->data.type != TP_CALL_FUNC)
		err_exit_void("is not a function call");
	if(tree->child == NULL || tree->child->node == NULL)
		err_exit_void("call node without parameters node");

	print_asm(";call function\n"
			  "mov rbp, rsp\n"
			  "add rbp, %lu ;shift sp\n",
			  n_var);

	child_t *param = tree->child->node->child;
	while(param)
	{
		if(param->node == NULL)
			err_exit_void("non-existing param");

		print_asm("mov [rbp], %ld ;load parameter\n"
				  "add rbp, 1\n",
				  param->node->data.val.num);

		param = param->next;
	}

	print_asm("call %s\n"
			  "sub rbp, %lu ;shift back\n\n",
			  tree->data.val.name, n_var);
}

/* counts number of variables in tree */
static void MaxID(const node_t *tree, size_t *max_id)
{
	assert(max_id);
	if (tree == NULL)
		return;
	
	if(tree->data.type == TP_VAR && tree->data.val.id > *max_id)
		*max_id = tree->data.val.id;

	if (tree->child)
	{
		child_t *child = tree->child;
		while (child)
		{
			MaxID(child->node, max_id);
			child = child->next;
		}
	}
}

static size_t GetMaxID(const node_t *tree)
{
	assert(tree);

	size_t max_id = 0;
	MaxID(tree, &max_id);

	return max_id;
}

static void GnrtReturn(const node_t *tree, const size_t n_var)
{
	LEAVE_IF_ERR;
	assert(tree);
	if(!IS_(RETURN, &(tree->data)))
		err_exit_void("is not a 'return'");
	
	if(tree->child && tree->child->node)
		GnrtExpr(tree->child->node, n_var);
	
	print_asm("ret\n");
}
