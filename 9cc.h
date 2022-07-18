#ifndef NINECC_H
# define NINECC_H

// includes
#include <stdbool.h>
#include <stddef.h>

/* 構造体宣言 */
typedef struct Token	Token;
typedef struct Node		Node;
typedef struct LVar		LVar;

// トークンの種類
typedef enum
{
	TK_RESERVED,	// 記号
	TK_IDENT,		// 識別子
	TK_NUM,			// 整数トークン
	TK_EOF,			// 入力の終わりを示すトークン
}	TokenKind;

// トークン型
struct Token
{
	TokenKind	kind;	// トークンの型
	Token		*next;	// 次の入力トークン
	int			val;	// KindがTK_NUMの場合､その数値
	char		*str;	// トークン文字列
	size_t		len;	//トークンの長さ
};

// ローカル変数の型
struct LVar
{
	LVar	*next;	// 次の変数かNULL
	char	*name;	// 変数の名前
	size_t		len;	// 名前の長さ
	size_t		offset;	// RBPからのオフセット
};

// 抽象構文木のノードの種類
typedef enum
{
	ND_ADD,		// +
	ND_SUB,		// -
	ND_MUL,		// *
	ND_DIV,		// /
	ND_ASSIGN,	// =
	ND_LT,		// <
	ND_LTE,		// <=
	ND_GT,		// >
	ND_GTE,		// >=
	ND_EQ,		// ==
	ND_NEQ,		// !=
	ND_RETURN,	// return
	ND_IF,		// if
	ND_LVAR,	// ローカル変数
	ND_NUM,		// 整数
}	NodeKind;

// 抽象構文木のノードの型
struct Node
{
	Node		*next;	// 次のNode
	NodeKind	kind;	// ノードの型
	Node		*lhs;	// 左辺
	Node		*rhs;	// 右辺

	// if
	Node		*cond;
	Node		*then;
	Node		*els;

	size_t		val;	// kindがND_NUMの場合のみ使う
	size_t		offset;	// kindがoffsetの場合のみ使う
};

/* グローバル変数宣言 */

// 現在着目しているトークン
extern Token	*token;
// 入力プログラム
extern char	*user_input;
// 文を格納する配列
extern Node	*code[100];
// ローカル変数
extern LVar	*locals;

/* プロトタイプ宣言 */

/* tokenize.c */
void	error(char *fmt, ...);
void	error_at(char *loc, char *fmt, ...);
bool	consume(char *op);
Token	*consume_ident();
void	expect(char *op);
int		expect_number();
bool	at_eof();
int		is_tokstr(char c);
Token	*tokenize(void);

/* parse.c */
void	program(void);

/* codegen.c */
LVar	*find_lvar(Token *tok);
void	gen(Node *node);

#endif /* NINECC_H */
