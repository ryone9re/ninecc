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

char	*read_file(char *path)
{
	FILE	*fp = fopen(path, "r");
	if (!fp)
		exit_with_error();

	if (fseek(fp, 0, SEEK_END) == -1)
		exit_with_error();
	size_t	size = ftell(fp);
	if (fseek(fp, 0, SEEK_SET) == -1)
		exit_with_error();

	char	*buf = (char *)calloc(1, sizeof(char) * size + 2);
	fread(buf, size, 1, fp);

	if (size == 0 || buf[size - 1] != '\n')
		buf[size++] = '\n';
	buf[size] = '\0';
	fclose(fp);
	return (buf);
}

int	main(int argc, char **argv)
{
	if (argc != 2)
	{
		error("引数の個数が正しくありません\n");
		return (1);
	};

	user_input = NULL;
	token = NULL;

	// トークナイズしてパースする
	filename = argv[1];
	user_input = read_file(filename);
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
