#include <eui/euiMessageSet.h>

namespace eui {

/** @brief Creates an empty message set. */
MessageSet::MessageSet() = default;

/**
 * @brief Looks up a message by its resource label.
 * @param[in] pLabel Null-terminated label to search for in the initialized message set.
 * @return A view of the matching text, or an empty view when the label is absent.
 */
MessageString MessageSet::findMessage(const char* pLabel) const {
    return tryFindMessage(pLabel);
}

/**
 * @brief Attempts to find a message without copying its text.
 * @param[in] pLabel Null-terminated label to search for in the initialized message set.
 * @return A view of the matching text, or an empty view when the label is absent.
 */
MessageString MessageSet::tryFindMessage(const char* pLabel) const {
    const int index = LMS_GetTextIndexByLabel(mMsgFile, pLabel);
    if (index < 0) {
        return MessageString();
    }

    const int length = calcTextSizeByIndex(index) >> 1;
    const char16_t* pText = nullptr;
    if (static_cast<u32>(mTextNum) > static_cast<u32>(index)) {
        pText = static_cast<const char16_t*>(LMS_GetText(mMsgFile, index));
    }

    return MessageString(length, pText);
}

/**
 * @brief Looks up a message by its resource index.
 * @param[in] index Zero-based text index; negative and out-of-range indices are rejected.
 * @return A view of the indexed text, or an empty view for an invalid index.
 */
MessageString MessageSet::findMessageByIndex(int index) const {
    return tryFindMessageByIndex(index);
}

/**
 * @brief Attempts to find a message by index without copying its text.
 * @param[in] index Zero-based text index; negative and out-of-range indices are rejected.
 * @return A view of the indexed text, or an empty view for an invalid index.
 */
MessageString MessageSet::tryFindMessageByIndex(int index) const {
    if (!hasMessageByIndex(index)) {
        return MessageString();
    }

    const int length = calcTextSizeByIndex(index) >> 1;
    const char16_t* pText = nullptr;
    if (static_cast<u32>(mTextNum) > static_cast<u32>(index)) {
        pText = static_cast<const char16_t*>(LMS_GetText(mMsgFile, index));
    }

    return MessageString(length, pText);
}

/**
 * @brief Tests whether a message index is in range.
 * @param[in] index Zero-based text index to test.
 * @return True if the index identifies a message in this set.
 */
bool MessageSet::hasMessageByIndex(int index) const {
    return static_cast<u32>(mTextNum) > static_cast<u32>(index);
}

/**
 * @brief Tests whether a label exists in the initialized message set.
 * @param[in] pLabel Null-terminated message label to test.
 * @return True if the label resolves to a nonnegative text index.
 */
bool MessageSet::hasMessage(const char* pLabel) const {
    return LMS_GetTextIndexByLabel(mMsgFile, pLabel) >= 0;
}

/**
 * @brief Gets the stored message length for a label, including any control tags.
 * @param[in] pLabel Null-terminated label to search for in the initialized message set.
 * @return Length in UTF-16 code units, or -1 when the label is absent.
 */
int MessageSet::calcMessageLength(const char* pLabel) const {
    const int index = LMS_GetTextIndexByLabel(mMsgFile, pLabel);
    return index < 0 ? -1 : calcTextSizeByIndex(index) >> 1;
}

/**
 * @brief Gets the stored message length for an index, including any control tags.
 * @param[in] index Zero-based text index to query.
 * @return Length in UTF-16 code units, or -1 when the index is invalid.
 */
int MessageSet::calcMessageLengthByIndex(int index) const {
    return hasMessageByIndex(index) ? calcTextSizeByIndex(index) >> 1 : -1;
}

}  // namespace eui
