#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>

namespace eui {
class Animator;
}

namespace nn::font {
class Font;

template <typename T>
class TagProcessorBase;
}  // namespace nn::font

namespace nn::ui2d {
class Layout;
class Pane;
class TextBox;
}  // namespace nn::ui2d

namespace al {
class IUseMessageSystem;
class MessageHolder;
class MessageSystem;
class ReplaceTagProcessorBase;

class LayoutPaneGroup {
public:
    struct AnimNameNode {
        const char* mName;
        AnimNameNode* mNext;
    };

    LayoutPaneGroup(const char* pGroupName);

    void startAnim(const char* pAnimName);
    eui::Animator* getAnimator(const char* pAnimName) const;
    void setAnimFrame(f32 frame);
    void setAnimFrameRate(f32 frameRate);
    f32 getAnimFrame() const;
    f32 getAnimFrameMax() const;
    f32 getAnimFrameMax(const char* pAnimName) const;
    f32 getAnimFrameRate() const;
    bool isAnimExist(const char* pAnimName) const;
    eui::Animator* tryGetAnimator(const char* pAnimName) const;
    bool isAnimEnd() const;
    bool isAnimOneTime() const;
    bool isAnimOneTime(const char* pAnimName) const;
    bool isAnimPlaying() const;
    const char* getPlayingAnimName() const;
    void pushAnimName(const char* pAnimName);
    void createAnimator(nn::ui2d::Layout* pLayout);
    void animate(bool isUpdateFrame);

    const char* getGroupName() const { return mGroupName; }

private:
    const char* mGroupName;
    eui::Animator* mPlayingAnimator = nullptr;
    eui::Animator** mAnimators = nullptr;
    s32 mAnimatorNum = 0;
    AnimNameNode* mAnimNames = nullptr;
};

static_assert(sizeof(LayoutPaneGroup) == 0x28);

void setTextBoxString(nn::ui2d::TextBox* pTextBox, const char16_t* pString, u16 charNum);
void setTextBoxStringLength(nn::ui2d::TextBox* pTextBox, const char16_t* pString, u16 charNum,
                            u16 length, u32 page);
void setTextBoxTagProcessor(nn::ui2d::TextBox* pTextBox,
                            nn::font::TagProcessorBase<u16>* pTagProcessor);
void initTextBoxPane(nn::ui2d::TextBox* pTextBox, const MessageHolder* pHolder,
                     const char* pLabel, u32 bufferSize);
void reallocateTextBoxStringBuffer(nn::ui2d::TextBox* pTextBox, u32 bufferSize);
void initTextBoxRecursive(nn::ui2d::Pane* pPane, const MessageHolder* pHolder,
                          const char* pLabel, u32 bufferSize);
void initTextBoxRecursiveWithSelfTextId(nn::ui2d::Pane* pPane, u32 bufferSize,
                                        const MessageSystem* pMessageSystem,
                                        const nn::ui2d::Layout* pLayout,
                                        const nn::ui2d::Layout* pPartsLayout);
void replaceTextBoxMessage(nn::ui2d::TextBox* pTextBox, const MessageHolder* pHolder,
                           const char* pLabel);
void replaceTextBoxMessage(nn::ui2d::TextBox* pTextBox, const char16_t* pMessage,
                           const ReplaceTagProcessorBase* pReplaceTagProcessor,
                           const IUseMessageSystem* pMessageSystem);
void setTextBoxNumberTagArgRecursive(nn::ui2d::Pane* pPane, s32 index, s32 number);
void calcPaneRectPos(sead::Vector2f* pTopLeft, sead::Vector2f* pBottomRight,
                     const nn::ui2d::Pane* pPane);
const char* getPaneName(const nn::ui2d::Pane* pPane);
void showPaneRecursive(nn::ui2d::Pane* pPane);
void hidePaneRecursive(nn::ui2d::Pane* pPane);
void setTextBoxPaneFont(nn::ui2d::Pane* pPane, const nn::font::Font* pFont);
void requestCaptureRecursive(nn::ui2d::Pane* pPane);
}  // namespace al
