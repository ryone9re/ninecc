#ifndef TYPES_H
# define TYPES_H

#include "include.h"
#include "struct.h"

Type	*new_type(TypeKind tk, Type *ptr_to);
Type	*new_type_array(TypeKind tk, Type* ptr_to, size_t len);
Type	*new_type_from_str(char *str);
void	add_type(Program *prog);

#endif /* TYPES_H */
