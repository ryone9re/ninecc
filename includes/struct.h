#ifndef STRUCT_H
# define STRUCT_H

#include "include.h"

/* トークンの種類 */
typedef enum
{
	TK_EOF,			// 入力の終わりを示すトークン
	TK_RESERVED,	// 予約語
	TK_IDENT,		// 識別子
	TK_NUM,			// 整数トークン
	TK_SIZEOF,		// sizeof演算子
	TK_STRING,		// 文字列リテラル
}	TokenKind;

/* トークン型 */
typedef struct Token	Token;
struct Token
{
	TokenKind	kind;	// トークンの型
	Token		*next;	// 次の入力トークン
	size_t		val;	// KindがTK_NUMの場合､その数値
	char		*str;	// トークン文字列
	size_t		len;	// トークンの長さ

	char		*ctx;	// 文字列リテラルの文字
	size_t		clen;	// 文字列リテラルの長さ
};

/* 型の種類 */
typedef enum
{
	TYPE_CHAR,	// 文字型
	TYPE_INT,	// 整数型
	TYPE_PTR,	// ポインタ型
	TYPE_ARRAY,	// 配列
}	TypeKind;

/* 型の構造 */
typedef struct Type Type;
struct Type
{
	TypeKind	kind;		// 型の種類
	Type		*ptr_to;	// ポインタ型の場合､その指し示す先
	size_t		array_len;	// 配列の長さ
};

/* 変数の型 */
typedef struct Var		Var;
struct Var
{
	Type		*type;		// 変数の型
	char		*name;		// 変数の名前
	bool		is_local;	// ローカル変数かどうか

	/* ローカル変数時 */
	size_t		offset;		// RBPからのオフセット

	/* 文字列リテラル */
	char		*ctx;		// 文字列リテラル
	size_t		clen;		// 文字列リテラルの長さ
};

/* 変数のリスト */
typedef struct VarList	VarList;
struct VarList
{
	VarList	*next;	// 次の変数
	Var		*var;	// 変数
};

/* 抽象構文木のノードの種類 */
typedef enum
{
	ND_NULL,	// 空
	ND_ADD,		// +
	ND_SUB,		// -
	ND_MUL,		// *
	ND_DIV,		// /
	ND_MOD,		// %
	ND_SIZEOF,	// sizeof
	ND_ASSIGN,	// =
	ND_LT,		// <
	ND_LTE,		// <=
	ND_GT,		// >
	ND_GTE,		// >=
	ND_EQ,		// ==
	ND_NEQ,		// !=
	ND_ADDR,	// アドレス演算子(&)
	ND_DEREF,	// 間接演算子(*)
	ND_RETURN,	// return
	ND_IF,		// if
	ND_WHILE,	// while
	ND_FOR,		// for
	ND_BLOCK,	// {...}
	ND_FUNCALL,	// 関数呼び出し
	ND_VAR,		// 変数
	ND_NUM,		// 整数
}	NodeKind;

/* 抽象構文木のノードの型 */
typedef struct Node		Node;
struct Node
{
	NodeKind	kind;	// ノードの型
	Token		*tok;	// ノードに対応するトークン
	Node		*next;	// 次のNode
	Type		*type;	// ノードの型

	Node		*lhs;	// 左辺
	Node		*rhs;	// 右辺

	// if, while, for
	Node		*cond;
	Node		*then;
	Node		*els;
	Node		*init;
	Node		*inc;

	// Block
	Node		*body;

	// 関数呼び出し
	char		*funcname;	// 関数名
	Node		*args;		// 実引数

	Var			*var;		// kindがND_VARの場合のみ使う
	int			val;		// kindがND_NUMの場合のみ使う
};

typedef struct Function	Function;
struct Function
{
	Function	*next;
	char		*name;
	Node		*node;
	VarList		*params;
	VarList		*locals;
	size_t		stack_size;
};

typedef struct Program	Program;
struct Program
{
	Function	*functions;
	VarList		*globals;
};

#endif /* STRUCT_H */
