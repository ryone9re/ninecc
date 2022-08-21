#ifndef TOKENIZE_H
# define TOKENIZE_H

#include "include.h"
#include "struct.h"

Token	*peek(char *op);
Token	*consume(char *op);
Token	*consume_ident(void);
Token	*consume_string(void);
void	expect(char *op);
size_t	expect_number(void);
char	*expect_ident(void);
char	*expect_specified_ident(char *str);
Token	*expect_type(void);
bool	at_eof(void);
Token	*tokenize(void);

#endif /* TOKENIZE_H */
