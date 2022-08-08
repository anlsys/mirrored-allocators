#ifndef COPY_ENGINE_INTERNAL_H
#define COPY_ENGINE_INTERNAL_H

#include <mam.h>
#include "utarray.h"
#ifdef HASH_NONFATAL_OOM 
#undef HASH_NONFATAL_OOM 
#endif 
#define HASH_NONFATAL_OOM 1 
#include "uthash.h"


struct _mam_field_s {
	const char        *name;
	size_t             offset;
	mam_field_type_t   field_type;
	UT_hash_handle     hh_name;
};
typedef struct _mam_field_s _mam_field_t;

struct _mam_pointer_s {
	mam_context_t    context;
	mam_field_type_t field_type;
};

static const UT_icd _mam_dimension_icd = {
	sizeof(mam_dimension_t *),
	NULL,
	NULL,
	NULL
};

struct _mam_array_s {
	mam_context_t     context;
	ssize_t           elem_size;
	ssize_t           total_size;
	size_t            alignment;
	mam_field_type_t  field_type;
	UT_array         *dimensions;
	bool              frozen;
};

static const UT_icd _mam_field_icd = {
	sizeof(_mam_field_t *),
	NULL,
	NULL,
	NULL
};

struct _mam_construct_s {
	mam_context_t         context;
	const char           *name;
	mam_construct_type_t  type;
	bool                  packed;
	size_t                last_offset;
	size_t                total_size;
	size_t                alignment;
	UT_array             *fields;
	_mam_field_t         *fields_name_hash;
	bool                  frozen;
};

struct _mam_platform_s {
	const char     *name;
	mam_type_map_t  type_map;
	mam_endian_t    endian;
};

struct _mam_variable_s {
	mam_context_t      context;
	const char        *name;
	mam_field_type_t   field_type;
};

static const UT_icd _mam_object_icd = {
	sizeof(void *),
	NULL,
	NULL,
	NULL
};

struct _mam_context_s {
	const char     *name;
	mam_platform_t  platform;
	UT_array       *pointers;
	UT_array       *arrays;
	UT_array       *constructs;
	UT_array       *variables;
};


#endif //COPY_ENGINE_INTERNAL_H
