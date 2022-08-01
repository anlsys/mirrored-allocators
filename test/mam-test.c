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

#define NUM_ALLOC 1000000
#define BASE_ALLOC_SIZE 0x10
#define ALLOC_RANDOM 0x90
#define MAX_ALIGNEMENT_BITS 8

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

	sz = BASE_ALLOC_SIZE;
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
	size_t           n_buffs, sz, size, size2;
	mam_buff_desc_t *desc, *desc2;
	void            *addr, *m_addr, *addr2, *m_addr2;

	MAM_CHECK(mam_allocator_create(
		&allocator, &alloc1, &alloc2));
	sz = BASE_ALLOC_SIZE;
	MAM_CHECK(mam_allocator_alloc(
		allocator, sz, &addr, &m_addr, &desc));
	memset( addr, 0, sz );
	memset( m_addr, 0, sz );
	MAM_CHECK(mam_allocator_alloc(
		allocator, sz, &addr2, &m_addr2, &desc2));
	memset( addr2, 0, sz );
	memset( m_addr2, 0, sz );
	assert( desc == desc2 );
	assert( desc->size - desc->free == 2 * sz );
	assert( (intptr_t)addr2 - (intptr_t)addr == sz );
	assert( (intptr_t)m_addr2 - (intptr_t)m_addr == sz );

	size = sz;
	addr = addr2;
	m_addr = m_addr2;
	desc = desc2;
	for (int i = 0; i < NUM_ALLOC; i++) {
		sz = BASE_ALLOC_SIZE + (rand() % ALLOC_RANDOM);
		size_t align = (1 << (rand() % MAX_ALIGNEMENT_BITS));
		MAM_CHECK(mam_allocator_alloc_aligned(
			allocator, sz, align, &addr2, &m_addr2, &desc2));
		memset( addr2, 0, sz );
		memset( m_addr2, 0, sz );
		assert( 0 == (intptr_t)m_addr2 % align );
		if (desc == desc2) {
			assert( (intptr_t)addr2 - (intptr_t)addr >= size );
			assert( (intptr_t)m_addr2 - (intptr_t)m_addr >= size );
		}
		size = sz;
		addr = addr2;
		m_addr = m_addr2;
		desc = desc2;
	}
	MAM_CHECK(mam_allocator_destroy(allocator));
}

int main() {
	test_create();
	test_alloc();
}
