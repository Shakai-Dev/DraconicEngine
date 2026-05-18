module;
#include <cstddef>
#include <source_location>

export module core.memory.allocator;
export import core.memory.slice;

export namespace draco::memory
{
	enum class Error
	{
		Okay,
		OutOfMemory,
		NotImplemented,
		IllegalAddressRange,
		Other, // This one shouldn't be needed. If you see it returned, make a
			   // new error.
	};

	struct AllocatorVTbl;

	struct Allocator
	{
		AllocatorVTbl *vtbl;
		void *allocatorData;
		inline Error alloc(
			Slice *dst,
			size_t size,
			size_t align
#ifdef DEBUG
			, std::source_location loc = std::source_location::current()
			// std::stacktrace trace = std::stacktrace::current()
#endif
		);
		inline Error free(Slice block);
		inline Error freeAll();
	};

	struct AllocatorVTbl
	{
		using AllocFn = Error (*)(
			Allocator alloc,
			Slice *dst,
			size_t size,
			size_t align
#ifdef DEBUG
			, std::source_location loc
#endif
		);
		using FreeFn = Error (*)(Allocator alloc, Slice block);
		using FreeAllFn = Error (*)(Allocator alloc);
		AllocFn alloc;
		FreeFn free;
		FreeAllFn freeAll;
	};

	Error nilAlloc(
		Allocator alloc,
		Slice *dst,
		size_t size,
		size_t align
#ifdef DEBUG
		, std::source_location loc
#endif
	);

	Error nilFree(Allocator alloc, Slice block);

	Error nilFreeAll(Allocator alloc);

	void asAllocatorVoid(Allocator *dst, void *alloc, AllocatorVTbl *vtbl);
	inline Error Allocator::alloc(
			Slice *dst,
			size_t size,
			size_t align
#ifdef DEBUG
			, std::source_location loc
#endif
	)
	{
		return vtbl->alloc(
			*this,
			dst,
			size,
			align
#ifdef DEBUG
			, loc
#endif
		);
	}

	inline Error Allocator::free(Slice block)
	{
		return vtbl->free(*this, block);
	}

	inline Error Allocator::freeAll()
	{
		return vtbl->freeAll(*this);
	}
}
