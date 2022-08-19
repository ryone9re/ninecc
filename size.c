#include "9cc.h"

size_t	size_of(Type *type)
{
	switch (type->kind)
	{
	case TYPE_ARRAY:
		return (size_of(type->ptr_to) * type->array_len);

	case TYPE_INT:
		return (8);

	case TYPE_PTR:
		return (8);

	default:
		return (8);
	}
}
