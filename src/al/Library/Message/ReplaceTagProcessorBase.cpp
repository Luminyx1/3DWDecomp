#include "Library/Message/ReplaceTagProcessorBase.hpp"

#include <cstring>
#include <prim/seadStringUtil.h>

#include "Library/Message/MessageHolder.hpp"
#include "Library/Message/MessageTag.hpp"
#include "Project/Base/StringOpUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
const char16_t* sNumberFormats[] = {u"%02d", u"%03d", u"%04d", u"%05d",
                                    u"%2d",  u"%3d",  u"%4d",  u"%5d"};
}

/**
 * Writes a picture replacement for a tag.
 * @param pDst output buffer
 * @param rTag tag to replace
 * @return number of written characters
 */
s32 ReplaceTagProcessorBase::replacePictureGroup(char16_t* pDst, const MessageTag& rTag) const {
    return 0;
}

/**
 * Writes the number argument referenced by a tag.
 * @param pDst output buffer
 * @param rTag number tag
 * @param args message arguments
 * @return number of written characters
 */
s32 ReplaceTagProcessorBase::replaceNumberGroup(char16_t* pDst, const MessageTag& rTag,
                                                std::va_list args) const {
    std::va_list argsCopy;
    va_copy(argsCopy, args);
    s32 argIndex = rTag.getParam32(0);
    s32 value = 0;
    for (s32 i = 0; i <= argIndex; i++) {
        value = va_arg(argsCopy, s32);
    }
    const char16_t* format;
    if (static_cast<u16>(rTag.getType() - 1) < 8) {
        format = sNumberFormats[static_cast<s16>(rTag.getType() - 1)];
    } else {
        format = u"%d";
    }
    return sead::StringUtil::sw16printf(pDst, 0x100, format, value);
}

/**
 * Writes the string argument referenced by a tag.
 * @param pDst output buffer
 * @param rTag string tag
 * @param args message arguments
 * @return number of written characters
 */
s32 ReplaceTagProcessorBase::replaceStringGroup(char16_t* pDst, const MessageTag& rTag,
                                                std::va_list args) const {
    std::va_list argsCopy;
    va_copy(argsCopy, args);
    s32 argIndex = rTag.getParam32(0);
    const char16_t* string = nullptr;
    for (s32 i = 0; i <= argIndex; i++) {
        string = va_arg(argsCopy, const char16_t*);
    }
    return copyMessageWithTag(pDst, 0x100, string);
}

/**
 * Writes a label replacement for a tag.
 * @param pDst output buffer
 * @param rTag tag to replace
 * @return number of written characters
 */
s32 ReplaceTagProcessorBase::replaceTagLabel(char16_t* pDst, const MessageTag& rTag) const {
    return 0;
}

/**
 * Copies the current page of a message, replacing its replacement tags.
 * @param pDst output buffer
 * @param pMsgSystem message system
 * @param pSrc message
 * @return number of written characters
 */
s32 ReplaceTagProcessorBase::replace(char16_t* pDst, const IUseMessageSystem* pMsgSystem,
                                     const char16_t* pSrc) const {
    char16_t* out = pDst;
    while (*pSrc) {
        if (isMessageTagPageBreak(pMsgSystem, pSrc)) {
            *out = 0;
            return out - pDst;
        }
        if (!isMessageTagMark(*pSrc)) {
            *out++ = *pSrc++;
            continue;
        }
        MessageTag tag(pSrc);
        const char* groupName = getMessageTagGroupName(pMsgSystem, tag.getGroup());
        const char* tagName = getMessageTagName(pMsgSystem, tag.getGroup(), tag.getType());
        if (isEqualString(groupName, "Replace")) {
            if (isEqualString(tagName, "PlayerIcon")) {
                out += replacePictureGroup(out, tag);
            }
        } else if (isEqualString(groupName, "ReplaceTagLabel")) {
            out += replaceTagLabel(out, tag);
        } else if (isEqualString(groupName, "ProjectTag")) {
            out += replaceProjectTag(out, tag, pMsgSystem);
        } else {
            memcpy(out, pSrc, tag.getSkipLength() * sizeof(char16_t));
            out += tag.getSkipLength();
        }
        pSrc += tag.getSkipLength();
    }
    *out = 0;
    return out - pDst;
}

/**
 * Copies a message, replacing its argument tags with the given arguments.
 * @param pDst output buffer
 * @param dstSize output buffer length
 * @param pMsgSystem message system
 * @param pSrc message
 * @return number of written characters
 */
s32 ReplaceTagProcessorBase::replaceArgs(char16_t* pDst, s32 dstSize,
                                         const IUseMessageSystem* pMsgSystem,
                                         const char16_t* pSrc, ...) const {
    char16_t buffer[0x200];
    std::va_list args;
    va_start(args, pSrc);
    s32 length = replaceArgsVaList(buffer, pMsgSystem, pSrc, args);
    if (length >= dstSize) {
        buffer[dstSize - 1] = 0;
    }
    memcpy(pDst, buffer, static_cast<u32>(length + 1) * sizeof(char16_t));
    va_end(args);
    return length;
}

/**
 * Copies a message, replacing its argument tags with the given arguments.
 * @param pDst output buffer
 * @param pMsgSystem message system
 * @param pSrc message
 * @param args message arguments
 * @return number of written characters
 */
s32 ReplaceTagProcessorBase::replaceArgsVaList(char16_t* pDst, const IUseMessageSystem* pMsgSystem,
                                               const char16_t* pSrc, std::va_list args) const {
    char16_t* out = pDst;
    while (*pSrc) {
        if (!isMessageTagMark(*pSrc)) {
            *out++ = *pSrc++;
            continue;
        }
        MessageTag tag(pSrc);
        if (!tag.getTag()) {
            pSrc++;
            continue;
        }
        const char* groupName = getMessageTagGroupName(pMsgSystem, tag.getGroup());
        if (!groupName) {
            memcpy(out, pSrc, tag.getSkipLength() * sizeof(char16_t));
            out += tag.getSkipLength();
        } else if (isEqualString(groupName, "Number")) {
            const char* tagName = getMessageTagName(pMsgSystem, tag.getGroup(), tag.getType());
            if (isEqualString(tagName, "FigLeft") || isEqualString(tagName, "Fig02") ||
                isEqualString(tagName, "Fig03") || isEqualString(tagName, "Fig04") ||
                isEqualString(tagName, "Fig05") || isEqualString(tagName, "Fig_2") ||
                isEqualString(tagName, "Fig_3") || isEqualString(tagName, "Fig_4") ||
                isEqualString(tagName, "Fig_5")) {
                out += replaceNumberGroup(out, tag, args);
            }
        } else if (isEqualString(groupName, "String")) {
            out += replaceStringGroup(out, tag, args);
        } else {
            memcpy(out, pSrc, tag.getSkipLength() * sizeof(char16_t));
            out += tag.getSkipLength();
        }
        pSrc += tag.getSkipLength();
    }
    *out = 0;
    return out - pDst;
}
/**
 * Replaces the named string tags of a message whose name matches.
 * @param pDst output string
 * @param pMsgSystem message system
 * @param pString replacement string
 * @param pSrc message
 * @param pName tag name to replace
 * @return number of written characters
 */
s32 ReplaceTagProcessorBase::replaceNamedString(sead::BufferedSafeStringBase<char16_t>* pDst,
                                                const IUseMessageSystem* pMsgSystem,
                                                const char16_t* pString, const char16_t* pSrc,
                                                const char* pName) const {
    char16_t* out = pDst->getBuffer();
    char16_t* start = out;
    s32 size = calcMessageSizeWithoutNullCharacter(pSrc, nullptr);
    for (s32 i = 0; i <= size;) {
        if (!isMessageTagMark(*pSrc)) {
            *out++ = *pSrc++;
            i++;
            continue;
        }
        MessageTag tag(pSrc);
        const char* groupName = getMessageTagGroupName(pMsgSystem, tag.getGroup());
        const char* tagName = getMessageTagName(pMsgSystem, tag.getGroup(), tag.getType());
        s32 length;
        if (isEqualString(groupName, "String") && isStartWithString(tagName, "ReplaceString")) {
            char16_t paramName16[0x40] = {};
            char paramName[0x40] = {};
            u16 paramSize = tag.getParam16(0);
            memcpy(paramName16, tag.getParamPtr(2), paramSize);
            sead::StringUtil::convertUtf16ToUtf8(paramName, 0x40, paramName16, paramSize / 2);
            if (isEqualString(paramName, pName)) {
                length = calcMessageSizeWithoutNullCharacter(pString, nullptr);
                memcpy(out, pString, length * sizeof(char16_t));
                out += length;
                pSrc += tag.getSkipLength();
                i += tag.getSkipLength();
                continue;
            }
        }
        memcpy(out, pSrc, tag.getSkipLength() * sizeof(char16_t));
        length = tag.getSkipLength();
        out += length;
        pSrc += tag.getSkipLength();
        i += tag.getSkipLength();
    }
    return out - start;
}
}  // namespace al
