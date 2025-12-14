#include "NCC.h"

/* some ideas */

/*
Parse ::= Op+ 'EOF'
Op ::= Assign ';' | If | '{' Op+ '}' | ';'
If ::= 'if' '(' LongLogExpr ')' Op {'else' Op}
While ::= 'while' '(' LongLogExpr ')' Op {'else' Op}
//For ::= Var '~' ['[''('] Var | Num [']'')'] Op {'else' Op}
Asm ::= 'asm' '(' '"'.'"' ')'';'

Assign ::= Var '=' LongLogExpr
LongLogExpr ::= LogExpr{['and''or']LogExpr}*
LogExpr ::= Expr{['<''>''==']Expr}
Expr ::= Temp{['+''-']Temp}*
Temp ::= Prior{['*''/']Prior}*
Prior ::= '(' LongLogExpr ')' | Num | Var
Num ::= ['0'-'9']+
Var ::= ['A'-'Z', 'a'-'z', '0'-'9', '_']+
*/

/*
while(...)
{

} else
{

}

for i ~ (1, 10]
{
	
}

do
{

} while(...)

do
{

} for i ~ [0, 10)

asm("mov rax, rbx");



*/


static node_t *NewBinNode(const node_data_t data, node_t *l_val, node_t *r_val)
{
	node_t *eq = NewNode(data);
	AddChild(eq, l_val);
	AddChild(eq, r_val);

	return eq;
}



/*-------------------------------------------*/
static node_t *GetOp(node_data_t *data[]);
//static node_t *GetIf(node_data_t *data[]);
static node_t *GetWhileIf(node_data_t *data[], const node_data_t while_or_if);
static node_t *GetAsm(node_data_t *data[]);
static node_t *GetAssign(node_data_t *data[]);
static node_t *GetLongLogExpr(node_data_t *data[]);
static node_t *GetLogExpr(node_data_t *data[]);
static node_t *GetExpr(node_data_t *data[]);
static node_t *GetTemp(node_data_t *data[]);
static node_t *GetPrior(node_data_t *data[]);
static node_t *GetNum(node_data_t *data[]);
static node_t *GetVar(node_data_t *data[]);
/*-------------------------------------------*/

#include "ShortNamesDef.h"

node_t *Parse(toks_t *toks)
{
	if(toks == NULL)
	{
		print_err_msg("nullptr passed as tokens");
		return NULL;
	}

	node_t *node = NewNode(ROOT);
	assert(node);

	node_data_t *start_data = toks->data;
	node_t *new_node = GetOp(&(toks->data));
	if(new_node == NULL)
	{
		print_err_msg("Compilation failed");
		free(node);
		return NULL;
	}


	do
	{
		AddChild(node, new_node);
		new_node = GetOp(&(toks->data));
	} while (new_node);

	if(toks->data->type == TP_EOF)
		return node;

	print_err_msg("syntax error:");
	fprintf(stderr, colorize("\tcompile stopped here:", _BOLD_ _YELLOW_));
	PrintToks(toks->data, stderr);
	free(node);
	return NULL;
}

static node_t *GetOp(node_data_t *data[])
{
	assert(data);
	assert(*data);

	node_t *node = NewNode(OP_SEQ);
	assert(node);
	node_t *new_node = NULL;

	if((new_node = GetAssign(data)) && IS_(SEMICOLON, data))
	{
		AddChild(node, new_node);
	}
	else if((new_node = GetWhileIf(data, IF)) || (new_node = GetWhileIf(data, WHILE)))
	{
		AddChild(node, new_node);
	}
	else if(IS_(OPN_BRC, data))
	{
		*(data)++;

		new_node = GetOp(data);
		if(new_node == NULL)
		{
			print_err_msg("syntax error: -->");
			PrintToks(*data, stderr);
			return NULL;
		}

		do
		{
			AddChild(node, new_node);
			node = GetOp(data);
		} while (node);
		
		if(!IS_(CLS_BRC, data))
		{
			print_err_msg("missing '}': -->");
			PrintToks(*data, stderr);
			return NULL;
		}
	}
	else if(IS_(SEMICOLON, data))
	{
		(*data)++;
	}
	else
	{
		print_err_msg("syntax error: -->");
		PrintToks(*data, stderr);
		return NULL;
	}

	return node;
}

static node_t *GetWhileIf(node_data_t *data[], const node_data_t while_or_if)
{
	assert(data);
	assert(*data);

	node_t *node = NewNode(while_or_if);
	assert(node);
	node_t *new_node = NULL;

	if(IS_(while_or_if, data))
	{
		(*data)++;
		if(IS_(OPN_PAR, data))
		{
			(*data)++;
			if((new_node = GetLongLogExpr(data)) && IS_(CLS_PAR, data))
			{
				(*data)++;
				AddChild(node, new_node);
				if(new_node = GetOp(data))
				{
					AddChild(node, new_node);
				}
				else
					print_err_msg("missing operator");
			}
			else
				print_err_msg("wrong condition");
		}
		else
			print_err_msg("missing '('");
	}
	else
		return NULL;

	return node;
}

static node_t *GetAssign(node_data_t *data[])
{
	assert(data);
	assert(*data);

	node_t *node = NewNode(ASSIGN);
	assert(node);
	node_t *new_node = NULL;
	
	if(new_node = GetVar(data))
	{
		AddChild(node, new_node);
		if(IS_(ASSIGN, data))
		{
			(*data)++;
			if(new_node = GetLongLogExpr(data))
			{
				AddChild(node, new_node);
			}
			else
				print_err_msg("missing expression");
		}
		else
			print_err_msg("missing assign");
	}
	else
		return NULL;

	return node;
}

static node_t *GetLongLogExpr(node_data_t *data[])
{
	assert(data);
	assert(*data);
	
	node_t *node=
}
