#include "9cc.h"
#include <stdio.h>

char	*user_input;
Token	*token;

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		error("引数の個数が正しくありません\n");
		return (1);
	}

	// トークナイズしてパースする
	user_input = argv[1];
	token = tokenize();
	program();

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n\n");
	printf("main:\n");

	size_t	stack_size = 0;
	for (LVar *var = locals; var; var = var->next)
		stack_size = stack_size + 8;

	// プロローグ
	printf("\tpush rbp\n");
	printf("\tmov rbp, rsp\n");
	printf("\tsub rsp, %ld\n", stack_size);

	// 先頭の式から順にコード生成
	for (size_t i = 0; code[i]; i++)
	{
		gen(code[i]);

		// 式の評価結果としてスタックに一つの値が残っている
		// はずなので､スタックが溢れないようにポップしておく
		printf("\tpop rax\n");
	}

	// エピローグ
	printf(".L.return:\n");
	printf("\tmov rsp, rbp\n");
	printf("\tpop rbp\n");
	printf("\tret\n");

	return (0);
}
