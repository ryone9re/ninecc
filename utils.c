#include "9cc.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// printfと同じ引数を取る
void	error(char *fmt, ...)
{
	va_list	ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void	exit_with_error(void)
{
	fprintf(stderr, "%s\n",strerror(errno));
	exit(errno);
}

char	*substr(char *str, size_t len)
{
	char	*s = (char *)calloc(len + 1, sizeof(char));

	if (!s)
		exit_with_error();
	strncpy(s, str, len);
	return (s);
}
