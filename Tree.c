#include "NCC.h"

static node_t *NodeCopy(const node_t *node, size_t *call_count)
{
	assert(call_count);
	
	if (node == NULL || (*call_count)++ > MAX_REC_DEPTH)
		return NULL;	
	/*--------------------------------------------------*/
	
	node_t *new_node = (node_t *)calloc(1, sizeof(node_t));
	
	new_node->data.type = node->data.type;
	new_node->data.val = node->data.val;
	
	if(node->left)
	{
		new_node->left = NodeCopy(node->left, call_count);
		assert(new_node->left);
		new_node->left->prev = new_node;
	}
	
	if(node->right)
	{
		new_node->right = NodeCopy(node->right, call_count);
		assert(new_node->right);
		new_node->right->prev = new_node;
	}
	

	return new_node;
}

static node_t *_FindNode(node_t *tree, const node_data_t data, size_t *call_count)
{
	assert(call_count);
	
	if (tree == NULL || (*call_count)++ > MAX_REC_DEPTH)
		return NULL;

	if(memcmp(&(tree->data), &data, sizeof(node_data_t)) == 0)
		return tree;

	node_t *l_search = _FindNode(tree->left, data, call_count);
	if(l_search)
		return l_search;

	return _FindNode(tree->right, data, call_count);
}

static void _TreeDestroy(node_t *tree, size_t *call_count)
{
	assert(call_count);
	
	if (tree == NULL)
		return;

    if((*call_count)++ > MAX_REC_DEPTH)
        return;
	/*-----------------------------*/
    
	// if(tree->prev == NULL)
	// 	err = T_PARENT_NULLPTR;


	_TreeDestroy(tree->left, call_count);
	
	_TreeDestroy(tree->right, call_count);
	

	tree->data.type = T_NUM;
	tree->data.val.var = 0;
	tree->prev = NULL;
	tree->left = tree->right = NULL;
	
	free(tree);
}


void TieLeftToParent(node_t *node)
{
	assert(node);
	assert(node->left);
	assert(node->right);
	//assert(node->prev);

	node->data = node->left->data;

	if(node->prev == node || node->prev == NULL)
	{
		node_t *new_left = node->left->left;
		node_t *new_right = node->left->right;
		node_t *tying = node->left;

		node->left = new_left;
		if(new_left)
			new_left->prev = node;

		free(node->right);
		node->right = new_right;
		if(new_right)
			new_right->prev = node;

		free(tying);

		return;
	}
	else if(node->prev->left == node)
	{
		node->left->prev = node->prev;
		node->prev->left = node->left;
	}
	else if(node->prev->right == node)
	{
		node->left->prev = node->prev;
		node->prev->right = node->left;
	}
	else
	{
		print_err_msg("node->prev points to unknown");
		return;
	}

	free(node->right);
	free(node);
}

void TieRightToParent(node_t *node)
{
	assert(node);
	assert(node->left);
	assert(node->right);
	//assert(node->prev);
	
	node->data = node->right->data;

	if(node->prev == node || node->prev == NULL)
	{
		node_t *new_left = node->right->left;
		node_t *new_right = node->right->right;
		node_t *tying = node->right;

		free(node->left);
		node->left = new_left;
		if(new_left)
			new_left->prev = node;

		node->right = new_right;
		if(new_right)
			new_right->prev = node;

		free(tying);

		return;
	}
	else if(node->prev->left == node)
	{
		node->right->prev = node->prev;
		node->prev->left = node->right;
	}
	else if(node->prev->right == node)
	{
		node->right->prev = node->prev;
		node->prev->right = node->right;
	}
	else
	{
		print_err_msg("node->prev points to unknown");
		return;
	}

	free(node->left);
	free(node);
}

void AddLeaves(node_t *node)
{
	assert(node);
	
	node->left = (node_t *)calloc(1, sizeof(node_t));
	assert(node->left);
	
	node->right = (node_t *)calloc(1, sizeof(node_t));
	assert(node->right);

	node->left->prev = node->right->prev = node;
}


node_t *NewNode(const node_data_t data, node_t *left, node_t *right)
{
	// if(data.type >= N_OP)
	// 	return NULL;

	node_t *new_node = (node_t *)calloc(1, sizeof(node_t));
	assert(new_node);

	new_node->data = data;

	new_node->left = left;

	if(left)
		new_node->left->prev = new_node;
	new_node->right = right;
	if(right)
		new_node->right->prev = new_node;

	return new_node;
}





/* entrance to recursion */
node_t *TreeCopy(const node_t *tree)
{
	size_t call_count = 0;
	return NodeCopy(tree, &call_count);
}

node_t *FindNode(node_t *tree, const node_data_t data)
{
	size_t call_count = 0;
	return _FindNode(tree, data, &call_count);
}

void TreeDestroy(node_t *tree)
{
	size_t call_counts = 0;
	_TreeDestroy(tree, &call_counts);
}

void TokensDestroy(tokens_t *tokens)
{
	if(tokens == NULL)
		return;

	for (size_t i = 0; i < tokens->n_tok; i++)
	{
		if(tokens->tok[i].data.type == T_VAR)
		{
			free(tokens->tok[i].data.val.var);
			tokens->tok[i].data.val.var = NULL;
		}
	}

	free(tokens->tok);
	tokens->tok = NULL;
	free(tokens);
}
