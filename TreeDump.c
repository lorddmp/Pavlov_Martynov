#include "NCC.h"


static void PrintNodeData(const node_data_t data, FILE *out_file)
{
	assert(out_file);

	const char *data_s = NULL;
	long data_num = 0;

	switch(data.type)
	{
	case TP_EOF:
		data_s = "EOF";
		break;
	case TP_KWORD:
		data_s = KWORD_NAME[(int)data.val.kword];
		break;
	case TP_NUM:
		data_s = NULL;
		data_num = data.val.num;
		break;
	case TP_OP:
		data_s = OP_NAME[(int)data.val.op];
		break;
	case TP_SYMB:
		data_s = SYMB_NAME[(int)data.val.symb];
		break;
	case TP_VAR:
		data_s = NULL;
		data_num = (long)data.val.var;
		break;
	case TP_FUNC:
	case TP_LITERAL:
		data_s = data.val.name;
	default:
		data_s = "??";
		break;
	}
	
	if(data_s)
		fprintf(out_file, "%s", data_s);
	else
		fprintf(out_file, "%ld", data_num);
}

static void PrintDigraphNode(const node_t *node, FILE *dot_file, size_t *call_count)
{
	assert(call_count);
	assert(dot_file);
	assert(node);

	if ((*call_count)++ > MAX_REC_DEPTH)
	{
		print_err_msg("tree overflow");
		return;
	}
	
	/*------------------------------------*/

	fprintf(dot_file, "label%lu[shape=record, style=\"rounded, filled\", fillcolor=\"#a8daf0ff\", label=\"{ node[%p] | type = %s | val = ",
			(size_t)node, node, NODE_TYPE_NAME[node->data.type]);
	PrintNodeData(node->data, dot_file);
	fprintf(dot_file, " | prnt[%p]}\"];\n", node->parent);

	// if(node->left)
	// {
	// 	if(node->left->parent == node)
	// 		fprintf(dot_file, "label%lu->label%lu [color=purple, dir=both]\n", (size_t)node, (size_t)node->left);
	// 	else
	// 		fprintf(dot_file, "label%lu->label%lu [color=red]\n", (size_t)node, (size_t)node->left);
		
	// 	err = PrintDigraphNode(node->left, dot_file, call_count);
	// }
	
	if(node->child)
	{
		child_t *child = node->child;
		while (child)
		{
			if(child->node && child->node->parent == node)
				fprintf(dot_file, "label%lu->label%lu [color=purple, dir=both]\n", (size_t)node, (size_t)child->node);
			else
				fprintf(dot_file, "label%lu->label%lu [color=red]\n", (size_t)node, (size_t)child->node);

			PrintDigraphNode(child, dot_file, call_count);
			child = child->next;
		}
	}
}

static void PrintDigraphTree(const node_t *tree, FILE *dot_file)
{
	assert(tree);
	assert(dot_file);
	
	size_t call_count = 0;
	PrintDigraphNode(tree, dot_file, &call_count);
}

static void CreateDigraph(const node_t *tree, const char *dot_file_path)
{
    assert(dot_file_path);
	assert(tree);

	/*--------------------------------------*/

    FILE *dot_file = fopen(dot_file_path, "w");
	assert(dot_file);

	fprintf(dot_file, "digraph AST\n{\n");
	PrintDigraphTree(tree, dot_file);
	putc('}', dot_file);
	
	fclose(dot_file);
}


void TreeDumpHTML(const node_t *tree, const char *dot_file_path, const char *img_dir_path, const char *html_file_path, const char *caption)
{	
	if(dot_file_path == NULL || html_file_path == NULL || img_dir_path == NULL || caption == NULL)
	{
		print_err_msg("passed nullptr arguments");
		return;
	}

	static size_t call_count = 0;
    FILE *html_file = NULL;

    if(call_count == 0)
		remove(html_file_path);

	html_file = fopen(html_file_path, "a+");
	assert(html_file);

	CreateDigraph(tree, dot_file_path);

	char system_msg[100] = "";
    snprintf(system_msg, 100, "dot %s -Tsvg -o %s/%lu.svg\n", dot_file_path, img_dir_path, call_count);

    if(system(system_msg))
	{
		print_err_msg("build dot failed");
		goto exit;
	}

	fprintf(html_file, "<pre>\n\t<h2>%s</h2>\n", caption);
    fprintf(html_file, "\t<img  src=\"%s/%lu.svg\" alt=\"%s\" width=\"1200px\"/>\n", img_dir_path, call_count, caption);
    fprintf(html_file, "\t<hr>\n</pre>\n\n");
    
exit:
	fclose(html_file);
    call_count++;
}
