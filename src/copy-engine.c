#include "mam-internal.h"
#include "copy-engine-internal.h"
#include <stdarg.h>
#include <stdio.h>

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

static inline int _mam_is_little_endian(void) {
  const union { uint32_t u; char c[4]; } one = { 1 };
  return one.c[0];
}

mam_error_t
_mam_platform_create(
		const char     *name,
		mam_type_map_t *type_map,
		mam_endian_t    endian,
		mam_platform_t *platform_ret) {
	struct _mam_platform_s *platform = (struct _mam_platform_s *)
		calloc(1, sizeof(struct _mam_platform_s) + strlen(name) + 1);
	MAM_REFUTE(!platform, MAM_ENOMEM);
	char *p_name = (char *)platform + sizeof(struct _mam_platform_s);
	strcpy(p_name, name);
	platform->name = p_name;
	memcpy(platform->type_map, type_map, sizeof(mam_type_map_t));
	platform->endian = endian;
	*platform_ret = platform;
	return MAM_SUCCESS;
}

mam_error_t
mam_platform_create(
		const char     *name,
		mam_type_map_t *type_map,
		mam_endian_t    endian,
		mam_platform_t *platform_ret) {
	MAM_CHECK_PTR(name);
	MAM_CHECK_PTR(type_map);
	MAM_REFUTE(endian < MAM_ENDIAN_LITTLE || endian >= MAM_ENDIAN_MAX, MAM_EINVAL);
	MAM_CHECK_PTR(platform_ret);
	MAM_VALIDATE(_mam_platform_create(name, type_map, endian, platform_ret));
	return MAM_SUCCESS;
}

static inline mam_error_t
_mam_get_int(
		bool             sign,
		size_t           sz,
		mam_data_type_t *type_ret) {
	if (sign) {
		switch (sz) {
		case 1:
			*type_ret = MAM_DATA_TYPE_INT8;
			break;
		case 2:
			*type_ret = MAM_DATA_TYPE_INT16;
			break;
		case 4:
			*type_ret = MAM_DATA_TYPE_INT32;
			break;
		case 8:
			*type_ret = MAM_DATA_TYPE_INT64;
			break;
		case 16:
			*type_ret = MAM_DATA_TYPE_INT128;
			break;
		default :
			return MAM_EINVAL;
		}
	} else {
		switch (sz) {
		case 1:
			*type_ret = MAM_DATA_TYPE_UINT8;
			break;
		case 2:
			*type_ret = MAM_DATA_TYPE_UINT16;
			break;
		case 4:
			*type_ret = MAM_DATA_TYPE_UINT32;
			break;
		case 8:
			*type_ret = MAM_DATA_TYPE_UINT64;
			break;
		case 16:
			*type_ret = MAM_DATA_TYPE_UINT128;
			break;
		default :
			return MAM_EINVAL;
		}
	}
	return MAM_SUCCESS;
}

#define TYPE_MAP_OFFSET(type) \
	(&(*type_map_p)[MAM_MAPPED_TYPE_ ## type - MAM_MAPPED_TYPE_CHAR])

static inline mam_error_t
_mam_fill_host_type_map(
		mam_type_map_t *type_map_p) {
	MAM_VALIDATE(_mam_get_int(true, sizeof(char), TYPE_MAP_OFFSET(CHAR)));
	MAM_VALIDATE(_mam_get_int(false, sizeof(unsigned char), TYPE_MAP_OFFSET(UCHAR)));
	MAM_VALIDATE(_mam_get_int(true, sizeof(short), TYPE_MAP_OFFSET(SHORT)));
	MAM_VALIDATE(_mam_get_int(false, sizeof(unsigned short), TYPE_MAP_OFFSET(USHORT)));
	MAM_VALIDATE(_mam_get_int(true, sizeof(int), TYPE_MAP_OFFSET(INT)));
	MAM_VALIDATE(_mam_get_int(false, sizeof(unsigned int), TYPE_MAP_OFFSET(UINT)));
	MAM_VALIDATE(_mam_get_int(true, sizeof(long), TYPE_MAP_OFFSET(LONG)));
	MAM_VALIDATE(_mam_get_int(false, sizeof(unsigned long), TYPE_MAP_OFFSET(ULONG)));
	MAM_VALIDATE(_mam_get_int(true, sizeof(long long), TYPE_MAP_OFFSET(LONGLONG)));
	MAM_VALIDATE(_mam_get_int(false, sizeof(unsigned long long), TYPE_MAP_OFFSET(ULONGLONG)));
	MAM_VALIDATE(_mam_get_int(false, sizeof(size_t), TYPE_MAP_OFFSET(SIZE)));
	MAM_VALIDATE(_mam_get_int(true, sizeof(ssize_t), TYPE_MAP_OFFSET(SSIZE)));
	MAM_VALIDATE(_mam_get_int(false, sizeof(void*), TYPE_MAP_OFFSET(POINTER)));
	MAM_VALIDATE(_mam_get_int(true, sizeof(ptrdiff_t), TYPE_MAP_OFFSET(PTRDIFF)));
	MAM_VALIDATE(_mam_get_int(false, sizeof(intptr_t), TYPE_MAP_OFFSET(INTPTR)));
	MAM_VALIDATE(_mam_get_int(true, sizeof(uintptr_t), TYPE_MAP_OFFSET(UINTPTR)));
	if (sizeof(float) != 4)
		return MAM_EINVAL;
	*TYPE_MAP_OFFSET(FLOAT) = MAM_DATA_TYPE_FLOAT32;
	if (sizeof(double) != 8)
		return MAM_EINVAL;
	*TYPE_MAP_OFFSET(DOUBLE) = MAM_DATA_TYPE_FLOAT64;
	*TYPE_MAP_OFFSET(HALF) = MAM_DATA_TYPE_FLOAT16;
	return MAM_SUCCESS;
}

mam_error_t mam_platform_create_host(
		mam_platform_t *platform_ret) {
	MAM_CHECK_PTR(platform_ret);
	mam_type_map_t type_map;
	MAM_VALIDATE(_mam_fill_host_type_map(&type_map));
	mam_endian_t endian = _mam_is_little_endian() ? MAM_ENDIAN_LITTLE : MAM_ENDIAN_BIG;
	MAM_VALIDATE(_mam_platform_create("host", &type_map, endian, platform_ret));
	return MAM_SUCCESS;
}

extern mam_error_t
mam_platform_destroy(
		mam_platform_t platform) {
	MAM_CHECK_PTR(platform);
	free(platform);
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
	mam_context_t context = (struct _mam_context_s *)
		calloc(1, sizeof(struct _mam_context_s) + strlen(name) + 1);
	MAM_REFUTE(!context, MAM_ENOMEM);
	char *p_name = (char *)context + sizeof(struct _mam_context_s);
	strcpy(p_name, name);
	context->name = p_name;
	context->platform = platform;
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

mam_error_t
mam_context_get_variables_number(
		mam_context_t  context,
		size_t        *variables_number_ret) {
	MAM_CHECK_PTR(context);
	MAM_CHECK_PTR(variables_number_ret);
	*variables_number_ret = utarray_len(context->variables);
	return MAM_SUCCESS;
}

mam_error_t
mam_context_get_constructs_number(
		mam_context_t  context,
		size_t        *constructs_number_ret) {
	MAM_CHECK_PTR(context);
	MAM_CHECK_PTR(constructs_number_ret);
	*constructs_number_ret = utarray_len(context->constructs);
	return MAM_SUCCESS;
}

mam_error_t
mam_context_get_variable(
		mam_context_t   context,
		size_t          index,
		mam_variable_t *variable_ret) {
	MAM_CHECK_PTR(context);
	MAM_REFUTE(index >= utarray_len(context->variables), MAM_EINVAL);
	*variable_ret = *(mam_variable_t *)utarray_eltptr(context->variables, index);
	return MAM_SUCCESS;
}

mam_error_t
mam_context_get_construct(
		mam_context_t    context,
		size_t           index,
		mam_construct_t *construct_ret) {
	MAM_CHECK_PTR(context);
	MAM_REFUTE(index >= utarray_len(context->constructs), MAM_EINVAL);
	*construct_ret = *(mam_construct_t *)utarray_eltptr(context->constructs, index);
	return MAM_SUCCESS;
}

mam_error_t
mam_context_get_variable_by_name(
		mam_context_t   context,
		const char *    name,
		mam_variable_t *variable_ret) {
	MAM_CHECK_PTR(context);
	MAM_CHECK_PTR(name);
	size_t sz_name = strlen(name);
	MAM_REFUTE(!sz_name, MAM_EINVAL);
	mam_variable_t variable = NULL;
	HASH_FIND(hh_name, context->variables_name_hash,
		name, sz_name, variable);
	MAM_REFUTE(!variable, MAM_EINVAL);
	*variable_ret = variable;
	return MAM_SUCCESS;
}

mam_error_t
mam_context_get_construct_by_name(
		mam_context_t    context,
		const char *     name,
		mam_construct_t *construct_ret) {
	MAM_CHECK_PTR(context);
	MAM_CHECK_PTR(name);
	size_t sz_name = strlen(name);
	MAM_REFUTE(!sz_name, MAM_EINVAL);
	mam_construct_t construct = NULL;
	HASH_FIND(hh_name, context->constructs_name_hash,
		name, sz_name, construct);
	MAM_REFUTE(!construct, MAM_EINVAL);
	*construct_ret = construct;
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
static inline mam_error_t
_mam_context_create_construct(
		mam_context_t         context,
		const char           *name,
		bool                  packed,
		mam_construct_type_t  type,
		mam_construct_t *construct_ret) {
	MAM_CHECK_PTR(context);
	MAM_CHECK_PTR(name);
	MAM_CHECK_PTR(construct_ret);
	mam_construct_t construct = NULL;
	size_t sz_name = strlen(name);
	if (strlen(name)>0) {
		HASH_FIND(hh_name, context->constructs_name_hash,
			name, sz_name, construct);
		MAM_REFUTE(construct, MAM_EINVAL);
	}

	mam_error_t err = MAM_SUCCESS;
	construct = (struct _mam_construct_s *)
		calloc(1, sizeof(struct _mam_construct_s) + sz_name + 1);
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
	HASH_ADD_KEYPTR( hh_name, context->constructs_name_hash,
		construct->name, sz_name, construct );
	*construct_ret = construct;
	return MAM_SUCCESS;
err_arr:
	utarray_pop_back(context->constructs);
err_mem:
	if (construct->fields)
		utarray_free(construct->fields);
	free(construct);
	return err;
}

mam_error_t
mam_context_create_struct(
		mam_context_t    context,
		const char      *name,
		bool             packed,
		mam_construct_t *construct_ret) {
	MAM_VALIDATE(_mam_context_create_construct(
		context, name, packed, MAM_CONSTRUCT_TYPE_STRUCT, construct_ret));
	return MAM_SUCCESS;
}

mam_error_t
mam_context_create_union(
		mam_context_t    context,
		const char      *name,
		bool             packed,
		mam_construct_t *construct_ret) {
	MAM_VALIDATE(_mam_context_create_construct(
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
mam_construct_get_name(
		mam_construct_t   construct,
		const char      **name_ret) {
	MAM_CHECK_PTR(construct);
	MAM_CHECK_PTR(name_ret);
	*name_ret = construct->name;
	return MAM_SUCCESS;
}

mam_error_t
mam_construct_get_fields_number(
		mam_construct_t  construct,
		size_t          *fields_number_ret) {
	MAM_CHECK_PTR(construct);
	MAM_CHECK_PTR(fields_number_ret);
	*fields_number_ret = utarray_len(construct->fields);
	return MAM_SUCCESS;
}

mam_error_t
mam_construct_get_field(
		mam_construct_t    construct,
		size_t             index,
		const char       **name_ret,
		size_t            *offset_ret,
		size_t            *size_ret,
		mam_field_type_t  *field_type_ret) {
	MAM_CHECK_PTR(construct);
	MAM_REFUTE(index >= utarray_len(construct->fields), MAM_EINVAL);
	MAM_CHECK_PTR(name_ret);
	MAM_CHECK_PTR(offset_ret);
	MAM_CHECK_PTR(size_ret);
	MAM_CHECK_PTR(field_type_ret);
	struct _mam_field_s **p = (struct _mam_field_s **)utarray_eltptr(construct->fields, index);
	*name_ret = (*p)->name;
	*offset_ret = (*p)->offset;
	*size_ret = (*p)->size;
	memcpy(field_type_ret, &(*p)->field_type, sizeof(mam_field_type_t));
	return MAM_SUCCESS;
}

mam_error_t
mam_construct_get_field_by_name(
		mam_construct_t   construct,
		const char       *name,
		size_t           *offset_ret,
		size_t           *size_ret,
		mam_field_type_t *field_type_ret) {
	MAM_CHECK_PTR(construct);
	MAM_CHECK_PTR(name);
	MAM_CHECK_PTR(offset_ret);
	MAM_CHECK_PTR(size_ret);
	MAM_CHECK_PTR(field_type_ret);
	size_t sz_name = strlen(name);
	MAM_REFUTE(!sz_name, MAM_EINVAL);
	struct _mam_field_s *field = NULL;
	HASH_FIND(hh_name, construct->fields_name_hash,
		name, sz_name, field);
	MAM_REFUTE(!field, MAM_EINVAL);
	*offset_ret = field->offset;
	*size_ret = field->size;
	memcpy(field_type_ret, &field->field_type, sizeof(mam_field_type_t));
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
	case MAM_COMPLEX_TYPE_UNION:
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

static inline void
_mam_freeze_field_type(
		mam_field_type_t *field_type) {
	int32_t type = field_type->type;
	switch (type) {
	case MAM_COMPLEX_TYPE_STRUCT:
	case MAM_COMPLEX_TYPE_UNION:
		field_type->construct->frozen = true;
		break;
	case MAM_COMPLEX_TYPE_ARRAY:
		field_type->array->frozen = true;
		break;
	default:
		return;
	}
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
	MAM_REFUTE(construct->frozen, MAM_EFROZEN);
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
	ssize_t ssz;
	size_t align;
	MAM_VALIDATE(_mam_get_field_type_size_align(
		construct->context->platform, field_type, &ssz, &align));
	MAM_REFUTE(ssz < 0, MAM_EINVAL);
	size_t size = (size_t)ssz;

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
	field->size = size;

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
	_mam_freeze_field_type(field_type);
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

mam_error_t
mam_pointer_get_field_type(
		mam_pointer_t     pointer,
		mam_field_type_t *field_type_ret) {
	MAM_CHECK_PTR(pointer);
	MAM_CHECK_PTR(field_type_ret);
	memcpy(field_type_ret, &pointer->field_type, sizeof(mam_field_type_t));
	return MAM_SUCCESS;
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
	_mam_freeze_field_type(field_type);
	*array_ret = array;
	return MAM_SUCCESS;
err_arr:
	utarray_free(array->dimensions);
err_mem:
	free(array);
	return err;
}

mam_error_t
mam_array_get_field_type(
		mam_array_t      array,
		mam_field_type_t *field_type_ret) {
	MAM_CHECK_PTR(array);
	MAM_CHECK_PTR(field_type_ret);
	memcpy(field_type_ret, &array->field_type, sizeof(mam_field_type_t));
	return MAM_SUCCESS;
}

mam_error_t
mam_array_get_num_dimensions(
		mam_array_t  array,
		size_t      *num_dimension_ret) {
	MAM_CHECK_PTR(array);
	MAM_CHECK_PTR(num_dimension_ret);
	*num_dimension_ret = utarray_len(array->dimensions);
	return MAM_SUCCESS;
}

extern mam_error_t
mam_array_get_dimension(
		mam_array_t      array,
		size_t           index,
		mam_dimension_t *dimension_ret) {
	MAM_CHECK_PTR(array);
	MAM_CHECK_PTR(dimension_ret);
	MAM_REFUTE(index >= utarray_len(array->dimensions), MAM_EINVAL);
	memcpy(dimension_ret, *(mam_dimension_t **)utarray_eltptr(array->dimensions, index), sizeof(mam_dimension_t));
	return MAM_SUCCESS;
}

extern mam_error_t
mam_array_get_size(
		mam_array_t  array,
		ssize_t     *size_ret) {
	MAM_CHECK_PTR(array);
	MAM_CHECK_PTR(size_ret);
	*size_ret = array->total_size;
	return MAM_SUCCESS;
}

extern mam_error_t
mam_array_get_align(
		mam_array_t  array,
		size_t      *align_ret) {
	MAM_CHECK_PTR(array);
	MAM_CHECK_PTR(align_ret);
	*align_ret = array->alignment;
	return MAM_SUCCESS;
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
	MAM_REFUTE(array->frozen, MAM_EFROZEN);
	MAM_CHECK_PTR(dimension);
	//TODO: should validate dimension content here

	mam_error_t err = MAM_SUCCESS;
	size_t sz = sizeof(mam_dimension_t);
	sz += dimension->type == MAM_DIMENSION_TYPE_VARIABLE ?
		strlen(dimension->path) + 1 : 0;
	sz += dimension->padded && dimension->pad.type == MAM_DIMENSION_TYPE_VARIABLE ?
		strlen(dimension->pad.path) + 1 : 0;
	mam_dimension_t *p_dimension = (mam_dimension_t *)calloc(1, sz);
	MAM_REFUTE(!p_dimension, MAM_ENOMEM);
	memcpy(p_dimension, dimension, sizeof(mam_dimension_t));
	utarray_push_back(array->dimensions, &p_dimension);
	char *mem = (char *)p_dimension + sizeof(mam_dimension_t);
	if (dimension->type == MAM_DIMENSION_TYPE_VARIABLE) {
		char *p_path = mem;
		strcpy(p_path, dimension->path);
		p_dimension->path = p_path;
		mem += strlen(dimension->path) + 1;
	}
	if (dimension->padded && dimension->pad.type == MAM_DIMENSION_TYPE_VARIABLE) {
		char *p_path = mem;
		strcpy(p_path, dimension->pad.path);
		p_dimension->pad.path = p_path;
	}

	mam_dimension_type_t type;
	size_t               count;
	if (dimension->padded) {
		type = dimension->pad.type;
		if (type == MAM_DIMENSION_TYPE_FIXED)
			count = dimension->pad.count;
	} else {
		type = dimension->type;
		if (type == MAM_DIMENSION_TYPE_FIXED)
			count = dimension->count;
	}

	if (type == MAM_DIMENSION_TYPE_FIXED) {
		if (array->total_size == 0 && array->elem_size >= 0)
			array->total_size = array->elem_size * count;
		else if (array->total_size > 0)
			array->total_size *= count;
	} else
		array->total_size = -1;

	return MAM_SUCCESS;
err_mem:
	free(p_dimension);
	return err;
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
	mam_variable_t variable = NULL;
	HASH_FIND(hh_name, context->variables_name_hash,
		name, sz_name, variable);
	MAM_REFUTE(variable, MAM_EINVAL);

	ssize_t size;
	size_t align;
	MAM_VALIDATE(_mam_get_field_type_size_align(
		context->platform, field_type, &size, &align));

	mam_error_t err = MAM_SUCCESS;
	variable = (struct _mam_variable_s *)
		calloc(1, sizeof(struct _mam_variable_s) + sz_name + 1);
	MAM_REFUTE(!variable, MAM_ENOMEM);
	char *p_name = (char *)variable + sizeof(struct _mam_variable_s);
	strcpy(p_name, name);
	variable->context = context;
	variable->name = p_name;
	variable->alignment = align;
	variable->size = size;
	memcpy(&variable->field_type, field_type, sizeof(mam_field_type_t));
	_mam_freeze_field_type(field_type);
	utarray_push_back(context->variables, &variable);
	HASH_ADD_KEYPTR( hh_name, context->variables_name_hash,
		variable->name, sz_name, variable );
	*variable_ret = variable;
	return MAM_SUCCESS;
err_arr:
	utarray_pop_back(context->variables);
err_mem:
	free(variable);
	return err;
}

mam_error_t
mam_variable_get_field_type(
		mam_variable_t    variable,
		mam_field_type_t *field_type_ret) {
	MAM_CHECK_PTR(variable);
	MAM_CHECK_PTR(field_type_ret);
	memcpy(field_type_ret, &variable->field_type, sizeof(mam_field_type_t));
	return MAM_SUCCESS;
}

mam_error_t
mam_variable_get_name(
		mam_variable_t   variable,
		const char     **name_ret) {
	MAM_CHECK_PTR(variable);
	MAM_CHECK_PTR(name_ret);
	*name_ret = variable->name;
	return MAM_SUCCESS;
}

mam_error_t
mam_variable_get_size(
	mam_variable_t  variable,
	ssize_t        *size_ret) {
	MAM_CHECK_PTR(variable);
	MAM_CHECK_PTR(size_ret);
	*size_ret = variable->size;
	return MAM_SUCCESS;
}

mam_error_t
mam_variable_get_align(
	mam_variable_t  variable,
	size_t         *align_ret) {
	MAM_CHECK_PTR(variable);
	MAM_CHECK_PTR(align_ret);
	*align_ret = variable->alignment;
	return MAM_SUCCESS;
}

static inline mam_error_t
_mam_pointer_destroy(mam_pointer_t pointer) {
	free(pointer);
	return MAM_SUCCESS;
}

static inline mam_error_t
_mam_array_destroy(mam_array_t array) {
	mam_dimension_t **p = NULL;
	while ((p = (mam_dimension_t **)utarray_next(array->dimensions, p)))
		free(*p);
	utarray_free(array->dimensions);
	free(array);
	return MAM_SUCCESS;
}

static inline mam_error_t
_mam_construct_destroy(mam_construct_t construct) {
	struct _mam_field_s **p = NULL;
	HASH_CLEAR(hh_name, construct->fields_name_hash);
	while ((p = (struct _mam_field_s **)utarray_next(construct->fields, p)))
		free(*p);
	utarray_free(construct->fields);
	free(construct);
	return MAM_SUCCESS;
}

static inline mam_error_t
_mam_variable_destroy(mam_variable_t variable) {
	free(variable);
	return MAM_SUCCESS;
}

mam_error_t
mam_context_destroy(
		mam_context_t context) {
	MAM_CHECK_PTR(context);
	HASH_CLEAR(hh_name, context->variables_name_hash);
	HASH_CLEAR(hh_name, context->constructs_name_hash);
	mam_pointer_t *p_p = NULL;
	while ((p_p = (mam_pointer_t *)utarray_next(context->pointers, p_p)))
		MAM_VALIDATE(_mam_pointer_destroy(*p_p));
	mam_array_t *p_a = NULL;
	while ((p_a = (mam_array_t *)utarray_next(context->arrays, p_a)))
		MAM_VALIDATE(_mam_array_destroy(*p_a));
	mam_construct_t *p_c = NULL;
	while ((p_c = (mam_construct_t *)utarray_next(context->constructs, p_c)))
		MAM_VALIDATE(_mam_construct_destroy(*p_c));
	mam_variable_t *p_v = NULL;
	while ((p_v = (mam_variable_t *)utarray_next(context->variables, p_v)))
		MAM_VALIDATE(_mam_variable_destroy(*p_v));
	utarray_free(context->pointers);
	utarray_free(context->arrays);
	utarray_free(context->constructs);
	utarray_free(context->variables);
	free(context);
	return MAM_SUCCESS;
}
