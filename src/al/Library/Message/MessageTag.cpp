#include "Library/Message/MessageTag.hpp"

#include <nn/font/font_TagProcessorBase.h>

namespace al {
/**
 * Creates a tag view of the character preceding the print context's current position.
 * @param pContext print context whose previous character is a tag mark
 */
MessageTag::MessageTag(const nn::font::PrintContext<u16>* pContext) {
    mTag = nullptr;
    const char16_t* tag = reinterpret_cast<const char16_t*>(pContext->str) - 1;
    if (*tag == 0xf) {
        mTag = tag;
        return;
    }
    if (*tag == 0xe) {
        mTag = tag;
        return;
    }
    mTag = nullptr;
}

/**
 * Creates a tag view of a message string position.
 * @param pTag string position that may start with a tag mark
 */
MessageTag::MessageTag(const char16_t* pTag) {
    mTag = nullptr;
    if (*pTag == 0xf) {
        mTag = pTag;
        return;
    }
    if (*pTag == 0xe) {
        mTag = pTag;
        return;
    }
    mTag = nullptr;
}

/**
 * Returns the number of characters taken up by the tag.
 * @return tag length in characters
 */
s32 MessageTag::getSkipLength() const {
    return (mTag[3] >> 1) + 4;
}

/**
 * Returns an 8-bit tag parameter.
 * @param index byte offset into the parameters
 * @return the parameter
 */
u8 MessageTag::getParam8(s32 index) const {
    return reinterpret_cast<const u8*>(mTag + 4)[index];
}

/**
 * Returns a 16-bit tag parameter.
 * @param index parameter index
 * @return the parameter
 */
u16 MessageTag::getParam16(s32 index) const {
    return reinterpret_cast<const u16*>(mTag + 4)[index];
}

/**
 * Returns a 32-bit tag parameter.
 * @param index parameter index
 * @return the parameter
 */
u32 MessageTag::getParam32(s32 index) const {
    return reinterpret_cast<const u32*>(mTag + 4)[index];
}

/**
 * Returns a pointer to the tag parameters.
 * @param index byte offset into the parameters
 * @return pointer to the parameter
 */
const u8* MessageTag::getParamPtr(s32 index) const {
    return reinterpret_cast<const u8*>(mTag + 4) + index;
}
}  // namespace al
