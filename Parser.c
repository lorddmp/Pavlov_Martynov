#include "NCC.h"

/* some ideas */

/*
Parse ::= DeclFunc+ 'EOF'
Op ::= Assign ';' | If | While | Asm ';' | DeclFunc | CallFunc ';' | '{' Op+ '}' | ';'

DeclFunc ::= 'func' '.' '(' {Var ','}* ')' Op
CallFunc ::= . '(' {{Num | Var | CallFunc}','}* ')'

If ::= 'if' '(' OrExpr ')' Op {'else' Op}
While ::= 'while' '(' OrExpr ')' Op {'else' Op}
//For ::= Var '~' ['[''('] Var | Num [']'')'] Op {'else' Op}
Asm ::= 'asm' '(' '"'.'"' ')'

Assign ::= Var '=' OrExpr

OrExpr ::= AndExpr{['or']AndExpr}*
AndExpr ::= CompExpr{['and']CompExpr}*
CompExpr ::= Expr{['<''>''==']Expr}
Expr ::= Temp{['+''-']Temp}*
Temp ::= Prim{['*''/']Prim}*
Prim ::= '(' OrExpr ')' | Num | Var
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

static nametbl_t *TblInit(void)
{
	nametbl_t *nametbl = (nametbl_t *)calloc(1, sizeof(nametbl_t));
	assert(nametbl);

	return nametbl;
}

static size_t TblAddName(const char *name, nametbl_t *nametbl)
{
	assert(name);
	assert(nametbl);
	
	if(nametbl->size + 1 >= nametbl->cap)
	{
		nametbl->cap = 2 * (nametbl->size + 1);
		nametbl->cell = (cell_t *)reallocarray(nametbl->cell, nametbl->cap, sizeof(cell_t));
		if(nametbl->cell == NULL)
		{
			print_err_msg("nametable overflow");
			abort();
		}
	}

	nametbl->cell[nametbl->size].name = name;
	
	if(nametbl->size)
		nametbl->cell[nametbl->size].id = nametbl->cell[nametbl->size - 1].id + 1;
	else
		nametbl->cell[0].id = 0;

	return nametbl->size++;
}

static size_t TblGetID(const char *name, nametbl_t *nametbl)
{
	assert(name);
	assert(nametbl);

	for (size_t i = 0; i < nametbl->size; i++)
		if(strcmp(name, nametbl->cell[i].name) == 0)
			return nametbl->cell[i].id;

	return TblAddName(name, nametbl);
}

static void TblDestroy(nametbl_t *nametbl)
{
	if(nametbl == NULL)
		return;

	free(nametbl->cell);
	nametbl->cell = NULL;
	nametbl->size = nametbl->cap = 0;

	free(nametbl);
}

static node_t *NewBinNode(const node_data_t data, node_t *l_val, node_t *r_val)
{
	node_t *eq = NewNode(data);
	AddChild(eq, l_val);
	AddChild(eq, r_val);

	return eq;
}



/*-------------------------------------------*/
static node_t *GetOp(node_data_t *data[], nametbl_t *nametbl);

static node_t *GetDeclFunc(node_data_t *data[]);
static node_t *GetCallFunc(node_data_t *data[], nametbl_t *nametbl);

static node_t *GetReturn(node_data_t *data[], nametbl_t *nametbl);

static node_t *GetWhileIf(node_data_t *data[], nametbl_t *nametbl, const node_data_t while_or_if);
static node_t *GetAsm(node_data_t *data[]);

static node_t *GetAssign(node_data_t *data[], nametbl_t *nametbl);

//static node_t *GetLogExpr(node_data_t *data[], nametbl_t *nametbl);
static node_t *GetOrExpr(node_data_t *data[], nametbl_t *nametbl);
static node_t *GetAndExpr(node_data_t *data[], nametbl_t *nametbl);
static node_t *GetCompExpr(node_data_t *data[], nametbl_t *nametbl);

static node_t *GetExpr(node_data_t *data[], nametbl_t *nametbl);
static node_t *GetTemp(node_data_t *data[], nametbl_t *nametbl);
static node_t *GetPrim(node_data_t *data[], nametbl_t *nametbl);
static node_t *GetNum(node_data_t *data[]);
static node_t *GetVar(node_data_t *data[], nametbl_t *nametbl);
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

	node_data_t *data = toks->data;
	node_t *new_node = GetDeclFunc(&(data));
	if(new_node == NULL)
	{
		print_err_msg("Compilation failed");
		TreeDestroy(node);
		return NULL;
	}

	do
	{
		AddChild(node, new_node);
		new_node = GetDeclFunc(&(data));
	} while (new_node);

	if(data->type == TP_EOF)
		return node;

	print_err_msg("syntax error:");
	fprintf(stderr, colorize("\tcompile stopped here:", _BOLD_ _YELLOW_));
	PrintToks(data, stderr);
	TreeDestroy(node);
	return NULL;
}

static node_t *GetDeclFunc(node_data_t *data[])
{
	assert(data);
	assert(*data);

	node_t *node = NULL, *arg_node = NULL;
	nametbl_t *nametbl = NULL;

	if(IS_(FUNC, *data))
	{
		(*data)++;
		if((**data).type == TP_IDENT && IS_(OPN_PAR, *data + 1))
		{
			//node = NewBinNode(FUNC_DECL((**data).val.name), NewNode(PARAM), NULL);
			
			node = NewNode(FUNC_DECL((**data).val.name));
			AddChild(node, NewNode(PARAM));
			nametbl = TblInit();
			(*data) += 2;
			
			while((**data).type == TP_IDENT)
			{
				arg_node = GetVar(data, nametbl);
				if(arg_node == NULL)
				{
					print_err_msg("invalid arguments");
					goto err_exit;
				}
				
				AddChild(node->child->node, arg_node);
				
				if(IS_(COMMA, *data))
					(*data)++;
				else
					break;
			}
			
			if (IS_(CLS_PAR, *data))
			{
				(*data)++;
				//break;
			}
			else
			{
				print_err_msg("missing ',' or ')'");
				goto err_exit;
			}

			arg_node = GetOp(data, nametbl);
			if(arg_node == NULL || arg_node->data.type != TP_OP_SEQ)
			{
				print_err_msg("invalid body");
				TreeDestroy(arg_node);
				goto err_exit;
			}

			AddChild(node, arg_node);
		}
		else
		{
			print_err_msg("missing identifier");
			goto err_exit;
		}
	}

	TblDestroy(nametbl);
	return node;
err_exit:
	PrintToks(*data, stderr);
	TreeDestroy(node);
	
	TblDestroy(nametbl);
	return NULL;
}

static node_t *GetCallFunc(node_data_t *data[], nametbl_t *nametbl)
{
	assert(data);
	assert(*data);
	assert(nametbl);

	node_t *node = NULL, *arg_node = NULL;

	if((**data).type == TP_IDENT && IS_(OPN_PAR, *data + 1))
	{
		node = NewNode(FUNC_CALL((**data).val.name));
		AddChild(node, NewNode(PARAM));
		(*data) += 2;
		
		
		while(arg_node = GetOrExpr(data, nametbl))
		{
			AddChild(node->child->node, arg_node);
			if(IS_(COMMA, *data))
				(*data)++;
			else
				break;
		}
		if(IS_(CLS_PAR, *data))
			(*data)++;
		else
		{
			print_err_msg("missing ',' or ')'");
			TreeDestroy(node);
			//TreeDestroy(arg_node);
			return NULL;
		}
	}
	else
		return NULL;

	return node;
}

static node_t *GetOp(node_data_t *data[], nametbl_t *nametbl)
{
	assert(data);
	assert(*data);
	assert(nametbl);

	//node_t *node = NewNode(OP_SEQ);
	//assert(node);
	node_t *node = NULL;
	node_t *new_node = NULL;

	if((new_node = GetCallFunc(data, nametbl)) && IS_(SEMICOLON, *data))
	{
		(*data)++;
		node = new_node;
	}
	else if((new_node = GetAssign(data, nametbl)) && IS_(SEMICOLON, *data))
	{
		(*data)++;
		//AddChild(node, new_node);
		node = new_node;
	}
	else if((new_node = GetReturn(data, nametbl)) && IS_(SEMICOLON, *data))
	{
		(*data)++;
		node = new_node;
	}
	else if((new_node = GetWhileIf(data, nametbl, IF)) || (new_node = GetWhileIf(data, nametbl, WHILE)))
	{
		//AddChild(node, new_node);
		node = new_node;
	}
	else if((new_node = GetAsm(data)) && IS_(SEMICOLON, *data))
	{
		(*data)++;
		node = new_node;
	}
	else if(IS_(OPN_BRC, *data))
	{
		(*data)++;
		node = NewNode(OP_SEQ);
		assert(node);

		new_node = GetOp(data, nametbl);
		if(new_node == NULL)
		{
			print_err_msg("syntax error: -->");
			PrintToks(*data, stderr);
			TreeDestroy(node);
			return NULL;
		}

		do
		{
			AddChild(node, new_node);
			new_node = GetOp(data, nametbl);
		} while (new_node);
		
		if(IS_(CLS_BRC, *data))
		{
			(*data)++;
		}
		else
		{
			print_err_msg("missing '}': -->");
			PrintToks(*data, stderr);
			TreeDestroy(node);
			return NULL;
		}
	}
	else if(IS_(SEMICOLON, *data))
	{
		do
			(*data)++;
		while (IS_(SEMICOLON, *data));
	}
	// else
	// {
	// 	//print_err_msg("syntax error: -->");
	// 	//PrintToks(*data, stderr);
	// 	//TreeDestroy(node);
	// 	return NULL;
	// }

	return node;
}

static node_t *GetReturn(node_data_t *data[], nametbl_t *nametbl)
{
	assert(data);
	assert(*data);
	assert(nametbl);

	node_t *node = NULL, *arg_node = NULL;

	if(IS_(RETURN, *data))
	{
		(*data)++;
		arg_node = GetOrExpr(data, nametbl);
		if(arg_node == NULL)
		{
			print_err_msg("missing expression");
			return NULL;
		}

		node = NewNode(RETURN);
		AddChild(node, arg_node);
	}

	return node;
}

static node_t *GetWhileIf(node_data_t *data[], nametbl_t *nametbl, const node_data_t while_or_if)
{
	assert(data);
	assert(*data);
	assert(nametbl);

	node_t *node = NewNode(while_or_if);
	assert(node);
	node_t *new_node = NULL;

	if(IS_(while_or_if, *data))
	{
		(*data)++;
		if(IS_(OPN_PAR, *data))
		{
			(*data)++;
			if((new_node = GetOrExpr(data, nametbl)) && IS_(CLS_PAR, *data))
			{
				(*data)++;
				AddChild(node, new_node);
				if(new_node = GetOp(data, nametbl))
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
	{
		TreeDestroy(node);
		return NULL;
	}

	return node;
}

static node_t *GetAsm(node_data_t *data[])
{
	assert(data);
	assert(*data);

	node_t *node = NULL;
	if (IS_(ASM, *data))
	{
		(*data)++;
		if(IS_(OPN_PAR, *data))
		{
			(*data)++;
			if((**data).type == TP_LITERAL)
			{
				node = NewNode(ASM);
				AddChild(node, NewNode(**data));
				(*data)++;
			}
			else
				print_err_msg("argument isn't a literal");
		}
		else
			print_err_msg("missing '('");
		
		if(IS_(CLS_PAR, *data))
			(*data)++;
		else
		{
			print_err_msg("missing ')'");
			TreeDestroy(node);
			node = NULL;
		}
	}

	return node;
}

static node_t *GetAssign(node_data_t *data[], nametbl_t *nametbl)
{
	assert(data);
	assert(*data);
	assert(nametbl);

	node_t *node = NewNode(ASSIGN);
	assert(node);
	node_t *new_node = NULL;
	
	if(new_node = GetVar(data, nametbl))
	{
		AddChild(node, new_node);
		if(IS_(ASSIGN, *data))
		{
			(*data)++;
			if(new_node = GetOrExpr(data, nametbl))
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
	{
		TreeDestroy(node);
		return NULL;
	}

	return node;
}

static node_t *GetOrExpr(node_data_t *data[], nametbl_t *nametbl)
{
	assert(data);
	assert(*data);
	assert(nametbl);

	node_t *node = GetAndExpr(data, nametbl), *new_node = NULL, *arg_node = NULL;
	if(node == NULL)
		return NULL;
	
	while(IS_(OR, *data))
	{
		// new_node = NewNode(OR);
		// assert(new_node);
		(*data)++;

		arg_node = GetAndExpr(data, nametbl);
		if(arg_node == NULL)
		{
			print_err_msg("missing expression");
			TreeDestroy(node);
			//TreeDestroy(new_node);
			return NULL;
		}

		new_node = NewBinNode(OR, node, arg_node);
		node = new_node;
	}

	return node;
}

static node_t *GetAndExpr(node_data_t *data[], nametbl_t *nametbl)
{
	assert(data);
	assert(*data);
	assert(nametbl);

	node_t *node = GetCompExpr(data, nametbl), *new_node = NULL, *arg_node = NULL;
	if(node == NULL)
		return NULL;
	
	while(IS_(AND, *data))
	{
		// new_node = NewNode(OR);
		// assert(new_node);
		(*data)++;

		arg_node = GetCompExpr(data, nametbl);
		if(arg_node == NULL)
		{
			print_err_msg("missing expression");
			TreeDestroy(node);
			//TreeDestroy(new_node);
			return NULL;
		}

		// new_node = NewBinNode(AND, node, arg_node);
		// node = new_node;
		node = NewBinNode(AND, node, arg_node);
	}

	return node;
}

static node_t *GetCompExpr(node_data_t *data[], nametbl_t *nametbl)
{
	assert(data);
	assert(*data);
	assert(nametbl);

	node_t *node = GetExpr(data, nametbl), *new_node = NULL, *arg_node = NULL;
	
	
	if(IS_(GREATER, *data) || IS_(LESS, *data) || IS_(EQ, *data))
	{
		node_data_t op = **data;
		(*data)++;

		arg_node = GetExpr(data, nametbl);
		if(arg_node == NULL)
		{
			print_err_msg("missing expression");
			TreeDestroy(node);
			//TreeDestroy(new_node);
			return NULL;
		}

		node = NewBinNode(op, node, arg_node);
	}

	return node;
}

static node_t *GetExpr(node_data_t *data[], nametbl_t *nametbl)
{
	assert(data);
	assert(*data);
	assert(nametbl);

	node_t *new_node = NULL, *arg_node = NULL;
	node_t *node = GetTemp(data, nametbl);
	if(node == NULL)
		return NULL;
	
	while(IS_(ADD, *data) || IS_(SUB, *data))
	{
		new_node = NewNode(**data);
		assert(new_node);
		(*data)++;

		AddChild(new_node, node);
		arg_node = GetTemp(data, nametbl);
		if(arg_node == NULL)
		{
			TreeDestroy(node);
			TreeDestroy(new_node);
			return NULL;
		}

		AddChild(new_node, arg_node);
		node = new_node;
	}

	return node;
}

static node_t *GetTemp(node_data_t *data[], nametbl_t *nametbl)
{
	assert(data);
	assert(*data);
	assert(nametbl);

	node_t *new_node = NULL, *arg_node = NULL;
	node_t *node = GetPrim(data, nametbl);
	if(node == NULL)
		return NULL;
	
	while(IS_(MUL, *data) || IS_(DIV, *data))
	{
		new_node = NewNode(**data);
		assert(new_node);
		(*data)++;

		AddChild(new_node, node);
		arg_node = GetPrim(data, nametbl);
		if(arg_node == NULL)
		{
			TreeDestroy(node);
			TreeDestroy(new_node);
			return NULL;
		}

		AddChild(new_node, arg_node);
		node = new_node;
	}

	return node;
}

static node_t *GetPrim(node_data_t *data[], nametbl_t *nametbl)
{
	assert(data);
	assert(*data);
	assert(nametbl);

	node_t *node = NULL;

	if(IS_(OPN_PAR, *data))
	{
		(*data)++;
		if(node = GetOrExpr(data, nametbl))
		{
			if(IS_(CLS_PAR, *data))
			{
				(*data)++;
			}
			else
				print_err_msg("missing ')'");
		}
		else
			print_err_msg("missing expression");
	}
	else if(node = GetCallFunc(data, nametbl));
	else if(node = GetNum(data));
	else
		node = GetVar(data, nametbl);

	return node;
}

static node_t *GetNum(node_data_t *data[])
{
	assert(data);
	assert(*data);
	
	if((**data).type == TP_NUM)
	{
		node_t *node = NewNode(**data);
		(*data)++;
		return node;
	}

	return NULL;
}

static node_t *GetVar(node_data_t *data[], nametbl_t *nametbl)
{
	assert(data);
	assert(*data);
	
	if((**data).type == TP_IDENT)
	{
		node_t *node = NewNode(VAR(TblGetID((**data).val.name, nametbl)));
		
		(*data)++;
		return node;
	}

	return NULL;
}

