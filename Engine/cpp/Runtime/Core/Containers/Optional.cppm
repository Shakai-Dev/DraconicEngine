module;

#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

export module core.containers.optional;
export import core.stdtypes;

export namespace draco::containers {

    /// @brief Tag structure defining empty nullable states.
    struct NullOpt {} inline constexpr nullopt{};

    /// @brief Value wrapper structure using local union configurations to represent an optional entity.
    /// @tparam T Underlying data object type.
    template <typename T>
    struct Optional
    {
    private:
        union
        {
            T valueStorage;
            char dummy;
        };
        bool hasValue;

    public:
        constexpr Optional() : dummy{}, hasValue(false) {}
        constexpr Optional(NullOpt) : dummy{}, hasValue(false) {}

        constexpr Optional(T const &value) : hasValue(true)
        {
            ::new (static_cast<void *>(&valueStorage)) T(value);
        }

        constexpr Optional(T &&value) : hasValue(true)
        {
            ::new (static_cast<void *>(&valueStorage)) T(std::move(value));
        }

        constexpr Optional(Optional const &other) : hasValue(other.hasValue)
        {
            if (hasValue)
            {
                ::new (static_cast<void *>(&valueStorage)) T(other.valueStorage);
            }
        }

        constexpr Optional(Optional &&other) noexcept(std::is_nothrow_move_constructible_v<T>)
            : hasValue(other.hasValue)
        {
            if (hasValue)
            {
                ::new (static_cast<void *>(&valueStorage)) T(std::move(other.valueStorage));
            }
        }

        ~Optional()
        {
            Reset();
        }

        constexpr Optional &operator=(NullOpt)
        {
            Reset();
            return *this;
        }

        constexpr Optional &operator=(Optional const &combat)
        {
            if (this != &combat)
            {
                Reset();
                if (combat.hasValue)
                {
                    ::new (static_cast<void *>(&valueStorage)) T(combat.valueStorage);
                    hasValue = true;
                }
            }
            return *this;
        }

        constexpr Optional &operator=(Optional &&combat) noexcept(std::is_nothrow_move_constructible_v<T>)
        {
            if (this != &combat)
            {
                Reset();
                if (combat.hasValue)
                {
                    ::new (static_cast<void *>(&valueStorage)) T(std::move(combat.valueStorage));
                    hasValue = true;
                }
            }
            return *this;
        }

        /// @brief Validates if an initialized object entry value is valid & configured.
        constexpr bool HasValue() const { return hasValue; }
        constexpr explicit operator bool() const { return hasValue; }

        /// @brief Returns reference to contained type with checking asset triggers.
        constexpr T &Value()
        {
            assert(hasValue);
            return valueStorage;
        }

        /// @brief Returns constant reference to contained type with checking asset triggers.
        constexpr T const &Value() const
        {
            assert(hasValue);
            return valueStorage;
        }

        constexpr T &operator*() { return Value(); }
        constexpr T const &operator*() const { return Value(); }

        constexpr T *operator->() { return &Value(); }
        constexpr T const *operator->() const { return &Value(); }

        /// @brief Unwraps the object, falling back to a custom default reference configuration if empty.
        constexpr T ValueOr(T &&defaultValue) const &
        {
            return hasValue ? valueStorage : std::forward<T>(defaultValue);
        }

        /// @brief Destructs active values & returns optional wrapper interface context back to null state.
        constexpr void Reset()
        {
            if (hasValue)
            {
                valueStorage.~T();
                hasValue = false;
            }
        }
    };
}
