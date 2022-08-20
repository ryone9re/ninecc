#include "9cc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// エラー箇所を報告する
void	error_at(char *loc, char *fmt, ...)
{
	va_list	ap;
	va_start(ap, fmt);

	char	*line = loc;
	while (user_input < line && line[-1] != '\n')
		line--;

	char	*end = loc;
	while (*end != '\n')
		end++;

	size_t	line_num = 1;
	for (char *p = user_input; p < line; p++)
	{
		if (*p == '\n')
			line_num++;
	}

	int	indent = fprintf(stderr, "%s:%zu: ", filename, line_num);
	fprintf(stderr, "%.*s\n", (int)(end - line), line);

	int	pos = loc - line + indent;
	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// 次のトークンが期待している文字のときにはそのトークンを返す｡
// トークンは読み進めない｡
Token	*peek(char *op)
{
	if (strlen(op) != token->len ||
		memcmp(token->str, op, token->len) != 0)
		return (NULL);
	return (token);
}

// 次のトークンが期待している記号のときには､トークンを1つ読み進めて
// そのトークンを返す｡それ以外の場合にはNULLを返す｡
Token	*consume(char *op)
{
	if (!peek(op))
		return (NULL);
	Token	*tok = token;
	token = token->next;
	return (tok);
}

// 次のトークンがidentifierのときには､トークンを1つ読み進めて
// そのトークンを返す｡それ以外の場合にはNULLを返す｡
Token	*consume_ident(void)
{
	if (token->kind == TK_IDENT)
	{
		Token	*tok = token;
		token = token->next;
		return (tok);
	}
	return (NULL);
}

// 次のトークンが文字列リテラルの場合には､トークンを1つ読み進めて
// そのトークンを返す｡それ以外の場合にはNULLを返す｡
Token	*consume_string(void)
{
	if (token->kind == TK_STRING)
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
	if (!peek(op))
		error_at(token->str, "'%c'ではありません", op);
	token = token->next;
}

// 次のトークンが数値の場合､トークンを1つ読み進めてその数値を返す｡
// それ以外の場合にはエラーを報告する｡
size_t	expect_number(void)
{
	if (token->kind != TK_NUM)
		error_at(token->str, "数ではありません");
	size_t	val = token->val;
	token = token->next;
	return (val);
}

// 次のトークンが識別子の場合､トークンを1つ読み進めてその文字を返す｡
// それ以外の場合にはエラーを報告する｡
char	*expect_ident(void)
{
	if (token->kind != TK_IDENT)
		error_at(token->str, "識別子ではありません");
	char	*str = substr(token->str, token->len);
	token = token->next;
	return (str);
}

// 次のトークンが指定された識別子の場合､トークンを1つ読み進める｡
// それ以外の場合にはエラーを報告する｡
char	*expect_specified_ident(char *str)
{
	if (token->kind != TK_IDENT)
		error_at(token->str, "指定された識別子ではありません");
	if (strncmp(token->str, str, token->len))
		error_at(token->str, "指定された識別子ではありません");
	token = token->next;
	return (str);
}

// 次のトークンが型名の場合､トークンを1つ読み進めて型を返す｡
// それ以外の場合にはエラーを報告する｡
Type	*expect_type(void)
{
	Type	*ty;

	if (token->kind != TK_IDENT)
		error_at(token->str, "不正な型名です");
	ty = new_type_from_str(token->str);
	if (!ty)
		error_at(token->str, "不正な型名です");
	token = token->next;
	return (ty);
}

// 終端チェック
bool	at_eof(void)
{
	return (token->kind == TK_EOF);
}

// 新しいトークンを作成してcurに繋げる
static Token	*new_token(TokenKind kind, Token *cur, char *str, size_t len)
{
	Token	*tok = (Token *)calloc(1, sizeof(Token));
	if (!tok)
		exit_with_error();
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;
	return (tok);
}

// identifierを構成する文字か判定
int	is_tokstr(char c)
{
	return (isalnum(c) || c == '_');
}

// pがqで始まるか判定
static bool	starts_with(char *p, char *q)
{
	return (strncmp(p, q, strlen(q)) == 0);
}

// 予約語か確認
static char	*starts_with_reserved(char *p)
{
	size_t	l = strlen(p);

	static char	*kw[] = {"return", "if", "else", "while", "for"};

	for (size_t i = 0; i < sizeof(kw) / sizeof(*kw); i++)
	{
		size_t	len = strlen(kw[i]);
		if (l < len)
			continue ;
		if (starts_with(p, kw[i]) && !is_tokstr(p[len]))
			return (kw[i]);
	}

	static char	*ops[] = {"==", "!=", "<=", ">="};

	for (size_t i = 0; i < sizeof(ops) / sizeof(*ops); i++)
	{
		size_t	len = strlen(ops[i]);
		if (l < len)
			continue ;
		if (starts_with(p, ops[i]))
			return (ops[i]);
	}

	return (NULL);
}

// sizeofの演算子か確認
static char	*starts_with_sizeof(char *p)
{
	size_t	l = strlen(p);

	static char	*kw[] = {"sizeof"};

	for (size_t i = 0; i < sizeof(kw) / sizeof(*kw); i++)
	{
		size_t	len = strlen(kw[i]);
		if (l < len)
			continue ;
		if (starts_with(p, kw[i]) && !is_tokstr(p[len]))
			return (kw[i]);
	}

	return (NULL);
}

// 入力文字列pをトークナイズしてそれを返す
Token	*tokenize(void)
{
	char	*p = user_input;
	Token	head = {};
	Token	*cur = &head;

	while (*p)
	{
		// 空白文字をスキップ
		if (isspace(*p))
		{
			p++;
			continue ;
		}

		// 行コメント
		if (strncmp(p, "//", 2) == 0)
		{
			p += 2;
			while (*p != '\n')
				p++;
			continue ;
		}

		// ブロックコメント
		if (strncmp(p, "/*", 2) == 0)
		{
			char	*q = strstr(p + 2, "*/");
			if (!q)
				error_at(p, "コメントが閉じられていません");
			p = q + 2;
			continue ;
		}

		// ブロック
		if (*p == '{' || *p == '}')
		{
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		// 予約語
		char	*q = starts_with_reserved(p);
		if (q)
		{
			size_t	len = strlen(q);
			cur = new_token(TK_RESERVED, cur, p, len);
			p = p + len;
			continue ;
		}

		// sizeof演算子
		q = starts_with_sizeof(p);
		if (q)
		{
			size_t	len = strlen(q);
			cur = new_token(TK_SIZEOF, cur, p, len);
			p = p + len;
			continue ;
		}

		// 区切り文字
		if (ispunct(*p))
		{
			// 文字列リテラル
			if (*p == '"')
			{
				char	*start = p++;

				while (*p && *p != '"')
					p++;
				if (!*p)
					error_at(p, "文字列リテラルが閉じられていません");
				p++;
				cur = new_token(TK_STRING, cur, start, p - start);
				cur->ctx = substr(start + 1, p - start - 2);
				cur->clen = p - start - 1;
				continue ;
			}

			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue ;
		}

		// 識別子
		if (isalpha(*p) || *p == '_')
		{
			char	*q = p++;
			while (is_tokstr(*p))
				p++;
			cur = new_token(TK_IDENT, cur, q, p - q);
			continue ;
		}

		// 数値
		if (isdigit(*p))
		{
			char	*q = p;
			cur = new_token(TK_NUM, cur, p, 0);
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue ;
		}

		error_at(p, "トークナイズできません");
	}

	new_token(TK_EOF, cur, p, 0);
	return (head.next);
}
