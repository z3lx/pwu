#include "pwu/TracedException.hpp"

#include <exception>
#include <format>
#include <source_location>
#include <thread>
#include <typeinfo>
#include <utility>
#include <vector>

namespace pwu {
TracedException::TracedException(
    const std::exception_ptr exception,
    const std::source_location location)
    : originalException { exception } {
    AppendTrace(exception);
    AppendTrace(location);
}

TracedException::TracedException(
    TracedException& exception,
    std::source_location location)
    : originalException { exception.originalException }
    , formattedTrace { exception.formattedTrace }
    , trace { exception.trace } {
    AppendTrace(location);
}

TracedException::TracedException(
    TracedException&& exception,
    const std::source_location location)
    : originalException { std::move(exception.originalException) }
    , formattedTrace { std::move(exception.formattedTrace) }
    , trace { std::move(exception.trace) } {
    AppendTrace(location);
}

TracedException::~TracedException() noexcept = default;

std::exception_ptr
TracedException::GetOriginalException() const noexcept {
    return originalException;
}

const std::vector<std::source_location>&
TracedException::GetTrace() const noexcept {
    return trace;
}

const char* TracedException::what() const noexcept {
    return formattedTrace.c_str();
}

void TracedException::AppendTrace(const std::exception_ptr exception) try {
    std::rethrow_exception(exception);
} catch (const std::exception& e) {
    formattedTrace.append(std::format(
        "Exception in thread {} {}: {}",
        std::this_thread::get_id(),
        typeid(e).name(),
        e.what()
    ));
} catch (...) {
    formattedTrace.append("Unknown exception");
}

void TracedException::AppendTrace(const std::source_location location) {
    if (location.file_name() == nullptr ||
        location.file_name()[0] == '\0') {
        return;
    }
    trace.push_back(location);
    formattedTrace.append(std::format(
        "\n    at {} ({}:{}:{})",
        location.function_name(),
        location.file_name(),
        location.line(),
        location.column()
    ));
}
} // namespace pwu
