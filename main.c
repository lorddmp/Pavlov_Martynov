#include "NCC.h"

static inline void PrintUsage(void)
{
	fprintf(stderr, colorize("Usage:\t", _BOLD_ _YELLOW_)
						colorize("ncc ", _BOLD_ _GREEN_) colorize("<file>\n", _BOLD_ _MAGENTA_)
						colorize("\t[-o <output filename>]\t", _BOLD_ _CYAN_)
						colorize("Default output filename is 'a.out'\n", _GREEN_)
						colorize("\t[--asm <asm filename>]\t", _BOLD_ _CYAN_)
						colorize("Will be produced if this flag typed\n", _GREEN_)
						colorize("\t\t[--help]\t", _BOLD_ _CYAN_)
						colorize("display this information\n\n", _GREEN_));
}

int main(int argc, char *argv[])
{
	char *in_filename = NULL;
	char *out_asm_filename = NULL;
	char *out_filename = NULL;
	int need_asm = 0;
	int compile_status = 0;

	for (int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-o") == 0)
		{
			if(++i < argc && out_filename == NULL)
				out_filename = strdup(argv[i]);
		}
		else if(strcmp(argv[i], "--asm") == 0)
		{
			need_asm = 1;
			if (++i < argc && out_asm_filename == NULL)
				out_asm_filename = strdup(argv[i]);
		}
		else if(strcmp(argv[i], "--help") == 0)
		{
			PrintUsage();
			goto exit;
		}
		else if (in_filename == NULL)
			in_filename = strdup(argv[i]);
	}
	
	if(in_filename == NULL)
	{
		PrintUsage();
		compile_status = 1;
		goto exit;
	}

	if(out_filename == NULL)
		out_filename = strdup("a.out");			/* default name */
	if(out_asm_filename == NULL)
		out_asm_filename = strdup("asm.out");	/* default name */

	compile_status = Compile(in_filename, out_filename, out_asm_filename, need_asm);

exit:
	free(out_asm_filename);
	free(out_filename);
	free(in_filename);

	return compile_status;
}

