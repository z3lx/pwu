module;

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

export module pwu:String;

import :ErrorHandling;

import std;

export namespace pwu {
template <
    typename InputContiguousContainer,
    typename OutputContiguousContainer
>
void U16ToU8(
    const InputContiguousContainer& inputBuffer,
    OutputContiguousContainer& outputBuffer
);

template <
    typename InputContiguousContainer,
    typename OutputContiguousContainer = std::u8string
>
[[nodiscard]] OutputContiguousContainer U16ToU8(
    const InputContiguousContainer& inputBuffer
);

template <
    typename InputContiguousContainer,
    typename OutputContiguousContainer
>
void U8ToU16(
    const InputContiguousContainer& inputBuffer,
    OutputContiguousContainer& outputBuffer
);

template <
    typename InputContiguousContainer,
    typename OutputContiguousContainer = std::u16string
>
[[nodiscard]] OutputContiguousContainer U8ToU16(
    const InputContiguousContainer& inputBuffer
);
} // namespace pwu

template <typename T>
T CeilDiv(const T a, const T b) noexcept {
    return (a + b - 1) / b;
}

template <
    typename InputContiguousContainer,
    typename OutputContiguousContainer,
    typename InputChunkAddressCallable,
    typename InputChunkCharCountCallable
>
void U16ToCodePage(
    const UINT codePage,
    const InputContiguousContainer& inputBuffer,
    OutputContiguousContainer& outputBuffer,
    InputChunkAddressCallable inputChunkAddressCallable,
    InputChunkCharCountCallable inputChunkCharCountCallable) {
    using InputElementT = typename InputContiguousContainer::value_type;
    const size_t inputCharCount =
        inputBuffer.size() * sizeof(InputElementT) / sizeof(wchar_t);

    // Calculate output buffer size
    size_t outputSize = 0;
    for (size_t inputCharOffset = 0; inputCharOffset < inputCharCount;) {
        const LPCWCH inputChunkAddress = inputChunkAddressCallable(
            inputBuffer,
            inputCharOffset
        );
        const int inputChunkCharCount = inputChunkCharCountCallable(
            inputBuffer,
            inputCharOffset
        );
        const int outputChunkSize = WideCharToMultiByte(
            codePage,
            0,
            inputChunkAddress,
            inputChunkCharCount,
            nullptr,
            0,
            nullptr,
            nullptr
        );
        pwu::ThrowLastWin32ErrorIf(outputChunkSize == 0);
        inputCharOffset += inputChunkCharCount;
        outputSize += outputChunkSize;
    }

    using OutputElementT = typename OutputContiguousContainer::value_type;
    const size_t outputElementCount = CeilDiv(
        outputSize,
        sizeof(OutputElementT)
    );
    outputBuffer.resize(outputElementCount);

    // Convert to code page
    size_t outputSizeOffset = 0;
    for (size_t inputCharOffset = 0; inputCharOffset < inputCharCount;) {
        const LPCWCH inputChunkAddress = inputChunkAddressCallable(
            inputBuffer,
            inputCharOffset
        );
        const int inputChunkCharCount = inputChunkCharCountCallable(
            inputBuffer,
            inputCharOffset
        );
        const auto outputChunkAddress =
            reinterpret_cast<char*>(outputBuffer.data()) +
            outputSizeOffset;
        const auto outputChunkSize = static_cast<int>(std::min(
            static_cast<size_t>(std::numeric_limits<int>::max()),
            outputSize - outputSizeOffset
        ));
        const int bytesWritten = WideCharToMultiByte(
            codePage,
            0,
            inputChunkAddress,
            inputChunkCharCount,
            outputChunkAddress,
            outputChunkSize,
            nullptr,
            nullptr
        );
        pwu::ThrowLastWin32ErrorIf(bytesWritten == 0);
        inputCharOffset += inputChunkCharCount;
        outputSizeOffset += bytesWritten;
    }
}

template <
    typename InputContiguousContainer,
    typename OutputContiguousContainer,
    typename InputChunkAddressCallable,
    typename InputChunkSizeCallable
>
void CodePageToU16(
    const UINT codePage,
    const InputContiguousContainer& inputBuffer,
    OutputContiguousContainer& outputBuffer,
    InputChunkAddressCallable inputChunkAddressCallable,
    InputChunkSizeCallable inputChunkSizeCallable) {
    using InputElementT = typename InputContiguousContainer::value_type;
    const size_t inputSize = inputBuffer.size() * sizeof(InputElementT);

    // Calculate output buffer char count
    size_t outputCharCount = 0;
    for (size_t inputSizeOffset = 0; inputSizeOffset < inputSize;) {
        const LPCCH inputChunkAddress = inputChunkAddressCallable(
            inputBuffer,
            inputSizeOffset
        );
        const int inputChunkSize = inputChunkSizeCallable(
            inputBuffer,
            inputSizeOffset
        );
        const int outputChunkCharCount = MultiByteToWideChar(
            codePage,
            0,
            inputChunkAddress,
            inputChunkSize,
            nullptr,
            0
        );
        pwu::ThrowLastWin32ErrorIf(outputChunkCharCount == 0);
        inputSizeOffset += inputChunkSize;
        outputCharCount += outputChunkCharCount;
    }

    using OutputElementT = typename OutputContiguousContainer::value_type;
    const size_t outputElementCount = CeilDiv(
        outputCharCount * sizeof(wchar_t),
        sizeof(OutputElementT)
    );
    outputBuffer.resize(outputElementCount);

    // Convert to UTF-16
    size_t outputCharCountOffset = 0;
    for (size_t inputSizeOffset = 0; inputSizeOffset < inputSize;) {
        const LPCCH inputChunkAddress = inputChunkAddressCallable(
            inputBuffer,
            inputSizeOffset
        );
        const int inputChunkSize = inputChunkSizeCallable(
            inputBuffer,
            inputSizeOffset
        );
        const auto outputChunkAddress =
            reinterpret_cast<wchar_t*>(outputBuffer.data()) +
            outputCharCountOffset;
        const auto outputChunkCharCount = static_cast<int>(std::min(
            static_cast<size_t>(std::numeric_limits<int>::max()),
            outputCharCount - outputCharCountOffset
        ));
        const int charsWritten = MultiByteToWideChar(
            codePage,
            0,
            inputChunkAddress,
            inputChunkSize,
            outputChunkAddress,
            outputChunkCharCount
        );
        pwu::ThrowLastWin32ErrorIf(charsWritten == 0);
        inputSizeOffset += inputChunkSize;
        outputCharCountOffset += charsWritten;
    }
}

template <typename InputContiguousContainer>
LPCCH U8InputChunkAddress(
    const InputContiguousContainer& inputBuffer,
    const size_t viewSizeOffset) {
    return reinterpret_cast<const char*>(inputBuffer.data()) +
        viewSizeOffset;
};

// Continuation bytes handling to avoid splitting characters
template <typename InputContiguousContainer>
int U8InputChunkSize(
    const InputContiguousContainer& inputBuffer,
    const size_t inputSizeOffset) {
    // 1 to 1 byte to character conversion ratio worst case scenario
    constexpr int maxInputChunkSize = std::numeric_limits<int>::max();

    using InputElementT = typename InputContiguousContainer::value_type;
    const size_t inputSize = inputBuffer.size() * sizeof(InputElementT);
    const size_t remainingInputSize = inputSize - inputSizeOffset;

    if (remainingInputSize <= maxInputChunkSize) {
        return static_cast<int>(remainingInputSize);
    }

    const auto isContinuationByte = [](const char byte) -> bool {
        return (byte & 0xC0) == 0x80;
    };
    const char* lastChar =
        reinterpret_cast<const char*>(inputBuffer.data()) +
        inputSizeOffset + maxInputChunkSize - 1;
    std::uint8_t dropped = 0;
    if (isContinuationByte(lastChar[1])) {
        for (std::uint8_t i = 0; i < 3; ++i) {
            ++dropped;
            if (!isContinuationByte(lastChar[-i])) {
                break;
            }
        }
    }
    return maxInputChunkSize - dropped;
};

template <typename InputContiguousContainer>
LPCWCH U16InputChunkAddress(
    const InputContiguousContainer& inputBuffer,
    const size_t inputCharOffset) {
    return reinterpret_cast<const wchar_t*>(inputBuffer.data()) +
        inputCharOffset;
};

// Surrogate pair handling to avoid splitting characters
template <typename InputContiguousContainer>
int U16InputChunkCharCount(
    const InputContiguousContainer& inputBuffer,
    const size_t inputCharOffset) {
    // 1 to 3 character to byte conversion ratio worst case scenario
    constexpr int maxInputChunkCharCount =
        std::numeric_limits<int>::max() / 3;

    using InputElementT = typename InputContiguousContainer::value_type;
    const size_t inputCharCount =
        inputBuffer.size() * sizeof(InputElementT) / sizeof(wchar_t);
    const size_t remainingInputCharCount = inputCharCount - inputCharOffset;

    if (remainingInputCharCount <= maxInputChunkCharCount) {
        return static_cast<int>(remainingInputCharCount);
    }

    const wchar_t* lastChar =
        reinterpret_cast<const wchar_t*>(inputBuffer.data()) +
        inputCharOffset + maxInputChunkCharCount - 1;
    return IS_HIGH_SURROGATE(*lastChar) ?
        maxInputChunkCharCount - 1 : maxInputChunkCharCount;
};

namespace pwu {
template <
    typename InputContiguousContainer,
    typename OutputContiguousContainer
>
void U16ToU8(
    const InputContiguousContainer& inputBuffer,
    OutputContiguousContainer& outputBuffer) {
    U16ToCodePage(
        CP_UTF8,
        inputBuffer,
        outputBuffer,
        U16InputChunkAddress<InputContiguousContainer>,
        U16InputChunkCharCount<InputContiguousContainer>
    );
}

template <
    typename InputContiguousContainer,
    typename OutputContiguousContainer
>
OutputContiguousContainer U16ToU8(
    const InputContiguousContainer& inputBuffer) {
    OutputContiguousContainer outputBuffer;
    U16ToU8(inputBuffer, outputBuffer);
    return outputBuffer;
}

template <
    typename InputContiguousContainer,
    typename OutputContiguousContainer
>
void U8ToU16(
    const InputContiguousContainer& inputBuffer,
    OutputContiguousContainer& outputBuffer) {
    CodePageToU16(
        CP_UTF8,
        inputBuffer,
        outputBuffer,
        U8InputChunkAddress<InputContiguousContainer>,
        U8InputChunkSize<InputContiguousContainer>
    );
}

template <
    typename InputContiguousContainer,
    typename OutputContiguousContainer
>
OutputContiguousContainer U8ToU16(
    const InputContiguousContainer& inputBuffer) {
    OutputContiguousContainer outputBuffer;
    U8ToU16(inputBuffer, outputBuffer);
    return outputBuffer;
}
} // namespace pwu
