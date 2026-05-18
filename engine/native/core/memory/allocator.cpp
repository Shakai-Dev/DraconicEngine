module;

#include <cstddef>
#include <source_location>

module core.memory.allocator;

namespace draco::memory
{
	Error nilAlloc(
		Allocator alloc,
		Slice *dst,
		size_t size,
		size_t align
#ifdef DEBUG
		, std::source_location loc
#endif
	)
	{
		return Error::NotImplemented;
	}

	Error nilFree(Allocator alloc, Slice block)
	{
		return Error::NotImplemented;
	}

	Error nilFreeAll(Allocator alloc)
	{
		return Error::NotImplemented;
	}

	void asAllocatorVoid(Allocator *dst, void *alloc, AllocatorVTbl *vtbl)
	{
		dst->allocatorData = (void*)alloc;
		dst->vtbl = vtbl;
	}
}
