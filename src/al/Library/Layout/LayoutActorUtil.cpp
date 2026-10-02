#include "Library/Layout/LayoutActorUtil.hpp"

#include <cstdarg>

#include <agl/common/aglTextureData.h>
#include <agl/g3d/aglNW4FToNN.h>
#include <eui/euiAlignPane.h>
#include <eui/euiFontMgr.h>
#include <eui/euiLayoutEx.h>
#include <eui/euiPartsEx.h>
#include <eui/euiScreen.h>
#include <eui/euiTextBoxEx.h>
#include <eui/euiUtility.h>
#include <gfx/nin/seadGraphicsNvn.h>
#include <gfx/seadColor.h>
#include <heap/seadDisposer.h>
#include <nn/g3d/g3d_Resources.h>
#include <nn/gfx/gfx_ResTextureData.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/ui2d/ui2d_Material.h>
#include <nn/ui2d/ui2d_Picture.h>
#include <nn/ui2d/ui2d_TextBox.h>
#include <nn/ui2d/ui2d_TextureContainer.h>
#include <nn/ui2d/ui2d_Util.h>
#include <nn/ui2d/ui2d_Window.h>
#include <prim/seadStringUtil.h>

#include "Library/Controller/InputFunction.hpp"
#include "Library/Layout/IUseLayout.hpp"
#include "Library/Layout/LayoutActor.hpp"
#include "Library/Layout/LayoutKeeper.hpp"
#include "Library/Layout/LayoutPaneGroup.hpp"
#include "Library/Layout/LayoutPartsActorKeeper.hpp"
#include "Library/Layout/LayoutSceneInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Library/Message/CustomTagProcessor.hpp"
#include "Library/Message/MessageHolder.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Library/Screen/ScreenFunction.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
namespace {
class DisposerTextureInfo : public nn::ui2d::ResourceTextureInfo, public sead::IDisposer {
public:
    DisposerTextureInfo() {}

    ~DisposerTextureInfo() override {
        if (mDescriptor.IsValid()) {
            const nn::gfx::TextureView* view = getResTextureView();
            eui::UnregisterSlotForTexture(&mDescriptor, *view, nullptr);
        }
    }

private:
    const nn::gfx::TextureView* getResTextureView() const {
        return static_cast<const nn::gfx::TextureView*>(
            m_pResource->ToData().pTextureView.Get());
    }
};

struct TextBoxTextPosition {
    u16 _0 : 2;
    bool isTextPositionDirty : 1;
    u16 _3 : 13;
    u8 textPosition;
};

inline TextBoxTextPosition* getTextPosition(nn::ui2d::TextBox* pTextBox) {
    return reinterpret_cast<TextBoxTextPosition*>(reinterpret_cast<u8*>(pTextBox) + 0x114);
}

inline nn::ui2d::Layout* getLayout(const IUseLayout* pLayout) {
    return pLayout->getLayoutKeeper()->getLayout();
}

inline nn::ui2d::Pane* getRootPane(const IUseLayout* pLayout) {
    return getLayout(pLayout)->GetRootPane();
}

inline nn::ui2d::Pane* findPane(const IUseLayout* pLayout, const char* pPaneName) {
    return getRootPane(pLayout)->FindPaneByName(pPaneName, true);
}

inline const sead::Matrix34f& getGlobalMtx(const nn::ui2d::Pane* pPane) {
    return *reinterpret_cast<const sead::Matrix34f*>(pPane->GetGlobalMtx());
}

inline const nn::util::neon::MatrixColumnMajor4x3fType& getGlobalMtxNeon(
    const nn::ui2d::Pane* pPane) {
    return *reinterpret_cast<const nn::util::neon::MatrixColumnMajor4x3fType*>(
        pPane->GetGlobalMtx());
}

inline sead::Vector3f& getTrans(nn::ui2d::Pane* pPane) {
    return *reinterpret_cast<sead::Vector3f*>(&pPane->mPositionX);
}

inline void setTrans(nn::ui2d::Pane* pPane, const sead::Vector3f& rTrans) {
    *reinterpret_cast<nn::util::Float3*>(&pPane->mPositionX) =
        *reinterpret_cast<const nn::util::Float3*>(&rTrans);
}

inline void setTrans(nn::ui2d::Pane* pPane, f32 x, f32 y, f32 z) {
    pPane->mPositionX = x;
    pPane->mPositionY = y;
    pPane->mPositionZ = z;
}

inline void setSize(nn::ui2d::Pane* pPane, f32 width, f32 height) {
    pPane->mSizeX = width;
    pPane->mSizeY = height;
}

inline void setRotate(nn::ui2d::Pane* pPane, const sead::Vector3f& rRotate) {
    *reinterpret_cast<nn::util::Float3*>(&pPane->mRotationX) =
        *reinterpret_cast<const nn::util::Float3*>(&rRotate);
}

inline void setScale(nn::ui2d::Pane* pPane, const sead::Vector2f& rScale) {
    *reinterpret_cast<nn::util::Float2*>(&pPane->mScaleX) =
        *reinterpret_cast<const nn::util::Float2*>(&rScale);
}

inline sead::Vector3f& getRotate(nn::ui2d::Pane* pPane) {
    return *reinterpret_cast<sead::Vector3f*>(&pPane->mRotationX);
}

inline sead::Vector2f& getScale(nn::ui2d::Pane* pPane) {
    return *reinterpret_cast<sead::Vector2f*>(&pPane->mScaleX);
}

inline void markGlobalMtxDirty(nn::ui2d::Pane* pPane) {
    pPane->mFlags |= 0x10;
}

inline bool isHitPane(const nn::ui2d::Pane* pPane, const sead::Vector2f& rPos) {
    const nn::util::neon::MatrixColumnMajor4x3fType& mtx = getGlobalMtxNeon(pPane);
    f32 scaleY = mtx._m.val[1][1];
    f32 scaleX = mtx._m.val[0][0];

    if (isNearZero(scaleX, 0.001f) || isNearZero(scaleY, 0.001f)) {
        return false;
    }

    nn::util::Float2 pos;
    pos.x = rPos.x;
    pos.y = rPos.y;
    return nn::ui2d::IsContain(pPane, pos);
}

nn::ui2d::Pane* findHitPaneRecursive(nn::ui2d::Pane* pPane, const sead::Vector2f& rPos) {
    if ((pPane->mFlags & 1) != 0) {
        for (nn::util::IntrusiveListNode* node = &pPane->m_Children;
             pPane->m_Children.m_Next != node; node = node->m_Prev) {
            nn::ui2d::Pane* hitPane =
                findHitPaneRecursive(nn::ui2d::Pane::FromLink(node->m_Prev), rPos);

            if (hitPane != nullptr) {
                return hitPane;
            }
        }

        if (eui::DynamicCast<nn::ui2d::Picture>(pPane) != nullptr ||
            eui::DynamicCast<nn::ui2d::TextBox>(pPane) != nullptr ||
            eui::DynamicCast<nn::ui2d::Window>(pPane) != nullptr) {
            if (isHitPane(pPane, rPos)) {
                return pPane;
            }
        }
    }

    return nullptr;
}

void updatePaneRecursive(LayoutActor* pActor, eui::LayoutEx* pLayout, nn::ui2d::Pane* pPane) {
    eui::PartsEx* parts = eui::DynamicCast<eui::PartsEx>(pPane);

    if (parts != nullptr) {
        pLayout = static_cast<eui::LayoutEx*>(parts->m_pLayout);
    }

    eui::AdjustPaneSizeToTextSize(pPane, pLayout);
    eui::TextBoxEx* textBox = eui::DynamicCast<eui::TextBoxEx>(pPane);

    if (textBox != nullptr &&
        isExistMessageTagPadSwitch(pActor, reinterpret_cast<const char16_t*>(
                                               textBox->GetStringBuffer()))) {
        getTextPosition(textBox)->isTextPositionDirty = true;
    }

    nn::util::IntrusiveListNode* head = &pPane->m_Children;

    for (nn::util::IntrusiveListNode* node = head->m_Next; node != head; node = node->m_Next) {
        updatePaneRecursive(pActor, pLayout, nn::ui2d::Pane::FromLink(node));
    }
}
}  // namespace

/**
 * Kills a layout actor if it is alive.
 * @param pActor layout actor
 * @return whether the actor was killed
 */
bool killLayoutIfActive(LayoutActor* pActor) {
    if (pActor->isAlive()) {
        pActor->kill();
        return true;
    }

    return false;
}

/**
 * Makes a layout actor appear if it is dead.
 * @param pActor layout actor
 * @return whether the actor appeared
 */
bool appearLayoutIfDead(LayoutActor* pActor) {
    if (pActor->isAlive()) {
        return false;
    }

    pActor->appear();
    return true;
}

/**
 * Checks whether a layout actor is alive.
 * @param pActor layout actor
 * @return whether the actor is alive
 */
bool isActive(const LayoutActor* pActor) {
    return pActor->isAlive();
}

/**
 * Checks whether a layout actor is dead.
 * @param pActor layout actor
 * @return whether the actor is dead
 */
bool isDead(const LayoutActor* pActor) {
    return !pActor->isAlive();
}

/**
 * Calculates the global translation of the root pane.
 * @param pOut output translation
 * @param pLayout layout user
 */
void calcTrans(sead::Vector3f* pOut, const IUseLayout* pLayout) {
    getGlobalMtx(getRootPane(pLayout)).getTranslation(*pOut);
}

/**
 * Gets the local translation of the root pane.
 * @param pLayout layout user
 * @return the local translation
 */
sead::Vector3f getLocalTrans(const IUseLayout* pLayout) {
    return getTrans(getRootPane(pLayout));
}

/**
 * Gets a pointer to the local translation of the root pane.
 * @param pLayout layout user
 * @return the local translation
 */
sead::Vector3f* getLocalTransPtr(const IUseLayout* pLayout) {
    return &getTrans(getRootPane(pLayout));
}

/**
 * Calculates the global scale of the root pane.
 * @param pOut output scale
 * @param pLayout layout user
 */
void calcScale(sead::Vector3f* pOut, const IUseLayout* pLayout) {
    const sead::Matrix34f& mtx = getGlobalMtx(getRootPane(pLayout));
    sead::Vector3f axisX(mtx(0, 0), mtx(1, 0), mtx(2, 0));
    sead::Vector3f axisY(mtx(0, 1), mtx(1, 1), mtx(2, 1));
    sead::Vector3f axisZ(mtx(0, 2), mtx(1, 2), mtx(2, 2));
    f32 scaleX = axisX.length();
    f32 scaleY = axisY.length();
    f32 scaleZ = axisZ.length();
    pOut->x = scaleX;
    pOut->y = scaleY;
    pOut->z = scaleZ;
}

/**
 * Gets the local scale of the root pane.
 * @param pLayout layout user
 * @return the local scale
 */
sead::Vector2f getLocalScale(const IUseLayout* pLayout) {
    return getScale(getRootPane(pLayout));
}

/**
 * Sets the local translation of the root pane.
 * @param pLayout layout user
 * @param rTrans translation
 */
void setLocalTrans(IUseLayout* pLayout, const sead::Vector3f& rTrans) {
    nn::ui2d::Pane* pane = getRootPane(pLayout);
    setTrans(pane, rTrans);
    markGlobalMtxDirty(pane);
}

/**
 * Sets the local translation of the root pane.
 * @param pLayout layout user
 * @param rTrans translation
 */
void setLocalTrans(IUseLayout* pLayout, const sead::Vector2f& rTrans) {
    nn::ui2d::Pane* pane = getRootPane(pLayout);
    setTrans(pane, rTrans.x, rTrans.y, 0.0f);
    markGlobalMtxDirty(pane);
}

/**
 * Sets the local scale of the root pane.
 * @param pLayout layout user
 * @param scale scale
 */
void setLocalScale(IUseLayout* pLayout, f32 scale) {
    nn::ui2d::Pane* pane = getRootPane(pLayout);
    pane->mScaleX = scale;
    pane->mScaleY = scale;
    markGlobalMtxDirty(pane);
}

/**
 * Sets the local scale of the root pane.
 * @param pLayout layout user
 * @param rScale scale
 */
void setLocalScale(IUseLayout* pLayout, const sead::Vector2f& rScale) {
    nn::ui2d::Pane* pane = getRootPane(pLayout);
    setScale(pane, rScale);
    markGlobalMtxDirty(pane);
}

/**
 * Sets the local alpha of the root pane.
 * @param pLayout layout user
 * @param alpha alpha
 */
void setLocalAlpha(IUseLayout* pLayout, f32 alpha) {
    nn::ui2d::Pane* pane = getRootPane(pLayout);
    pane->mAlpha = alpha;
}

/**
 * Calculates the global translation of a pane.
 * @param pOut output translation
 * @param pLayout layout user
 * @param pPaneName pane name
 */
void calcPaneTrans(sead::Vector3f* pOut, const IUseLayout* pLayout, const char* pPaneName) {
    sead::Matrix34f mtx;
    calcPaneMtx(&mtx, pLayout, pPaneName);
    mtx.getTranslation(*pOut);
}

/**
 * Calculates the global matrix of a pane.
 * @param pOut output matrix
 * @param pLayout layout user
 * @param pPaneName pane name
 */
void calcPaneMtx(sead::Matrix34f* pOut, const IUseLayout* pLayout, const char* pPaneName) {
    calcPaneMtx(pOut, pLayout, findPane(pLayout, pPaneName));
}

/**
 * Calculates the global translation of a pane.
 * @param pOut output translation
 * @param pLayout layout user
 * @param pPane pane
 */
void calcPaneTrans(sead::Vector3f* pOut, const IUseLayout* pLayout, const nn::ui2d::Pane* pPane) {
    sead::Matrix34f mtx;
    calcPaneMtx(&mtx, pLayout, pPane);
    mtx.getTranslation(*pOut);
}

/**
 * Calculates the global matrix of a pane.
 * @param pOut output matrix
 * @param pLayout layout user
 * @param pPane pane
 */
void calcPaneMtx(sead::Matrix34f* pOut, const IUseLayout* pLayout, const nn::ui2d::Pane* pPane) {
    makeMtx34f(pOut, getGlobalMtxNeon(pPane));
}

/**
 * Calculates the global translation of a pane.
 * @param pOut output translation
 * @param pLayout layout user
 * @param pPaneName pane name
 */
void calcPaneTrans(sead::Vector2f* pOut, const IUseLayout* pLayout, const char* pPaneName) {
    sead::Matrix34f mtx;
    calcPaneMtx(&mtx, pLayout, pPaneName);
    pOut->set(mtx(0, 3), mtx(1, 3));
}

/**
 * Calculates the global translation of a pane.
 * @param pOut output translation
 * @param pLayout layout user
 * @param pPane pane
 */
void calcPaneTrans(sead::Vector2f* pOut, const IUseLayout* pLayout, const nn::ui2d::Pane* pPane) {
    sead::Matrix34f mtx;
    calcPaneMtx(&mtx, pLayout, pPane);
    pOut->set(mtx(0, 3), mtx(1, 3));
}

/**
 * Calculates the global scale of a pane.
 * @param pOut output scale
 * @param pLayout layout user
 * @param pPaneName pane name
 */
void calcPaneScale(sead::Vector3f* pOut, const IUseLayout* pLayout, const char* pPaneName) {
    sead::Matrix34f mtx;
    calcPaneMtx(&mtx, pLayout, pPaneName);
    sead::Vector3f axisX(mtx(0, 0), mtx(1, 0), mtx(2, 0));
    sead::Vector3f axisY(mtx(0, 1), mtx(1, 1), mtx(2, 1));
    sead::Vector3f axisZ(mtx(0, 2), mtx(1, 2), mtx(2, 2));
    pOut->set(axisX.length(), axisY.length(), axisZ.length());
}

/**
 * Calculates the global scale of a pane.
 * @param pOut output scale
 * @param pLayout layout user
 * @param pPane pane
 */
void calcPaneScale(sead::Vector3f* pOut, const IUseLayout* pLayout, const nn::ui2d::Pane* pPane) {
    sead::Matrix34f mtx;
    calcPaneMtx(&mtx, pLayout, pPane);
    sead::Vector3f axisX(mtx(0, 0), mtx(1, 0), mtx(2, 0));
    sead::Vector3f axisY(mtx(0, 1), mtx(1, 1), mtx(2, 1));
    sead::Vector3f axisZ(mtx(0, 2), mtx(1, 2), mtx(2, 2));
    pOut->set(axisX.length(), axisY.length(), axisZ.length());
}

/**
 * Calculates the global size of a pane.
 * @param pOut output size
 * @param pLayout layout user
 * @param pPaneName pane name
 */
void calcPaneSize(sead::Vector3f* pOut, const IUseLayout* pLayout, const char* pPaneName) {
    const nn::ui2d::Pane* pane = findPane(pLayout, pPaneName);
    sead::Matrix34f mtx;
    calcPaneMtx(&mtx, pLayout, pPaneName);
    pOut->setRotated(mtx, sead::Vector3f(pane->mSizeX, pane->mSizeY, 0.0f));
}

/**
 * Calculates the global size of a pane.
 * @param pOut output size
 * @param pLayout layout user
 * @param pPane pane
 */
void calcPaneSize(sead::Vector3f* pOut, const IUseLayout* pLayout, const nn::ui2d::Pane* pPane) {
    sead::Matrix34f mtx;
    calcPaneMtx(&mtx, pLayout, pPane);
    pOut->setRotated(mtx, sead::Vector3f(pPane->mSizeX, pPane->mSizeY, 0.0f));
}

/**
 * Gets the global matrix of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return the global matrix
 */
const sead::Matrix34f& getPaneMtx(const IUseLayout* pLayout, const char* pPaneName) {
    return getGlobalMtx(findPane(pLayout, pPaneName));
}

/**
 * Gets a pointer to the global matrix of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return the global matrix
 */
const sead::Matrix34f* getPaneMtxRaw(const IUseLayout* pLayout, const char* pPaneName) {
    return &getGlobalMtx(findPane(pLayout, pPaneName));
}

/**
 * Gets the global alpha of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return the global alpha
 */
f32 getGlobalAlpha(const IUseLayout* pLayout, const char* pPaneName) {
    return findPane(pLayout, pPaneName)->mAlphaInfluence;
}

/**
 * Sets the local translation of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param rTrans translation
 */
void setPaneLocalTrans(IUseLayout* pLayout, const char* pPaneName, const sead::Vector2f& rTrans) {
    nn::ui2d::Pane* pane = findPane(pLayout, pPaneName);
    setTrans(pane, rTrans.x, rTrans.y, 0.0f);
    markGlobalMtxDirty(pane);
}

/**
 * Sets the local translation of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param rTrans translation
 */
void setPaneLocalTrans(IUseLayout* pLayout, const char* pPaneName, const sead::Vector3f& rTrans) {
    nn::ui2d::Pane* pane = findPane(pLayout, pPaneName);
    setTrans(pane, rTrans);
    markGlobalMtxDirty(pane);
}

/**
 * Sets the local rotation of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param rRotate rotation
 */
void setPaneLocalRotate(IUseLayout* pLayout, const char* pPaneName,
                        const sead::Vector3f& rRotate) {
    nn::ui2d::Pane* pane = findPane(pLayout, pPaneName);
    setRotate(pane, rRotate);
    markGlobalMtxDirty(pane);
}

/**
 * Sets the local scale of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param rScale scale
 */
void setPaneLocalScale(IUseLayout* pLayout, const char* pPaneName, const sead::Vector2f& rScale) {
    nn::ui2d::Pane* pane = findPane(pLayout, pPaneName);
    setScale(pane, rScale);
    markGlobalMtxDirty(pane);
}

/**
 * Sets the local size of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param rSize size
 */
void setPaneLocalSize(IUseLayout* pLayout, const char* pPaneName, const sead::Vector2f& rSize) {
    nn::ui2d::Pane* pane = findPane(pLayout, pPaneName);
    setSize(pane, rSize.x, rSize.y);
    markGlobalMtxDirty(pane);
}

/**
 * Sets the local alpha of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param alpha alpha
 */
void setPaneLocalAlpha(IUseLayout* pLayout, const char* pPaneName, f32 alpha) {
    nn::ui2d::Pane* pane = findPane(pLayout, pPaneName);
    pane->mAlpha = alpha;
}

/**
 * Gets the local translation of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return the local translation
 */
sead::Vector3f getPaneLocalTrans(const IUseLayout* pLayout, const char* pPaneName) {
    return getTrans(findPane(pLayout, pPaneName));
}

/**
 * Gets the local size of a pane.
 * @param pOut output size
 * @param pLayout layout user
 * @param pPaneName pane name
 */
void getPaneLocalSize(sead::Vector2f* pOut, const IUseLayout* pLayout, const char* pPaneName) {
    nn::ui2d::Pane* pane = findPane(pLayout, pPaneName);
    pOut->set(pane->mSizeX, pane->mSizeY);
}

/**
 * Gets the local rotation of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return the local rotation
 */
sead::Vector3f getPaneLocalRotate(const IUseLayout* pLayout, const char* pPaneName) {
    return getRotate(findPane(pLayout, pPaneName));
}

/**
 * Gets the local scale of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return the local scale
 */
sead::Vector2f getPaneLocalScale(const IUseLayout* pLayout, const char* pPaneName) {
    return getScale(findPane(pLayout, pPaneName));
}

/**
 * Gets the size of the text draw rectangle of a text box pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return the size of the text draw rectangle
 */
sead::Vector2f getTextBoxDrawRectSize(const IUseLayout* pLayout, const char* pPaneName) {
    auto* textBox = static_cast<nn::ui2d::TextBox*>(findPane(pLayout, pPaneName));
    nn::font::Rectangle rect = textBox->GetTextDrawRect();
    return {rect.GetWidth(), rect.GetHeight()};
}

/**
 * Shows a pane and its children.
 * @param pLayout layout user
 * @param pPaneName pane name
 */
void showPane(IUseLayout* pLayout, const char* pPaneName) {
    showPaneRecursive(findPane(pLayout, pPaneName));
}

/**
 * Hides a pane and its children.
 * @param pLayout layout user
 * @param pPaneName pane name
 */
void hidePane(IUseLayout* pLayout, const char* pPaneName) {
    hidePaneRecursive(findPane(pLayout, pPaneName));
}

/**
 * Shows a pane without touching its children.
 * @param pLayout layout user
 * @param pPaneName pane name
 */
void showPaneNoRecursive(IUseLayout* pLayout, const char* pPaneName) {
    findPane(pLayout, pPaneName)->mFlags |= 1;
}

/**
 * Hides a pane without touching its children.
 * @param pLayout layout user
 * @param pPaneName pane name
 */
void hidePaneNoRecursive(IUseLayout* pLayout, const char* pPaneName) {
    findPane(pLayout, pPaneName)->mFlags &= ~1;
}

/**
 * Checks whether a pane is hidden.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return whether the pane is hidden
 */
bool isHidePane(const IUseLayout* pLayout, const char* pPaneName) {
    return (findPane(pLayout, pPaneName)->mFlags & 1) == 0;
}

/**
 * Shows the root pane and its children.
 * @param pLayout layout user
 */
void showPaneRoot(IUseLayout* pLayout) {
    showPaneRecursive(getRootPane(pLayout));
}

/**
 * Hides the root pane and its children.
 * @param pLayout layout user
 */
void hidePaneRoot(IUseLayout* pLayout) {
    hidePaneRecursive(getRootPane(pLayout));
}

/**
 * Shows the root pane without touching its children.
 * @param pLayout layout user
 */
void showPaneRootNoRecursive(IUseLayout* pLayout) {
    getRootPane(pLayout)->mFlags |= 1;
}

/**
 * Hides the root pane without touching its children.
 * @param pLayout layout user
 */
void hidePaneRootNoRecursive(IUseLayout* pLayout) {
    getRootPane(pLayout)->mFlags &= ~1;
}

/**
 * Checks whether the root pane is hidden.
 * @param pLayout layout user
 * @return whether the root pane is hidden
 */
bool isHidePaneRoot(const IUseLayout* pLayout) {
    return (getRootPane(pLayout)->mFlags & 1) == 0;
}

/**
 * Checks whether a pane exists.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return whether the pane exists
 */
bool isExistPane(const IUseLayout* pLayout, const char* pPaneName) {
    return findPane(pLayout, pPaneName) != nullptr;
}

/**
 * Checks whether a layout position is inside a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param rPos layout position
 * @return whether the position is inside the pane
 */
bool isContainPointPane(const IUseLayout* pLayout, const char* pPaneName,
                        const sead::Vector2f& rPos) {
    return isHitPane(findPane(pLayout, pPaneName), rPos);
}

/**
 * Checks whether the touch position of a port is inside a pane.
 * @param pLayout layout user
 * @param port controller port
 * @param pPaneName pane name
 * @return whether the touch position is inside the pane
 */
bool isContainPointPane(const IUseLayout* pLayout, s32 port, const char* pPaneName) {
    sead::Vector2f pos(0.0f, 0.0f);
    calcTouchLayoutPos(&pos, port);
    return isContainPointPane(pLayout, pPaneName, pos);
}

/**
 * Finds the topmost visible pane at a layout position.
 * @param pLayout layout user
 * @param rPos layout position
 * @return the hit pane, or nullptr
 */
nn::ui2d::Pane* findHitPaneFromLayoutPos(const IUseLayout* pLayout, const sead::Vector2f& rPos) {
    return findHitPaneRecursive(getRootPane(pLayout), rPos);
}

/**
 * Checks whether any visible pane is at a layout position.
 * @param pLayout layout user
 * @param rPos layout position
 * @return whether a pane was hit
 */
bool isExistHitPaneFromLayoutPos(const IUseLayout* pLayout, const sead::Vector2f& rPos) {
    return findHitPaneFromLayoutPos(pLayout, rPos) != nullptr;
}

/**
 * Finds the topmost visible pane at a screen position.
 * @param pLayout layout user
 * @param rPos screen position
 * @return the hit pane, or nullptr
 */
nn::ui2d::Pane* findHitPaneFromScreenPos(const IUseLayout* pLayout, const sead::Vector2f& rPos) {
    sead::Vector2f layoutPos;
    calcLayoutPosFromScreenPos(&layoutPos, rPos);
    return findHitPaneFromLayoutPos(pLayout, layoutPos);
}

/**
 * Checks whether any visible pane is at a screen position.
 * @param pLayout layout user
 * @param rPos screen position
 * @return whether a pane was hit
 */
bool isExistHitPaneFromScreenPos(const IUseLayout* pLayout, const sead::Vector2f& rPos) {
    return findHitPaneFromScreenPos(pLayout, rPos) != nullptr;
}

/**
 * Checks whether the touch position of a port is inside a pane.
 * @param pLayout layout user
 * @param port controller port
 * @param pPaneName pane name
 * @return whether the touch position is inside the pane
 */
bool isTouchPosInPane(const IUseLayout* pLayout, s32 port, const char* pPaneName) {
    sead::Vector2f pos(0.0f, 0.0f);
    calcTouchLayoutPos(&pos, port);
    return isContainPointPane(pLayout, pPaneName, pos);
}

/**
 * Places the four cursor corner panes around the cursor position pane of a target layout.
 * @param pCursor cursor layout
 * @param pTarget target layout
 */
void setCursorPanePos(IUseLayout* pCursor, const IUseLayout* pTarget) {
    sead::Vector3f size;
    calcPaneSize(&size, pTarget, "CursorPosition");
    sead::Vector3f trans;
    calcTrans(&trans, pTarget);
    f32 halfWidth = size.x * 0.5f;
    f32 halfHeight = size.y * 0.5f;
    setPaneLocalTrans(pCursor, "CursorTL", trans + sead::Vector3f(-halfWidth, halfHeight, 0.0f));
    setPaneLocalTrans(pCursor, "CursorTR", trans + sead::Vector3f(halfWidth, halfHeight, 0.0f));
    setPaneLocalTrans(pCursor, "CursorBL", trans + sead::Vector3f(-halfWidth, -halfHeight, 0.0f));
    setPaneLocalTrans(pCursor, "CursorBR", trans + sead::Vector3f(halfWidth, -halfHeight, 0.0f));
}

/**
 * Sets the vertex color of the left vertices of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param rColor color
 */
void setPaneVtxColor(const IUseLayout* pLayout, const char* pPaneName,
                     const sead::Color4u8& rColor) {
    nn::ui2d::Pane* pane = findPane(pLayout, pPaneName);
    nn::util::Unorm8x4 color = {{rColor.r, rColor.g, rColor.b, rColor.a}};
    pane->SetVertexColor(0, color);
    pane->SetVertexColor(2, color);
}

/**
 * Checks whether a pane was touched this frame.
 * @param pLayout layout user
 * @param port controller port
 * @param pPaneName pane name
 * @return whether the pane was touched this frame
 */
bool isTriggerTouchPane(const IUseLayout* pLayout, s32 port, const char* pPaneName) {
    if (!isPadTriggerTouch(port)) {
        return false;
    }

    return isTouchPosInPane(pLayout, port, pPaneName);
}

/**
 * Checks whether a pane is being touched.
 * @param pLayout layout user
 * @param port controller port
 * @param pPaneName pane name
 * @return whether the pane is being touched
 */
bool isHoldTouchPane(const IUseLayout* pLayout, s32 port, const char* pPaneName) {
    if (!isPadHoldTouch(port)) {
        return false;
    }

    return isTouchPosInPane(pLayout, port, pPaneName);
}

/**
 * Checks whether the touch on a pane was released this frame.
 * @param pLayout layout user
 * @param port controller port
 * @param pPaneName pane name
 * @return whether the touch was released on the pane
 */
bool isReleaseTouchPane(const IUseLayout* pLayout, s32 port, const char* pPaneName) {
    if (!isPadReleaseTouch(port)) {
        return false;
    }

    return isTouchPosInPane(pLayout, port, pPaneName);
}

/**
 * Counts the children of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return the number of children
 */
s32 getPaneChildNum(const IUseLayout* pLayout, const char* pPaneName) {
    nn::ui2d::Pane* pane = findPane(pLayout, pPaneName);
    nn::util::IntrusiveListNode* head = &pane->m_Children;
    s32 num = 0;

    for (nn::util::IntrusiveListNode* node = head->m_Next; node != head; node = node->m_Next) {
        num++;
    }

    return num;
}

/**
 * Gets the name of a child of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param index child index
 * @return the child name, or an empty string
 */
const char* getPaneChildName(const IUseLayout* pLayout, const char* pPaneName, s32 index) {
    nn::ui2d::Pane* pane = findPane(pLayout, pPaneName);
    nn::util::IntrusiveListNode* head = &pane->m_Children;
    s32 i = 0;

    for (nn::util::IntrusiveListNode* node = head->m_Next; node != head; node = node->m_Next) {
        if (i == index) {
            return nn::ui2d::Pane::FromLink(node)->GetName();
        }

        i++;
    }

    return "";
}

/**
 * Sets the string of a text box pane with an explicit length.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param pString string
 * @param pos start position
 * @param length string length
 */
void setPaneStringLength(IUseLayout* pLayout, const char* pPaneName, const char16_t* pString,
                         u16 pos, u16 length, u32 unk) {
    auto* textBox = static_cast<nn::ui2d::TextBox*>(findPane(pLayout, pPaneName));
    setTextBoxStringLength(textBox, pString, pos, length, unk);
    eui::Screen* screen = static_cast<eui::LayoutEx*>(getLayout(pLayout))->getScreen();

    if (screen == nullptr || (screen->mFlags & 0x20) == 0) {
        return;
    }

    for (nn::ui2d::Pane* pane = textBox->GetParent(); pane != nullptr; pane = pane->GetParent()) {
        eui::AlignPane* alignPane = eui::DynamicCast<eui::AlignPane>(pane);

        if (alignPane != nullptr) {
            alignPane->mDirty = true;
            return;
        }
    }
}

/**
 * Sets the string of a text box pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param pString string
 * @param pos start position
 */
void setPaneString(IUseLayout* pLayout, const char* pPaneName, const char16_t* pString, u16 pos,
                   u32 unk) {
    setPaneStringLength(pLayout, pPaneName, pString, pos,
                        calcMessageSizeWithoutNullCharacter(pString, nullptr), unk);
}

/**
 * Sets a text box pane to the last digit of a number.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param value number
 * @param pos start position
 */
void setPaneCounterDigit1(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos) {
    sead::WFormatFixedSafeString<4> str(u"%01d", value % 10);
    setPaneString(pLayout, pPaneName, str.cstr(), pos);
}

/**
 * Sets a text box pane to the last two digits of a number, zero padded.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param value number
 * @param pos start position
 */
void setPaneCounterDigit2(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos) {
    sead::WFormatFixedSafeString<4> str(u"%02d", value % 100);
    setPaneString(pLayout, pPaneName, str.cstr(), pos);
}

/**
 * Sets a text box pane to the last three digits of a number, zero padded.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param value number
 * @param pos start position
 */
void setPaneCounterDigit3(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos) {
    sead::WFormatFixedSafeString<4> str(u"%03d", value % 1000);
    setPaneString(pLayout, pPaneName, str.cstr(), pos);
}

/**
 * Sets a text box pane to the last four digits of a number, zero padded.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param value number
 * @param pos start position
 */
void setPaneCounterDigit4(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos) {
    sead::WFormatFixedSafeString<5> str(u"%04d", value % 10000);
    setPaneString(pLayout, pPaneName, str.cstr(), pos);
}

/**
 * Sets a text box pane to the last five digits of a number, zero padded.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param value number
 * @param pos start position
 */
void setPaneCounterDigit5(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos) {
    sead::WFormatFixedSafeString<6> str(u"%05d", value % 100000);
    setPaneString(pLayout, pPaneName, str.cstr(), pos);
}

/**
 * Sets a text box pane to the last six digits of a number, zero padded.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param value number
 * @param pos start position
 */
void setPaneCounterDigit6(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos) {
    sead::WFormatFixedSafeString<7> str(u"%06d", value % 1000000);
    setPaneString(pLayout, pPaneName, str.cstr(), pos);
}

/**
 * Sets a text box pane to the last digit of a number.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param value number
 * @param pos start position
 */
void setPaneNumberDigit1(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos) {
    sead::WFormatFixedSafeString<4> str(u"%d", value % 10);
    setPaneString(pLayout, pPaneName, str.cstr(), pos);
}

/**
 * Sets a text box pane to the last two digits of a number.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param value number
 * @param pos start position
 */
void setPaneNumberDigit2(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos) {
    sead::WFormatFixedSafeString<4> str(u"%d", value % 100);
    setPaneString(pLayout, pPaneName, str.cstr(), pos);
}

/**
 * Sets a text box pane to the last three digits of a number.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param value number
 * @param pos start position
 */
void setPaneNumberDigit3(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos) {
    sead::WFormatFixedSafeString<4> str(u"%d", value % 1000);
    setPaneString(pLayout, pPaneName, str.cstr(), pos);
}

/**
 * Sets a text box pane to the last four digits of a number.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param value number
 * @param pos start position
 */
void setPaneNumberDigit4(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos) {
    sead::WFormatFixedSafeString<5> str(u"%d", value % 10000);
    setPaneString(pLayout, pPaneName, str.cstr(), pos);
}

/**
 * Sets a text box pane to the last five digits of a number.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param value number
 * @param pos start position
 */
void setPaneNumberDigit5(IUseLayout* pLayout, const char* pPaneName, s32 value, u16 pos) {
    sead::WFormatFixedSafeString<6> str(u"%d", value % 100000);
    setPaneString(pLayout, pPaneName, str.cstr(), pos);
}

/**
 * Sets the string of a text box pane from a printf style format.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param pFormat format string
 */
void setPaneStringFormat(IUseLayout* pLayout, const char* pPaneName, const char* pFormat, ...) {
    std::va_list args;
    va_start(args, pFormat);
    StringTmp<1024> str;
    str.formatV(pFormat, args);
    va_end(args);

    sead::WFixedSafeString<1024> wideStr;
    const char* src = str.cstr();
    sead::StringUtil::convertUtf8ToUtf16(wideStr.getBuffer(), wideStr.getBufferSize(), src, -1);
    setPaneString(pLayout, pPaneName, wideStr.cstr());
}

/**
 * Centers a text box pane and its text horizontally.
 * @param pLayout layout user
 * @param pPaneName pane name
 */
void setTextPositionCenterH(IUseLayout* pLayout, const char* pPaneName) {
    findPane(pLayout, pPaneName)->mOriginFlags &= ~3;
    auto* textBox = static_cast<nn::ui2d::TextBox*>(findPane(pLayout, pPaneName));
    TextBoxTextPosition* textPosition = getTextPosition(textBox);
    bool isChanged = (textPosition->textPosition & 3) != 0;
    textPosition->isTextPositionDirty = textPosition->isTextPositionDirty | isChanged;

    if (isChanged) {
        textPosition->textPosition &= ~3;
    }
}

/**
 * Sets a text box pane to an icon followed by the last three digits of a number.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param pIcon icon character
 * @param value number
 * @param pos start position
 */
void setPaneCounterDigit3WithIcon(IUseLayout* pLayout, const char* pPaneName,
                                  const char16_t* pIcon, s32 value, u16 pos) {
    sead::WFormatFixedSafeString<5> str(u"%c%03d", *pIcon, value % 1000);
    setPaneString(pLayout, pPaneName, str.cstr(), pos);
}

/**
 * Initializes the text boxes of a pane from a message.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param pHolder message holder
 * @param pLabel message label
 */
void initPaneMessage(IUseLayout* pLayout, const char* pPaneName, const MessageHolder* pHolder,
                     const char* pLabel, u32 unk) {
    initTextBoxRecursive(findPane(pLayout, pPaneName), pHolder, pLabel, unk);
}

/**
 * Sets the string of a text box pane to a system message.
 * @param pActor layout actor
 * @param pPaneName pane name
 * @param pFileName message file name
 * @param pLabel message label
 */
void setPaneSystemMessage(LayoutActor* pActor, const char* pPaneName, const char* pFileName,
                          const char* pLabel) {
    setPaneString(pActor, pPaneName, getSystemMessageString(pActor, pFileName, pLabel));
}

/**
 * Sets the string of a text box pane to a stage message.
 * @param pActor layout actor
 * @param pPaneName pane name
 * @param pFileName message file name
 * @param pLabel message label
 */
void setPaneStageMessage(LayoutActor* pActor, const char* pPaneName, const char* pFileName,
                         const char* pLabel) {
    const char16_t* message = nullptr;

    if (!tryGetStageMessageString(&message, pActor, pFileName, pLabel)) {
        message = u"NULL";
    }

    setPaneString(pActor, pPaneName, message);
}

/**
 * Gets the string buffer of a text box pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return the string buffer
 */
const char16_t* getPaneStringBuffer(const IUseLayout* pLayout, const char* pPaneName) {
    auto* textBox = static_cast<nn::ui2d::TextBox*>(findPane(pLayout, pPaneName));
    return reinterpret_cast<const char16_t*>(textBox->GetStringBuffer());
}

/**
 * Gets the string buffer length of a text box pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return always 0
 */
s32 getPaneStringBufferLength(const IUseLayout* pLayout, const char* pPaneName) {
    return 0;
}

/**
 * Sets the font of a text box pane.
 * @param pActor layout actor
 * @param pPaneName pane name
 * @param pFontName font name
 */
void setTextBoxPaneFont(const LayoutActor* pActor, const char* pPaneName, const char* pFontName) {
    nn::ui2d::Pane* pane = findPane(pActor, pPaneName);
    const eui::FontMgr* fontMgr = pActor->getLayoutSceneInfo()->getFontMgr();
    setTextBoxPaneFont(pane, fontMgr->getFont(sead::SafeString(pFontName)));
}

/**
 * Resizes all panes of a layout actor to fit their text.
 * @param pActor layout actor
 */
void adjustPaneSizeToTextSizeAll(const LayoutActor* pActor) {
    nn::ui2d::Pane* rootPane = pActor->getLayoutKeeper()->getLayout()->GetRootPane();
    eui::IteratePaneForSetupPaneAfterBuild(
        rootPane, static_cast<eui::LayoutEx*>(pActor->getLayoutKeeper()->getLayout()));
}

/**
 * Requests a capture update for all panes of a layout actor.
 * @param pActor layout actor
 */
void requestCaptureRecursive(const LayoutActor* pActor) {
    requestCaptureRecursive(pActor->getLayoutKeeper()->getLayout()->GetRootPane());
}

/**
 * Sets the ruby scale of the tag processor of a layout actor.
 * @param pActor layout actor
 * @param scale ruby scale
 */
void setRubyScale(const LayoutActor* pActor, f32 scale) {
    pActor->getLayoutKeeper()->getTagProcessor()->setRubyScale(scale);
}

/**
 * Creates an empty placement texture info.
 * @return the texture info
 */
nn::ui2d::TextureInfo* createTextureInfo() {
    return new nn::ui2d::PlacementTextureInfo();
}

/**
 * Creates a texture info for a texture.
 * @param rTexture texture
 * @param isPlacement whether only the texture size is used
 * @return the texture info
 */
nn::ui2d::TextureInfo* createTextureInfo(const agl::TextureData& rTexture, bool isPlacement) {
    nn::ui2d::TextureInfo* info = new nn::ui2d::PlacementTextureInfo();

    if (isPlacement) {
        u16 width = rTexture.getWidth(0);
        u16 height = rTexture.getHeight(0);
        auto* placementInfo = eui::DynamicCast<nn::ui2d::PlacementTextureInfo>(info);

        if (placementInfo != nullptr) {
            placementInfo->mWidth = width;
            placementInfo->mHeight = height;
        }
    } else {
        eui::SetupTextureInfoByAglTextureData(info, rTexture, nullptr);
    }

    return info;
}

/**
 * Creates a texture info that shares the texture of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @return the texture info
 */
nn::ui2d::TextureInfo* createTextureInfo(const IUseLayout* pLayout, const char* pPaneName) {
    nn::ui2d::TextureInfo* info = new nn::ui2d::PlacementTextureInfo();
    getPaneTextureInfo(info, pLayout, pPaneName);
    return info;
}

/**
 * Copies the texture descriptor of a pane into a texture info.
 * @param pOut output texture info
 * @param pLayout layout user
 * @param pPaneName pane name
 */
void getPaneTextureInfo(nn::ui2d::TextureInfo* pOut, const IUseLayout* pLayout,
                        const char* pPaneName) {
    nn::ui2d::Material* material = findPane(pLayout, pPaneName)->GetMaterial(0);

    if (material != nullptr && (material->mResourceCounts & 3) != 0) {
        const nn::ui2d::TextureInfo* textureInfo = material->GetFirstTexMap()->m_pTextureInfo;
        pOut->mDescriptor.Invalidate();
        pOut->mDescriptor = textureInfo->mDescriptor;
    }
}

/**
 * Creates a texture info for a texture in a bfres file of an archive.
 * @param pArchiveName archive name
 * @param pFileName bfres file name without extension
 * @param pTextureName texture name
 * @return the texture info, or nullptr if the texture doesn't exist
 */
nn::ui2d::TextureInfo* createTextureInfo(const char* pArchiveName, const char* pFileName,
                                         const char* pTextureName) {
    Resource* resource = findOrCreateResource(pArchiveName, nullptr);
    StringTmp<256> fileName("%s.bfres", pFileName);
    nn::g3d::ResFile* resFile =
        nn::g3d::ResFile::ResCast(resource->getOtherFile(fileName, nullptr));
    nn::gfx::ResTexture* texture = agl::g3d::ResFile::GetTexture(resFile, pTextureName);
    auto* info = new DisposerTextureInfo();

    if (texture == nullptr) {
        return nullptr;
    }

    nn::ui2d::LoadTexture(info, reinterpret_cast<nn::gfx::Device*>(
                                    sead::GraphicsNvn::instance()->getGfxDevice()),
                          texture);
    eui::RegisterSlotForTexture(&info->mDescriptor, *info->GetTextureView(), nullptr);
    return info;
}

/**
 * Updates a texture info from a texture.
 * @param pInfo texture info
 * @param rTexture texture
 */
void updateTextureInfo(nn::ui2d::TextureInfo* pInfo, const agl::TextureData& rTexture) {
    eui::SetupTextureInfoByAglTextureData(pInfo, rTexture, nullptr);
}

/**
 * Sets the texture of a pane.
 * @param pLayout layout user
 * @param pPaneName pane name
 * @param pInfo texture info
 */
void setPaneTexture(IUseLayout* pLayout, const char* pPaneName,
                    const nn::ui2d::TextureInfo* pInfo) {
    nn::ui2d::Material* material = findPane(pLayout, pPaneName)->GetMaterial(0);

    if (material != nullptr && (material->mResourceCounts & 3) != 0) {
        material->SetTextureInfo(0, pInfo);
    }
}

/**
 * Registers a parts actor to a layout actor.
 * @param pActor layout actor
 * @param pPartsActor parts actor
 */
void registerLayoutPartsActor(LayoutActor* pActor, LayoutActor* pPartsActor) {
    pActor->getLayoutPartsActorKeeper()->resisterPartsActor(pPartsActor);
}

/**
 * Resizes all panes to their text and flags text boxes that contain pad switch tags.
 * @param pActor layout actor
 */
void updateLayoutPaneRecursive(LayoutActor* pActor) {
    auto* layout = static_cast<eui::LayoutEx*>(pActor->getLayoutKeeper()->getLayout());
    updatePaneRecursive(pActor, layout, pActor->getLayoutKeeper()->getLayout()->GetRootPane());
}

/**
 * Gets the number of pane groups of a layout actor.
 * @param pActor layout actor
 * @return the number of pane groups
 */
s32 getLayoutPaneGroupNum(LayoutActor* pActor) {
    return pActor->getLayoutKeeper()->getGroupNum();
}

/**
 * Gets a pane group of a layout actor by index.
 * @param pActor layout actor
 * @param index group index
 * @return the pane group
 */
LayoutPaneGroup* getLayoutPaneGroup(LayoutActor* pActor, s32 index) {
    return pActor->getLayoutKeeper()->getGroup(index);
}

/**
 * Gets a pane group of a layout actor by name.
 * @param pActor layout actor
 * @param pGroupName group name
 * @return the pane group
 */
LayoutPaneGroup* getLayoutPaneGroup(const LayoutActor* pActor, const char* pGroupName) {
    return pActor->getLayoutKeeper()->getGroup(pGroupName);
}

/**
 * Gets the number of parts actors of a layout actor.
 * @param pActor layout actor
 * @return the number of parts actors
 */
s32 getLayoutPartsNum(LayoutActor* pActor) {
    LayoutPartsActorKeeper* keeper = pActor->getLayoutPartsActorKeeper();

    if (keeper == nullptr) {
        return 0;
    }

    return keeper->getPartsActorNum();
}

/**
 * Gets a parts actor of a layout actor by index.
 * @param pActor layout actor
 * @param index parts actor index
 * @return the parts actor, or nullptr
 */
LayoutActor* getLayoutPartsActor(LayoutActor* pActor, s32 index) {
    return pActor->getLayoutPartsActorKeeper()->getPartsActor(index);
}

/**
 * Gets a parts actor of a layout actor by name.
 * @param pActor layout actor
 * @param pName parts actor name
 * @return the parts actor
 */
LayoutActor* getLayoutPartsActor(LayoutActor* pActor, const char* pName) {
    return pActor->getLayoutPartsActorKeeper()->getPartsActor(pName);
}
}  // namespace al
