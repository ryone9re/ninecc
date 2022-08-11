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
	};

	// トークナイズしてパースする
	user_input = argv[1];
	token = tokenize();
	Function	*prog = program();

	size_t	stack_size = 0;
	for (Var *var = prog->locals; var; var = var->next)
		stack_size = stack_size + 8;
	prog->stack_size = stack_size;

	codegen(prog);

	return (0);
}
