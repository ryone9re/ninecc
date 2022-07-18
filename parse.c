#include "9cc.h"
#include <stdlib.h>

// 2項演算子のNodeを作成
static Node	*new_node(NodeKind kind, Node *lhs, Node *rhs)
{
	Node	*node = (Node *)calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return (node);
}

// 単項演算子(整数値)のNodeを作成
static Node	*new_node_number(int val)
{
	Node	*node = (Node *)calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return (node);
}

static Node	*stmt();
static Node	*expr();
static Node	*assign();
static Node	*equality();
static Node	*relational();
static Node	*add();
static Node	*mul();
static Node	*unary();
static Node	*primary();

void	program()
{
	int	i = 0;

	while (!at_eof() && i < 100)
		code[i++] = stmt();
	code[i] = NULL;
}

static Node	*stmt()
{
	Node	*node;

	if (token->kind == TK_RETURN)
	{
		token = token->next;
		node = (Node *)calloc(1, sizeof(Node));
		node->kind = ND_RETURN;
		node->lhs = expr();
	}
	else
	{
		node = expr();
	}
	expect(";");
	return (node);
}

static Node	*expr()
{
	return (assign());
}

static Node	*assign()
{
	Node	*node = equality();

	if (consume("="))
		node = new_node(ND_ASSIGN, node, assign());
	return (node);
}

static Node	*equality()
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

static Node	*relational()
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

static Node	*add()
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

static Node	*mul()
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

static Node	*unary()
{
	if (consume("+"))
		return primary();
	else if (consume("-"))
		return (new_node(ND_SUB, new_node_number(0), primary()));
	return primary();
}

static Node	*primary()
{
	if (consume("("))
	{
		Node	*node = expr();
		consume(")");
		return (node);
	}

	Token	*tok = consume_ident();
	if (tok)
	{
		Node	*node = (Node *)calloc(1, sizeof(Node));
		node->kind = ND_LVAR;

		LVar	*lvar = find_lvar(tok);
		if (lvar)
			node->offset = lvar->offset;
		else
		{
			lvar = (LVar *)calloc(1, sizeof(LVar));
			lvar->next = locals;
			lvar->name = tok->str;
			lvar->len = tok->len;
			if (locals)
				lvar->offset = locals->offset + 8;
			else
				lvar->offset = 8;
			node->offset = lvar->offset;
			locals = lvar;
		}
		return (node);
	}

	return (new_node_number(expect_number()));
}
