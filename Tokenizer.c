#include "NCC.h"

/*-------------------------------------------*/
static void TokRealloc(toks_t *toks);
static int SkipComments(const char **s);
static int Lexem(toks_t *toks, const char **s);
static int Number(toks_t *toks, const char **s);
static int Ident(toks_t *toks, const char **s);
static int Literal(toks_t *toks, const char **s);
/*-------------------------------------------*/

static void TokRealloc(toks_t *toks)
{
	assert(toks);
	
	if(toks->size + 1 >= toks->cap)
	{
		toks->cap = 2 * (toks->size + 1);
		toks->data = (node_data_t *)reallocarray(toks->data, toks->cap, sizeof(node_data_t));
		if(toks->data == NULL)
		{
			print_err_msg("tokens overflow");
			abort();
		}
	}
}

static int SkipComments(const char **s)
{
	assert(s);
	assert(*s);

	const char *old_s = *s;
	
	while (isspace(**s))
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

	if(*s == old_s)
		return 0;

	return 1;
}

static int Lexem(toks_t *toks, const char **s)
{
	assert(toks);
	assert(s);
	assert(*s);

	for (size_t i = 0; i < N_LEXS; i++)
	{
		if(strncmp(LEXS[i].name, *s, strlen(LEXS[i].name)) == 0)
		{
			toks->data[toks->size++] = LEXS[i].data;
			(*s) += strlen(LEXS[i].name);
			return 1;
		}
	}

	return 0;
}

static int Number(toks_t *toks, const char **s)
{
	assert(toks);
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

	toks->data[toks->size++] = (const node_data_t){.type = TP_NUM, .val.num = num};

	return 1;
}

static int Ident(toks_t *toks, const char **s)
{
	assert(toks);
	assert(s);
	assert(*s);
	
	if(isdigit(**s))
		return 0;

	int var_len = 0;
	node_data_t data = {.type = TP_IDENT};
	if (sscanf(*s, "%m[A-Za-z0-9_]%n", &(data.val.name), &var_len) > 0)
	{
		(*s) += var_len;
		toks->data[toks->size++] = data;

		return 1;
	}

	free(data.val.name);
	return 0;
}

static int Literal(toks_t *toks, const char **s)
{
	assert(toks);
	assert(s);
	assert(*s);
	
	if(**s == '"')
	{
		(*s)++;
		
		node_data_t data = (const node_data_t){.type = TP_LITERAL};
		int lit_len = 0;
		if (sscanf(*s, "%m[^\"]%n", &(data.val.name), &lit_len) > 0)
		{
			(*s) += lit_len;
			if(**s != '"')
			{
				print_err_msg("missing '\"'");
				print_wrong_s(*s - lit_len);
				free(data.val.name);
				
				return 0;
			}

			(*s)++;

			toks->data[toks->size++] = data;
			return 1;
		}
	}

	return 0;
}

toks_t *Tokenize(const char *s)
{
	assert(s);

	toks_t *toks = (toks_t *)calloc(1, sizeof(toks_t));
	assert(toks);

	while(*s > 0)
	{
		TokRealloc(toks);

		if (SkipComments(&s));
		else if(Number(toks, &s));
		else if(Lexem(toks, &s));
		else if(Ident(toks, &s));
		else if(Literal(toks, &s));
		else
		{
			print_err_msg("syntax error");
			fprintf(stderr, colorize("-->", _BOLD_ _YELLOW_) colorize("%.20s...\n\n", _CYAN_), s);
			return NULL;
		}
	}
	TokRealloc(toks);
	toks->data[toks->size++] = (const node_data_t){.type = TP_EOF};

	return toks;
}

void ToksDestroy(toks_t *toks)
{
	if(toks == NULL)
		return;

	for (size_t i = 0; i < toks->size; i++)
	{
		if(toks->data[i].type == TP_IDENT || toks->data[i].type == TP_LITERAL)
		{
			free(toks->data[i].val.name);
			toks->data[i].val.name = NULL;
		}
	}

	free(toks->data);
	toks->data = NULL;
	toks->size = toks->cap = 0;
	free(toks); /* dubiously, but ok */
}

