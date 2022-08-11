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

	for (Function *f = prog; f; f = f->next)
	{
		size_t	offset = 0;
		for (Var *var = f->locals; var; var = var->next)
		{
			offset = offset + 8;
			var->offset = offset;
		}
		f->stack_size = offset;
	}

	codegen(prog);

	return (0);
}
