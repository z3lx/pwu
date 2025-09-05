export module pwu:ScopeExit;

import std;

export namespace pwu {
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
        : callable([&callable]() -> Callable {
            if constexpr (!std::is_lvalue_reference_v<T> &&
                std::is_nothrow_constructible_v<Callable, T>) {
                return Callable(std::forward<T>(callable));
            } else try {
                return Callable(callable);
            } catch (...) {
                callable();
                throw;
            }
        }())
        , active { true } {}

    ScopeExit(ScopeExit&& other)
        noexcept (
            std::is_nothrow_move_constructible_v<Callable> ||
            std::is_nothrow_copy_constructible_v<Callable>
        )
        requires (
            std::is_nothrow_move_constructible_v<Callable> ||
            std::is_copy_constructible_v<Callable>
        )
        : callable([&other]() -> Callable {
            if constexpr (std::is_nothrow_move_constructible_v<Callable>) {
                return Callable(std::forward<Callable>(other.callable));
            } else {
                return Callable(other.callable);
            }
        }())
        , active { other.active } {
        other.Release();
    }

    ScopeExit(const ScopeExit&) = delete;

    ScopeExit& operator=(const ScopeExit&) = delete;
    ScopeExit& operator=(ScopeExit&&) = delete;

    void Release() noexcept {
        active = false;
    }

    ~ScopeExit() noexcept {
        if (active) {
            callable();
        }
    }

private:
    Callable callable;
    bool active;
};

template <typename Callable>
ScopeExit(Callable) -> ScopeExit<Callable>;
} // namespace pwu
