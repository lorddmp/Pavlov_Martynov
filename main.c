#include "NCC.h"


int main(void)
{
	char *s = NULL;
	
	ReadFileToBuf("f.txt", &s);
	assert(s);
	tokens_t *tokens = Tokenize(s);
	assert(tokens);
	assert(tokens->tok);

	char *tok_name = NULL;
	for (size_t i = 0; i < tokens->n_tok; i++)
	{
		switch(tokens->tok[i].data.type)
		{
		case T_KWORD:
			tok_name = KWORD_NAME[tokens->tok[i].data.val.kword];
			break;
		case T_NUM:
			tok_name = NULL;
			printf("[%lu].data = %ld\n", i, tokens->tok[i].data.val.num);
			break;
		case T_OP:
			tok_name = OP_NAME[tokens->tok[i].data.val.op];
			break;
		case T_SYMB:
			tok_name = SYMB_NAME[tokens->tok[i].data.val.symb];
			break;
		case T_VAR:
			tok_name = tokens->tok[i].data.val.var;
			break;
		default:
			tok_name = NULL;
			printf("[%lu].data = ??\n", i);
			break;
		}
		
		if(tok_name)
			printf("[%lu].data = {%s}\n", i, tok_name);
	}

	TokensDestroy(tokens);
	free(s);

	return 0;
}