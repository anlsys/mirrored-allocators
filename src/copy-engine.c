#include "mam-internal.h"
#include "copy-engine-internal.h"
#include <stdarg.h>

const size_t mam_data_type_size[MAM_DATA_TYPE_MAX] = {
	1,
	1,
	2,
	2,
	4,
	4,
	8,
	8,
	16,
	16,
	2,
	2,
	4,
	8
};

const size_t mam_data_type_align[MAM_DATA_TYPE_MAX] = {
	1,
	1,
	2,
	2,
	4,
	4,
	8,
	8,
	16,
	16,
	2,
	2,
	4,
	8
};

mam_error_t
mam_platform_create(
		const char     *name,
		mam_type_map_t *type_map,
		mam_endian_t    endian,
		mam_platform_t *platform_ret) {
	MAM_CHECK_PTR(name);
	MAM_CHECK_PTR(type_map);
	MAM_REFUTE(endian < MAM_ENDIAN_LITTLE || endian >= MAM_ENDIAN_MAX, MAM_EINVAL);
	struct _mam_platform_s *platform = (struct _mam_platform_s *)
		calloc(1, sizeof(struct _mam_platform_s) + strlen(name) + 1);
	MAM_REFUTE(!platform, MAM_ENOMEM);
	char *p_name = (char *)platform + sizeof(struct _mam_platform_s);
	strcpy(p_name, name);
	platform->name = p_name;
	memcpy(platform->type_map, type_map, sizeof(mam_type_map_t));
	platform->endian = endian;
	return MAM_SUCCESS;
}

#undef  utarray_oom
#define utarray_oom() { \
        MAM_RAISE_ERR_GOTO(err, MAM_ENOMEM, err_mem); \
}

mam_error_t
mam_context_create(
		const char     *name,
		mam_platform_t  platform,
		mam_context_t  *context_ret) {
	MAM_CHECK_PTR(name);
	MAM_CHECK_PTR(platform);
	MAM_CHECK_PTR(context_ret);

	mam_error_t err = MAM_SUCCESS;
	struct _mam_context_s *context = (struct _mam_context_s *)
		calloc(1, sizeof(struct _mam_context_s) + strlen(name) + 1);
	MAM_REFUTE(!context, MAM_ENOMEM);
	char *p_name = (char *)context + sizeof(struct _mam_context_s);
	strcpy(p_name, name);
	context->name = p_name;
	utarray_new(context->pointers, &ut_ptr_icd);
	utarray_new(context->arrays, &ut_ptr_icd);
	utarray_new(context->constructs, &ut_ptr_icd);
	utarray_new(context->variables, &ut_ptr_icd);
	*context_ret = context;
	return MAM_SUCCESS;
err_mem:
	if (context->pointers)
		utarray_free(context->pointers);
	if (context->arrays)
		utarray_free(context->arrays);
	if (context->constructs)
		utarray_free(context->constructs);
	free(context);
	return err;
}

#undef  utarray_oom
#define utarray_oom() { \
        MAM_RAISE_ERR_GOTO(err, MAM_ENOMEM, err_mem); \
}

static inline mam_error_t
_mam_context_construct_create(
		mam_context_t         context,
		const char           *name,
		bool                  packed,
		mam_construct_type_t  type,
		mam_construct_t *construct_ret) {
	MAM_CHECK_PTR(context);
	MAM_CHECK_PTR(name);
	MAM_CHECK_PTR(construct_ret);

	mam_error_t err = MAM_SUCCESS;
	struct _mam_construct_s *construct = (struct _mam_construct_s *)
		calloc(1, sizeof(struct _mam_construct_s) + strlen(name) + 1);
	MAM_REFUTE(!construct, MAM_ENOMEM);
	char *p_name = (char *)construct + sizeof(struct _mam_construct_s);
	strcpy(p_name, name);
	construct->name = p_name;
	construct->context = context;
	construct->type = type;
	construct->packed = packed;
	construct->alignment = 1;
	utarray_new(construct->fields, &_mam_field_icd);
	utarray_push_back(context->constructs, &construct);
	*construct_ret = construct;
err_mem:
	if (construct->fields)
		utarray_free(construct->fields);
	free(construct);
}

mam_error_t
mam_context_struct_create(
		mam_context_t    context,
		const char      *name,
		bool             packed,
		mam_construct_t *construct_ret) {
	MAM_VALIDATE(_mam_context_construct_create(
		context, name, packed, MAM_CONSTRUCT_TYPE_STRUCT, construct_ret));
	return MAM_SUCCESS;
}

mam_error_t
mam_context_union_create(
		mam_context_t    context,
		const char      *name,
		bool             packed,
		mam_construct_t *construct_ret) {
	MAM_VALIDATE(_mam_context_construct_create(
		context, name, packed, MAM_CONSTRUCT_TYPE_UNION, construct_ret));
	return MAM_SUCCESS;
}

mam_error_t
mam_construct_get_type(
		mam_construct_t       construct,
		mam_construct_type_t *type_ret) {
	MAM_CHECK_PTR(construct);
	MAM_CHECK_PTR(type_ret);
	*type_ret = construct->type;
	return MAM_SUCCESS;
}

mam_error_t
mam_construct_get_size(
		mam_construct_t  construct,
		size_t          *size_ret) {
	MAM_CHECK_PTR(construct);
	MAM_CHECK_PTR(size_ret);
	*size_ret = construct->total_size;
	return MAM_SUCCESS;
}

mam_error_t
mam_construct_get_align(
		mam_construct_t  construct,
		size_t          *align_ret) {
	MAM_CHECK_PTR(construct);
	MAM_CHECK_PTR(align_ret);
	*align_ret = construct->alignment;
	return MAM_SUCCESS;
}

static inline mam_error_t
_mam_get_field_type_size_align(
		mam_platform_t    platform,
		mam_field_type_t *field_type,
		ssize_t          *size_ret,
		size_t           *align_ret) {
	int32_t type = field_type->type;
	if (type >= MAM_MAPPED_TYPE_CHAR && type < MAM_MAPPED_TYPE_MAX)
		type = platform->type_map[type - MAM_MAPPED_TYPE_CHAR];
	switch (type) {
	case MAM_COMPLEX_TYPE_STRUCT:
	case MAM_CONPLEX_TYPE_UNION:
	{
		mam_construct_t construct = field_type->construct;
		*size_ret = construct->total_size;
		*align_ret = construct->alignment;
		break;
	}
	case MAM_COMPLEX_TYPE_ARRAY:
	{
		mam_array_t array = field_type->array;
		*size_ret = (ssize_t)array->total_size;
		*align_ret = array->alignment;
		break;
	}
	default:
		*size_ret = (ssize_t)mam_data_type_size[type];
		*align_ret = mam_data_type_align[type];
	}
	return MAM_SUCCESS;
}

#undef  utarray_oom
#define utarray_oom() { \
        MAM_RAISE_ERR_GOTO(err, MAM_ENOMEM, err_mem); \
}
#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt) { \
	MAM_RAISE_ERR_GOTO(err, MAM_ENOMEM, err_arr); \
}
mam_error_t
mam_construct_add_field(
		mam_construct_t   construct,
		const char       *name,
		mam_field_type_t *field_type,
		...) {
	MAM_CHECK_PTR(construct);
	MAM_CHECK_PTR(name);
	MAM_CHECK_PTR(field_type);
	//TODO: should validate field_type content here
	size_t sz_name = strlen(name);
	MAM_REFUTE(!sz_name, MAM_EINVAL);
	struct _mam_field_s *field = NULL;
	HASH_FIND(hh_name, construct->fields_name_hash,
		name, sz_name, field);
	MAM_REFUTE(field, MAM_EINVAL);
	va_list args;
	va_start(args, field_type);
	mam_field_option_t opt = (mam_field_option_t)va_arg(args, int32_t);
	bool user_offset = false;
	size_t offset = 0;
	while (opt != MAM_FIELD_OPTION_END) {
		switch (opt) {
		case MAM_FIELD_OPTION_OFFSET:
			user_offset = true;
			offset = (size_t)va_arg(args, size_t);
			MAM_REFUTE(offset < construct->last_offset, MAM_EINVAL);
			MAM_REFUTE(construct->type == MAM_CONSTRUCT_TYPE_UNION && offset != 0, MAM_EINVAL);
			break;
		default:
			MAM_RAISE(MAM_EINVAL);
		}
		opt = va_arg(args, int32_t);
	}
	va_end(args);
	ssize_t size;
	size_t align;
	MAM_VALIDATE(_mam_get_field_type_size_align(
		construct->context->platform, field_type, &size, &align));
	MAM_REFUTE(size < 0, MAM_EINVAL);


	mam_error_t err = MAM_SUCCESS;
	field = (struct _mam_field_s *)
		calloc(1, sizeof(struct _mam_field_s) + sz_name + 1);
	MAM_REFUTE(!field, MAM_ENOMEM);
	char *p_name = (char *)field + sizeof(struct _mam_field_s);
	strcpy(p_name, name);
	field->name = p_name;
	memcpy(&field->field_type, field_type, sizeof(mam_field_type_t));
	utarray_push_back(construct->fields, &field);
	HASH_ADD_KEYPTR( hh_name, construct->fields_name_hash,
		field->name, sz_name, field );

	if (!user_offset && construct->type == MAM_CONSTRUCT_TYPE_STRUCT) {
		offset = construct->last_offset;
		if (!construct->packed) {
			size_t mask = align - 1;
			size_t pad = offset & mask;
			if (pad)
				offset += align - pad; 
		}
	}
	field->offset = offset;

	if (!construct->packed)
		construct->alignment = align > construct->alignment ? align : construct->alignment;
	if (construct->type == MAM_CONSTRUCT_TYPE_STRUCT) {
		construct->last_offset = offset + size;
		construct->total_size = construct->last_offset;
	} else
		construct->total_size = size > construct->total_size ? size : construct->total_size;
	if (!construct->packed) {
		size_t mask = construct->alignment - 1;
		size_t pad = construct->total_size & mask;
		if (pad)
			construct->total_size += construct->alignment - pad;
	}
	return MAM_SUCCESS;
err_arr:
	utarray_pop_back(construct->fields);
err_mem:
	free(field);
	return err;
}

#undef  utarray_oom
#define utarray_oom() { \
        MAM_RAISE_ERR_GOTO(err, MAM_ENOMEM, err_mem); \
}
mam_error_t
mam_context_create_pointer(
		mam_context_t     context,
		mam_field_type_t *field_type,
		mam_pointer_t    *pointer_ret) {
	MAM_CHECK_PTR(context);
	MAM_CHECK_PTR(field_type);
	//TODO: should validate field_type content here
	MAM_CHECK_PTR(pointer_ret);

	mam_error_t err = MAM_SUCCESS;
	mam_pointer_t pointer = (struct _mam_pointer_s *)
		calloc(1, sizeof(struct _mam_pointer_s));
	pointer->context = context;
	memcpy(&pointer->field_type, field_type, sizeof(mam_field_type_t));
	utarray_push_back(context->pointers, &pointer);
	*pointer_ret = pointer;
	return MAM_SUCCESS;
err_mem:
	free(pointer);
	return err;
}

#undef  utarray_oom
#define utarray_oom() { \
        MAM_RAISE_ERR_GOTO(err, MAM_ENOMEM, err_mem); \
}
mam_error_t
mam_context_create_array(
		mam_context_t     context,
		mam_field_type_t *field_type,
		mam_array_t      *array_ret) {
	MAM_CHECK_PTR(context);
	MAM_CHECK_PTR(field_type);
	//TODO: should validate field_type content here
	MAM_CHECK_PTR(array_ret);

	ssize_t size;
	size_t align;
	MAM_VALIDATE(_mam_get_field_type_size_align(
		context->platform, field_type, &size, &align));

	mam_error_t err = MAM_SUCCESS;
	mam_array_t array = (struct _mam_array_s *)
		calloc(1, sizeof(struct _mam_array_s));
	array->context = context;
	array->elem_size = size;
	array->total_size = 0;
	array->alignment = align;
	memcpy(&array->field_type, field_type, sizeof(mam_field_type_t));
	utarray_new(array->dimensions, &_mam_dimension_icd);
#undef  utarray_oom
#define utarray_oom() { \
        MAM_RAISE_ERR_GOTO(err, MAM_ENOMEM, err_arr); \
}
	utarray_push_back(context->arrays, &array);
	return MAM_SUCCESS;
err_arr:
	utarray_free(array->dimensions);
err_mem:
	free(array);
	return err;
}

#undef  utarray_oom
#define utarray_oom() { \
        MAM_RAISE_ERR_GOTO(err, MAM_ENOMEM, err_mem); \
}
mam_error_t
mam_array_add_dimension(
		mam_array_t      array,
		mam_dimension_t *dimension) {
	MAM_CHECK_PTR(array);
	MAM_CHECK_PTR(dimension);
	//TODO: should validate dimension content here

	mam_error_t err = MAM_SUCCESS;
	mam_dimension_t *p_dimension = (mam_dimension_t *)
		calloc(1, sizeof(mam_dimension_t) +
			(dimension->type == MAM_DIMENSION_TYPE_VARIABLE ?
				strlen(dimension->path) + 1 :
				dimension->count));
	MAM_REFUTE(!p_dimension, MAM_ENOMEM);
	utarray_push_back(array->dimensions, &p_dimension);
	
	p_dimension->type = dimension->type;
	if (dimension->type == MAM_DIMENSION_TYPE_VARIABLE) {
		char *p_path = (char *)p_dimension + sizeof(mam_dimension_t);
		strcpy(p_path, dimension->path);
		p_dimension->path = p_path;
		array->total_size = -1;
	} else {
		p_dimension->count = dimension->count;
		if (array->total_size == 0 && array->elem_size >= 0)
			array->total_size = array->elem_size * dimension->count;
		else if (array->total_size > 0)
			array->total_size *= dimension->count;
	}
	return MAM_SUCCESS;
err_mem:
	free(p_dimension);
	return err;
}

#undef  utarray_oom
#define utarray_oom() { \
        MAM_RAISE_ERR_GOTO(err, MAM_ENOMEM, err_mem); \
}
mam_error_t
mam_context_create_variable(
		mam_context_t     context,
		const char       *name,
		mam_field_type_t *field_type,
		mam_variable_t   *variable_ret) {
	MAM_CHECK_PTR(context);
	MAM_CHECK_PTR(name);
	MAM_CHECK_PTR(field_type);
	//TODO: should validate field_type content here
	MAM_CHECK_PTR(variable_ret);
	size_t sz_name = strlen(name);
	MAM_REFUTE(!sz_name, MAM_EINVAL);

	mam_error_t err = MAM_SUCCESS;
	mam_variable_t variable = (struct _mam_variable_s *)
		calloc(1, sizeof(struct _mam_variable_s) + sz_name + 1);
	MAM_REFUTE(!variable, MAM_ENOMEM);
	utarray_push_back(context->variables, &variable);
	char *p_name = (char *)variable + sizeof(struct _mam_variable_s);
	strcpy(p_name, name);
	variable->context = context;
	variable->name = p_name;
	memcpy(&variable->field_type, field_type, sizeof(mam_field_type_t));
	*variable_ret = variable;
	return MAM_SUCCESS;
err_mem:
	free(variable);
	return err;
}
