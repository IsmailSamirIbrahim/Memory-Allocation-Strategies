#include "memory/memory.h"
#include "memory/block.h"

#include <malloc.h>
#include <string.h>
#include <assert.h>

namespace msa
{
	struct IArena
	{
		Block memory;
		uint8_t* head;
		size_t allocation_count;
	};

	// Helper functions
	inline static bool
	is_power_of_two(uint8_t align)
	{
		return (align & (align - 1)) == 0;
	}

	inline static uint8_t*
	align_forward(uint8_t* address, uint8_t align)
	{
		assert(is_power_of_two(align));

		uintptr_t new_address, specified_alignment, modulo;

		new_address = (uintptr_t)address;
		specified_alignment = (uintptr_t)align;

		modulo = new_address & (specified_alignment - 1); // ==> (address % _align)

		if (modulo != 0)
			new_address += specified_alignment - modulo;

		return (uint8_t*)new_address;
	}


	// API
	Arena
	arena_new(size_t memory_size)
	{
		Arena self = (Arena)::malloc(sizeof(IArena));

		self->memory = Block{ ::malloc(memory_size), memory_size };
		self->head = (uint8_t*)self->memory.ptr;
		self->allocation_count = 0;

		return self;
	}

	void
	arena_free(Arena self)
	{
		::free(self->memory.ptr);
		::free(self);
	}

	void
	arena_free_all(Arena self)
	{
		self->head = (uint8_t*)self->memory.ptr;
		self->allocation_count = 0;
	}

	Block
	arena_alloc(Arena self, size_t size, uint8_t align)
	{
		uint8_t* ptr = align_forward(self->head, align);
		uintptr_t padded = ptr - self->head;
		uintptr_t offset = ptr - self->memory.ptr;

		assert(offset + size <= self->memory.size && "Arena allocator doesn't have enough memory");

		self->head += (size + padded);
		++self->allocation_count;
		::memset(ptr, 0, size);

		return Block{ ptr, size };
	}

}