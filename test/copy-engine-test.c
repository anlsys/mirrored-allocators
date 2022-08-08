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

void test_create() {
	mam_platform_t   platform;
	mam_context_t    context;
	mam_construct_t  my_union, my_struct;
	mam_array_t      my_static_array, my_dynamic_array;
	mam_pointer_t    my_pointer;
	mam_variable_t   variable_sz;
	mam_field_type_t field_type;
	mam_dimension_t  dimension;
	size_t           size, align;

	MAM_CHECK(mam_platform_create_host(&platform));
	assert(platform);
	MAM_CHECK(mam_context_create("ctx", platform, &context));
	assert(context);

	field_type.type = MAM_MAPPED_TYPE_DOUBLE;
	MAM_CHECK(mam_context_create_array(context, &field_type, &my_static_array));
	assert(my_static_array);
	dimension.type = MAM_DIMENSION_TYPE_FIXED;
	dimension.count = 3;
	MAM_CHECK(mam_array_add_dimension(my_static_array, &dimension));
	dimension.count = 15;
	MAM_CHECK(mam_array_add_dimension(my_static_array, &dimension));

	MAM_CHECK(mam_context_create_union(context, "my_union_u", false, &my_union));
	assert(my_union);
	field_type.type = MAM_MAPPED_TYPE_DOUBLE;
	MAM_CHECK(mam_construct_add_field(my_union, "d", &field_type, MAM_FIELD_OPTION_END));
	field_type.type = MAM_MAPPED_TYPE_INT;
	MAM_CHECK(mam_construct_add_field(my_union, "i", &field_type, MAM_FIELD_OPTION_END));

	MAM_CHECK(mam_context_create_struct(context, "my_sytuct_s", false, &my_struct));
	assert(my_struct);
	field_type.type = MAM_COMPLEX_TYPE_STRUCT;
	field_type.construct = my_struct;
	MAM_CHECK(mam_context_create_pointer(context, &field_type, &my_pointer));
	assert(my_pointer);

	field_type.type = MAM_MAPPED_TYPE_FLOAT;
	MAM_CHECK(mam_construct_add_field(my_struct, "f", &field_type, MAM_FIELD_OPTION_END));
	field_type.type = MAM_CONPLEX_TYPE_UNION;
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

int main() {
	test_create();
}
