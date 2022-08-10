#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void	exit_with_error()
{
	fprintf(stderr, "%s\n",strerror(errno));
	exit(errno);
}

char	*substr(char *str, size_t len)
{
	char	*s = (char *)calloc(len + 1, sizeof(char));

	if (!s)
		return (NULL);
	strncpy(s, str, len);
	return (s);
}
