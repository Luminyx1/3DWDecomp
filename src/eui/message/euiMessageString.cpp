#include <eui/euiMessageString.h>

namespace eui {

/** @brief Creates an empty message view. */
MessageString::MessageString() : m_pText(nullptr), mLength(0) {}

/**
 * @brief Creates a message view with an explicit character count.
 * @param[in] length Number of UTF-16 code units in the message view.
 * @param[in] pText Character storage referenced by the view; not copied.
 */
MessageString::MessageString(int length, const char16_t* pText)
    : m_pText(pText), mLength(length) {}

/**
 * @brief Creates a message view from a character range.
 * @param[in] pBegin First UTF-16 code unit in the message range.
 * @param[in] pEnd One-past-the-end pointer for the message range.
 */
MessageString::MessageString(const char16_t* pBegin, const char16_t* pEnd)
    : m_pText(pBegin), mLength(pEnd - pBegin) {}

/**
 * @brief Creates a message view over a null-terminated safe string.
 * @param[in] rText Safe string whose character storage is referenced by the view.
 */
MessageString::MessageString(const sead::SafeStringBase<char16_t>& rText)
    : m_pText(rText.cstr()), mLength(rText.calcLength()) {}

/**
 * @brief Copies a message view without copying its characters.
 * @param[in] rOther Source object to copy.
 */
MessageString::MessageString(const MessageString& rOther)
    : m_pText(rOther.m_pText), mLength(rOther.mLength) {}

/**
 * @brief Assigns another message view.
 * @param[in] rOther Source object to copy.
 */
MessageString& MessageString::operator=(const MessageString& rOther) {
    m_pText = rOther.m_pText;
    mLength = rOther.mLength;
    return *this;
}

/**
 * @brief Returns a character, falling back to the first for an invalid index.
 * @param[in] index Zero-based UTF-16 code-unit index in the message view.
 */
const char16_t& MessageString::operator[](int index) const {
    return static_cast<u32>(index) < mLength ? m_pText[index] : *m_pText;
}

/** @brief Returns the beginning of the message view. */
MessageString::Iterator MessageString::begin() const {
    return {m_pText, 0};
}

/** @brief Returns the end of the message view. */
MessageString::Iterator MessageString::end() const {
    return {m_pText, mLength};
}

/**
 * @brief Returns an iterator clamped to the end of the message.
 * @param[in] index Zero-based UTF-16 code-unit index in the message view.
 */
MessageString::Iterator MessageString::toIterator(int index) const {
    if (static_cast<u32>(index) >= mLength) {
        index = mLength;
    }
    return {m_pText, static_cast<u32>(index)};
}

/**
 * @brief Copies message characters while skipping embedded control tags.
 * @param[out] pOutput Destination buffer for the null-terminated text with control tags removed.
 */
bool MessageString::tryMakeTagStrippedString(sead::BufferedSafeStringBase<char16_t>* pOutput) const {
    auto* pBuffer = pOutput->getBuffer();
    const int capacity = pOutput->getBufferSize();
    int length = 0;
    const auto* pCurrent = m_pText;
    const auto* pEnd = m_pText + static_cast<int>(mLength);
    bool overflow = capacity < 1;
    if (static_cast<int>(mLength) > 0) {
        do {
            if (overflow) {
                pBuffer[capacity - 1] = 0;
                return false;
            }
            const auto ch = *pCurrent;
            if ((ch | 1) == 0xf) {
                if (ch == 0xe) {
                    pCurrent = reinterpret_cast<const char16_t*>(
                        reinterpret_cast<const char*>(pCurrent) + pCurrent[3] + 8);
                } else if (ch == 0xf) {
                    pCurrent += 3;
                }
            } else {
                pBuffer[length++] = ch;
                ++pCurrent;
            }
            overflow = length >= capacity;
        } while (reinterpret_cast<uintptr_t>(pCurrent) < reinterpret_cast<uintptr_t>(pEnd));
    }
    pBuffer[overflow ? capacity - 1 : length] = 0;
    return !overflow;
}

/** @brief Counts printable characters, including text represented by ruby tags. */
int MessageString::countPrintableStringLength() const {
    int length = 0;
    const auto* pCurrent = m_pText;
    const auto* pEnd = m_pText + static_cast<int>(mLength);
    if (static_cast<int>(mLength) > 0) {
        do {
            const auto ch = *pCurrent;
            if ((ch | 1) == 0xf) {
                const char16_t* pTag;
                pCurrent = readTag_(pCurrent, &pTag);
                if (pTag[1] == 0 && pTag[2] == 0) {
                    length += pTag[5] / 2;
                }
            } else {
                ++length;
                ++pCurrent;
            }
        } while (reinterpret_cast<uintptr_t>(pCurrent) < reinterpret_cast<uintptr_t>(pEnd));
    }
    return length;
}

}  // namespace eui
