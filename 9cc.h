#ifndef NINECC_H
# define NINECC_H

// includes
#include <stdbool.h>
#include <stddef.h>

// 構造体宣言

typedef struct Token	Token;
typedef struct Node		Node;

// トークンの種類
typedef enum
{
	TK_RESERVED,	// 記号
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

// 抽象構文木のノードの型
struct Node
{
	NodeKind	kind;	// ノードの型
	Node		*lhs;	// 左辺
	Node		*rhs;	// 右辺
	int			val;	// kindがND_NUMの場合のみ使う
};

// グローバル変数宣言

// 現在着目しているトークン
extern Token	*token;
// 入力プログラム
extern char	*user_input;

// 関数宣言

// エラーを報告するための関数
// printfと同じ引数を取る
void	error(char *fmt, ...);

// エラー箇所を報告する
void	error_at(char *loc, char *fmt, ...);

// 次のトークンが期待している記号のときには､トークンを1つ読み進めて
// 真を返す｡それ以外の場合には偽を返す｡
bool	consume(char *op);

// 次のトークンが期待している記号のときには､トークンを1つ読み進める｡
// それ以外の場合にはエラーを報告する｡
void	expect(char *op);

// 次のトークンが数値の場合､トークンを1つ読み進めてその数値を返す｡
// それ以外の場合にはエラーを報告する｡
int	expect_number();

// 終端チェック
bool	at_eof();

// 新しいトークンを作成してcurに繋げる
Token	*new_token(TokenKind kind, Token *cur, char *str, size_t len);

// 入力文字列pをトークナイズしてそれを返す
Token	*tokenize(char *p);


// 2項演算子のNodeを作成
Node	*new_node(NodeKind kind, Node *lhs, Node *rhs);

// 整数値のNodeを作成
Node	*new_node_number(int val);

// 括弧のNodeを作成
Node	*primary();

// 単項演算子のNodeを作成
Node	*unary();

// 乗除算のNodeを作成
Node	*mul();

// 加減算のNodeを作成
Node	*add();

// 比較演算子のNodeを作成
Node	*relational();

// 等価演算子のNodeを作成
Node	*equality();

// 評価関数
Node	*expr();

// コード生成
void	gen(Node *node);

#endif /* NINECC_H */
