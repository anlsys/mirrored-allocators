#include <mirrored-allocators.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#define MAM_CHECK_ERROR(cmd, error) do {  \
	mam_error_t _err = cmd;           \
	assert( error == _err );          \
} while(0)

#define MAM_CHECK(cmd) MAM_CHECK_ERROR(cmd, MAM_SUCCESS)

void test_create() {
	mam_allocator_t      allocator;
	mam_platform_alloc_t alloc1 = {
		malloc,
		free
	};
	mam_platform_alloc_t alloc2 = {
		malloc,
		free
	};
	size_t           n_buffs, sz;
	mam_buff_desc_t *desc;
	void            *addr, *m_addr;

	MAM_CHECK(mam_allocator_create(
		&allocator, &alloc1, &alloc2));
	MAM_CHECK(mam_allocator_get_buffer_descs(
		allocator, 0, NULL, &n_buffs));
	assert( 0 == n_buffs );

	sz = 0x10;
	MAM_CHECK(mam_allocator_alloc(
		allocator, sz, &addr, &m_addr, &desc));
	assert( addr );
	assert( m_addr );
	assert( desc );
	assert( desc->size - desc->free == sz );
	assert( addr == desc->addr );
	assert( m_addr == desc->m_addr );
	assert( 1 == desc->count );
	assert( !desc->ready );
	MAM_CHECK(mam_allocator_destroy(allocator));
	MAM_CHECK_ERROR(mam_allocator_create(
		NULL, &alloc1, &alloc2), MAM_EINVAL);
}

void test_alloc() {
	mam_allocator_t      allocator;
	mam_platform_alloc_t alloc1 = {
		malloc,
		free
	};
	mam_platform_alloc_t alloc2 = {
		malloc,
		free
	};
	size_t           n_buffs, sz;
	mam_buff_desc_t *desc, *desc2;
	void            *addr, *m_addr, *addr2, *m_addr2;

	MAM_CHECK(mam_allocator_create(
		&allocator, &alloc1, &alloc2));
	sz = 0x10;
	MAM_CHECK(mam_allocator_alloc(
		allocator, sz, &addr, &m_addr, &desc));
	MAM_CHECK(mam_allocator_alloc(
		allocator, sz, &addr2, &m_addr2, &desc2));
	assert( desc == desc2 );
	assert( desc->size - desc->free == 2 * sz );
	assert( (intptr_t)addr2 - (intptr_t)addr == sz );
	assert( (intptr_t)m_addr2 - (intptr_t)m_addr == sz );
	MAM_CHECK(mam_allocator_destroy(allocator));
}

int main() {
	test_create();
	test_alloc();
}
