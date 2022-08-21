#ifndef GEN_H
# define GEN_H

#include "include.h"
#include "struct.h"

Var		*find_lvar(Token *tok);
void	codegen(Program *prog);

#endif /* GEN_H */
