#include "9cc.h"
#include <stdio.h>
#include <string.h>

static char		*argreg1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char		*argreg8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

static size_t	labelseq = 1;
static char		*funcname;

static void	gen(Node *node);

static void	gen_addr(Node *node)
{
	switch (node->kind)
	{
	case ND_VAR:
		if (node->var->is_local)
		{
			printf("\tlea rax, [rbp-%zu]\n", node->var->offset);
			printf("\tpush rax\n");
		}
		else
			printf("\tpush offset %s\n", node->var->name);
		return ;
	case ND_DEREF:
		gen(node->lhs);
		return ;
	default:
		error("代入の左辺が変数ではありません");
	}
}

static void	gen_lval(Node *node)
{
	if (node->type->kind == TYPE_ARRAY)
		error_at(node->tok->str, "左辺値ではありません");
	gen_addr(node);
}

static void	load(Type *type)
{
	printf("\tpop rax\n");
	switch (size_of(type))
	{
	case 1:
		printf("\tmovsx rax, BYTE PTR [rax]\n");
		break ;
	default:
		printf("\tmov rax, [rax]\n");
		break ;
	}
	printf("\tpush rax\n");
}

static void	store(Type *type)
{
	printf("\tpop rdi\n");
	printf("\tpop rax\n");
	switch (size_of(type))
	{
	case 1:
		printf("\tmov [rax], dil\n");
		break ;
	default:
		printf("\tmov [rax], rdi\n");
		break ;
	}
	printf("\tpush rdi\n");
}

static void	gen(Node *node)
{
	if (node == NULL)
		return ;
	switch (node->kind)
	{
	case ND_NULL:
		return ;
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
			printf("\tje .Lelse%zu\n", labelseq);
			gen(node->then);
			printf("\tjmp .Lend%zu\n", labelseq);
			printf(".Lelse%zu:\n", labelseq);
			gen(node->els);
			printf("\tjmp .Lend%zu\n", labelseq);
		}
		else
		{
			printf("\tje .Lend%zu\n", labelseq);
			gen(node->then);
		}
		printf(".Lend%zu:\n", labelseq);
		labelseq++;
		return ;
	case ND_WHILE:
		printf(".Lbegin%zu:\n", labelseq);
		gen(node->cond);
		printf("\tpop rax\n");
		printf("\tcmp rax, 0\n");
		printf("\tje .Lend%zu\n", labelseq);
		gen(node->then);
		printf("\tjmp .Lbegin%zu\n", labelseq);
		printf(".Lend%zu:\n", labelseq);
		labelseq++;
		return ;
	case ND_FOR:
		for (Node *n = node->init; n; n = n->next)
			gen(n);
		printf(".Lbegin%zu:\n", labelseq);
		if (node->cond)
		{
			gen(node->cond);
			printf("\tpop rax\n");
			printf("\tcmp rax, 0\n");
			printf("\tje .Lend%zu\n", labelseq);
		}
		gen(node->then);
		for (Node *n = node->inc; n; n = n->next)
			gen(n);
		printf("\tjmp .Lbegin%zu\n", labelseq);
		printf(".Lend%zu:\n", labelseq);
		labelseq++;
		return ;
	case ND_BLOCK:
		for (Node *n = node->body; n; n = n->next)
			gen(n);
		return ;
	case ND_ASSIGN:
		gen_lval(node->lhs);
		gen(node->rhs);
		store(node->type);
		return ;
	case ND_ADDR:
		gen_addr(node->lhs);
		return ;
	case ND_DEREF:
		gen(node->lhs);
		if (node->type->kind != TYPE_ARRAY)
			load(node->type);
		return ;
	case ND_VAR:
		gen_addr(node);
		if (node->type->kind != TYPE_ARRAY)
			load(node->type);
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
			printf("\tpop %s\n", argreg8[i - 1]);

		// RSPのアライン
		printf("\tmov rax, rsp\n");
		printf("\tand rax, 15\n");
		printf("\tjnz .L.call.%zu\n", labelseq);
		printf("\tmov rax, 0\n");
		printf("\tcall %s\n", node->funcname);
		printf("\tjmp .L.end.%zu\n", labelseq);
		printf(".L.call.%zu:\n", labelseq);
		printf("\tsub rsp, 8\n");
		printf("\tmov rax, 0\n");
		printf("\tcall %s\n", node->funcname);
		printf("\tadd rsp, 8\n");
		printf(".L.end.%zu:\n", labelseq);
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
		if (node->type->ptr_to)
			printf("\timul rdi, %zu\n", size_of(node->type->ptr_to));
		printf("\tadd rax, rdi\n");
		break ;
	case ND_SUB:
		if (node->type->ptr_to)
			printf("\timul rdi, %zu\n", size_of(node->type->ptr_to));
		printf("\tsub rax, rdi\n");
		break ;
	case ND_MUL:
		printf("\timul rax, rdi\n");
		break ;
	case ND_DIV:
		printf("\tcqo\n");
		printf("\tidiv rdi\n");
		break ;
	case ND_MOD:
		printf("\tcqo\n");
		printf("\tidiv rdi\n");
		printf("\tpush rdx\n");
		return ;
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

static void	emit_data(VarList *globals)
{
	printf(".data\n");
	for (VarList *glb = globals; glb; glb = glb->next)
	{
		printf("%s:\n", glb->var->name);

		if (glb->var->ctx)
		{
			for (size_t i = 0; i < glb->var->clen; i++)
				printf("\t.byte %d\n", glb->var->ctx[i]);
			continue ;
		}

		printf("\t.zero %zu\n", size_of(glb->var->type));
	}
}

// 受けた引数をスタックに積む
static void	load_arg(Var *var, int idx)
{
	switch (size_of(var->type))
	{
	case 1:
		printf("\tmov [rbp-%zu], %s\n", var->offset, argreg1[idx]);
		break ;
	default:
		printf("\tmov [rbp-%zu], %s\n", var->offset, argreg8[idx]);
		break ;
	}
}

static void	emit_func(Function *functions)
{
	printf(".text\n");
	for (Function *func = functions; func; func = func->next)
	{
		// 関数宣言
		funcname = func->name;
		printf(".global %s\n", funcname);
		printf("%s:\n", funcname);

		// プロローグ
		printf("\tpush rbp\n");
		printf("\tmov rbp, rsp\n");
		printf("\tsub rsp, %zu\n", func->stack_size);

		// 受けた引数をスタックに積む
		size_t	i = 0;
		for (VarList *vl = func->params; vl; vl = vl->next)
			load_arg(vl->var, i++);

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

void	codegen(Program *prog)
{
	// シンタックス宣言
	printf(".intel_syntax noprefix\n");

	// グローバル変数宣言
	emit_data(prog->globals);

	// 先頭から順に関数のコード生成
	emit_func(prog->functions);
}
