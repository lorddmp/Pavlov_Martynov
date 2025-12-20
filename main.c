#include "NCC.h"


int main(void)
{
	char *s = NULL;
	
	ReadFileToBuf("f.txt", &s);
	assert(s);
	
	toks_t *toks = Tokenize(s);
	assert(toks);
	assert(toks->data);

	PrintToks(toks->data, stdout);
	node_t *tree = Parse(toks);
	assert(tree);
	TreeDumpHTML(tree, "f.dot", "./Img", "f.html", "test");

	FILE *asm_out = fopen("f.asm", "w");
	assert(asm_out);

	CompileTree(tree, asm_out);

	fclose(asm_out);
	TreeDestroy(tree);
	ToksDestroy(toks);
	free(s);

	return 0;
}