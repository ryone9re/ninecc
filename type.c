#include "9cc.h"
#include <stdlib.h>

// 型の宣言
Type	*new_type(TypeKind tk, Type *ptr_to)
{
	Type	*type;

	type = (Type *)calloc(1, sizeof(Type));
	if (!type)
		exit_with_error();
	type->ty = tk;
	type->ptr_to = ptr_to;
	return (type);
}
