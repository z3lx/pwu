#include "pwu/TracedException.hpp"

#include <exception>
#include <format>
#include <iterator>
#include <source_location>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <typeinfo>

namespace {
void AppendOriginal(std::string& message, const std::exception* original) {
    if (original != nullptr){
        std::format_to(
            std::back_inserter(message),
            "Exception in thread {} {}: {}",
            std::this_thread::get_id(),
            typeid(*original).name(),
            original->what()
        );
    } else {
        std::format_to(
            std::back_inserter(message),
            "Exception in thread {}: Unknown exception",
            std::this_thread::get_id()
        );
    }
}

void AppendCause(std::string& message, const std::exception* cause) {
    if (cause != nullptr) {
        std::format_to(
            std::back_inserter(message),
            "\nCaused by {}: {}",
            typeid(*cause).name(),
            cause->what()
        );
    } else {
        std::format_to(
            std::back_inserter(message),
            "\nCaused by: Unknown exception"
        );
    }
}

void AppendTrace(
    std::string& message,
    const std::span<const std::source_location> trace) {
    for (const std::source_location traceElement : trace) {
        std::format_to(
            std::back_inserter(message),
            "\n    at {} ({}:{}:{})",
            traceElement.function_name(),
            traceElement.file_name(),
            traceElement.line(),
            traceElement.column()
        );
    }
}
} // namespace

namespace pwu {
TracedException::TracedException(
    const std::exception_ptr previous,
    const std::exception_ptr current,
    const std::source_location location) {
    // Set original, cause, and trace
    try {
        std::rethrow_exception(current);
    } catch (const TracedException& exception) {
        original = exception.original;
        trace.append_range(exception.trace);
        cause = exception.cause;
    } catch (...) {
        original = current;
        if (previous != nullptr) try {
            std::rethrow_exception(previous);
        } catch (const TracedException&) {
            cause = previous;
        } catch (...) {}
    }
    trace.push_back(location);

    // Format message
    try {
        std::rethrow_exception(original);
    } catch (const std::exception& exception) {
        AppendOriginal(message, &exception);
    } catch (...) {
        AppendOriginal(message, nullptr);
    }
    AppendTrace(message, trace);
    std::exception_ptr nextCause = cause;
    while (nextCause != nullptr) {
        try {
            std::rethrow_exception(nextCause);
        } catch (const TracedException& exception) {
            nextCause = exception.cause;
            try {
                std::rethrow_exception(exception.original);
            } catch (const std::exception& exception) {
                AppendCause(message, &exception);
            } catch (...) {
                AppendCause(message, nullptr);
            }
            AppendTrace(message, exception.trace);
        }
    }
}

TracedException::~TracedException() noexcept = default;

std::exception_ptr TracedException::GetOriginal() const noexcept {
    return original;
}

std::exception_ptr TracedException::GetCause() const noexcept {
    return cause;
}

std::span<const std::source_location> TracedException::GetTrace() const noexcept {
    return trace;
}

std::string_view TracedException::GetFormattedTrace() const noexcept {
    return message;
}

void TracedException::ThrowOriginal() const {
    std::rethrow_exception(original);
}

void TracedException::ThrowCause() const {
    std::rethrow_exception(cause);
}

const char* TracedException::what() const noexcept {
    return message.c_str();
}
} // namespace pwu
