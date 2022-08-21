#ifndef ERROR_H
# define ERROR_H

#include "include.h"
#include "struct.h"

#include <stdarg.h>
#include <stdio.h>

void	error(char *fmt, ...);
void	error_at(char *loc, char *fmt, ...);
void	exit_with_error(void);

#endif /* ERROR_H */
