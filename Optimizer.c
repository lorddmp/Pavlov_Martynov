#include "NCC.h"

static size_t _FoldConst(node_t *tree, size_t *call_count)
{
	assert(call_count);

	if(tree == NULL || (*call_count)++ > MAX_REC_DEPTH)
		return 0;

	double calc = 0;
	size_t n_fold = 0;

	switch(tree->op.type)
	{
	case OP_NUM:
		return 0;
	case OP_ARIFM:
		n_fold += _FoldConst(tree->left, call_count);
		n_fold += _FoldConst(tree->right, call_count);

		calc = Calc(tree, NULL);
		if(!isnan(calc) && !isinf(calc))
		{
			n_fold++;
			tree->op.type = OP_NUM;
			tree->op.val.num = calc;
			
			TreeDestroy(tree->left);
			TreeDestroy(tree->right);
			tree->left = tree->right = NULL;
		}
		
		break;
	case OP_ELFUNC:
		n_fold += _FoldConst(tree->right, call_count);
		
		break;
	
	
	case OP_VAR:
	case N_OP:
	default:
		break;
	}

	return n_fold;
}

static size_t _FoldNeutral(node_t *tree, size_t *call_count)
{
	assert(call_count);
	
	if (tree == NULL || (*call_count)++ > MAX_REC_DEPTH || tree->left == NULL || tree->right == NULL)
		return 0;
	
	
	/*-----------------------------------------*/
	size_t n_fold = 0;
	
	n_fold += _FoldNeutral(tree->left, call_count);
	n_fold += _FoldNeutral(tree->right, call_count);
	
	if(tree->op.type != OP_ARIFM)
		return n_fold;
	
	if(tree->left == NULL || tree->right == NULL)
	{
		print_err_msg("operation, but not binary...");
		return 0;
	}

	switch(tree->op.val.arifm)
	{
	case AR_SUB:
		if(NodeNumEqTo(0, tree->left))
		{
			tree->op.val.arifm = AR_MUL;
			tree->left->op.val.num = -1;
			return ++n_fold;
		}
	case AR_ADD:
		if (NodeNumEqTo(0, tree->left))
		{
			TieRightToParent(tree);
			n_fold++;
		}
		else if(NodeNumEqTo(0, tree->right))
		{
			TieLeftToParent(tree);
			n_fold++;
		}
		
		break;
	case AR_MUL:
		if(NodeNumEqTo(0, tree->left) || NodeNumEqTo(0, tree->right))
		{
			tree->op.type = OP_NUM;
			tree->op.val.num = 0;
			
			TreeDestroy(tree->left);
			TreeDestroy(tree->right);
			tree->left = tree->right = NULL;
			
			n_fold++;
		}
		else if(NodeNumEqTo(1, tree->left))
		{
			TieRightToParent(tree);
			n_fold++;
		}
		else if(NodeNumEqTo(1, tree->right))
		{
			TieLeftToParent(tree);
			n_fold++;
		}
		
		break;
	case AR_DIV:
		if(NodeNumEqTo(0, tree->left))
		{
			tree->op.type = OP_NUM;
			tree->op.val.num = 0;
			
			TreeDestroy(tree->left);
			TreeDestroy(tree->right);
			tree->left = tree->right = NULL;
			
			n_fold++;
		}
		else if(NodeNumEqTo(1, tree->right))
		{
			TieLeftToParent(tree);
			n_fold++;
		}
		
		break;
	case AR_POW:
		if(NodeNumEqTo(1, tree->right))
		{
			TieLeftToParent(tree);
			n_fold++;
		}
		else if(NodeNumEqTo(0, tree->right) || NodeNumEqTo(1, tree->left))
		{
			tree->op.type = OP_NUM;
			tree->op.val.num = 1;
			
			TreeDestroy(tree->left);
			TreeDestroy(tree->right);
			tree->left = tree->right = NULL;
			
			n_fold++;
		}
		else if(NodeNumEqTo(0, tree->left))
		{
			tree->op.type = OP_NUM;
			tree->op.val.num = 0;
			
			TreeDestroy(tree->left);
			TreeDestroy(tree->right);
			tree->left = tree->right = NULL;
			
			n_fold++;
		}
		
		break;
	default:
		print_err_msg("tree wrong data (hint: op.val.arifm is out of range arifm_t)");
		break;
	}

	return n_fold;
}

/* entrance to recursion */
size_t FoldConst(node_t *tree)
{
	size_t call_count = 0;
	return _FoldConst(tree, &call_count);
}

size_t FoldNeutral(node_t *tree)
{
	size_t call_count = 0;
	return _FoldNeutral(tree, &call_count);
}


