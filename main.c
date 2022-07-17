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
