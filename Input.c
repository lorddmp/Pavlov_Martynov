#include "NCC.h"


long ReadFileToBuf(const char *file_path, char **buf)
{
    if(file_path == NULL || buf == NULL)
		return -1;

	FILE *file = fopen(file_path, "r");
    if(file == NULL)
    {
		print_err_msg("input file cannot be open");
		return -1;
	}

    struct stat file_info = {};
    if(stat(file_path, &file_info))
        return -1;
    
    long buf_size = file_info.st_size;

    *buf = (char *)calloc((size_t)buf_size + 2, sizeof(char));
	if(*buf == NULL)
	{
		print_err_msg("buffer overflow");
		return -1;
	}

	if(fread(*buf, sizeof(char), (size_t)buf_size, file) != (size_t)buf_size)
        return -1;

    fclose(file);

    (*buf)[buf_size] = '\0';
    (*buf)[buf_size + 1] = EOF;

    return buf_size;
}
/*--------------------------------------------------*/



#include "DSLdef.h"

#include "DSLundef.h"
#undef EXISTS_OR_LEAVE