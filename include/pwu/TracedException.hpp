#pragma once

#include <exception>
#include <memory>
#include <source_location>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>

namespace pwu {
class TracedException final : public std::exception {
public:
    TracedException(
        std::exception_ptr previous,
        std::exception_ptr current,
        std::source_location location
    );
    TracedException(const TracedException& other) noexcept;
    ~TracedException() noexcept override;

    [[nodiscard]] std::exception_ptr GetOriginal() const noexcept;
    [[nodiscard]] std::exception_ptr GetCause() const noexcept;
    [[nodiscard]] std::span<const std::source_location> GetTrace() const noexcept;
    [[nodiscard]] std::string_view GetFormattedTrace() const noexcept;

    void ThrowOriginal() const;
    void ThrowCause() const;

    [[nodiscard]] const char* what() const noexcept override;

private:
    struct Data;
    std::shared_ptr<Data> data;
};

template <typename Callable>
requires std::is_invocable_v<Callable>
void CatchThrowTraced(
    Callable&& callable,
    const std::source_location location = std::source_location::current()) {
    const std::exception_ptr previous = std::current_exception();
    try {
        std::forward<Callable>(callable)();
    } catch (...) {
        const std::exception_ptr current = std::current_exception();
        throw TracedException { previous, current, location };
    }
}

inline void ThrowCaughtTraced(
    const std::source_location location = std::source_location::current()) {
    const std::exception_ptr exception = std::current_exception();
    try {
        std::rethrow_exception(exception);
    } catch (...) {
        throw TracedException { nullptr, exception, location };
    }
}

template <typename Exception>
requires std::is_convertible_v<Exception, std::exception>
void ThrowTraced(
    Exception&& exception,
    const std::source_location location = std::source_location::current()) {
    const std::exception_ptr previous = std::current_exception();
    const std::exception_ptr current = std::make_exception_ptr(
        std::forward<Exception>(exception)
    );
    throw TracedException { previous, current, location };
}
} // namespace pwu
