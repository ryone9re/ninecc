#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>

char	*filename;
char	*user_input;
Token	*token;

static size_t	align_to(size_t n, size_t align)
{
	return ((n + align - 1) & ~(align - 1));
}

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		error("引数の個数が正しくありません\n");
		return (1);
	};

	// トークナイズしてパースする
	filename = argv[1];
	// user_input = read_file(filename);
	user_input = filename;
	token = tokenize();
	Program	*prog = program();
	add_type(prog);

	for (Function *f = prog->functions; f; f = f->next)
	{
		size_t	offset = 0;
		for (VarList *vl = f->locals; vl; vl = vl->next)
		{
			offset = offset + size_of(vl->var->type);
			vl->var->offset = offset;
		}
		f->stack_size = align_to(offset, 8);
	}

	codegen(prog);

	return (0);
}
