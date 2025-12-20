#include "NCC.h"

#define SYS_MSG_SIZE 100

int Compile(const char *in_filename, const char *out_filename, const char *out_asm_filename, const int need_asm)
{
	assert(in_filename);
	assert(out_filename);
	assert(out_asm_filename);

	FILE *output_asm = NULL;
	char *code = NULL;
	toks_t *toks = NULL;
	node_t *tree = NULL;
	char sys_msg[SYS_MSG_SIZE] = "";

	snprintf(sys_msg, SYS_MSG_SIZE, "myasm %s -o %s", out_asm_filename, out_filename);

	output_asm = fopen(out_asm_filename, "w");
	if (output_asm == NULL)
	{
		print_err_msg("output file(s) cannot be open");
		goto err_exit;
	}

	code = NULL;
	if(ReadFileToBuf(in_filename, &code) == 0)
		goto err_exit;
	assert(code);

	toks = Tokenize(code);
	if(toks == NULL || toks->data == NULL)
		goto err_exit;

	tree = Parse(toks);
	if(tree == NULL)
		goto err_exit;

	//TreeDumpHTML(tree, "f.dot", "./Img", "f.html", "AST dump");
	
	if(CompileTree(tree, output_asm))
		goto err_exit;
	
	fclose(output_asm);
	output_asm = NULL;
	
	if (system(sys_msg))
		goto err_exit;

	/*---------------------*/
	free(code);
	ToksDestroy(toks);
	TreeDestroy(tree);
	if (!need_asm)
		remove(out_asm_filename);
	return 0;

err_exit:
	free(code);
	ToksDestroy(toks);
	TreeDestroy(tree);
	if(!need_asm)
		remove(out_filename);
	if(output_asm)
		fclose(output_asm);
	if (output_asm && !need_asm)
		remove(out_asm_filename);
	return 1;
}

#undef SYS_MSG_SIZE
