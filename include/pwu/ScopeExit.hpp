#pragma once

#include <cstddef>
#include <new>
#include <type_traits>
#include <utility>

// std::experimental::scope_exit

namespace pwu {
template <typename Callable>
requires std::is_nothrow_invocable_v<Callable>
class ScopeExit {
public:
    template <typename T>
    requires (
        !std::is_same_v<std::remove_cvref_t<T>, ScopeExit> &&
        std::is_constructible_v<Callable, T>
    )
    explicit ScopeExit(T&& callable)
        noexcept (
            std::is_nothrow_constructible_v<Callable, T> ||
            std::is_nothrow_constructible_v<Callable, T&>
        )
        : storage {}
        , active { true } {
        if constexpr (!std::is_lvalue_reference_v<T> &&
            std::is_nothrow_constructible_v<Callable, T>) {
            new (storage) Callable(std::forward<T>(callable));
        } else try {
            new (storage) Callable(callable);
        } catch (...) {
            callable();
            throw;
        }
    }

    ScopeExit(ScopeExit&& other)
        noexcept (
            std::is_nothrow_move_constructible_v<Callable> ||
            std::is_nothrow_copy_constructible_v<Callable>
        )
        requires (
            std::is_nothrow_move_constructible_v<Callable> ||
            std::is_copy_constructible_v<Callable>
        )
        : storage {}
        , active { other.active } {
        auto callable = reinterpret_cast<Callable*>(other.storage);
        if constexpr (std::is_nothrow_move_constructible_v<Callable>) {
            new (storage) Callable(std::forward<Callable>(*callable));
        } else {
            new (storage) Callable(*callable);
        }
        other.Release();
    }

    ScopeExit(const ScopeExit&) = delete;

    ScopeExit& operator=(const ScopeExit&) = delete;
    ScopeExit& operator=(ScopeExit&&) = delete;

    void Release() noexcept {
        active = false;
    }

    ~ScopeExit() noexcept {
        auto callable = reinterpret_cast<Callable*>(storage);
        if (active) {
            callable->operator()();
        }
        callable->~Callable();
    }

private:
    alignas(Callable) std::byte storage[sizeof(Callable)];
    bool active;
};

template <typename Callable>
ScopeExit(Callable) -> ScopeExit<Callable>;
} // namespace pwu
