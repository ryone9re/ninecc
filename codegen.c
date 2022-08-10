#include "9cc.h"
#include <stdio.h>
#include <string.h>

Node			*code[100];
LVar			*locals;
static size_t	labelseq = 1;
static char		*argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

static void	gen_lval(Node *node)
{
	if (node->kind != ND_LVAR)
		error("代入の左辺が変数ではありません");

	printf("\tmov rax, rbp\n");
	printf("\tsub rax, %ld\n", node->offset);
	printf("\tpush rax\n");
}

LVar	*find_lvar(Token *tok)
{
	for (LVar *var = locals; var; var = var->next)
	{
		if (var->len == tok->len && !memcmp(var->name, tok->str, var->len))
			return (var);
	}
	return (NULL);
}

void	gen(Node *node)
{
	if (node == NULL)
		return ;
	switch (node->kind)
	{
	case ND_RETURN:
		gen(node->lhs);
		printf("\tpop rax\n");
		printf("\tmov rsp, rbp\n");
		printf("\tpop rbp\n");
		printf("\tret\n");
		return ;
	case ND_IF:
		gen(node->cond);
		printf("\tpop rax\n");
		printf("\tcmp rax, 0\n");
		if (node->els)
		{
			printf("\tje .Lelse%ld\n", labelseq);
			gen(node->then);
			printf("\tjmp .Lend%ld\n", labelseq);
			printf(".Lelse%ld:\n", labelseq);
			gen(node->els);
			printf("\tjmp .Lend%ld\n", labelseq);
		}
		else
		{
			printf("\tje .Lend%ld\n", labelseq);
			gen(node->then);
		}
		printf(".Lend%ld:\n", labelseq);
		labelseq++;
		return ;
	case ND_WHILE:
		printf(".Lbegin%ld:\n", labelseq);
		gen(node->cond);
		printf("\tpop rax\n");
		printf("\tcmp rax, 0\n");
		printf("\tje .Lend%ld\n", labelseq);
		gen(node->then);
		printf("\tjmp .Lbegin%ld\n", labelseq);
		printf(".Lend%ld:\n", labelseq);
		labelseq++;
		return ;
	case ND_FOR:
		gen(node->lhs);
		printf(".Lbegin%ld:\n", labelseq);
		gen(node->cond);
		printf("\tpop rax\n");
		printf("\tcmp rax, 0\n");
		printf("\tje .Lend%ld\n", labelseq);
		gen(node->then);
		gen(node->rhs);
		printf("\tjmp .Lbegin%ld\n", labelseq);
		printf(".Lend%ld:\n", labelseq);
		labelseq++;
		return ;
	case ND_BLOCK:
		for (Node *n = node->body; n; n = n->next)
			gen(n);
		return ;
	case ND_NUM:
		printf("\tpush %ld\n", node->val);
		return ;
	case ND_FUNCALL:
	{
		Node	*n = node->args;

		for (size_t i = 0; n && i < (sizeof(argreg) / sizeof(*argreg)); i++)
		{
			gen(n);
			printf("\tpop %s\n", argreg[i]);
			n = n->next;
		}
		printf("\tmov rax, rsp\n");
		printf("\tand rax, 15\n");
		printf("\tjnz .L.call.%ld\n", labelseq);
		printf("\tmov rax, 0\n");
		printf("\tcall %s\n", node->funcname);
		printf("\tjmp .L.end.%ld\n", labelseq);
		printf(".L.call.%ld:\n", labelseq);
		printf("\tsub rsp, 8\n");
		printf("\tmov rax, 0\n");
		printf("\tcall %s\n", node->funcname);
		printf("\tadd rsp, 8\n");
		printf(".L.end.%ld:\n", labelseq);
		printf("\tpush rax\n");
		labelseq++;
		return ;
	}
	case ND_LVAR:
		gen_lval(node);
		printf("\tpop rax\n");
		printf("\tmov rax, [rax]\n");
		printf("\tpush rax\n");
		return ;
	case ND_ASSIGN:
		gen_lval(node->lhs);
		gen(node->rhs);

		printf("\tpop rdi\n");
		printf("\tpop rax\n");
		printf("\tmov [rax], rdi\n");
		printf("\tpush rdi\n");
		return ;
	default:
		break ;
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
		return ;
	}

	printf("\tpush rax\n");
}
