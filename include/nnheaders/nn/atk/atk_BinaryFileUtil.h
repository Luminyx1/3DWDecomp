#pragma once
#include <nn/types.h>
#include <cstddef>
#include <cstring>

namespace nn::atk::detail::file {
/**
 * @brief Resolves a record stored at a byte offset from a binary resource base.
 * @tparam T Type of the referenced record.
 * @param pBase Base address used by the resource's relative reference.
 * @param offset Byte displacement; the caller must ensure the record lies within the resource.
 * @return Typed address of the referenced record.
 */
template <typename T> inline const T* AtOffset(const void* pBase, ptrdiff_t offset) {
    return reinterpret_cast<const T*>(static_cast<const u8*>(pBase) + offset);
}
/**
 * @brief Finds a packed optional parameter's word index.
 * @param flags Bit mask identifying which parameter words are present.
 * @param bit Parameter bit to locate, in [0, 31].
 * @return Word index including the flags word, or zero when the parameter is absent.
 */
inline u32 GetParameterIndex(u32 flags, unsigned bit) {
    if (!(flags & (1u << bit))) {
        return 0;
    }
    u32 index = 0;
    for (unsigned i = 0; i < bit; ++i) {
        if (flags & (1u << i)) {
            ++index;
        }
    }
    return index + 1;
}
/**
 * @brief Reads a present parameter from a packed optional-parameter list.
 * @param pFlags Flags word followed by the packed parameter words.
 * @param bit Present parameter's bit, in [0, 31].
 * @return Stored parameter word.
 */
inline u32 GetParameter(const u32* pFlags, unsigned bit) {
    size_t index = 0;
    for (unsigned i = 0; i < bit; ++i) {
        index += (*pFlags >> i) & 1;
    }
    return pFlags[index + 1];
}
/**
 * @brief Reads a floating-point parameter without changing its bit representation.
 * @param pFlags Flags word followed by packed parameter words.
 * @param bit Present floating-point parameter's bit, in [0, 31].
 * @return Floating-point value represented by the stored word.
 */
inline float GetFloatParameter(const u32* pFlags, unsigned bit) {
    u32 bits = GetParameter(pFlags, bit);
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
} // namespace nn::atk::detail::file
