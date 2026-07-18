module;

#include <cassert>
#include <cstring>
#include <new>
#include <type_traits>
#include <utility>

export module core.containers.array;
export import core.stdtypes;
export import core.memory;

import core.defs;
import core.math.functions;

using namespace draco::memory;

export namespace draco::containers
{

    /// @brief Type-erased structure handling the internal allocation & layout logic of arrays.
    /// This keeps heavy looping & re-allocation code out of template instantiations.
    struct UntypedArray
    {
        Allocator allocator;
        void *buffer;
        isize size;
        isize capacity;

        enum class Error
        {
            Okay = (int)memory::Error::Okay,
            OutOfMemory,
            InvalidArgument
        };

        using RelocateFn = void (*)(void *dst, void *src, isize count);
        using DestroyFn = void (*)(void *data, isize count);

        UntypedArray(Allocator alloc) : allocator{alloc}, buffer{nullptr}, size{0}, capacity{0} {}

        /// @brief Reserves capacity for at least the specified number of items.
        /// @param newCapacity Requested element capacity capacity.
        /// @param itemSize Size of each element in bytes.
        /// @param itemAlign Alignment requirement of the elements.
        /// @param relocate Callback to move elements to new memory storage.
        /// @param destroy Callback to destruct active elements.
        /// @return Error code indicating success or out of memory conditions.
        Error Reserve(isize newCapacity, isize itemSize, isize itemAlign, RelocateFn relocate, DestroyFn destroy)
        {
            if (newCapacity < 0 || itemSize <= 0)
                return Error::InvalidArgument;

            if (newCapacity <= capacity)
                return Error::Okay;

            isize maxRepresentable = ~((isize)0) & ~(isize)((unsigned long long)1 << (sizeof(isize) * 8 - 1));
            if (newCapacity > maxRepresentable / itemSize)
                return Error::OutOfMemory;

            Slice newDst;
            auto memoryErr = allocator.alloc(&newDst, newCapacity * itemSize, itemAlign);
            if (memoryErr != memory::Error::Okay)
            {
                return Error::OutOfMemory;
            }

            if (buffer)
            {
                if (relocate)
                {
                    relocate(newDst.data, buffer, size);
                }
                else
                {
                    std::memcpy(newDst.data, buffer, size * itemSize);
                }

                if (destroy)
                {
                    destroy(buffer, size);
                }

                Slice oldDst = {.data = buffer, .size = (usize)capacity * itemSize};
                allocator.free(oldDst);
            }

            capacity = newCapacity;
            buffer = newDst.data;
            return Error::Okay;
        }

        /// @brief Grows the capacity geometric policy tracking to guarantee efficient push operations.
        /// @param minCapacity Minimum total element count capacity required.
        /// @param itemSize Size of each element in bytes.
        /// @param itemAlign Alignment requirement of the elements.
        /// @param relocate Callback to move elements to new memory storage.
        /// @param destroy Callback to destruct active elements.
        /// @return Error code indicating success or out of memory conditions.
        Error GrowCapacity(isize minCapacity, isize itemSize, isize itemAlign, RelocateFn relocate, DestroyFn destroy)
        {
            if (capacity >= minCapacity)
                return Error::Okay;
            isize newCapacity = capacity + draco::math::max(capacity / 2, minCapacity);
            return Reserve(newCapacity, itemSize, itemAlign, relocate, destroy);
        }
    };

    /// @brief Type-safe template wrapper over an untyped dynamic continuous storage array.
    /// @tparam T Type of elements stored inside the container.
    template <typename T>
    struct Array
    {
    private:
        UntypedArray internal;

        static void RelocateElements(void *dst, void *src, isize count)
        {
            T *dstTyped = static_cast<T *>(dst);
            T *srcTyped = static_cast<T *>(src);
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                std::memcpy(dstTyped, srcTyped, count * sizeof(T));
            }
            else
            {
                isize constructed = 0;
                try
                {
                    for (isize i = 0; i < count; ++i)
                    {
                        ::new (static_cast<void *>(dstTyped + i)) T(std::move(srcTyped[i]));
                        constructed++;
                    }
                }
                catch (...)
                {
                    for (isize i = 0; i < constructed; ++i)
                    {
                        dstTyped[i].~T();
                    }
                    throw;
                }
            }
        }

        static void DestroyElements(void *data, isize count)
        {
            if constexpr (!std::is_trivially_destructible_v<T>)
            {
                T *typedData = static_cast<T *>(data);
                for (isize i = 0; i < count; ++i)
                {
                    typedData[i].~T();
                }
            }
        }

    public:
        using Error = UntypedArray::Error;

        Array(Allocator allocator) : internal(allocator) {}

        ~Array()
        {
            Clear();
            if (internal.buffer)
            {
                Slice oldDst = {.data = internal.buffer, .size = (usize)internal.capacity * sizeof(T)};
                internal.allocator.free(oldDst);
            }
        }

        Array(Array const &) = delete;
        Array &operator=(Array const &) = delete;

        Array(Array &&other) noexcept : internal(other.internal.allocator)
        {
            internal.buffer = other.internal.buffer;
            internal.size = other.internal.size;
            internal.capacity = other.internal.capacity;

            other.internal.buffer = nullptr;
            other.internal.size = 0;
            other.internal.capacity = 0;
        }

        Array &operator=(Array &&other) noexcept
        {
            if (this != &other)
            {
                Clear();
                if (internal.buffer)
                {
                    Slice oldDst = {.data = internal.buffer, .size = (usize)internal.capacity * sizeof(T)};
                    internal.allocator.free(oldDst);
                }
                internal.allocator = other.internal.allocator;
                internal.buffer = other.internal.buffer;
                internal.size = other.internal.size;
                internal.capacity = other.internal.capacity;

                other.internal.buffer = nullptr;
                other.internal.size = 0;
                other.internal.capacity = 0;
            }
            return *this;
        }

        /// @brief Access element at index with debug bounds verification.
        T &operator[](isize index)
        {
            assert(index >= 0 && index < internal.size);
            return static_cast<T *>(internal.buffer)[index];
        }

        /// @brief Access element at index with debug bounds verification.
        T const &operator[](isize index) const
        {
            assert(index >= 0 && index < internal.size);
            return static_cast<T const *>(internal.buffer)[index];
        }

        /// @brief Reserves space manually for future modifications.
        Error Reserve(isize newCapacity)
        {
            return internal.Reserve(newCapacity, sizeof(T), alignof(T), RelocateElements, DestroyElements);
        }

        /// @brief Pushes a copy of an element onto the back of the array.
        Error Push(T const &value)
        {
            T tmp(value);
            Error err = internal.GrowCapacity(internal.size + 1, sizeof(T), alignof(T), RelocateElements, DestroyElements);
            if (err != Error::Okay)
                return err;

            ::new (static_cast<void *>(static_cast<T *>(internal.buffer) + internal.size)) T(std::move(tmp));
            internal.size++;
            return Error::Okay;
        }

        /// @brief Pushes a moved element onto the back of the array.
        Error Push(T &&value)
        {
            T tmp(std::move(value));
            Error err = internal.GrowCapacity(internal.size + 1, sizeof(T), alignof(T), RelocateElements, DestroyElements);
            if (err != Error::Okay)
                return err;

            ::new (static_cast<void *>(static_cast<T *>(internal.buffer) + internal.size)) T(std::move(tmp));
            internal.size++;
            return Error::Okay;
        }

        /// @brief Constructs an element directly at the back of the array in place.
        template <typename... Args>
        Error Emplace(Args &&...args)
        {
            if constexpr ((std::is_same_v<std::decay_t<Args>, T> || ...))
            {
                T tmp(std::forward<Args>(args)...);
                Error err = internal.GrowCapacity(internal.size + 1, sizeof(T), alignof(T), RelocateElements, DestroyElements);
                if (err != Error::Okay)
                    return err;

                ::new (static_cast<void *>(static_cast<T *>(internal.buffer) + internal.size)) T(std::move(tmp));
                internal.size++;
                return Error::Okay;
            }
            else
            {
                Error err = internal.GrowCapacity(internal.size + 1, sizeof(T), alignof(T), RelocateElements, DestroyElements);
                if (err != Error::Okay)
                    return err;

                ::new (static_cast<void *>(static_cast<T *>(internal.buffer) + internal.size)) T(std::forward<Args>(args)...);
                internal.size++;
                return Error::Okay;
            }
        }

        /// @brief Removals the final item of the array & calls destructor logic if applicable.
        void Pop()
        {
            if (internal.size > 0)
            {
                internal.size--;
                if constexpr (!std::is_trivially_destructible_v<T>)
                {
                    static_cast<T *>(internal.buffer)[internal.size].~T();
                }
            }
        }

        /// @brief Clear out all elements without altering capacity parameters.
        void Clear()
        {
            DestroyElements(internal.buffer, internal.size);
            internal.size = 0;
        }

        T *Data() { return static_cast<T *>(internal.buffer); }
        T const *Data() const { return static_cast<T const *>(internal.buffer); }
        isize Size() const { return internal.size; }
        isize Capacity() const { return internal.capacity; }
        bool Empty() const { return internal.size == 0; }

        T *begin() { return Data(); }
        T *end() { return internal.size == 0 ? Data() : Data() + internal.size; }
        T const *begin() const { return Data(); }
        T const *end() const { return internal.size == 0 ? Data() : Data() + internal.size; }
    };
}
