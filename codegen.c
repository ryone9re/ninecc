#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>

// 2項演算子のNodeを作成
Node	*new_node(NodeKind kind, Node *lhs, Node *rhs)
{
	Node	*node = (Node *)calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return (node);
}

// 単項演算子(整数値)のNodeを作成
Node	*new_node_number(int val)
{
	Node	*node = (Node *)calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return (node);
}

Node	*expr();

Node	*primary()
{
	if (consume("("))
	{
		Node	*node = expr();
		consume(")");
		return (node);
	}

	return (new_node_number(expect_number()));
}

Node	*unary()
{
	if (consume("+"))
		return primary();
	else if (consume("-"))
		return (new_node(ND_SUB, new_node_number(0), primary()));
	return primary();
}

Node	*mul()
{
	Node	*node = unary();

	while (true)
	{
		if (consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else
			return (node);
	}
}

Node	*add()
{
	Node	*node = mul();

	while (true)
	{
		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else
			return (node);
	}
}

Node	*relational()
{
	Node	*node = add();

	while (true)
	{
		if (consume("<="))
			node = new_node(ND_LTE, node, add());
		else if (consume(">="))
			node = new_node(ND_GTE, node, add());
		else if (consume("<"))
			node = new_node(ND_LT, node, add());
		else if (consume(">"))
			node = new_node(ND_GT, node, add());
		else
			return (node);
	}
}

Node	*equality()
{
	Node	*node = relational();

	while (true)
	{
		if (consume("=="))
			node = new_node(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_node(ND_NEQ, node, relational());
		else
			return (node);
	}
}

Node	*expr()
{
	return (equality());
}

void	gen(Node *node)
{
	if (node->kind == ND_NUM)
	{
		printf("\tpush %d\n", node->val);
		return ;
	}

	gen(node->lhs);
	gen(node->rhs);

	printf("\tpop rdi\n");
	printf("\tpop rax\n");

	switch (node->kind)
	{
		case ND_ADD:
			printf("\tadd rax, rdi\n");
			break ;
		case ND_SUB:
			printf("\tsub rax, rdi\n");
			break ;
		case ND_MUL:
			printf("\timul rax, rdi\n");
			break ;
		case ND_DIV:
			printf("\tcqo\n");
			printf("\tidiv rdi\n");
			break ;
		case ND_LT:
		case ND_GT:
		case ND_LTE:
		case ND_GTE:
		case ND_EQ:
		case ND_NEQ:
			if (node->kind == ND_LT || node->kind == ND_LTE ||
				node->kind == ND_EQ || node->kind == ND_NEQ)
				printf("\tcmp rax, rdi\n");
			else
				printf("\tcmp rdi, rax\n");
			if (node->kind == ND_LT || node->kind == ND_GT)
				printf("\tsetl al\n");
			else if (node->kind == ND_LTE || node->kind == ND_GTE)
				printf("\tsetle al\n");
			else if (node->kind == ND_EQ)
				printf("\tsete al\n");
			else
				printf("\tsetne al\n");
			printf("\tmovzb rax, al\n");
			break ;
		default:
			break ;
	}

	printf("\tpush rax\n");
}
