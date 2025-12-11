#include "NCC.h"

static void TokensRealloc(tokens_t *tokens, const size_t need_n)
{
	assert(tokens);
	
	// if(tokens->n_tok == 0)
	// {
	// 	tokens->tok = (node_t *)calloc(need_n, sizeof(node_t));
	// 	assert(tokens->tok);

	// 	tokens->n_tok = need_n;
	// }
	if(tokens->n_tok <= need_n)
	{
		tokens->tok = (node_t *)reallocarray(tokens->tok, need_n * 2, sizeof(node_t));
		assert(tokens->tok);

		tokens->n_tok = need_n * 2;
	}
}

static int SkipComments(const char **s)
{
	assert(s);
	assert(*s);

	while(isspace(**s))
			(*s)++;
		
	if(strncmp(*s, "/*", 2) == 0)
	{
		(*s) += 2;
		while(**s && strncmp(*s, "*/", 2))
		{
			(*s)++;
			while (**s && **s != '*')
				(*s)++;
		}
		
		if(**s)
			(*s) += 2;
		else
			return 0;
	}
	else
		return 0;

	return 1;
}

static int Lexem(node_t *new_node, const char **s)
{
	assert(new_node);
	assert(s);
	assert(*s);

	for (size_t i = 0; i < N_LEXS; i++)
	{
		if(strncmp(LEXS[i].name, *s, strlen(LEXS[i].name)) == 0)
		{
			new_node->data = LEXS[i].data;
			(*s) += strlen(LEXS[i].name);
			return 1;
		}
	}

	return 0;
}

static int Number(node_t *new_node, const char **s)
{
	assert(new_node);
	assert(s);
	assert(*s);

	const char *old_s = *s;
	char *end_s = NULL;
	long num = strtol(*s, &end_s, 10);

	if(isalpha(*end_s) || *end_s == '_')
		return 0;
	
	(*s) += end_s - old_s;

	if(*s == old_s)
		return 0;

	new_node->data = (const node_data_t){.type = T_NUM, .val.num = num};

	return 1;
}

static int Ident(node_t *new_node, const char **s)
{
	assert(new_node);
	assert(s);
	assert(*s);
	
	if(isdigit(**s))
		return 0;

	int var_len = 0;
	node_data_t data = {.type = T_VAR};
	if (sscanf(*s, "%m[A-Za-z0-9_]%n", &(data.val.var), &var_len) > 0)
	{
		(*s) += var_len;
		new_node->data = data;

		return 1;
	}

	return 0;
}

tokens_t *Tokenize(const char *s)
{
	assert(s);

	tokens_t *tokens = (tokens_t *)calloc(1, sizeof(tokens_t));
	tokens->tok = (node_t *)calloc(1, sizeof(node_t));
	assert(tokens->tok);
	tokens->n_tok = 1;

	size_t n_tok = 0;
	while(*s > 0)
	{
		TokensRealloc(tokens, n_tok + 1);

		if (SkipComments(&s))
			;
		else if(Lexem(&(tokens->tok[n_tok]), &s))
			n_tok++;
		else if(Number(&(tokens->tok[n_tok]), &s))
			n_tok++;
		else if(Ident(&(tokens->tok[n_tok]), &s))
			n_tok++;
		else
		{
			print_err_msg("syntax error");
			fprintf(stderr, colorize("-->", _BOLD_ _YELLOW_) colorize("%20s\n\n", _CYAN_), s);
			return NULL;
		}
	}

	tokens->n_tok = n_tok;

	return tokens;
}

