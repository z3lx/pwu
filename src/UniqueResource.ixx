export module pwu:UniqueResource;

import std;

export namespace pwu {
template <typename R, typename D>
requires (
    (std::is_object_v<R> || std::is_lvalue_reference_v<R>) &&
    std::is_move_constructible_v<std::remove_reference_t<R>> && (
        std::is_copy_constructible_v<std::remove_reference_t<R>> ||
        std::is_nothrow_constructible_v<std::remove_reference_t<R>>
    ) &&
    std::is_destructible_v<D> &&
    std::is_move_constructible_v<D> && (
        std::is_copy_constructible_v<D> ||
        std::is_nothrow_constructible_v<D>
    ) &&
    std::is_nothrow_invocable_v<D, R>
)
class UniqueResource {
    using RS = std::conditional_t<
        std::is_object_v<R>,
        R,
        std::reference_wrapper<std::remove_reference_t<R>>
    >;

public:
    UniqueResource()
    requires (
        std::is_default_constructible_v<R> &&
        std::is_default_constructible_v<D>
    )
    : resource()
    , deleter()
    , owner { false } {}

    template <typename RR, typename DD>
    UniqueResource(RR& resource, DD&& deleter)
        noexcept (
            (
                std::is_nothrow_constructible_v<RS, RR> ||
                std::is_nothrow_constructible_v<RS, RR&>
            ) && (
                std::is_nothrow_constructible_v<D, DD> ||
                std::is_nothrow_constructible_v<D, DD&>
            )
        )
        requires (
            std::is_constructible_v<RS, RR> &&
            std::is_constructible_v<D, DD> && (
                std::is_nothrow_constructible_v<RS, RR> ||
                std::is_constructible_v<RS, RR&>
            ) && (
                std::is_nothrow_constructible_v<D, DD> ||
                std::is_constructible_v<D, DD&>
            )
        )
        : resource([&resource, &deleter]() -> RS {
            if constexpr (std::is_nothrow_constructible_v<RS, RR>) {
                return RS(std::forward<RR>(resource));
            } else try {
                return RS(resource);
            } catch (...) {
                deleter(resource);
                throw;
            }
        }())
        , deleter([&deleter, this]() -> D {
            if constexpr (std::is_nothrow_constructible_v<D, DD>) {
                return D(std::forward<DD>(deleter));
            } else try {
                return D(deleter);
            } catch (...) {
                deleter(this->resource);
                throw;
            }
        }())
        , owner { true } {}

    UniqueResource(UniqueResource&& other)
        noexcept (
            std::is_nothrow_move_constructible_v<R> &&
            std::is_nothrow_move_constructible_v<D>
        )
        : resource([&other]() -> RS {
            if constexpr (std::is_nothrow_move_constructible_v<RS>) {
                return RS(std::move(other.resource));
            } else {
                return RS(other.resource);
            }
        }())
        , deleter([&other, this]() -> D {
            if constexpr (std::is_nothrow_move_constructible_v<D>) {
                return D(std::move(other.deleter));
            } else try {
                return D(other.deleter);
            } catch (...) {
                if constexpr (std::is_nothrow_move_constructible_v<RS>) {
                    if (other.owner) {
                        other.deleter(resource);
                        other.Release();
                    }
                }
                throw;
            }
        }())
        , owner { other.owner } {
        other.Release();
    }

    ~UniqueResource() {
        Reset();
    }

    UniqueResource& operator=(UniqueResource&& other)
        noexcept (
            std::is_nothrow_move_assignable_v<RS> &&
            std::is_nothrow_move_assignable_v<D>
        )
        requires (
            (
                std::is_nothrow_move_assignable_v<RS>
                    ? std::is_move_assignable_v<RS>
                    : std::is_copy_assignable_v<RS>
            ) && (
                std::is_nothrow_move_assignable_v<D>
                    ? std::is_move_assignable_v<D>
                    : std::is_copy_assignable_v<D>
            )
        ) {
        Reset();
        const auto assign = []<typename T>(
            T& thisMember, T& otherMember) -> void {
            if constexpr (std::is_nothrow_move_assignable_v<T>) {
                thisMember = std::move(otherMember);
            } else {
                thisMember = otherMember;
            }
        };
        if constexpr (!std::is_nothrow_move_assignable_v<D> &&
            std::is_nothrow_move_assignable_v<RS>) {
            assign(deleter, other.deleter);
            assign(resource, other.resource);
        } else {
            assign(resource, other.resource);
            assign(deleter, other.deleter);
        }
        owner = other.owner;
        other.Release();
        return *this;
    }

    void Release() noexcept {
        owner = false;
    }

    void Reset() noexcept {
        if (owner) {
            deleter(resource);
            owner = false;
        }
    }

    template <typename RR>
    void Reset(RR&& newResource)
        noexcept (
            std::is_nothrow_assignable_v<RS, RR> ||
            std::is_nothrow_assignable_v<RS, RR&>
        )
        requires (
            std::is_assignable_v<RS, RR> && (
                std::is_nothrow_assignable_v<RS, RR> ||
                std::is_assignable_v<RS, RR&>
            )
        ) {
        Reset();
        if constexpr (std::is_nothrow_assignable_v<RS, RR>) {
            resource = std::forward<RR>(newResource);
        } else try {
            resource = std::as_const(newResource);
        } catch (...) {
            deleter(newResource);
            throw;
        }
        owner = true;
    }

    [[nodiscard]] const R& Get() const noexcept {
        return resource;
    }

    [[nodiscard]] const D& GetDeleter() const noexcept {
        return deleter;
    }

    std::add_lvalue_reference_t<std::remove_pointer_t<R>>
    operator*() const noexcept
        requires (
            std::is_pointer_v<R> &&
            !std::is_void_v<std::remove_pointer_t<R>>
        ) {
        return *resource;
    }

    R operator->() const noexcept
        requires (
            std::is_pointer_v<R> &&
            !std::is_void_v<std::remove_pointer_t<R>>
        ) {
        return *resource;
    }

private:
    RS resource;
    D deleter;
    bool owner;
};

template <class R, class D>
UniqueResource(R, D) -> UniqueResource<R, D>;
} // namespace pwu
