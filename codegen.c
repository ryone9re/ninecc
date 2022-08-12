#include "9cc.h"
#include <stdio.h>
#include <string.h>

static char		*argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

static size_t	labelseq = 1;
static char		*funcname;

static void	gen(Node *node);

static void	gen_lval(Node *node)
{
	switch (node->kind)
	{
	case ND_VAR:
		printf("\tlea rax, [rbp-%ld]\n", node->var->offset);
		printf("\tpush rax\n");
		return ;
	case ND_DEREF:
		gen(node->lhs);
		return ;
	default:
		error("代入の左辺が変数ではありません");
	}
}

static void	load(void)
{
	printf("\tpop rax\n");
	printf("\tmov rax, [rax]\n");
	printf("\tpush rax\n");
}

static void	store(void)
{
	printf("\tpop rdi\n");
	printf("\tpop rax\n");
	printf("\tmov [rax], rdi\n");
	printf("\tpush rdi\n");
}

static void	gen(Node *node)
{
	if (node == NULL)
		return ;
	switch (node->kind)
	{
	case ND_RETURN:
		gen(node->lhs);
		printf("\tpop rax\n");
		printf("\tjmp .L.return.%s\n", funcname);
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
		gen(node->init);
		printf(".Lbegin%ld:\n", labelseq);
		if (node->cond)
		{
			gen(node->cond);
			printf("\tpop rax\n");
			printf("\tcmp rax, 0\n");
			printf("\tje .Lend%ld\n", labelseq);
		}
		gen(node->then);
		gen(node->inc);
		printf("\tjmp .Lbegin%ld\n", labelseq);
		printf(".Lend%ld:\n", labelseq);
		labelseq++;
		return ;
	case ND_BLOCK:
		for (Node *n = node->body; n; n = n->next)
			gen(n);
		return ;
	case ND_ASSIGN:
		gen_lval(node->lhs);
		gen(node->rhs);
		store();
		return ;
	case ND_ADDR:
		gen_lval(node->lhs);
		return ;
	case ND_DEREF:
		gen(node->lhs);
		load();
		return ;
	case ND_VAR:
		gen_lval(node);
		load();
		return ;
	case ND_FUNCALL:
	{
		size_t	nargs = 0;

		for (Node *n = node->args; n; n = n->next)
		{
			gen(n);
			nargs++;
		}

		for (size_t i = nargs; i; i--)
			printf("\tpop %s\n", argreg[i - 1]);

		// RSPのアライン
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
	case ND_NUM:
		printf("\tpush %d\n", node->val);
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

void	codegen(Function *prog)
{
	// シンタックス宣言
	printf(".intel_syntax noprefix\n");

	// 先頭から順にコード生成
	for (Function *func = prog; func; func = func->next)
	{
		// 関数宣言
		funcname = func->name;
		printf(".global %s\n", funcname);
		printf("%s:\n", funcname);

		// プロローグ
		printf("\tpush rbp\n");
		printf("\tmov rbp, rsp\n");
		printf("\tsub rsp, %ld\n", func->stack_size);

		// 受けた引数をスタックに積む
		size_t	i = 0;
		for (VarList *vl = func->params; vl; vl = vl->next)
			printf("\tmov [rbp-%ld], %s\n", vl->var->offset, argreg[i++]);

		// 関数の実装
		for (Node *node = func->node; node; node = node->next)
			gen(node);

		// エピローグ
		printf(".L.return.%s:\n", funcname);
		printf("\tmov rsp, rbp\n");
		printf("\tpop rbp\n");
		printf("\tret\n");
	}
}
