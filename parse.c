#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

VarList	*locals;

// ローカル変数を名前で探す
Var	*find_lvar(Token *tok)
{
	for (VarList *vl = locals; vl; vl = vl->next)
	{
		if (strlen(vl->var->name) == tok->len && !strncmp(vl->var->name, tok->str, tok->len))
			return (vl->var);
	}
	return (NULL);
}

// 空Nodeを作成
static Node	*new_node(NodeKind kind, Token *tok)
{
	Node	*node = (Node *)calloc(1, sizeof(Node));
	if (!node)
		exit_with_error();
	node->kind = kind;
	node->tok = tok;
	return (node);
}

// 二項演算子Nodeを作成
static Node	*new_binary_node(NodeKind kind, Node *lhs, Node *rhs, Token *tok)
{
	Node	*node = new_node(kind, tok);
	if (!node)
		exit_with_error();
	node->lhs = lhs;
	node->rhs = rhs;
	return (node);
}

// 単項演算子Nodeを作成
static Node	*new_unary_node(NodeKind kind, Node *expr, Token *tok)
{
	Node	*node = new_node(kind, tok);
	if (!node)
		exit_with_error();
	node->lhs = expr;
	return (node);
}

// 整数値Nodeを作成
static Node	*new_number_node(int val, Token *tok)
{
	Node	*node = new_node(ND_NUM, tok);
	if (!node)
		exit_with_error();
	node->val = val;
	return (node);
}

// 変数Nodeを作成
static Node	*new_var_node(Var *var, Token *tok)
{
	Node	*node = new_node(ND_VAR, tok);
	if (!node)
		exit_with_error();
	node->var = var;
	return (node);
}

// 新規変数を作成
static Var	*new_lvar(char *name)
{
	Var	*var = (Var *)calloc(1, sizeof(Var));
	if (!var)
		exit_with_error();
	var->name = name;

	VarList	*vl = (VarList *)calloc(1, sizeof(VarList));
	if (!vl)
		exit_with_error();
	vl->var = var;
	vl->next = locals;
	locals = vl;
	return (var);
}

static Function	*function(void);
static VarList	*params(void);
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

	// 返り値の型
	expect_specified_ident("int");

	// 関数名宣言
	Token	*funcdec = consume_ident();
	if (!funcdec)
		error("関数宣言が不正です");

	// 関数生成
	Function	*func = (Function *)calloc(1, sizeof(Function));
	if (!func)
		exit_with_error();
	func->name = substr(funcdec->str, funcdec->len);

	// 仮引数宣言
	func->params = params();

	expect("{");

	Node	head = {};
	Node	*cur = &head;

	while (!consume("}"))
	{
		cur->next = stmt();
		cur = cur->next;
	}

	func->node = head.next;
	func->locals = locals;
	return (func);
}

static VarList	*params(void)
{
	expect("(");

	if (consume(")"))
		return (NULL);

	expect_specified_ident("int");

	VarList	*head = (VarList *)calloc(1, sizeof(VarList));
	if (!head)
		exit_with_error();
	VarList	*cur = head;
	cur->var = new_lvar(expect_ident());

	while (!consume(")"))
	{
		expect(",");
		expect_specified_ident("int");
		cur->next = (VarList *)calloc(1, sizeof(VarList));
		cur->next->var = new_lvar(expect_ident());
		cur = cur->next;
	}

	return (head);
}

static Node	*stmt(void)
{
	Node	*node;
	Token	*tok;

	if ((tok = consume("return")))
	{
		node = new_unary_node(ND_RETURN, expr(), tok);
		expect(";");
		return (node);
	}

	if ((tok = consume("if")))
	{
		expect("(");
		node = new_node(ND_IF, tok);
		node->cond = expr();
		expect(")");
		node->then = stmt();
		if (consume("else"))
			node->els = stmt();
		return (node);
	}

	if ((tok = consume("while")))
	{
		expect("(");
		node = new_node(ND_WHILE, tok);
		node->cond = expr();
		expect(")");
		node->then = stmt();
		return (node);
	}

	if ((tok = consume("for")))
	{
		expect("(");
		node = new_node(ND_FOR, tok);
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

	if ((tok = consume("{")))
	{
		Node	head = {};
		Node	*cur = &head;

		while (!consume("}"))
		{
			cur->next = stmt();
			cur = cur->next;
		}

		node = new_node(ND_BLOCK, tok);
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
	Token	*tok;

	if ((tok = consume("=")))
		node = new_binary_node(ND_ASSIGN, node, assign(), tok);
	return (node);
}

static Node	*equality(void)
{
	Node	*node = relational();
	Token	*tok;

	while (true)
	{
		if ((tok = consume("==")))
			node = new_binary_node(ND_EQ, node, relational(), tok);
		else if ((tok = consume("!=")))
			node = new_binary_node(ND_NEQ, node, relational(), tok);
		else
			return (node);
	}
}

static Node	*relational(void)
{
	Node	*node = add();
	Token	*tok;

	while (true)
	{
		if ((tok = consume("<=")))
			node = new_binary_node(ND_LTE, node, add(), tok);
		else if ((tok = consume(">=")))
			node = new_binary_node(ND_GTE, node, add(), tok);
		else if ((tok = consume("<")))
			node = new_binary_node(ND_LT, node, add(), tok);
		else if ((tok = consume(">")))
			node = new_binary_node(ND_GT, node, add(), tok);
		else
			return (node);
	}
}

static Node	*add(void)
{
	Node	*node = mul();
	Token	*tok;

	while (true)
	{
		if ((tok = consume("+")))
			node = new_binary_node(ND_ADD, node, mul(), tok);
		else if ((tok = consume("-")))
			node = new_binary_node(ND_SUB, node, mul(), tok);
		else
			return (node);
	}
}

static Node	*mul(void)
{
	Node	*node = unary();
	Token	*tok;

	while (true)
	{
		if ((tok = consume("*")))
			node = new_binary_node(ND_MUL, node, unary(), tok);
		else if ((tok = consume("/")))
			node = new_binary_node(ND_DIV, node, unary(), tok);
		else
			return (node);
	}
}

static Node	*unary(void)
{
	Token	*tok;

	if ((tok = consume("+")))
		return (unary());
	if ((tok = consume("-")))
		return (new_binary_node(ND_SUB, new_number_node(0, token), unary(), tok));
	if ((tok = consume("&")))
		return (new_unary_node(ND_ADDR, unary(), tok));
	if ((tok = consume("*")))
		return (new_unary_node(ND_DEREF, unary(), tok));
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
		// 関数呼び出し
		if (consume("("))
		{
			Node	*node = new_node(ND_FUNCALL, tok);
			node->funcname = substr(tok->str, tok->len);
			node->args = args();
			return (node);
		}

		// 変数宣言
		Token	*vardec = consume_ident();
		if (vardec)
		{
			new_lvar(substr(vardec->str, vardec->len));
			return (new_node(ND_NULL, tok));
		}

		Var	*lvar = find_lvar(tok);
		if (!lvar)
			error_at(tok->str, "未定義の変数です");
		return (new_var_node(lvar, tok));
	}

	return (new_number_node(expect_number(), token));
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
