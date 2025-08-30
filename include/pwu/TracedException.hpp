#pragma once

#include <exception>
#include <source_location>
#include <string>
#include <vector>

namespace pwu {
class TracedException final : public std::exception {
public:
    explicit TracedException(
        std::exception_ptr exception,
        std::source_location location = std::source_location::current()
    );

    explicit TracedException(
        TracedException& exception,
        std::source_location location = std::source_location::current()
    );

    explicit TracedException(
        TracedException&& exception,
        std::source_location location = std::source_location::current()
    );

    ~TracedException() noexcept override;

    [[nodiscard]] std::exception_ptr
    GetOriginalException() const noexcept;
    [[nodiscard]] const std::vector<std::source_location>&
    GetTrace() const noexcept;

    [[nodiscard]] const char* what() const noexcept override;

private:
    void AppendTrace(std::exception_ptr exception);
    void AppendTrace(std::source_location location);

    std::exception_ptr originalException;
    std::string formattedTrace;
    std::vector<std::source_location> trace;
};
} // namespace pwu
