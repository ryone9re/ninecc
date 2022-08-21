#include "9cc.h"
#include <stdlib.h>
#include <string.h>

// 型の宣言
Type	*new_type(TypeKind tk, Type *ptr_to)
{
	Type	*type;

	type = (Type *)calloc(1, sizeof(Type));
	if (!type)
		exit_with_error();
	type->kind = tk;
	type->ptr_to = ptr_to;
	return (type);
}

Type	*new_type_array(TypeKind tk, Type* ptr_to, size_t len)
{
	Type	*type = new_type(tk, ptr_to);
	type->array_len = len;
	return (type);
}

Type	*new_type_from_str(char *str)
{
	if (!strncmp(str, "char", strlen("char")))
		return (new_type(TYPE_CHAR, NULL));
	if (!strncmp(str, "int", strlen("int")))
		return (new_type(TYPE_INT, NULL));
	return (NULL);
}

static void	visit(Node *node)
{
	if (!node)
		return ;

	visit(node->lhs);
	visit(node->rhs);
	visit(node->cond);
	visit(node->then);
	visit(node->els);
	visit(node->init);
	visit(node->inc);

	for (Node *n = node->body; n; n = n->next)
		visit(n);
	for (Node *n = node->args; n; n = n->next)
		visit(n);

	switch (node->kind)
	{
	case ND_VAR:
		node->type = node->var->type;
		return ;
	case ND_MUL:
	case ND_DIV:
	case ND_MOD:
	case ND_LT:
	case ND_LTE:
	case ND_GT:
	case ND_GTE:
	case ND_EQ:
	case ND_NEQ:
	case ND_PRE_INC:
	case ND_PRE_DEC:
	case ND_POS_INC:
	case ND_POS_DEC:
	case ND_FUNCALL:
	case ND_NUM:
		node->type = new_type(TYPE_INT, NULL);
		return ;
	case ND_ADD:
		// 右オペランドにポインタがきてたら入れ替える
		if (node->rhs->type->ptr_to)
		{
			Node	*tmp = node->lhs;
			node->lhs = node->rhs;
			node->rhs = tmp;
		}
		// ポインタ同士の加算は不正
		if (node->rhs->type->ptr_to)
			error_at(node->tok->str, "ポインタ同士の演算は不正です");
		node->type = node->lhs->type;
		return ;
	case ND_SUB:
		if (node->rhs->type->ptr_to)
			error_at(node->rhs->tok->str, "ポインタの値で引くことはできません");
		node->type = node->lhs->type;
		return ;
	case ND_ASSIGN:
		node->type = node->lhs->type;
		return ;
	case ND_ADDR:
		if (node->lhs->type->kind == TYPE_ARRAY)
			node->type = new_type(TYPE_PTR, node->lhs->type->ptr_to);
		else
			node->type = new_type(TYPE_PTR, node->lhs->type);
		return ;
	case ND_DEREF:
		if (!node->lhs->type->ptr_to)
			error_at(node->tok->str, "参照先がありません");
		node->type = node->lhs->type->ptr_to;
		return ;
	case ND_SIZEOF:
		node->val = size_of(node->lhs->type);
		node->kind = ND_NUM;
		node->type = new_type(TYPE_INT, NULL);
		node->lhs = NULL;
		return ;
	default:
		return ;
	}
}

void	add_type(Program *prog)
{
	for (Function *f = prog->functions; f; f = f->next)
		for (Node *n = f->node; n; n = n->next)
			visit(n);
}
