#include "NCC.h"


int main(void)
{
	char *s = NULL;
	
	ReadFileToBuf("f.txt", &s);
	assert(s);
	
	toks_t *toks = Tokenize(s);
	assert(toks);
	assert(toks->data);

	PrintToks(toks, stdout);

	TokensDestroy(toks);
	free(s);

	return 0;
}