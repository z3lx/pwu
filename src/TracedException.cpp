module pwu:TracedException;

import :TracedException;

import std;

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
struct TracedException::Data {
    std::exception_ptr original;
    std::exception_ptr cause;
    std::vector<std::source_location> trace;
    std::string message;
};

TracedException::TracedException(
    const std::exception_ptr previous,
    const std::exception_ptr current,
    const std::source_location location)
    : data { std::make_shared<Data>() } {
    // Set original, cause, and trace
    try {
        std::rethrow_exception(current);
    } catch (const TracedException& exception) {
        data->original = exception.data->original;
        data->trace.append_range(exception.data->trace);
        data->cause = exception.data->cause;
    } catch (...) {
        data->original = current;
        if (previous != nullptr) try {
            std::rethrow_exception(previous);
        } catch (const TracedException&) {
            data->cause = previous;
        } catch (...) {}
    }
    data->trace.push_back(location);

    // Format message
    try {
        std::rethrow_exception(data->original);
    } catch (const std::exception& exception) {
        AppendOriginal(data->message, &exception);
    } catch (...) {
        AppendOriginal(data->message, nullptr);
    }
    AppendTrace(data->message, data->trace);
    std::exception_ptr nextCause = data->cause;
    while (nextCause != nullptr) {
        try {
            std::rethrow_exception(nextCause);
        } catch (const TracedException& exception) {
            nextCause = exception.data->cause;
            try {
                std::rethrow_exception(exception.data->original);
            } catch (const std::exception& exception) {
                AppendCause(data->message, &exception);
            } catch (...) {
                AppendCause(data->message, nullptr);
            }
            AppendTrace(data->message, exception.data->trace);
        }
    }
}

TracedException::TracedException(const TracedException& other) noexcept
    : data { other.data } {}

TracedException::~TracedException() noexcept = default;

std::exception_ptr TracedException::GetOriginal() const noexcept {
    return data->original;
}

std::exception_ptr TracedException::GetCause() const noexcept {
    return data->cause;
}

std::span<const std::source_location> TracedException::GetTrace() const noexcept {
    return data->trace;
}

std::string_view TracedException::GetFormattedTrace() const noexcept {
    return data->message;
}

void TracedException::ThrowOriginal() const {
    std::rethrow_exception(data->original);
}

void TracedException::ThrowCause() const {
    std::rethrow_exception(data->cause);
}

const char* TracedException::what() const noexcept {
    return data->message.c_str();
}
} // namespace pwu
