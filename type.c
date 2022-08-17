#include "9cc.h"
#include <stdlib.h>

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
	case ND_EQ:
	case ND_NEQ:
	case ND_LT:
	case ND_LTE:
	case ND_GT:
	case ND_GTE:
	case ND_FUNCALL:
	case ND_NUM:
		node->type = new_type(TYPE_INT, NULL);
		return ;
	case ND_ADD:
		// 右オペランドにポインタがきてたら入れ替える
		if (node->rhs->type->kind == TYPE_PTR)
		{
			Node	*tmp = node->lhs;
			node->lhs = node->rhs;
			node->rhs = tmp;
		}
		// ポインタ同士の加算は不正
		if (node->rhs->type->kind == TYPE_PTR)
			error_at(node->tok->str, "ポインタ同士の演算は不正です");
		node->type = node->lhs->type;
		return ;
	case ND_SUB:
		if (node->rhs->type->kind == TYPE_PTR)
			error_at(node->rhs->tok->str, "ポインタの値で引くことはできません");
		node->type = node->lhs->type;
		return ;
	case ND_ASSIGN:
		node->type = node->lhs->type;
		return ;
	case ND_ADDR:
		node->type = new_type(TYPE_PTR, node->lhs->type);
		return ;
	case ND_DEREF:
		if (node->lhs->type->kind == TYPE_PTR)
			node->type = node->lhs->type->ptr_to;
		else
			node->type = new_type(TYPE_INT, NULL);
		return ;
	default:
		return ;
	}
}

void	add_type(Function *func)
{
	for (Function *f = func; f; f = f->next)
		for (Node *n = f->node; n; n = n->next)
			visit(n);
}
