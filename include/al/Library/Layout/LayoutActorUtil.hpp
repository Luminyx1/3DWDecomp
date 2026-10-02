#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace agl {
class TextureData;
}

namespace nn::ui2d {
class Pane;
class TextureInfo;
}  // namespace nn::ui2d

namespace sead {
class Color4u8;
}

namespace al {
class IUseLayout;
class LayoutActor;
class LayoutPaneGroup;
class MessageHolder;

bool killLayoutIfActive(LayoutActor* pActor);
bool appearLayoutIfDead(LayoutActor* pActor);
bool isActive(const LayoutActor* pActor);
bool isDead(const LayoutActor* pActor);
void calcTrans(sead::Vector3f* pOut, const IUseLayout* pLayout);
sead::Vector3f getLocalTrans(const IUseLayout* pLayout);
sead::Vector3f* getLocalTransPtr(const IUseLayout* pLayout);
void calcScale(sead::Vector3f* pOut, const IUseLayout* pLayout);
sead::Vector2f getLocalScale(const IUseLayout* pLayout);
void setLocalTrans(IUseLayout* pLayout, const sead::Vector3f& rTrans);
void setLocalTrans(IUseLayout* pLayout, const sead::Vector2f& rTrans);
void setLocalScale(IUseLayout* pLayout, f32 scale);
void setLocalScale(IUseLayout* pLayout, const sead::Vector2f& rScale);
void setLocalAlpha(IUseLayout* pLayout, f32 alpha);
void calcPaneTrans(sead::Vector3f* pOut, const IUseLayout* pLayout, const char* pPaneName);
void calcPaneMtx(sead::Matrix34f* pOut, const IUseLayout* pLayout, const char* pPaneName);
void calcPaneTrans(sead::Vector3f* pOut, const IUseLayout* pLayout, const nn::ui2d::Pane* pPane);
void calcPaneMtx(sead::Matrix34f* pOut, const IUseLayout* pLayout, const nn::ui2d::Pane* pPane);
void calcPaneTrans(sead::Vector2f* pOut, const IUseLayout* pLayout, const char* pPaneName);
void calcPaneTrans(sead::Vector2f* pOut, const IUseLayout* pLayout, const nn::ui2d::Pane* pPane);
void calcPaneScale(sead::Vector3f* pOut, const IUseLayout* pLayout, const char* pPaneName);
void calcPaneScale(sead::Vector3f* pOut, const IUseLayout* pLayout, const nn::ui2d::Pane* pPane);
void calcPaneSize(sead::Vector3f* pOut, const IUseLayout* pLayout, const char* pPaneName);
void calcPaneSize(sead::Vector3f* pOut, const IUseLayout* pLayout, const nn::ui2d::Pane* pPane);
const sead::Matrix34f& getPaneMtx(const IUseLayout* pLayout, const char* pPaneName);
const sead::Matrix34f* getPaneMtxRaw(const IUseLayout* pLayout, const char* pPaneName);
f32 getGlobalAlpha(const IUseLayout* pLayout, const char* pPaneName);
void setPaneLocalTrans(IUseLayout* pLayout, const char* pPaneName, const sead::Vector2f& rTrans);
void setPaneLocalTrans(IUseLayout* pLayout, const char* pPaneName, const sead::Vector3f& rTrans);
void setPaneLocalRotate(IUseLayout* pLayout, const char* pPaneName, const sead::Vector3f& rRotate);
void setPaneLocalScale(IUseLayout* pLayout, const char* pPaneName, const sead::Vector2f& rScale);
void setPaneLocalSize(IUseLayout* pLayout, const char* pPaneName, const sead::Vector2f& rSize);
void setPaneLocalAlpha(IUseLayout* pLayout, const char* pPaneName, f32 alpha);
sead::Vector3f getPaneLocalTrans(const IUseLayout* pLayout, const char* pPaneName);
void getPaneLocalSize(sead::Vector2f* pOut, const IUseLayout* pLayout, const char* pPaneName);
sead::Vector3f getPaneLocalRotate(const IUseLayout* pLayout, const char* pPaneName);
sead::Vector2f getPaneLocalScale(const IUseLayout* pLayout, const char* pPaneName);
sead::Vector2f getTextBoxDrawRectSize(const IUseLayout* pLayout, const char* pPaneName);
void showPane(IUseLayout* pLayout, const char* pPaneName);
void hidePane(IUseLayout* pLayout, const char* pPaneName);
void showPaneNoRecursive(IUseLayout* pLayout, const char* pPaneName);
void hidePaneNoRecursive(IUseLayout* pLayout, const char* pPaneName);
bool isHidePane(const IUseLayout* pLayout, const char* pPaneName);
void showPaneRoot(IUseLayout* pLayout);
void hidePaneRoot(IUseLayout* pLayout);
void showPaneRootNoRecursive(IUseLayout* pLayout);
void hidePaneRootNoRecursive(IUseLayout* pLayout);
bool isHidePaneRoot(const IUseLayout* pLayout);
bool isExistPane(const IUseLayout* pLayout, const char* pPaneName);
bool isContainPointPane(const IUseLayout* pLayout, const char* pPaneName,
                        const sead::Vector2f& rPos);
bool isContainPointPane(const IUseLayout* pLayout, s32 port, const char* pPaneName);
nn::ui2d::Pane* findHitPaneFromLayoutPos(const IUseLayout* pLayout, const sead::Vector2f& rPos);
bool isExistHitPaneFromLayoutPos(const IUseLayout* pLayout, const sead::Vector2f& rPos);
nn::ui2d::Pane* findHitPaneFromScreenPos(const IUseLayout* pLayout, const sead::Vector2f& rPos);
bool isExistHitPaneFromScreenPos(const IUseLayout* pLayout, const sead::Vector2f& rPos);
bool isTouchPosInPane(const IUseLayout* pLayout, s32 port, const char* pPaneName);
void setCursorPanePos(IUseLayout* pCursor, const IUseLayout* pTarget);
void setPaneVtxColor(const IUseLayout* pLayout, const char* pPaneName, const sead::Color4u8& rColor);
bool isTriggerTouchPane(const IUseLayout* pLayout, s32 port, const char* pPaneName);
bool isHoldTouchPane(const IUseLayout* pLayout, s32 port, const char* pPaneName);
bool isReleaseTouchPane(const IUseLayout* pLayout, s32 port, const char* pPaneName);
s32 getPaneChildNum(const IUseLayout* pLayout, const char* pPaneName);
const char* getPaneChildName(const IUseLayout* pLayout, const char* pPaneName, s32 index);
void setPaneStringLength(IUseLayout* pLayout, const char* pPaneName, const char16_t* pString,
                         u16 pos, u16 length, u32 = 0xffffffff);
void setPaneString(IUseLayout* pLayout, const char* pPaneName, const char16_t* pString,
                   u16 pos = 0, u32 = 0xffffffff);
void setPaneCounterDigit1(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos);
void setPaneCounterDigit2(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos);
void setPaneCounterDigit3(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos);
void setPaneCounterDigit4(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos);
void setPaneCounterDigit5(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos);
void setPaneCounterDigit6(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos);
void setPaneNumberDigit1(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos);
void setPaneNumberDigit2(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos);
void setPaneNumberDigit3(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos);
void setPaneNumberDigit4(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos);
void setPaneNumberDigit5(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos);
void setPaneStringFormat(IUseLayout* pLayout, const char* pPaneName, const char* pFormat, ...);
void setTextPositionCenterH(IUseLayout* pLayout, const char* pPaneName);
void setPaneCounterDigit3WithIcon(IUseLayout* pLayout, const char* pPaneName,
                                  const char16_t* pIcon, s32 value, u16 pos);
void initPaneMessage(IUseLayout* pLayout, const char* pPaneName, const MessageHolder* pHolder,
                     const char* pLabel, u32 unk);
void setPaneSystemMessage(LayoutActor* pActor, const char* pPaneName, const char* pFileName,
                          const char* pLabel);
void setPaneStageMessage(LayoutActor* pActor, const char* pPaneName, const char* pFileName,
                         const char* pLabel);
const char16_t* getPaneStringBuffer(const IUseLayout* pLayout, const char* pPaneName);
s32 getPaneStringBufferLength(const IUseLayout* pLayout, const char* pPaneName);
void setTextBoxPaneFont(const LayoutActor* pActor, const char* pPaneName, const char* pFontName);
void adjustPaneSizeToTextSizeAll(const LayoutActor* pActor);
void requestCaptureRecursive(const LayoutActor* pActor);
void setRubyScale(const LayoutActor* pActor, f32 scale);
nn::ui2d::TextureInfo* createTextureInfo();
nn::ui2d::TextureInfo* createTextureInfo(const agl::TextureData& rTexture, bool isPlacement);
nn::ui2d::TextureInfo* createTextureInfo(const IUseLayout* pLayout, const char* pPaneName);
void getPaneTextureInfo(nn::ui2d::TextureInfo* pOut, const IUseLayout* pLayout,
                        const char* pPaneName);
nn::ui2d::TextureInfo* createTextureInfo(const char* pArchiveName, const char* pFileName,
                                         const char* pTextureName);
void updateTextureInfo(nn::ui2d::TextureInfo* pInfo, const agl::TextureData& rTexture);
void setPaneTexture(IUseLayout* pLayout, const char* pPaneName,
                    const nn::ui2d::TextureInfo* pInfo);
void registerLayoutPartsActor(LayoutActor* pActor, LayoutActor* pPartsActor);
void updateLayoutPaneRecursive(LayoutActor* pActor);
s32 getLayoutPaneGroupNum(LayoutActor* pActor);
LayoutPaneGroup* getLayoutPaneGroup(LayoutActor* pActor, s32 index);
LayoutPaneGroup* getLayoutPaneGroup(const LayoutActor* pActor, const char* pGroupName);
s32 getLayoutPartsNum(LayoutActor* pActor);
LayoutActor* getLayoutPartsActor(LayoutActor* pActor, s32 index);
LayoutActor* getLayoutPartsActor(LayoutActor* pActor, const char* pName);
}  // namespace al
