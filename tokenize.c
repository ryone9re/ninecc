#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// 次のトークンがidentifierのときには､トークンを1つ読み進めて
// そのトークンを返す｡それ以外の場合にはNULLを返す｡
Token	*consume_ident()
{
	if (token->kind == TK_IDENT)
	{
		Token	*tok = token;
		token = token->next;
		return (tok);
	}
	return (NULL);
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

// トークンを構成する文字か判定
int	is_tokstr(char c)
{
	return (isalnum(c) || c == '_');
}

// 新しいトークンを作成してcurに繋げる
static Token	*new_token(TokenKind kind, Token *cur, char *str, size_t len)
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

		// return文
		if (strncmp(p, "return", 6) == 0 && !is_tokstr(p[6]))
		{
			cur = new_token(TK_RETURN, cur, p, 6);
			p = p + 6;
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

		// 代入
		if (strncmp(p, "=", 1) == 0)
		{
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue ;
		}

		// 文終了
		if (strncmp(p, ";", 1) == 0)
		{
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue ;
		}

		// 識別子
		if (isalpha(*p) || *p == '_')
		{
			size_t	len = 0;
			while (is_tokstr(p[len]))
				len++;
			cur = new_token(TK_IDENT, cur, p, len);
			p = p + len;
			continue ;
		}

		// 数値
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
