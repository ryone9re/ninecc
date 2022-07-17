#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum
{
	TK_RESERVED,	// 記号
	TK_NUM,			// 整数トークン
	TK_EOF,			// 入力の終わりを示すトークン
}	TokenKind;

typedef struct Token	Token;

// トークン型
struct Token
{
	TokenKind	kind;	// トークンの型
	Token		*next;	// 次の入力トークン
	int			val;	// KindがTK_NUMの場合､その数値
	char		*str;	// トークン文字列
	size_t		len;	//トークンの長さ
};

// 現在着目しているトークン
Token	*token;

// 入力プログラム
char	*user_input;

// エラーを報告するための関数
// printfと同じ引数を取る
void	error(char *fmt, ...)
{
	va_list	ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// エラー箇所を報告する
void	error_at(char *loc, char *fmt, ...)
{
	va_list	ap;
	va_start(ap, fmt);

	int	pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// 次のトークンが期待している記号のときには､トークンを1つ読み進めて
// 真を返す｡それ以外の場合には偽を返す｡
bool	consume(char *op)
{
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len) != 0)
		return (false);
	token = token->next;
	return (true);
}

// 次のトークンが期待している記号のときには､トークンを1つ読み進める｡
// それ以外の場合にはエラーを報告する｡
void	expect(char *op)
{
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len) != 0)
		error_at(token->str, "'%c'ではありません", op);
	token = token->next;
}

// 次のトークンが数値の場合､トークンを1つ読み進めてその数値を返す｡
// それ以外の場合にはエラーを報告する｡
int	expect_number()
{
	if (token->kind != TK_NUM)
	{
		error_at(token->str, "数ではありません");
		exit(1);
	}
	int	val = token->val;
	token = token->next;
	return (val);
}

// 終端チェック
bool	at_eof()
{
	return (token->kind == TK_EOF);
}

// 新しいトークンを作成してcurに繋げる
Token	*new_token(TokenKind kind, Token *cur, char *str, size_t len)
{
	Token	*tok = (Token *)calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return (tok);
}

// 入力文字列pをトークナイズしてそれを返す
Token	*tokenize(char *p)
{
	Token	head;
	head.next = NULL;
	Token	*cur = &head;

	while (*p)
	{
		// 空白文字をスキップ
		if (isspace(*p))
		{
			p++;
			continue ;
		}

		// 括弧
		if (*p == '(' || *p == ')')
		{
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue ;
		}

		// 単項演算子
		if (*p == '+' || *p == '-')
		{
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue ;
		}

		// 乗除算
		if (*p == '*' || *p == '/')
		{
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue ;
		}

		// 加減算
		if (*p == '+' || *p == '-')
		{
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue ;
		}

		// 比較演算子(2字)
		if (strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0)
		{
			cur = new_token(TK_RESERVED, cur, p++, 2);
			p++;
			continue ;
		}

		// 比較演算子(1字)
		if (*p == '<' || *p == '>')
		{
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue ;
		}

		// 比等価演算子
		if (strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0)
		{
			cur = new_token(TK_RESERVED, cur, p++, 2);
			p++;
			continue ;
		}

		if (isdigit(*p))
		{
			size_t	len = 0;
			while (isdigit(p[len]))
				len++;
			cur = new_token(TK_NUM, cur, p, len);
			cur->val = strtol(p, &p, 10);
			continue ;
		}

		error_at(p, "トークナイズできません");
	}

	new_token(TK_EOF, cur, p, 0);
	return (head.next);
}

// 抽象構文木のノードの種類
typedef enum
{
	ND_ADD,	// +
	ND_SUB,	// -
	ND_MUL,	// *
	ND_DIV,	// /
	ND_LT,	// <
	ND_LTE,	// <=
	ND_GT,	// >
	ND_GTE,	// >=
	ND_EQ,	// ==
	ND_NEQ,	// !=
	ND_NUM,	// 整数
}	NodeKind;

typedef struct Node	Node;

// 抽象構文木のノードの型
struct Node
{
	NodeKind	kind;	// ノードの型
	Node		*lhs;	// 左辺
	Node		*rhs;	// 右辺
	int			val;	// kindがND_NUMの場合のみ使う
};

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

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		error("引数の個数が正しくありません\n");
		return (1);
	}

	// トークナイズしてパースする
	user_input = argv[1];
	token = tokenize(user_input);
	Node *node = expr();

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n\n");
	printf("main:\n");

	// 抽象構文木を下りながらコード生成
	gen(node);

	// スタックトップに式全体の値が残っているはずなので
	// それをraxにロードして関数の戻り地とする
	printf("\tpop rax\n");
	printf("\tret\n");
	return (0);
}
