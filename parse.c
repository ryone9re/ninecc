#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Var	*locals;

// ローカル変数を名前で探す
Var	*find_lvar(Token *tok)
{
	for (Var *var = locals; var; var = var->next)
	{
		if (var->len == tok->len && !memcmp(var->name, tok->str, var->len))
			return (var);
	}
	return (NULL);
}

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

static Function	*function(void);
static Var		*params(void);
static Node		*stmt(void);
static Node		*expr(void);
static Node		*assign(void);
static Node		*equality(void);
static Node		*relational(void);
static Node		*add(void);
static Node		*mul(void);
static Node		*unary(void);
static Node		*primary(void);
static Node		*args(void);

Function	*program(void)
{
	Function	prog = {};
	Function	*cur = &prog;

	while (!at_eof())
	{
		cur->next = function();
		cur = cur->next;
	}
	return (prog.next);
}

Function	*function(void)
{
	locals = NULL;

	// 関数名宣言
	Token	*funcdec = consume_ident();
	if (!funcdec)
		error("関数宣言が不正です");

	// 仮引数宣言
	expect("(");
	if (!consume(")"))
	{
		locals = params();
		while (consume(","))
			locals = params();
		expect(")");
	}

	expect("{");

	Node	head = {};
	Node	*cur = &head;

	while (!consume("}"))
	{
		cur->next = stmt();
		cur = cur->next;
	}

	Function	*func = (Function *)calloc(1, sizeof(Function));
	if (!func)
		exit_with_error();
	func->name = substr(funcdec->str, funcdec->len);
	func->node = head.next;
	func->locals = locals;
	return (func);
}

static Var	*params(void)
{
	Token	*paramdec = consume_ident();
	if (!paramdec)
		error("仮引数宣言が不正です");

	Var	*params = (Var *)calloc(1, sizeof(Var));
	if (!params)
		exit_with_error();

	params->next = locals;
	params->name = substr(paramdec->str, paramdec->len);
	params->len = paramdec->len;
	if (locals)
		params->offset = locals->offset + 8;
	else
		params->offset = 8;

	return (params);
}

static Node	*stmt(void)
{
	Node	*node;

	if (consume("return"))
	{
		node = new_node(ND_RETURN, expr(), NULL);
		expect(";");
		return (node);
	}

	if (consume("if"))
	{
		expect("(");
		node = new_node(ND_IF, NULL, NULL);
		node->cond = expr();
		expect(")");
		node->then = stmt();
		if (consume("else"))
			node->els = stmt();
		return (node);
	}

	if (consume("while"))
	{
		expect("(");
		node = new_node(ND_WHILE, NULL, NULL);
		node->cond = expr();
		expect(")");
		node->then = stmt();
		return (node);
	}

	if (consume("for"))
	{
		expect("(");
		node = new_node(ND_FOR, NULL, NULL);
		if (!consume(";"))
		{
			node->init = expr();
			expect(";");
		}
		if (!consume(";"))
		{
			node->cond = expr();
			expect(";");
		}
		if (!consume(")"))
		{
			node->inc = expr();
			expect(")");
		}
		node->then = stmt();
		return (node);
	}

	if (consume("{"))
	{
		Node	head = {};
		Node	*cur = &head;

		while (!consume("}"))
		{
			cur->next = stmt();
			cur = cur->next;
		}

		node = new_node(ND_BLOCK, NULL, NULL);
		node->body = head.next;
		return (node);
	}

	node = expr();
	expect(";");
	return (node);
}

static Node	*expr(void)
{
	return (assign());
}

static Node	*assign(void)
{
	Node	*node = equality();

	if (consume("="))
		node = new_node(ND_ASSIGN, node, assign());
	return (node);
}

static Node	*equality(void)
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

static Node	*relational(void)
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

static Node	*add(void)
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

static Node	*mul(void)
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

static Node	*unary(void)
{
	if (consume("+"))
		return (unary());
	else if (consume("-"))
		return (new_node(ND_SUB, new_node_number(0), unary()));
	return primary();
}

static Node	*primary(void)
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

		if (consume("("))
		{
			node->kind = ND_FUNCALL;
			char	*s = substr(tok->str, tok->len);
			if (!s)
				exit_with_error();
			node->funcname = s;
			node->args = args();
			return (node);
		}

		node->kind = ND_VAR;
		Var	*lvar = find_lvar(tok);
		if (lvar)
			node->offset = lvar->offset;
		else
		{
			lvar = (Var *)calloc(1, sizeof(Var));
			if (!lvar)
				exit_with_error();
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

static Node	*args(void)
{
	if (consume(")"))
		return (NULL);

	Node	*head = assign();
	Node	*cur = head;
	while (consume(","))
	{
		cur->next = assign();
		cur = cur->next;
	}
	expect(")");
	return (head);
}
