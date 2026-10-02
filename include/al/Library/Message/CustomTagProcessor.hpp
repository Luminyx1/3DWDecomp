#pragma once

#include <eui/euiTagProcessor.h>

#include "Library/Message/IUseMessageSystem.hpp"

namespace al {
class CustomTagProcessor : public eui::TagProcessor, public IUseMessageSystem {
public:
    CustomTagProcessor(eui::MessageMgr* pMessageMgr, eui::FontMgr* pFontMgr,
                       const MessageSystem* pMessageSystem);

    const MessageSystem* getMessageSystem() const override;

    void setRubyScale(f32 scale) { mRubyScale = scale; }
    void setRubyCharSpace(f32 space) { mRubyCharSpace = space; }
    void setRubyBaseLineOffset(f32 offset) { mRubyBaseLineOffset = offset; }
    void setPictFontScale(f32 scale) { mPictFontScale = scale; }
    void setUseDeviceFontColor(bool isUse) { mIsUseDeviceFontColor = isUse; }

    void setEnableRuby(bool isEnable) {
        if (isEnable) {
            mFlags |= 1;
        } else {
            mFlags &= ~1;
        }
    }

private:
    f32 mRubyScale;
    f32 mRubyCharSpace;
    f32 mRubyBaseLineOffset;
    f32 mPictFontScale;
    const MessageSystem* mMessageSystem;
    void* _70;
    bool mIsUseDeviceFontColor;
    s32 _7c;
};

static_assert(sizeof(CustomTagProcessor) == 0x80);
}  // namespace al
