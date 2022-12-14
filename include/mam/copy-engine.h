#ifndef MAM_COPY_ENGINE
#define MAM_COPY_ENGINE
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum mam_endian_e {
	MAM_ENDIAN_LITTLE = 0,
	MAM_ENDIAN_BIG,
	MAM_ENDIAN_MAX,
	MAM_ENDIAN_FORCE_32BIT = INT32_MAX
};
typedef enum mam_endian_e mam_endian_t;

enum mam_data_type_e {
	MAM_DATA_TYPE_INT8 = 0,
	MAM_DATA_TYPE_UINT8,
	MAM_DATA_TYPE_INT16,
	MAM_DATA_TYPE_UINT16,
	MAM_DATA_TYPE_INT32,
	MAM_DATA_TYPE_UINT32,
	MAM_DATA_TYPE_INT64,
	MAM_DATA_TYPE_UINT64,
	MAM_DATA_TYPE_INT128,
	MAM_DATA_TYPE_UINT128,
	MAM_DATA_TYPE_FLOAT16,
	MAM_DATA_TYPE_BFLOAT16,
	MAM_DATA_TYPE_FLOAT32,
	MAM_DATA_TYPE_FLOAT64,
	MAM_DATA_TYPE_MAX,
	MAM_DATA_TYPE_FORCE_32BIT = INT32_MAX
};
typedef enum mam_data_type_e mam_data_type_t;

enum mam_complex_type_e {
	MAM_COMPLEX_TYPE_STRUCT = 0x100,
	MAM_COMPLEX_TYPE_UNION,
	MAM_COMPLEX_TYPE_ARRAY,
	MAM_COMPLEX_TYPE_MAX,
	MAM_COMPLEX_TYPE_FORCE_32BIT = INT32_MAX
};

enum mam_mapped_type_e {
	MAM_MAPPED_TYPE_CHAR = 0x200,
	MAM_MAPPED_TYPE_UCHAR,
	MAM_MAPPED_TYPE_SHORT,
	MAM_MAPPED_TYPE_USHORT,
	MAM_MAPPED_TYPE_INT,
	MAM_MAPPED_TYPE_UINT,
	MAM_MAPPED_TYPE_LONG,
	MAM_MAPPED_TYPE_ULONG,
	MAM_MAPPED_TYPE_LONGLONG,
	MAM_MAPPED_TYPE_ULONGLONG,
	MAM_MAPPED_TYPE_SIZE,
	MAM_MAPPED_TYPE_SSIZE,
	MAM_MAPPED_TYPE_POINTER,
	MAM_MAPPED_TYPE_PTRDIFF,
	MAM_MAPPED_TYPE_INTPTR,
	MAM_MAPPED_TYPE_UINTPTR,
	MAM_MAPPED_TYPE_HALF,
	MAM_MAPPED_TYPE_FLOAT,
	MAM_MAPPED_TYPE_DOUBLE,
	MAM_MAPPED_TYPE_MAX,
	MAM_MAPPED_TYPE_FORCE_32BIT = INT32_MAX
};
typedef enum mam_mapped_type_e mam_mapped_type_t;

typedef int32_t mam_type_t;

typedef mam_data_type_t mam_type_map_t[MAM_MAPPED_TYPE_MAX - MAM_MAPPED_TYPE_CHAR];

extern mam_type_map_t mam_default_type_map;

// a platform, describing among other things type constraints, and endianness
typedef struct _mam_platform_s *mam_platform_t;
// a context defined for a platform, gathering constructs definitions (structs, unions) and variables
typedef struct _mam_context_s *mam_context_t;
// a variable containing data to transform
typedef struct _mam_variable_s *mam_variable_t;
// a construct definition
typedef struct _mam_construct_s *mam_construct_t;
// potentially recursive types need to be defined using apis
typedef struct _mam_array_s *mam_array_t;
typedef struct _mam_pointer_s *mam_pointer_t;
// a transformation set (a set of rules to apply to a context)
typedef struct _mam_transform_set_s *mam_transform_set_t;

struct mam_field_type_s {
	mam_type_t type;
	union {
		mam_pointer_t   pointer;
		mam_array_t     array;
		mam_construct_t construct;
	};
	
};
typedef struct mam_field_type_s mam_field_type_t;

//extern mam_platform_t mam_current_platform;

extern mam_error_t
mam_platform_create(
	const char     *name,
	mam_type_map_t *type_map,
	mam_endian_t    endian,
	mam_platform_t *platform_ret);

extern mam_error_t
mam_platform_create_host(
	mam_platform_t *platform_ret);

extern mam_error_t
mam_platform_destroy(
	mam_platform_t platform);

extern mam_error_t
mam_context_create(
	const char     *name,
	mam_platform_t  platform,
	mam_context_t  *context_ret);

extern mam_error_t
mam_context_destroy(
	mam_context_t   context);

extern mam_error_t
mam_context_get_variables_number(
	mam_context_t  context,
	size_t        *variables_number_ret);

extern mam_error_t
mam_context_get_constructs_number(
	mam_context_t  context,
	size_t        *constructs_number_ret);

extern mam_error_t
mam_context_get_variable(
	mam_context_t   context,
	size_t          index,
	mam_variable_t *variable_ret);

extern mam_error_t
mam_context_get_construct(
	mam_context_t    context,
	size_t           index,
	mam_construct_t *construct_ret);

extern mam_error_t
mam_context_get_variable_by_name(
	mam_context_t   context,
	const char *    name,
	mam_variable_t *variable_ret);

extern mam_error_t
mam_context_get_construct_by_name(
	mam_context_t    context,
	const char *     name,
	mam_construct_t *construct_ret);

enum mam_construct_type_e {
	MAM_CONSTRUCT_TYPE_STRUCT = 0,
	MAM_CONSTRUCT_TYPE_UNION,
	MAM_CONSTRUCT_TYPE_MAX,
	MAM_CONSTRUCT_TYPE_FORCE_32BIT = INT32_MAX
};
typedef enum mam_construct_type_e mam_construct_type_t;

extern mam_error_t
mam_context_create_struct(
	mam_context_t    context,
	const char      *name,
	bool             packed,
	mam_construct_t *construct_ret);

extern mam_error_t
mam_context_create_union(
	mam_context_t    context,
	const char      *name,
	bool             packed,
	mam_construct_t *construct_ret);

extern mam_error_t
mam_construct_get_type(
	mam_construct_t       construct,
	mam_construct_type_t *type_ret);

extern mam_error_t
mam_construct_get_size(
	mam_construct_t  construct,
	size_t          *size_ret);

extern mam_error_t
mam_construct_get_align(
	mam_construct_t  construct,
	size_t          *align_ret);

extern mam_error_t
mam_construct_get_name(
	mam_construct_t   construct,
	const char      **name_ret);

extern mam_error_t
mam_construct_get_fields_number(
	mam_construct_t  construct,
	size_t          *field_count_ret);

extern mam_error_t
mam_construct_get_field(
	mam_construct_t    construct,
	size_t             index,
	const char       **name_ret,
	size_t            *offset_ret,
	size_t            *size_ret,
	mam_field_type_t  *field_type_ret);

extern mam_error_t
mam_construct_get_field_by_name(
	mam_construct_t   construct,
	const char       *name,
	size_t           *offset_ret,
	size_t           *size_ret,
	mam_field_type_t *field_type_ret);

enum mam_field_option_e {
	MAM_FIELD_OPTION_END = 0,
	MAM_FIELD_OPTION_OFFSET,
	MAM_FIELD_OPTION_MAX,
	MAM_FIELD_OPTION_FORCE_32BIT = INT32_MAX
};
typedef enum mam_field_option_e mam_field_option_t;

extern mam_error_t
mam_construct_add_field(
	mam_construct_t   construct,
	const char       *name,
	mam_field_type_t *field_type,
	...);

extern mam_error_t
mam_context_create_pointer(
	mam_context_t     context,
	mam_field_type_t *field_type,
	mam_pointer_t    *pointer_ret);

extern mam_error_t
mam_pointer_get_field_type(
	mam_pointer_t     pointer,
	mam_field_type_t *field_type_ret);

extern mam_error_t
mam_context_create_array(
	mam_context_t     context,
	mam_field_type_t *field_type,
	mam_array_t      *array_ret);

extern mam_error_t
mam_array_get_field_type(
	mam_array_t      array,
	mam_field_type_t *field_type_ret);

extern mam_error_t
mam_array_get_num_dimensions(
	mam_array_t  array,
	size_t      *num_dimension_ret);

extern mam_error_t
mam_array_get_size(
	mam_array_t  array,
	ssize_t     *size_ret);

extern mam_error_t
mam_array_get_align(
	mam_array_t  array,
	size_t      *align_ret);

enum mam_dimension_type_e {
	MAM_DIMENSION_TYPE_FIXED = 0,
	MAM_DIMENSION_TYPE_VARIABLE,
	MAM_DIMENSION_TYPE_MAX,
	MAM_DIMENSION_TYPE_FORCE_32BIT = INT32_MAX
};
typedef enum mam_dimension_type_e mam_dimension_type_t;

struct mam_dimension_s {
	mam_dimension_type_t  type;
	bool                  padded;
	union {
		size_t        count;
		const char   *path;
	};
	struct {
		mam_dimension_type_t  type;
		union {
			size_t        count;
			const char   *path;
		};
	} pad;
};
typedef struct mam_dimension_s mam_dimension_t;

extern mam_error_t
mam_array_add_dimension(
	mam_array_t      array,
	mam_dimension_t *dimension);

extern mam_error_t
mam_array_get_dimension(
	mam_array_t      array,
	size_t           index,
	mam_dimension_t *dimension_ret);

extern mam_error_t
mam_context_create_variable(
	mam_context_t     context,
	const char       *name,
	mam_field_type_t *field_type,
	mam_variable_t   *variable_ret);

extern mam_error_t
mam_variable_get_field_type(
	mam_variable_t    variable,
	mam_field_type_t *field_type_ret);

extern mam_error_t
mam_variable_get_name(
	mam_variable_t   variable,
	const char     **name_ret);

extern mam_error_t
mam_variable_get_size(
	mam_variable_t  variable,
	ssize_t        *size_ret);

extern mam_error_t
mam_variable_get_align(
	mam_variable_t  variable,
	size_t         *align_ret);

#ifdef __cplusplus
}
#endif

#endif //MAM_COPY_ENGINE
