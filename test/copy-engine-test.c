#include <mam.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MAM_CHECK_ERROR(cmd, error) do {  \
	mam_error_t _err = cmd;           \
	assert( error == _err );          \
} while(0)

#define MAM_CHECK(cmd) MAM_CHECK_ERROR(cmd, MAM_SUCCESS)

void test_define_context() {
	mam_platform_t   platform;
	mam_context_t    context;
	mam_construct_t  my_union, my_struct, construct_ret;
	mam_array_t      my_static_array, my_dynamic_array;
	mam_pointer_t    my_pointer;
	mam_variable_t   variable_sz, variable_ret;
	mam_field_type_t field_type;
	mam_dimension_t  dimension;
	size_t           number;

	MAM_CHECK(mam_platform_create_host(&platform));
	assert(platform);
	MAM_CHECK(mam_context_create("ctx", platform, &context));
	assert(context);
	MAM_CHECK(mam_context_get_constructs_number(context, &number));
	assert(number == 0);
	MAM_CHECK(mam_context_get_variables_number(context, &number));
	assert(number == 0);

	field_type.type = MAM_MAPPED_TYPE_DOUBLE;
	MAM_CHECK(mam_context_create_array(context, &field_type, &my_static_array));
	assert(my_static_array);
	dimension.type = MAM_DIMENSION_TYPE_FIXED;
	dimension.padded = false;
	dimension.count = 3;
	MAM_CHECK(mam_array_add_dimension(my_static_array, &dimension));
	dimension.count = 15;
	MAM_CHECK(mam_array_add_dimension(my_static_array, &dimension));

	MAM_CHECK(mam_context_create_union(context, "my_union_u", false, &my_union));
	assert(my_union);
	MAM_CHECK(mam_context_get_constructs_number(context, &number));
	assert(number == 1);
	MAM_CHECK(mam_context_get_construct(context, number - 1, &construct_ret));
	assert(construct_ret == my_union);
	construct_ret = NULL;
	MAM_CHECK(mam_context_get_construct_by_name(context, "my_union_u", &construct_ret));
	assert(construct_ret == my_union);
	field_type.type = MAM_MAPPED_TYPE_DOUBLE;
	MAM_CHECK(mam_construct_add_field(my_union, "d", &field_type, MAM_FIELD_OPTION_END));
	field_type.type = MAM_MAPPED_TYPE_INT;
	MAM_CHECK(mam_construct_add_field(my_union, "i", &field_type, MAM_FIELD_OPTION_END));

	MAM_CHECK(mam_context_create_struct(context, "my_struct_s", false, &my_struct));
	assert(my_struct);
	MAM_CHECK(mam_context_get_constructs_number(context, &number));
	assert(number == 2);
	MAM_CHECK(mam_context_get_construct(context, number - 1, &construct_ret));
	assert(construct_ret == my_struct);
	construct_ret = NULL;
	MAM_CHECK(mam_context_get_construct_by_name(context, "my_struct_s", &construct_ret));
	assert(construct_ret == my_struct);
	field_type.type = MAM_COMPLEX_TYPE_STRUCT;
	field_type.construct = my_struct;
	MAM_CHECK(mam_context_create_pointer(context, &field_type, &my_pointer));
	assert(my_pointer);

	field_type.type = MAM_MAPPED_TYPE_FLOAT;
	MAM_CHECK(mam_construct_add_field(my_struct, "f", &field_type, MAM_FIELD_OPTION_END));
	field_type.type = MAM_COMPLEX_TYPE_UNION;
	field_type.construct = my_union;
	MAM_CHECK(mam_construct_add_field(my_struct, "u", &field_type, MAM_FIELD_OPTION_END));
	field_type.type = MAM_MAPPED_TYPE_POINTER;
	field_type.pointer = my_pointer;
	MAM_CHECK(mam_construct_add_field(my_struct, "pnext", &field_type, MAM_FIELD_OPTION_END));
	field_type.type = MAM_COMPLEX_TYPE_ARRAY;
	field_type.array = my_static_array;
	MAM_CHECK(mam_construct_add_field(my_struct, "a", &field_type, MAM_FIELD_OPTION_END));

	field_type.type = MAM_MAPPED_TYPE_SIZE;
	MAM_CHECK(mam_context_create_variable(context, "sz", &field_type, &variable_sz));
	assert(variable_sz);
	MAM_CHECK(mam_context_get_variables_number(context, &number));
	assert(number == 1);
	MAM_CHECK(mam_context_get_variable(context, number - 1, &variable_ret));
	assert(variable_ret == variable_sz);
	variable_ret = NULL;
	MAM_CHECK(mam_context_get_variable_by_name(context, "sz", &variable_ret));
	assert(variable_ret == variable_sz);

	field_type.type = MAM_COMPLEX_TYPE_STRUCT;
	field_type.construct = my_struct;
	MAM_CHECK(mam_context_create_array(context, &field_type, &my_dynamic_array));
	assert(my_dynamic_array);
	dimension.type = MAM_DIMENSION_TYPE_VARIABLE;
	dimension.path = "//sz";
	MAM_CHECK(mam_array_add_dimension(my_dynamic_array, &dimension));

	MAM_CHECK(mam_context_destroy(context));
	MAM_CHECK(mam_platform_destroy(platform));
}

void test_struct() {
	mam_platform_t        platform;
	mam_context_t         context;
	mam_construct_t       my_struct, my_union;
	mam_array_t           my_array;
	size_t                size, align, offset, fields_number;
	mam_field_type_t      field_type, field_type_r;
	mam_construct_type_t  construct_type;
	const char           *name;
	mam_dimension_t       dimension;

	MAM_CHECK(mam_platform_create_host(&platform));
	assert(platform);
	MAM_CHECK(mam_context_create("ctx", platform, &context));
	assert(context);

	MAM_CHECK(mam_context_create_struct(context, "my_struct_s", false, &my_struct));
	assert(my_struct);
	MAM_CHECK(mam_construct_get_type(my_struct, &construct_type));
	assert(construct_type == MAM_CONSTRUCT_TYPE_STRUCT);
	MAM_CHECK(mam_construct_get_name(my_struct, &name));
	assert(!strcmp(name, "my_struct_s"));
	MAM_CHECK(mam_construct_get_fields_number(my_struct, &fields_number));
	assert(fields_number == 0);
	MAM_CHECK(mam_construct_get_size(my_struct, &size));
	assert(size == 0);
	MAM_CHECK(mam_construct_get_align(my_struct, &align));
	assert(align == 1);

	field_type.type = MAM_MAPPED_TYPE_HALF;
	MAM_CHECK(mam_construct_add_field(my_struct, "h", &field_type, MAM_FIELD_OPTION_END));
	MAM_CHECK(mam_construct_get_size(my_struct, &size));
	assert(size == 2);
	MAM_CHECK(mam_construct_get_align(my_struct, &align));
	assert(align == 2);
	MAM_CHECK(mam_construct_get_fields_number(my_struct, &fields_number));
	assert(fields_number == 1);
	MAM_CHECK(mam_construct_get_field(my_struct, fields_number - 1, &name, &offset, &size, &field_type_r));
	assert(!strcmp(name, "h"));
	assert(offset == 0);
	assert(size == 2);
	assert(field_type_r.type == MAM_MAPPED_TYPE_HALF);

	field_type.type = MAM_MAPPED_TYPE_FLOAT;
	MAM_CHECK(mam_construct_add_field(my_struct, "f", &field_type, MAM_FIELD_OPTION_END));
	MAM_CHECK(mam_construct_get_size(my_struct, &size));
	assert(size == 8);
	MAM_CHECK(mam_construct_get_align(my_struct, &align));
	assert(align == 4);
	MAM_CHECK(mam_construct_get_fields_number(my_struct, &fields_number));
	assert(fields_number == 2);
	MAM_CHECK(mam_construct_get_field(my_struct, fields_number - 1, &name, &offset, &size, &field_type_r));
	assert(!strcmp(name, "f"));
	assert(offset == 4);
	assert(size == 4);
	assert(field_type_r.type == MAM_MAPPED_TYPE_FLOAT);

	MAM_CHECK(mam_context_create_union(context, "my_union_u", false, &my_union));
	assert(my_union);
	field_type.type = MAM_MAPPED_TYPE_DOUBLE;
	MAM_CHECK(mam_construct_add_field(my_union, "d", &field_type, MAM_FIELD_OPTION_END));
	field_type.type = MAM_MAPPED_TYPE_INT;
	MAM_CHECK(mam_construct_add_field(my_union, "i", &field_type, MAM_FIELD_OPTION_END));
	field_type.type = MAM_COMPLEX_TYPE_UNION;
	field_type.construct = my_union;
	MAM_CHECK(mam_construct_add_field(my_struct, "u", &field_type, MAM_FIELD_OPTION_END));
	MAM_CHECK(mam_construct_get_size(my_struct, &size));
	assert(size == 16);
	MAM_CHECK(mam_construct_get_align(my_struct, &align));
	assert(align == 8);
	MAM_CHECK(mam_construct_get_fields_number(my_struct, &fields_number));
	assert(fields_number == 3);
	MAM_CHECK(mam_construct_get_field(my_struct, fields_number - 1, &name, &offset, &size, &field_type_r));
	assert(!strcmp(name, "u"));
	assert(offset == 8);
	assert(size == 8);
	assert(field_type_r.type == MAM_COMPLEX_TYPE_UNION);
	assert(field_type_r.construct == my_union);

	field_type.type = MAM_MAPPED_TYPE_FLOAT;
	MAM_CHECK(mam_context_create_array(context, &field_type, &my_array));
	assert(my_array);
	dimension.type = MAM_DIMENSION_TYPE_FIXED;
	dimension.padded = false;
	dimension.count = 3;
	MAM_CHECK(mam_array_add_dimension(my_array, &dimension));
	dimension.count = 15;
	MAM_CHECK(mam_array_add_dimension(my_array, &dimension));
	field_type.type = MAM_COMPLEX_TYPE_ARRAY;
	field_type.array = my_array;
	MAM_CHECK(mam_construct_add_field(my_struct, "a", &field_type, MAM_FIELD_OPTION_END));
	MAM_CHECK(mam_construct_get_size(my_struct, &size));
	assert(size == 200);
	MAM_CHECK(mam_construct_get_align(my_struct, &align));
	assert(align == 8);
	MAM_CHECK(mam_construct_get_fields_number(my_struct, &fields_number));
	assert(fields_number == 4);
	MAM_CHECK(mam_construct_get_field(my_struct, fields_number - 1, &name, &offset, &size, &field_type_r));
	assert(!strcmp(name, "a"));
	assert(offset == 16);
	assert(size == 180);
	assert(field_type_r.type == MAM_COMPLEX_TYPE_ARRAY);
	assert(field_type_r.array == my_array);


	MAM_CHECK(mam_construct_get_field_by_name(my_struct, "h", &offset, &size, &field_type_r));
	assert(offset == 0);
	assert(size == 2);
	assert(field_type_r.type == MAM_MAPPED_TYPE_HALF);

	MAM_CHECK(mam_construct_get_field_by_name(my_struct, "a", &offset, &size, &field_type_r));
	assert(offset == 16);
	assert(size == 180);
	assert(field_type_r.type == MAM_COMPLEX_TYPE_ARRAY);
	assert(field_type_r.array == my_array);

	MAM_CHECK(mam_construct_get_field_by_name(my_struct, "f", &offset, &size, &field_type_r));
	assert(offset == 4);
	assert(size == 4);
	assert(field_type_r.type == MAM_MAPPED_TYPE_FLOAT);

	MAM_CHECK(mam_construct_get_field_by_name(my_struct, "u", &offset, &size, &field_type_r));
	assert(offset == 8);
	assert(size == 8);
	assert(field_type_r.type == MAM_COMPLEX_TYPE_UNION);
	assert(field_type_r.construct == my_union);

	MAM_CHECK(mam_context_destroy(context));
	MAM_CHECK(mam_platform_destroy(platform));
}

void test_array() {
	mam_platform_t    platform;
	mam_context_t     context;
	mam_array_t       my_static_array, my_dynamic_array;
	size_t            align, num_dimensions;
	ssize_t           size;
	mam_field_type_t  field_type, field_type_r;
	mam_dimension_t   dimension, dimension_r;

	MAM_CHECK(mam_platform_create_host(&platform));
	assert(platform);
	MAM_CHECK(mam_context_create("ctx", platform, &context));
	assert(context);

	field_type.type = MAM_MAPPED_TYPE_DOUBLE;
	MAM_CHECK(mam_context_create_array(context, &field_type, &my_static_array));
	assert(my_static_array);
	MAM_CHECK(mam_array_get_field_type(my_static_array, &field_type_r));
	assert(field_type_r.type == MAM_MAPPED_TYPE_DOUBLE);
	MAM_CHECK(mam_array_get_num_dimensions(my_static_array, &num_dimensions));
	assert(num_dimensions == 0);
	MAM_CHECK(mam_array_get_size(my_static_array, &size));
	assert(size == 0);
	MAM_CHECK(mam_array_get_align(my_static_array, &align));
	assert(align == 8);

	dimension.type = MAM_DIMENSION_TYPE_FIXED;
	dimension.padded = false;
	dimension.count = 3;
	MAM_CHECK(mam_array_add_dimension(my_static_array, &dimension));
	MAM_CHECK(mam_array_get_num_dimensions(my_static_array, &num_dimensions));
	assert(num_dimensions == 1);
	MAM_CHECK(mam_array_get_size(my_static_array, &size));
	assert(size == 24);
	MAM_CHECK(mam_array_get_align(my_static_array, &align));
	assert(align == 8);

	dimension.count = 15;
	MAM_CHECK(mam_array_add_dimension(my_static_array, &dimension));
	MAM_CHECK(mam_array_get_num_dimensions(my_static_array, &num_dimensions));
	assert(num_dimensions == 2);
	MAM_CHECK(mam_array_get_size(my_static_array, &size));
	assert(size == 360);
	MAM_CHECK(mam_array_get_align(my_static_array, &align));
	assert(align == 8);

	MAM_CHECK(mam_array_get_dimension(my_static_array, 0, &dimension_r));
	assert(dimension_r.type == MAM_DIMENSION_TYPE_FIXED);
	assert(dimension_r.count == 3);
	MAM_CHECK(mam_array_get_dimension(my_static_array, 1, &dimension_r));
	assert(dimension_r.type == MAM_DIMENSION_TYPE_FIXED);
	assert(dimension_r.count == 15);


	MAM_CHECK(mam_context_create_array(context, &field_type, &my_dynamic_array));
	assert(my_dynamic_array);
	MAM_CHECK(mam_array_get_field_type(my_dynamic_array, &field_type_r));
	assert(field_type_r.type == MAM_MAPPED_TYPE_DOUBLE);
	MAM_CHECK(mam_array_get_num_dimensions(my_dynamic_array, &num_dimensions));
	assert(num_dimensions == 0);
	MAM_CHECK(mam_array_get_size(my_dynamic_array, &size));
	assert(size == 0);
	MAM_CHECK(mam_array_get_align(my_dynamic_array, &align));
	assert(align == 8);

	dimension.type = MAM_DIMENSION_TYPE_FIXED;
	dimension.count = 4;
	MAM_CHECK(mam_array_add_dimension(my_dynamic_array, &dimension));
	MAM_CHECK(mam_array_get_num_dimensions(my_dynamic_array, &num_dimensions));
	assert(num_dimensions == 1);
	MAM_CHECK(mam_array_get_size(my_dynamic_array, &size));
	assert(size == 32);
	MAM_CHECK(mam_array_get_align(my_dynamic_array, &align));
	assert(align == 8);

	dimension.type = MAM_DIMENSION_TYPE_VARIABLE;
	dimension.path = "//sz";
	MAM_CHECK(mam_array_add_dimension(my_dynamic_array, &dimension));
	MAM_CHECK(mam_array_get_num_dimensions(my_dynamic_array, &num_dimensions));
	assert(num_dimensions == 2);
	MAM_CHECK(mam_array_get_size(my_dynamic_array, &size));
	assert(size == -1);
	MAM_CHECK(mam_array_get_align(my_dynamic_array, &align));
	assert(align == 8);

	MAM_CHECK(mam_array_get_dimension(my_dynamic_array, 0, &dimension_r));
	assert(dimension_r.type == MAM_DIMENSION_TYPE_FIXED);
	assert(dimension_r.count == 4);
	MAM_CHECK(mam_array_get_dimension(my_dynamic_array, 1, &dimension_r));
	assert(dimension_r.type == MAM_DIMENSION_TYPE_VARIABLE);
	assert(!strcmp(dimension.path, "//sz"));

	MAM_CHECK(mam_context_destroy(context));
	MAM_CHECK(mam_platform_destroy(platform));
}

void test_pointer() {
	mam_platform_t   platform;
	mam_context_t    context;
	mam_pointer_t    my_pointer;
	mam_field_type_t field_type, field_type_r;

	MAM_CHECK(mam_platform_create_host(&platform));
	assert(platform);
	MAM_CHECK(mam_context_create("ctx", platform, &context));
	assert(context);

	field_type.type = MAM_DATA_TYPE_UINT32;
	MAM_CHECK(mam_context_create_pointer(context, &field_type, &my_pointer));
	assert(my_pointer);
	MAM_CHECK(mam_pointer_get_field_type(my_pointer, &field_type_r));
	assert(field_type_r.type == MAM_DATA_TYPE_UINT32);

	MAM_CHECK(mam_context_destroy(context));
	MAM_CHECK(mam_platform_destroy(platform));
}

void test_variable() {
	mam_platform_t    platform;
	mam_context_t     context;
	mam_variable_t    my_variable;
	mam_field_type_t  field_type, field_type_r;
	ssize_t           size;
	size_t            align;
	const char       *name;

	MAM_CHECK(mam_platform_create_host(&platform));
	assert(platform);
	MAM_CHECK(mam_context_create("ctx", platform, &context));
	assert(context);

	field_type.type = MAM_MAPPED_TYPE_SIZE;
	MAM_CHECK(mam_context_create_variable(context, "sz", &field_type, &my_variable));
	assert(my_variable);

	MAM_CHECK(mam_variable_get_name(my_variable, &name));
	assert(!strcmp(name, "sz"));
	MAM_CHECK(mam_variable_get_field_type(my_variable, &field_type_r));
	assert(field_type_r.type == MAM_MAPPED_TYPE_SIZE);
	MAM_CHECK(mam_variable_get_size(my_variable, &size));
	assert(size == sizeof(size_t));
	MAM_CHECK(mam_variable_get_align(my_variable, &align));
	assert(align == sizeof(size_t));

	MAM_CHECK(mam_context_destroy(context));
	MAM_CHECK(mam_platform_destroy(platform));
}

int main() {
	test_define_context();
	test_struct();
	test_array();
	test_pointer();
}
