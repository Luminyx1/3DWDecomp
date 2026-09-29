#include "utility/aglDebugTexturePage.h"

#include <gfx/seadFrameBuffer.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadTextWriter.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>

#include "common/aglDrawContext.h"
#include "common/aglRenderBuffer.h"
#include "common/aglRenderTarget.h"
#include "common/aglTextureData.h"
#include "common/aglTextureFormatInfo.h"
#include "common/aglTextureSampler.h"
#include "detail/aglRootNode.h"
#include "utility/aglDebugTextureDrawer.h"
#include "utility/aglDynamicTextureAllocator.h"
#include "utility/aglImageFilter2D.h"

namespace agl::utl {

namespace {

inline u32 getMipWidth(const TextureData& rTexture, s32 mipLevel)
{
    s32 width = rTexture.getSurface().mWidth >> mipLevel;
    return width > 1 ? width : 1;
}

inline u32 getMipHeight(const TextureData& rTexture, s32 mipLevel)
{
    s32 min = rTexture.getMinHeight_();
    s32 height = rTexture.getSurface().mHeight >> mipLevel;
    return min > height ? min : height;
}

inline u32 getWidth(const TextureData& rTexture)
{
    u32 width = rTexture.getSurface().mWidth;
    return width > 1 ? width : 1;
}

inline u32 getHeight(const TextureData& rTexture)
{
    s32 min = rTexture.getMinHeight_();
    s32 height = rTexture.getSurface().mHeight;
    return min > height ? min : height;
}

inline u32 getSlice(const TextureData& rTexture)
{
    s32 min = rTexture.getMinSlice_();
    s32 slice = rTexture.getSurface().mDepth;
    return min > slice ? min : slice;
}

inline f32 calcMipWidthSum(const TextureData& rTexture, u32 startMip, u32 endMip, u32 faceNum)
{
    f32 width = 0.0f;
    for (u32 i = startMip; i < endMip; i++)
    {
        width += getMipWidth(rTexture, i) * faceNum;
    }
    return width;
}

inline f32 calcMipHeightSum(const TextureData& rTexture, u32 startMip, u32 endMip)
{
    f32 height = 0.0f;
    for (u32 i = startMip; i < endMip; i++)
    {
        height += getMipHeight(rTexture, i);
    }
    return height;
}

inline void setNodeMeta(sead::hostio::Node* pNode, const char* pMeta)
{
    detail::RootNode::setNodeMeta(pNode, pMeta);
}

inline u32 getMipSlice(const TextureData& rTexture, s32 mipLevel)
{
    s32 min = rTexture.getMinSlice_();
    s32 slice = rTexture.getSurface().mDepth >> mipLevel;
    return min > slice ? min : slice;
}

inline void applyDrawOption(TextureSampler* pSampler, const TextureData& rTex,
                            const DebugTexturePage::DrawOption& rOption, bool isSpecial)
{
    if (rOption.mIsFilterLinear)
    {
        pSampler->setFilter(1, 1, 2);
    }
    else
    {
        pSampler->setFilter(0, 0, 1);
    }

    switch (rOption.mChannel)
    {
    case 1:
    {
        TextureCompSel compSel =
            TextureFormatInfo::getDefaultCompSel(TextureFormat(rTex.getTextureFormat()), 0);
        pSampler->setCompSel(compSel, compSel, compSel, cTextureCompSel_1);
        break;
    }
    case 2:
    {
        TextureCompSel compSel =
            TextureFormatInfo::getDefaultCompSel(TextureFormat(rTex.getTextureFormat()), 1);
        pSampler->setCompSel(compSel, compSel, compSel, cTextureCompSel_1);
        break;
    }
    case 3:
    {
        TextureCompSel compSel =
            TextureFormatInfo::getDefaultCompSel(TextureFormat(rTex.getTextureFormat()), 2);
        pSampler->setCompSel(compSel, compSel, compSel, cTextureCompSel_1);
        break;
    }
    case 4:
    {
        TextureCompSel compSel =
            TextureFormatInfo::getDefaultCompSel(TextureFormat(rTex.getTextureFormat()), 3);
        pSampler->setCompSel(compSel, compSel, compSel, cTextureCompSel_1);
        break;
    }
    default:
        break;
    }
}

}  // namespace

/**
 * Constructs an inactive page without contexts.
 */
DebugTexturePage::DebugTexturePage() : sead::TListNode<DebugTexturePage*>(this) {}

/**
 * Does nothing.
 * @param num unused
 * @param rName unused
 * @param pHeap unused
 */
void DebugTexturePage::setUp(u32 num, const sead::SafeString& rName, sead::Heap* pHeap) {}

/**
 * Does nothing.
 */
void DebugTexturePage::cleanUp() {}

/**
 * Registers a texture with the current context, either copying it or referencing it.
 * @param pDrawContext draw context
 * @param index context index the texture is meant for, or -1 for any
 * @param rTexture texture to register
 * @param rName name of the entry
 * @param type texture type
 * @param min minimum displayed value
 * @param max maximum displayed value
 * @param isLineBreakBefore whether to start a new line before the texture
 * @param isLineBreakAfter whether to start a new line after the texture
 * @param isSpecial special display flag
 * @param isCopy whether to copy the texture instead of referencing it
 * @return whether the texture was registered
 */
bool DebugTexturePage::entryTexture_(DrawContext* pDrawContext, s32 index,
                                     const TextureData& rTexture, const sead::SafeString& rName,
                                     DebugTexture::Type type, f32 min, f32 max,
                                     bool isLineBreakBefore, bool isLineBreakAfter,
                                     bool isSpecial, bool isCopy) const
{
    if (DynamicTextureAllocator::instance()->isCacheEnabled())
    {
        return false;
    }
    s32 contextIndex = mCurrentContext;
    if (index != -1 && contextIndex != index)
    {
        return false;
    }

    DebugTextureDrawer* pDrawer = DebugTextureDrawer::instance();
    if (!pDrawer)
    {
        return false;
    }
    DebugTexture* pTexture = pDrawer->popBack_();
    if (!pTexture)
    {
        return false;
    }

    pTexture->initialize(rName);
    mContexts[contextIndex].pushBack(pTexture, mSelectTextureName);
    if (isCopy)
    {
        pTexture->copyTexture(pDrawContext, rTexture, type, min, max, isLineBreakBefore,
                              isLineBreakAfter, isSpecial);
    }
    else
    {
        pTexture->setReference(rTexture, type, min, max, isLineBreakBefore, isLineBreakAfter,
                               isSpecial);
    }
    return true;
}

/**
 * Sets the name of the texture.
 * @param rName new name
 */
void DebugTexture::initialize(const sead::SafeString& rName)
{
    mName.copy(rName);
}

/**
 * Adds a texture to the context and selects it if its name matches the selected name.
 * @param pTexture texture to add
 * @param rSelectName name of the texture to select
 */
void DebugTexturePage::Context::pushBack(DebugTexture* pTexture,
                                         const sead::SafeString& rSelectName) const
{
    mTextures.pushBack(pTexture);
    copyTextureLabel();
    if (!rSelectName.isEmpty() && pTexture->getName() == rSelectName)
    {
        mSelectIndex = mTextures.indexOf(pTexture);
    }
}

/**
 * Allocates a copy of a texture and copies its contents.
 * @param pDrawContext draw context
 * @param rTexture texture to copy
 * @param type texture type
 * @param min minimum displayed value
 * @param max maximum displayed value
 * @param isLineBreakBefore whether to start a new line before the texture
 * @param isLineBreakAfter whether to start a new line after the texture
 * @param isSpecial special display flag
 * @return true
 */
bool DebugTexture::copyTexture(DrawContext* pDrawContext, const TextureData& rTexture, Type type,
                               f32 min, f32 max, bool isLineBreakBefore, bool isLineBreakAfter,
                               bool isSpecial)
{
    allocTexture(pDrawContext, rTexture, type, min, max, isLineBreakBefore, isLineBreakAfter,
                 isSpecial);
    rTexture.copyToAll(pDrawContext, mTexture);
    return true;
}

/**
 * Makes the debug texture reference an existing texture.
 * @param rTexture texture to reference
 * @param type texture type
 * @param min minimum displayed value
 * @param max maximum displayed value
 * @param isLineBreakBefore whether to start a new line before the texture
 * @param isLineBreakAfter whether to start a new line after the texture
 * @param isSpecial special display flag
 */
void DebugTexture::setReference(const TextureData& rTexture, Type type, f32 min, f32 max,
                                bool isLineBreakBefore, bool isLineBreakAfter, bool isSpecial)
{
    freeTexture();
    mTexture = &rTexture;
    mIsLineBreakBefore = isLineBreakBefore;
    mIsLineBreakAfter = isLineBreakAfter;
    mIsSpecial = isSpecial;
    mType = type;
    mMin = min;
    mMax = max;
    mIsAllocated = false;
}

/**
 * Registers the expanded hierarchical depth buffer of a depth target with the current context.
 * @param pDrawContext draw context
 * @param index context index the texture is meant for, or -1 for any
 * @param rDepth depth target to expand
 * @param rName name of the entry
 * @param type texture type
 * @param min minimum displayed value
 * @param max maximum displayed value
 * @param depthIndex index stored with the texture
 * @param isLineBreakBefore whether to start a new line before the texture
 * @param isLineBreakAfter whether to start a new line after the texture
 * @param isSpecial special display flag
 * @return whether the texture was registered
 */
bool DebugTexturePage::entryRenderTargetDepth_(DrawContext* pDrawContext, s32 index,
                                               const RenderTargetDepth& rDepth,
                                               const sead::SafeString& rName,
                                               DebugTexture::Type type, f32 min, f32 max,
                                               u8 depthIndex, bool isLineBreakBefore,
                                               bool isLineBreakAfter, bool isSpecial) const
{
    if (DynamicTextureAllocator::instance()->isCacheEnabled())
    {
        return false;
    }
    s32 contextIndex = mCurrentContext;
    if (index != -1 && contextIndex != index)
    {
        return false;
    }

    DebugTextureDrawer* pDrawer = DebugTextureDrawer::instance();
    if (!pDrawer)
    {
        return false;
    }
    DebugTexture* pTexture = pDrawer->popBack_();
    if (!pTexture)
    {
        return false;
    }

    pTexture->initialize(rName);
    mContexts[contextIndex].pushBack(pTexture, mSelectTextureName);
    pTexture->expand(pDrawContext, rDepth, type, min, max, depthIndex, isLineBreakBefore,
                     isLineBreakAfter, isSpecial);
    return true;
}

/**
 * Allocates a texture matching a depth target and expands its hierarchical depth buffer into it.
 * @param pDrawContext draw context
 * @param rDepth depth target to expand
 * @param type texture type
 * @param min minimum displayed value
 * @param max maximum displayed value
 * @param index index stored with the texture
 * @param isLineBreakBefore whether to start a new line before the texture
 * @param isLineBreakAfter whether to start a new line after the texture
 * @param isSpecial special display flag
 * @return true
 */
bool DebugTexture::expand(DrawContext* pDrawContext, const RenderTargetDepth& rDepth, Type type,
                          f32 min, f32 max, u8 index, bool isLineBreakBefore,
                          bool isLineBreakAfter, bool isSpecial)
{
    freeTexture();
    TextureFormat format = TextureFormat(rDepth.getTextureFormat());
    DynamicTextureAllocator* pAllocator = DynamicTextureAllocator::instance();
    switch (rDepth.getTextureType())
    {
    case NVN_TEXTURE_TARGET_2D:
        mTexture = pAllocator->allocWithoutContext(
            pDrawContext, mName, format, rDepth.getWidth(0), rDepth.getHeight(0),
            rDepth.getMipLevelNum(), nullptr, DynamicTextureAllocator::cAllocateType_2, true,
            false);
        break;
    case NVN_TEXTURE_TARGET_2D_ARRAY:
        mTexture = pAllocator->allocArrayWithoutContext(
            pDrawContext, mName, format, rDepth.getWidth(0), rDepth.getHeight(0),
            rDepth.getMipSlice(0), rDepth.getMipLevelNum(), nullptr,
            DynamicTextureAllocator::cAllocateType_2, true, false);
        break;
    default:
        break;
    }
    mIsLineBreakBefore = isLineBreakBefore;
    mIsLineBreakAfter = isLineBreakAfter;
    mIsSpecial = isSpecial;
    mType = type;
    mMin = min;
    mMax = max;
    mIndex = index;
    mIsAllocated = true;
    rDepth.expandHiZBufferToAllSlice(pDrawContext, mTexture);
    return true;
}

/**
 * Does nothing.
 * @param pDrawContext unused
 * @param index unused
 * @param rName unused
 */
void DebugTexturePage::entryBoundRenderBuffer(DrawContext* pDrawContext, s32 index,
                                              const sead::SafeString& rName) const
{
}

/**
 * Copies the currently bound color target into a newly allocated texture.
 * @param pDrawContext draw context
 * @param index index of the color target
 * @return whether a color target was bound
 */
bool DebugTexture::copyCurrentRenderTargetColor(DrawContext* pDrawContext, s32 index)
{
    TextureData texture;
    if (!RenderBuffer::initTextureDataFromBoundColor(pDrawContext, &texture, index))
    {
        return false;
    }
    allocTexture(pDrawContext, texture, cType_Color, 0.0f, 0.0f, true, true, true);
    RenderBuffer::copyTextureDataFromBoundColor(pDrawContext, mTexture, index);
    return true;
}

/**
 * Copies the currently bound depth target into a newly allocated texture.
 * @param pDrawContext draw context
 * @return whether a depth target was bound
 */
bool DebugTexture::copyCurrentRenderTargetDepth(DrawContext* pDrawContext)
{
    TextureData texture;
    if (!RenderBuffer::initTextureDataFromBoundDepth(pDrawContext, &texture))
    {
        return false;
    }
    allocTexture(pDrawContext, texture, cType_Color, 0.0f, 0.0f, true, true, true);
    RenderBuffer::copyTextureDataFromBoundDepth(pDrawContext, mTexture);
    return true;
}

/**
 * Activates or deactivates the page, clearing its textures when it is deactivated.
 * @param active whether the page is active
 */
void DebugTexturePage::setActive(bool active)
{
    if (active == mIsActive)
    {
        return;
    }

    DebugTextureDrawer* pDrawer = DebugTextureDrawer::instance();
    if (!pDrawer)
    {
        return;
    }

    if (active)
    {
        if (pDrawer->mActivePages.isFull())
        {
            pDrawer->mActivePages.popFront();
        }
        pDrawer->mActivePages.pushBack(this);
    }
    else
    {
        pDrawer->eraseActivePage_(this);
        clearEntryTextures_();
    }
    mIsActive = active;
}

/**
 * Returns the textures of every context to the drawer.
 */
void DebugTexturePage::clearEntryTextures_() const
{
    for (const Context& rContext : mContexts)
    {
        rContext.clearEntryTextures();
    }
}

/**
 * Draws the textures of the current context when the page is active.
 * @param pDrawContext draw context
 * @param rFrameBuffer frame buffer to draw to
 * @param rViewport viewport to draw in
 */
void DebugTexturePage::draw_(DrawContext* pDrawContext, const sead::LogicalFrameBuffer& rFrameBuffer,
                             const sead::Viewport& rViewport) const
{
    if (mIsActive)
    {
        getCurrentContext_().draw(pDrawContext, rFrameBuffer, rViewport, mDrawOption);
    }
}

/**
 * Returns every texture of the context to the drawer.
 */
void DebugTexturePage::Context::clearEntryTextures() const
{
    while (sead::TListNode<DebugTexture*>* pNode = mTextures.popFront())
    {
        pNode->mList = nullptr;
        pNode->mData->freeTexture();
        if (DebugTextureDrawer::instance())
        {
            DebugTextureDrawer::instance()->pushBack_(pNode->mData);
        }
    }
}

/**
 * Does nothing.
 * @param pContext unused
 */
void DebugTexturePage::genNodePage_(sead::hostio::Context* pContext) {}

/**
 * Generates the host IO message of the page.
 * @param pContext host IO context
 */
void DebugTexturePage::genMessage(sead::hostio::Context* pContext)
{
    genMessagePage_(pContext, nullptr, nullptr);
}

/**
 * Generates the host IO message of the page and updates its node meta data.
 * @param pContext host IO context
 * @param pReflexible owner of the page
 * @param pListener listener receiving the property events of the page
 */
void DebugTexturePage::genMessagePage_(sead::hostio::Context* pContext,
                                       sead::hostio::Reflexible* pReflexible,
                                       sead::hostio::PropertyEventListener* pListener)
{
    const char* pHeader;
    if (pReflexible)
    {
        mOwner = pReflexible;
        pHeader = "Debug display";
    }
    else
    {
        pHeader = mName.cstr();
    }

    {
        sead::FormatFixedSafeString<160> meta("Dir=X, Layout=Wrap, GroupHeader=%s, BgColor=%s",
                                              pHeader, "Transparent");
    }

    if (mIsActive)
    {
        for (u32 i = 0; i < u32(mContexts.size()); i++)
        {
            sead::FormatFixedSafeString<32> name("Context_%d", i);
        }
        getCurrentContext_().genMessageContextComboBox(pContext, true, pListener);
    }
    if (mIsActive)
    {
        getCurrentContext_().genMessageContextParameter(pContext, true, pListener);
    }

    updateNodeMeta_();
}

/**
 * Generates the host IO message of the page as part of its owner.
 * @param pContext host IO context
 * @param pReflexible owner of the page
 */
void DebugTexturePage::genMessagePage(sead::hostio::Context* pContext,
                                      sead::hostio::Reflexible* pReflexible)
{
    genMessagePage_(pContext, pReflexible, this);
}

/**
 * Updates the host IO node meta data of the page.
 * @return whether the cached meta data changed
 */
bool DebugTexturePage::updateNodeMeta_()
{
    sead::hostio::Node* pNode = this;
    sead::FormatFixedSafeString<64> meta("Icon=%s, BgColor=%s", mIsActive ? "TEXTURE" : "NOTE",
                                         "Transparent");
    setNodeMeta(pNode, meta.cstr());
    if (mNodeMeta != sead::SafeString(sead::SafeString::cEmptyString))
    {
        mNodeMeta.copy(sead::SafeString(sead::SafeString::cEmptyString));
        return true;
    }
    return false;
}

/**
 * Touches the texture labels shown in the host IO combo box.
 * @param pContext host IO context
 * @param isActive unused
 * @param pListener unused
 */
void DebugTexturePage::Context::genMessageContextComboBox(
    sead::hostio::Context* pContext, bool isActive, sead::hostio::PropertyEventListener* pListener)
{
    DebugTextureDrawer* pDrawer = DebugTextureDrawer::instance();
    if (pDrawer && mLabelNum > 0)
    {
        pDrawer->mTextureLabels[0].cstr();
        for (s32 i = 1; i < mLabelNum; i++)
        {
            DebugTextureDrawer::instance()->mTextureLabels[i].cstr();
        }
    }
}

/**
 * Formats the host IO parameter string of the context.
 * @param pContext host IO context
 * @param isActive unused
 * @param pListener unused
 */
void DebugTexturePage::Context::genMessageContextParameter(
    sead::hostio::Context* pContext, bool isActive, sead::hostio::PropertyEventListener* pListener)
{
    sead::FormatFixedSafeString<32> range("Min=-100, Max=%f", mContentHeight);
}

/**
 * Does nothing.
 * @param pEvent unused
 */
void DebugTexturePage::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

/**
 * Updates the host IO node meta data of the page.
 */
void DebugTexturePage::invalidateHostIoNode()
{
    updateNodeMeta_();
}

/**
 * Does nothing.
 * @param pEvent unused
 */
void DebugTexturePage::listenNodeEvent(const sead::hostio::NodeEvent* pEvent) {}

/**
 * Constructs an empty context.
 */
DebugTexturePage::Context::Context() = default;

/**
 * Destroys the context.
 */
DebugTexturePage::Context::~Context() {}

/**
 * Resets the scroll position and scale and sets the owner page.
 * @param pPage owner page
 * @param pHeap unused
 */
void DebugTexturePage::Context::setUp(const DebugTexturePage* pPage, sead::Heap* pHeap)
{
    mScroll = sead::Vector2f::zero;
    mScale = 1.0f;
    mSliceIndex = -1;
    mMipLevel = -1;
    mPage = pPage;
}

/**
 * Returns the texture selected for full screen display.
 * @return the selected texture, or nullptr if there is none
 */
const DebugTexture* DebugTexturePage::Context::searchFullScreenTexture() const
{
    if (mSelectIndex >= 0 && mLabelNum > mSelectIndex)
    {
        DebugTextureDrawer* pDrawer = DebugTextureDrawer::instance();
        if (!pDrawer)
        {
            return nullptr;
        }
        const sead::SafeString& rLabel = pDrawer->mTextureLabels[mSelectIndex];
        for (auto it = mTextures.begin(); it != mTextures.end(); ++it)
        {
            if (rLabel == (*it)->getName())
            {
                return *it;
            }
        }
    }
    mSelectIndex = -1;
    return nullptr;
}

/**
 * Frees the texture if it was allocated by the debug texture.
 */
void DebugTexture::freeTexture()
{
    if (mTexture)
    {
        if (mIsAllocated)
        {
            DynamicTextureAllocator::instance()->free(mTexture);
        }
        mIsAllocated = false;
        mTexture = nullptr;
    }
}

/**
 * Copies the names of the textures of the context into the label table of the drawer.
 */
void DebugTexturePage::Context::copyTextureLabel() const
{
    DebugTextureDrawer* pDrawer = DebugTextureDrawer::instance();
    if (!pDrawer)
    {
        return;
    }
    s32 num = 0;
    for (auto it = mTextures.begin(); it != mTextures.end(); ++it)
    {
        if (!DebugTextureDrawer::instance()->copyTextureLabel_(num, (*it)->getName()))
        {
            break;
        }
        num++;
    }
    mLabelNum = num;
}

/**
 * Does nothing.
 * @param pEvent unused
 */
void DebugTexturePage::Context::listenPropertyEventContext(const sead::hostio::PropertyEvent* pEvent)
{
}

/**
 * Constructs an unnamed debug texture without texture.
 */
DebugTexture::DebugTexture() : sead::TListNode<DebugTexture*>(this), mName(sead::SafeString("")) {}

/**
 * Frees the texture if it was allocated.
 */
DebugTexture::~DebugTexture()
{
    freeTexture();
}

/**
 * Allocates a texture with the same shape and format as a source texture.
 * @param pDrawContext draw context
 * @param rTexture source texture
 * @param type texture type
 * @param min minimum displayed value
 * @param max maximum displayed value
 * @param isLineBreakBefore whether to start a new line before the texture
 * @param isLineBreakAfter whether to start a new line after the texture
 * @param isSpecial special display flag
 */
void DebugTexture::allocTexture(DrawContext* pDrawContext, const TextureData& rTexture, Type type,
                                f32 min, f32 max, bool isLineBreakBefore, bool isLineBreakAfter,
                                bool isSpecial)
{
    freeTexture();
    DynamicTextureAllocator* pAllocator = DynamicTextureAllocator::instance();
    switch (rTexture.getTextureType())
    {
    case NVN_TEXTURE_TARGET_2D:
        mTexture = pAllocator->allocWithoutContext(
            pDrawContext, mName, TextureFormat(rTexture.getTextureFormat()), rTexture.getWidth(0),
            rTexture.getHeight(0), rTexture.getMipLevelNum(), nullptr,
            DynamicTextureAllocator::cAllocateType_2, true, false);
        break;
    case NVN_TEXTURE_TARGET_3D:
        mTexture = pAllocator->alloc3DWithoutContext(
            pDrawContext, mName, TextureFormat(rTexture.getTextureFormat()), rTexture.getWidth(0),
            rTexture.getHeight(0), rTexture.getMipSlice(0), rTexture.getMipLevelNum(), nullptr,
            DynamicTextureAllocator::cAllocateType_2, true, false);
        break;
    case NVN_TEXTURE_TARGET_2D_ARRAY:
        mTexture = pAllocator->allocArrayWithoutContext(
            pDrawContext, mName, TextureFormat(rTexture.getTextureFormat()), rTexture.getWidth(0),
            rTexture.getHeight(0), rTexture.getMipSlice(0), rTexture.getMipLevelNum(), nullptr,
            DynamicTextureAllocator::cAllocateType_2, true, false);
        break;
    case NVN_TEXTURE_TARGET_CUBEMAP:
    {
        u32 arrayNum = rTexture.getMipSlice(0) / 6;
        if (arrayNum == 1)
        {
            mTexture = pAllocator->allocCubeWithoutContext(
                pDrawContext, mName, TextureFormat(rTexture.getTextureFormat()),
                rTexture.getWidth(0), rTexture.getMipLevelNum(), nullptr,
                DynamicTextureAllocator::cAllocateType_2, true, false);
        }
        else
        {
            mTexture = pAllocator->allocCubeArrayWithoutContext(
                pDrawContext, mName, TextureFormat(rTexture.getTextureFormat()),
                rTexture.getWidth(0), rTexture.getMipSlice(0) / 6, rTexture.getMipLevelNum(),
                nullptr, DynamicTextureAllocator::cAllocateType_2, true, false);
        }
        break;
    }
    default:
        break;
    }
    mIsLineBreakBefore = isLineBreakBefore;
    mIsLineBreakAfter = isLineBreakAfter;
    mIsSpecial = isSpecial;
    mType = type;
    mMin = min;
    mMax = max;
    mIsAllocated = true;
}

/**
 * Calculates the size a texture occupies on the page, with its mip levels laid out side by side.
 * @param rTexture texture to measure
 * @return size of the texture in texels
 */
sead::Vector2f DebugTexturePage::Context::calcTextureDrawSize_(const TextureData& rTexture) const
{
    u32 mipLevelNum = rTexture.getMipLevelNum();
    u32 endMip = mMipLevel < 0 ? mipLevelNum : mMipLevel + 1;
    u32 startMip = mMipLevel < 0 ? 0 : mMipLevel;

    sead::Vector2f size;
    switch (rTexture.getTextureType())
    {
    case NVN_TEXTURE_TARGET_2D:
        size.x = getMipWidth(rTexture, startMip);
        size.y = calcMipHeightSum(rTexture, startMip, endMip);
        break;
    case NVN_TEXTURE_TARGET_3D:
    case NVN_TEXTURE_TARGET_2D_ARRAY:
    {
        size.y = getMipHeight(rTexture, startMip);
        f32 sliceNum = mSliceIndex < 0 ? getSlice(rTexture) : 1.0f;
        size.y = sliceNum * size.y;
        size.x = calcMipWidthSum(rTexture, startMip, endMip, 1);
        break;
    }
    case NVN_TEXTURE_TARGET_CUBEMAP:
    {
        size.y = getMipHeight(rTexture, startMip);
        f32 sliceNum = mSliceIndex < 0 ? getSlice(rTexture) : 1.0f;
        size.y = sliceNum * size.y * 0.5f;
        size.x = calcMipWidthSum(rTexture, startMip, endMip, 4);
        break;
    }
    default:
        size = sead::Vector2f::zero;
        break;
    }
    return size;
}

/**
 * Draws a texture with its mip levels, slices or cube map faces laid out next to each other.
 * @param pDrawContext draw context
 * @param rTexture debug texture to draw
 * @param rViewport viewport to draw into
 * @param rPos top left position
 * @param rScale draw scale
 * @param rOption draw options
 * @param isSpecial unused
 */
void DebugTexturePage::Context::drawTexture_(DrawContext* pDrawContext,
                                             const DebugTexture& rTexture,
                                             const sead::Viewport& rViewport,
                                             const sead::Vector2f& rPos,
                                             const sead::Vector2f& rScale,
                                             const DrawOption& rOption, bool isSpecial) const
{
    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    switch (rOption.mBlendType)
    {
    case 1:
        graphicsContext.setBlendEnable(true);
        graphicsContext.setBlendEquation(0, NVN_BLEND_EQUATION_ADD);
        graphicsContext.setBlendFactor(0, NVN_BLEND_FUNC_SRC_ALPHA,
                                       NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
        break;
    case 2:
        graphicsContext.setBlendEnable(true);
        graphicsContext.setBlendEquation(0, NVN_BLEND_EQUATION_ADD);
        graphicsContext.setBlendFactor(0, NVN_BLEND_FUNC_ONE, NVN_BLEND_FUNC_ONE);
        break;
    case 3:
        graphicsContext.setBlendEnable(true);
        graphicsContext.setBlendEquation(0, NVN_BLEND_EQUATION_ADD);
        graphicsContext.setBlendFactor(0, NVN_BLEND_FUNC_ZERO, NVN_BLEND_FUNC_SRC_COLOR);
        break;
    default:
        graphicsContext.setBlendEnable(false);
        break;
    }
    graphicsContext.apply(pDrawContext);

    const TextureData& rTex = *rTexture.getTexture();
    TextureSampler sampler(rTex);
    sampler.applyTextureData(rTex);
    sampler.setStencilMode(rTexture.mType == DebugTexture::cType_2);
    applyDrawOption(&sampler, rTex, rOption, isSpecial);

    u32 mipLevelNum = rTex.getMipLevelNum();
    u32 endMip = mMipLevel < 0 ? mipLevelNum : mMipLevel + 1;
    u32 startMip = mMipLevel & ~(mMipLevel >> 31);

    switch (rTex.getTextureType())
    {
    case NVN_TEXTURE_TARGET_2D:
    {
        sead::Vector2f pos = rPos;
        switch (rTexture.getType())
        {
        case DebugTexture::cType_1:
            ImageFilter2D::drawLinearDepth(pDrawContext, sampler, rViewport, rTexture.getMin(),
                                           rTexture.getMax(), rScale, rPos);
            break;
        case DebugTexture::cType_2:
        {
            f32 scale = 1.0f / rTexture.getIndex();
            ImageFilter2D::drawUint(pDrawContext, sampler, rViewport,
                                    sead::Vector4f(scale, scale, scale, 1.0f), rScale, rPos);
            break;
        }
        default:
            for (u32 mip = startMip; mip < endMip; mip++)
            {
                sead::Vector2f scale = rScale;
                scale.x = f32(getMipWidth(rTex, mip)) / f32(getWidth(rTex)) * scale.x;
                scale.y *= f32(getMipHeight(rTex, mip)) / f32(getHeight(rTex));
                ImageFilter2D::drawTextureMipLevel(pDrawContext, sampler, rViewport, mip, scale,
                                                   pos);
                pos.y += f32(getMipHeight(rTex, mip)) * rScale.y;
            }
            break;
        }
        break;
    }
    case NVN_TEXTURE_TARGET_3D:
    {
        sead::Vector2f pos = rPos;
        if (mSliceIndex >= 0)
        {
            u32 slice;
            if (mSliceIndex >= s32(getSlice(rTex)))
            {
                slice = getSlice(rTex) - 1;
            }
            else
            {
                slice = mSliceIndex;
            }
            for (u32 mip = startMip; mip < endMip; mip++)
            {
                sead::Vector2f scale = rScale;
                scale.x = f32(getMipWidth(rTex, mip)) / f32(getWidth(rTex)) * scale.x;
                scale.y *= f32(getMipHeight(rTex, mip)) / f32(getHeight(rTex));
                ImageFilter2D::drawTexture3D(pDrawContext, sampler, rViewport,
                                             f32(slice) / f32(getMipSlice(rTex, mip)), scale, pos,
                                             mip);
                pos.x += f32(getMipWidth(rTex, mip)) * rScale.x;
            }
        }
        else
        {
            for (u32 mip = startMip; mip < endMip; mip++)
            {
                sead::Vector2f scale = rScale;
                scale.x = f32(getMipWidth(rTex, mip)) / f32(getWidth(rTex)) * scale.x;
                scale.y *= f32(getMipHeight(rTex, mip)) / f32(getHeight(rTex));
                for (u32 slice = 0; slice < getMipSlice(rTex, mip); slice++)
                {
                    ImageFilter2D::drawTexture3D(pDrawContext, sampler, rViewport,
                                                 f32(slice) / f32(getMipSlice(rTex, mip)), scale,
                                                 pos, mip);
                    pos.y += f32(getMipHeight(rTex, mip)) * rScale.y;
                }
                pos.y = rPos.y;
                pos.x += f32(getMipWidth(rTex, mip)) * rScale.x;
            }
        }
        break;
    }
    case NVN_TEXTURE_TARGET_2D_ARRAY:
    {
        sead::Vector2f pos = rPos;
        if (mSliceIndex >= 0)
        {
            s32 slice;
            if (mSliceIndex >= s32(getSlice(rTex)))
            {
                slice = getSlice(rTex) - 1;
            }
            else
            {
                slice = mSliceIndex;
            }
            if (rTexture.getType() == DebugTexture::cType_2)
            {
                ImageFilter2D::drawUintArray(pDrawContext, sampler, rViewport, slice,
                                             sead::Vector4f(255.0f, 255.0f, 255.0f, 255.0f), rScale,
                                             rPos);
            }
            else if (rTexture.getType() == DebugTexture::cType_1)
            {
                ImageFilter2D::drawLinearDepthArray(pDrawContext, sampler, rViewport, slice,
                                                    rTexture.getMin(), rTexture.getMax(), rScale,
                                                    rPos);
            }
            else
            {
                pos.x = rPos.x;
                for (u32 mip = startMip; mip < endMip; mip++)
                {
                    sead::Vector2f scale = rScale;
                    scale.x = f32(getMipWidth(rTex, mip)) / f32(getWidth(rTex)) * scale.x;
                    scale.y *= f32(getMipHeight(rTex, mip)) / f32(getHeight(rTex));
                    ImageFilter2D::drawTexture2DArray(pDrawContext, sampler, rViewport, slice,
                                                      scale, pos, mip);
                    pos.x += f32(getMipWidth(rTex, mip)) * rScale.x;
                }
            }
        }
        else
        {
            for (u32 slice = 0; slice < getSlice(rTex); slice++)
            {
                pos.x = rPos.x;
                switch (rTexture.getType())
                {
                case DebugTexture::cType_2:
                {
                    f32 scale = 1.0f / rTexture.getIndex();
                    ImageFilter2D::drawUintArray(pDrawContext, sampler, rViewport, slice,
                                                 sead::Vector4f(scale, scale, scale, 1.0f), rScale,
                                                 pos);
                    break;
                }
                case DebugTexture::cType_1:
                    ImageFilter2D::drawLinearDepthArray(pDrawContext, sampler, rViewport, slice,
                                                        rTexture.getMin(), rTexture.getMax(),
                                                        rScale, pos);
                    break;
                default:
                    for (u32 mip = startMip; mip < endMip; mip++)
                    {
                        sead::Vector2f scale = rScale;
                        scale.x = f32(getMipWidth(rTex, mip)) / f32(getWidth(rTex)) * scale.x;
                        scale.y *= f32(getMipHeight(rTex, mip)) / f32(getHeight(rTex));
                        ImageFilter2D::drawTexture2DArray(pDrawContext, sampler, rViewport, slice,
                                                          scale, pos, mip);
                        pos.x += f32(getMipWidth(rTex, mip)) * rScale.x;
                    }
                    break;
                }
                pos.y += f32(getMipHeight(rTex, startMip)) * rScale.y;
            }
        }
        break;
    }
    case NVN_TEXTURE_TARGET_CUBEMAP:
    {
        sead::Vector2f pos = rPos;
        if (mSliceIndex >= 0)
        {
            u32 cube;
            if (mSliceIndex >= s32(getSlice(rTex)))
            {
                cube = getSlice(rTex) - 1;
            }
            else
            {
                cube = mSliceIndex;
            }
            for (u32 mip = startMip; mip < endMip; mip++)
            {
                sead::Vector2f scale = rScale;
                scale.x = f32(getMipWidth(rTex, mip)) / f32(getWidth(rTex)) * scale.x;
                scale.y *= f32(getMipHeight(rTex, mip)) / f32(getHeight(rTex));
                ImageFilter2D::drawTextureCubeArray(
                    pDrawContext, sampler, rViewport, cube, cCubeMapFace_PositiveY, scale,
                    pos + sead::Vector2f(f32(getMipWidth(rTex, mip)) * rScale.x, 0.0f), mip);
                ImageFilter2D::drawTextureCubeArray(
                    pDrawContext, sampler, rViewport, cube, cCubeMapFace_NegativeX, scale,
                    pos + sead::Vector2f(f32(getMipWidth(rTex, mip)) * rScale.x * 0.0f,
                                         f32(getMipHeight(rTex, mip)) * rScale.y),
                    mip);
                ImageFilter2D::drawTextureCubeArray(
                    pDrawContext, sampler, rViewport, cube, cCubeMapFace_PositiveZ, scale,
                    pos + sead::Vector2f(f32(getMipWidth(rTex, mip)) * rScale.x,
                                         f32(getMipHeight(rTex, mip)) * rScale.y),
                    mip);
                ImageFilter2D::drawTextureCubeArray(
                    pDrawContext, sampler, rViewport, cube, cCubeMapFace_PositiveX, scale,
                    pos + sead::Vector2f(f32(getMipWidth(rTex, mip)) * rScale.x * 2.0f,
                                         f32(getMipHeight(rTex, mip)) * rScale.y),
                    mip);
                ImageFilter2D::drawTextureCubeArray(
                    pDrawContext, sampler, rViewport, cube, cCubeMapFace_NegativeZ, scale,
                    pos + sead::Vector2f(f32(getMipWidth(rTex, mip)) * rScale.x * 3.0f,
                                         f32(getMipHeight(rTex, mip)) * rScale.y),
                    mip);
                ImageFilter2D::drawTextureCubeArray(
                    pDrawContext, sampler, rViewport, cube, cCubeMapFace_NegativeY, scale,
                    pos + sead::Vector2f(f32(getMipWidth(rTex, mip)) * rScale.x,
                                         f32(getMipHeight(rTex, mip)) * rScale.y * 2.0f),
                    mip);
                pos.x += f32(getMipWidth(rTex, mip)) * rScale.x * 4.0f;
            }
        }
        else
        {
            for (u32 cube = 0; cube < getSlice(rTex) / 6; cube++)
            {
                pos.x = rPos.x;
                for (u32 mip = startMip; mip < endMip; mip++)
                {
                    sead::Vector2f scale = rScale;
                    scale.x = f32(getMipWidth(rTex, mip)) / f32(getWidth(rTex)) * scale.x;
                    scale.y *= f32(getMipHeight(rTex, mip)) / f32(getHeight(rTex));
                    ImageFilter2D::drawTextureCubeArray(
                        pDrawContext, sampler, rViewport, cube, cCubeMapFace_PositiveY, scale,
                        pos + sead::Vector2f(f32(getMipWidth(rTex, mip)) * rScale.x, 0.0f), mip);
                    ImageFilter2D::drawTextureCubeArray(
                        pDrawContext, sampler, rViewport, cube, cCubeMapFace_NegativeX, scale,
                        pos + sead::Vector2f(f32(getMipWidth(rTex, mip)) * rScale.x * 0.0f,
                                             f32(getMipHeight(rTex, mip)) * rScale.y),
                        mip);
                    ImageFilter2D::drawTextureCubeArray(
                        pDrawContext, sampler, rViewport, cube, cCubeMapFace_PositiveZ, scale,
                        pos + sead::Vector2f(f32(getMipWidth(rTex, mip)) * rScale.x,
                                             f32(getMipHeight(rTex, mip)) * rScale.y),
                        mip);
                    ImageFilter2D::drawTextureCubeArray(
                        pDrawContext, sampler, rViewport, cube, cCubeMapFace_PositiveX, scale,
                        pos + sead::Vector2f(f32(getMipWidth(rTex, mip)) * rScale.x * 2.0f,
                                             f32(getMipHeight(rTex, mip)) * rScale.y),
                        mip);
                    ImageFilter2D::drawTextureCubeArray(
                        pDrawContext, sampler, rViewport, cube, cCubeMapFace_NegativeZ, scale,
                        pos + sead::Vector2f(f32(getMipWidth(rTex, mip)) * rScale.x * 3.0f,
                                             f32(getMipHeight(rTex, mip)) * rScale.y),
                        mip);
                    ImageFilter2D::drawTextureCubeArray(
                        pDrawContext, sampler, rViewport, cube, cCubeMapFace_NegativeY, scale,
                        pos + sead::Vector2f(f32(getMipWidth(rTex, mip)) * rScale.x,
                                             f32(getMipHeight(rTex, mip)) * rScale.y * 2.0f),
                        mip);
                    pos.x += f32(getMipWidth(rTex, mip)) * rScale.x * 4.0f;
                }
                pos.y += f32(getMipHeight(rTex, startMip) * 3) * rScale.y;
            }
        }
        break;
    }
    default:
        break;
    }
}

/**
 * Draws the name and format information of a texture with a drop shadow.
 * @param pDrawContext draw context
 * @param rLabel name of the texture
 * @param rTexture texture whose information is drawn
 * @param rViewport viewport to draw in
 * @param rPos top left position of the text
 * @param isDrawName whether to draw the name
 * @param isDrawInfo whether to draw the format information
 */
void DebugTexturePage::Context::drawLabel_(DrawContext* pDrawContext,
                                           const sead::SafeString& rLabel,
                                           const TextureData& rTexture,
                                           const sead::Viewport& rViewport,
                                           const sead::Vector2f& rPos, bool isDrawName,
                                           bool isDrawInfo) const
{
    if (!isDrawName && !isDrawInfo)
    {
        return;
    }

    sead::GraphicsContext graphicsContext;
    graphicsContext.setDepthEnable(false, false);
    graphicsContext.setBlendEnable(true);
    graphicsContext.apply(pDrawContext);
    pDrawContext->changeShaderMode(cShaderMode_UniformRegister, ShaderOptimizeType(0));

    sead::TextWriter writer(pDrawContext);
    writer.setViewport(&rViewport);
    writer.beginDraw();

    writer.setColor(sead::Color4f::cBlack);
    writer.setCursorFromTopLeft(rPos + sead::Vector2f::ones);
    if (isDrawName)
    {
        writer.printf("%s ", rLabel.cstr());
    }
    if (isDrawInfo)
    {
        writer.printf("%s (%d %d %d) <%d byte>", rTexture.getTextureFormatName().cstr(),
                      getWidth(rTexture), getHeight(rTexture), getSlice(rTexture),
                      rTexture.getImageByteSize() + rTexture.getSurface()._14);
    }

    writer.setColor(sead::Color4f::cWhite);
    writer.setCursorFromTopLeft(rPos);
    if (isDrawName)
    {
        writer.printf("%s ", rLabel.cstr());
    }
    if (isDrawInfo)
    {
        writer.printf("%s (%d %d %d) <%d byte>", rTexture.getTextureFormatName().cstr(),
                      getWidth(rTexture), getHeight(rTexture), getSlice(rTexture),
                      rTexture.getImageByteSize() + rTexture.getSurface()._14);
    }

    writer.endDraw();
}

/**
 * Draws the textures of the context in rows, or only the selected texture scaled to the screen.
 * @param pDrawContext draw context
 * @param rFrameBuffer frame buffer to draw to
 * @param rViewport viewport to draw in
 * @param rOption draw options of the page
 */
void DebugTexturePage::Context::draw(DrawContext* pDrawContext,
                                     const sead::LogicalFrameBuffer& rFrameBuffer,
                                     const sead::Viewport& rViewport,
                                     const DrawOption& rOption) const
{
    sead::Vector2f pos(mScroll.x, -mScroll.y);
    sead::Vector2f frameBufferSize;
    rViewport.getOnFrameBufferSize(&frameBufferSize, rFrameBuffer);
    sead::Vector2f scale;

    f32 rowHeight;
    if (mSelectIndex >= 0 && mSelectIndex < mLabelNum)
    {
        const DebugTexture* pTexture = searchFullScreenTexture();
        if (!pTexture)
        {
            return;
        }

        sead::Vector2f size = calcTextureDrawSize_(*pTexture->getTexture());
        const sead::BoundBox2f& rArea = rFrameBuffer.getPhysicalArea();
        f32 fit = sead::Mathf::min(rArea.getSizeX() / size.x, rArea.getSizeY() / size.y);
        f32 ratio = mScale * fit;
        scale.set(rFrameBuffer.getVirtualSize().x / rArea.getSizeX() * ratio,
                  rFrameBuffer.getVirtualSize().y / rArea.getSizeY() * ratio);
        drawTexture_(pDrawContext, *pTexture, rViewport, pos, scale, rOption,
                     pTexture->mIsSpecial);
        drawLabel_(pDrawContext, pTexture->getName(), *pTexture->getTexture(), rViewport, pos,
                   rOption.mIsDrawLabel, rOption.mIsDrawLabelBg);
        rowHeight = 0.0f;
    }
    else
    {
        const sead::BoundBox2f& rArea = rFrameBuffer.getPhysicalArea();
        scale.set(mScale * (rFrameBuffer.getVirtualSize().x / rArea.getSizeX()),
                  mScale * (rFrameBuffer.getVirtualSize().y / rArea.getSizeY()));

        rowHeight = 0.0f;
        for (auto it = mTextures.begin(); it != mTextures.end(); ++it)
        {
            const DebugTexture* pTexture = *it;
            if (pTexture->mIsLineBreakBefore)
            {
                pos.set(mScroll.x, pos.y + rowHeight);
                rowHeight = 0.0f;
            }

            sead::Vector2f size = calcTextureDrawSize_(*pTexture->getTexture());
            if (pos.x != mScroll.x && pos.x + size.x * scale.x > frameBufferSize.x)
            {
                pos.set(mScroll.x, pos.y + rowHeight);
                rowHeight = 0.0f;
            }
            else
            {
                rowHeight = sead::Mathf::max(rowHeight, size.y * scale.y);
            }

            drawTexture_(pDrawContext, **it, rViewport, pos, scale, rOption, (*it)->mIsSpecial);
            drawLabel_(pDrawContext, (*it)->getName(), *(*it)->getTexture(), rViewport, pos,
                       rOption.mIsDrawLabel, rOption.mIsDrawLabelBg);

            if ((*it)->mIsLineBreakAfter)
            {
                pos.set(mScroll.x, pos.y + rowHeight);
                rowHeight = 0.0f;
            }
            else
            {
                pos.x += size.x * scale.x;
                rowHeight = sead::Mathf::max(rowHeight, size.y * scale.y);
            }
        }
    }

    mContentHeight = sead::Mathf::clampMin(
        pos.y + rowHeight - rFrameBuffer.getPhysicalArea().getSizeY(), 100.0f);
}

}  // namespace agl::utl
