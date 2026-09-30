#pragma once

#include <basis/seadTypes.h>

namespace nn::font {
template <typename CharType>
struct PrintContext;
}

namespace al {
class MessageTag {
public:
    MessageTag(const nn::font::PrintContext<u16>* pContext);
    MessageTag(const char16_t* pTag);

    s32 getSkipLength() const;
    u8 getParam8(s32 index) const;
    u16 getParam16(s32 index) const;
    u32 getParam32(s32 index) const;
    const u8* getParamPtr(s32 index) const;

    const char16_t* getTag() const { return mTag; }
    u16 getGroup() const { return mTag[1]; }
    u16 getType() const { return mTag[2]; }
    u16 getParamSize() const { return mTag[3]; }

private:
    const char16_t* mTag;
};
}  // namespace al
