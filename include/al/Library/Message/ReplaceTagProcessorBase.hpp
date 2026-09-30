#pragma once

#include <basis/seadTypes.h>
#include <cstdarg>
#include <prim/seadSafeString.h>

namespace al {
class IUseMessageSystem;
class MessageTag;
struct ReplaceTimeInfo;

class ReplaceTagProcessorBase {
public:
    virtual s32 replacePictureGroup(char16_t* pDst, const MessageTag& rTag) const;
    virtual s32 replaceNumberGroup(char16_t* pDst, const MessageTag& rTag, std::va_list args) const;
    virtual s32 replaceStringGroup(char16_t* pDst, const MessageTag& rTag, std::va_list args) const;
    virtual s32 replaceTagLabel(char16_t* pDst, const MessageTag& rTag) const;

    virtual s32 replaceProjectTag(char16_t* pDst, const MessageTag& rTag,
                                  const IUseMessageSystem* pMsgSystem) const {
        return 0;
    }

    s32 replace(char16_t* pDst, const IUseMessageSystem* pMsgSystem, const char16_t* pSrc) const;
    s32 replaceArgs(char16_t* pDst, s32 size, const IUseMessageSystem* pMsgSystem,
                    const char16_t* pSrc, ...) const;
    s32 replaceArgsVaList(char16_t* pDst, const IUseMessageSystem* pMsgSystem,
                          const char16_t* pSrc, std::va_list args) const;
    s32 replaceNamedString(sead::BufferedSafeStringBase<char16_t>* pDst,
                            const IUseMessageSystem* pMsgSystem, const char16_t* pString,
                            const char16_t* pSrc, const char* pName) const;
    s32 replaceScore(sead::BufferedSafeStringBase<char16_t>* pDst,
                      const IUseMessageSystem* pMsgSystem, s32 score, const char16_t* pSrc,
                      const char* pName) const;
    s32 replaceCoinNum(sead::BufferedSafeStringBase<char16_t>* pDst,
                        const IUseMessageSystem* pMsgSystem, s32 coinNum, const char16_t* pSrc,
                        const char* pName) const;
    s32 replaceUserName(sead::BufferedSafeStringBase<char16_t>* pDst,
                         const IUseMessageSystem* pMsgSystem, const char16_t* pUserName,
                         const char16_t* pSrc, const char* pName) const;
    s32 replaceAmiiboName(sead::BufferedSafeStringBase<char16_t>* pDst,
                           const IUseMessageSystem* pMsgSystem, const char* pAmiiboName,
                           const char16_t* pSrc, const char* pName) const;
    s32 replaceTime(sead::BufferedSafeStringBase<char16_t>* pDst,
                     const IUseMessageSystem* pMsgSystem, const char16_t* pSrc, const char* pName,
                     const ReplaceTimeInfo& rInfo) const;
    s32 replaceTimeImpl(char16_t* pDst, const IUseMessageSystem* pMsgSystem, const char16_t* pSrc,
                        const ReplaceTimeInfo& rInfo) const;
};
}  // namespace al
