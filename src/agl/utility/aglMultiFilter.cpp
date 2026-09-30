#include "utility/aglMultiFilter.h"

#include <gfx/seadFrameBuffer.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadViewport.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <prim/seadSafeString.h>

#include "common/aglDrawContext.h"
#include "common/aglTextureData.h"
#include "detail/aglRootNode.h"
#include "utility/aglDebugTexturePage.h"
#include "utility/aglImageFilter2D.h"

namespace agl::utl {

namespace {

s32 compareSaveIndex(MultiFilterUnit* const* pA, MultiFilterUnit* const* pB)
{
    const auto* pNodeA = reinterpret_cast<const sead::TListNode<MultiFilterUnit*>*>(pA);
    const auto* pNodeB = reinterpret_cast<const sead::TListNode<MultiFilterUnit*>*>(pB);
    return pNodeA->mData->getSaveIndex() > pNodeB->mData->getSaveIndex() ? 1 : -1;
}

uintptr_t getEventId(const sead::hostio::PropertyEvent* pEvent)
{
    return reinterpret_cast<uintptr_t>(pEvent->getId());
}

}  // namespace

/**
 * Constructs the multi filter with no active units.
 */
MultiFilter::MultiFilter() : IParameterIO("aglmf", 0)
{
    detail::RootNode::setNodeMeta(this, "Icon=LIGHT");
    mDrawContext.mResultTexture = nullptr;
    mDrawContext.mFormat = TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
    mDrawContext.mRenderBuffer.setRenderTargetColor(&mDrawContext.mRenderTarget);
    mDrawContext.mAlphaCompSel = cTextureCompSel_1;
    mDrawContext.mIsChangeCompSel = false;
    mDrawContext.mSrcTexture = nullptr;
    mResultInfo.mFormat = TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
    mResultInfo.mWidth = 0;
    mResultInfo.mHeight = 0;
    mResultInfo.mMaxWidth = 0;
    mResultInfo.mMaxHeight = 0;
}

/**
 * Destroys every filter unit, the result texture and the debug page.
 */
MultiFilter::~MultiFilter()
{
    for (s32 type = 0; type < MultiFilterUnit::cFilterType_Num; type++)
    {
        for (s32 i = 0; i < cUnitNum; i++)
        {
            mUnits[type][i]->destroy();
            mUnits[type][i] = nullptr;
        }
    }

    freeResultTexture();

    if (mDebugTexturePage)
    {
        mDebugTexturePage->cleanUp();
        delete mDebugTexturePage;
        mDebugTexturePage = nullptr;
    }
}

/**
 * Frees the result texture and ends the texture cache frame.
 */
void MultiFilter::freeResultTexture() const
{
    if (mDrawContext.mResultTexture)
    {
        mDrawContext.mTextureCache.free(mDrawContext.mResultTexture);
        mDrawContext.mResultTexture = nullptr;
    }

    mDrawContext.mTextureCache.end();
}

/**
 * Creates the filter units of every type and the texture cache.
 * @param pHeap heap to allocate the units and the cache from
 * @param pDebugHeap unused
 */
void MultiFilter::initialize(sead::Heap* pHeap, sead::Heap* pDebugHeap)
{
    for (s32 type = 0; type < MultiFilterUnit::cFilterType_Num; type++)
    {
        const auto filterType = static_cast<MultiFilterUnit::FilterType>(type);
        addList(&mTypeParamLists[type], MultiFilterUnit::getFilterName(filterType).cstr());

        for (s32 i = 0; i < cUnitNum; i++)
        {
            MultiFilterUnit* pUnit = MultiFilterUnit::create(filterType, i, pHeap);
            pUnit->initialize(this, &mFreeUnits[type], pHeap);
            mTypeParamLists[type].addList(
                pUnit, sead::FormatFixedSafeString<32>(
                           "%s_%d", MultiFilterUnit::getFilterName(filterType).cstr(), i));
            mUnits[type][i] = pUnit;
        }
    }

    mDrawContext.mTextureCache.initialize(128, pHeap);
    addObj(&mParamObj, "mf_root_param");
}

/**
 * Sets whether the alpha channel of the source texture is used.
 * @param useAlpha whether the alpha channel is read from the texture
 */
void MultiFilter::setUseTextureAlpha(bool useAlpha)
{
    mDrawContext.mAlphaCompSel = useAlpha ? cTextureCompSel_A : cTextureCompSel_1;
}

/**
 * Calculates the size and format of the filtered texture.
 * @param width width of the source texture
 * @param height height of the source texture
 * @return the calculated result information
 */
const MultiFilterResultInfo& MultiFilter::calcResultInfo(s32 width, s32 height) const
{
    MultiFilterResultInfo* pInfo = &mResultInfo;

    if (isDrawable_())
    {
        pInfo->mFormat = TextureFormat::cTextureFormat_R8_G8_B8_A8_uNorm;
        pInfo->mWidth = width;
        pInfo->mHeight = height;
        pInfo->mMaxWidth = width;
        pInfo->mMaxHeight = height;
        const MultiFilterUnit& rTrimming = mTrimming;

        if (rTrimming.isEnable())
        {
            rTrimming.doCalcResultInfo_(pInfo);
        }

        for (const MultiFilterUnit* pUnit : mActiveUnits)
        {
            if (pUnit->isEnable())
            {
                pUnit->doCalcResultInfo_(pInfo);
            }
        }
    }

    return *pInfo;
}

/**
 * Applies every active filter to a texture.
 * @param pDrawContext draw context
 * @param rTexture source texture
 */
void MultiFilter::draw(DrawContext* pDrawContext, const TextureData& rTexture) const
{
    if (!isDrawable_())
    {
        return;
    }

    if (!mDrawContext.mTextureCache.begin())
    {
        return;
    }

    sead::GraphicsContext context;
    context.setDepthTestEnable(false);
    context.setBlendEnable(false);
    context.apply(pDrawContext);

    mDrawContext.mSrcTexture = &rTexture;
    mDrawContext.mFormat = static_cast<TextureFormat>(rTexture.getTextureFormat());
    mDrawContext.mIsChangeCompSel = false;
    mDrawContext.mSampler.setCompSel(cTextureCompSel_R, cTextureCompSel_G, cTextureCompSel_B,
                                     static_cast<TextureCompSel>(mDrawContext.mAlphaCompSel));
    mDrawContext.mSampler.setFilter(1, 1, 1);
    mDrawContext.mSampler.setWrap(7, 7, 7);

    s32 index = 0;

    if (mTrimming.isEnable())
    {
        drawFilter_(pDrawContext, mTrimming, index);
        index++;
    }

    for (auto it = mActiveUnits.begin(); it != mActiveUnits.end(); ++it)
    {
        drawFilter_(pDrawContext, **it, index++);
    }

    if (mDrawContext.mResultTexture)
    {
        bool isLinear = *mResultSamplerLinear;
        mDrawContext.mSampler.applyTextureData(*mDrawContext.mResultTexture);
        mDrawContext.mSampler.setFilter(isLinear, isLinear, 1);
    }
}

/**
 * Applies one filter unit when it is enabled.
 * @param pDrawContext draw context
 * @param rUnit unit to apply
 * @param index position of the unit in the filter chain
 */
void MultiFilter::drawFilter_(DrawContext* pDrawContext, const MultiFilterUnit& rUnit,
                              s32 index) const
{
    if (rUnit.isEnable())
    {
        rUnit.doDraw_(pDrawContext, &mDrawContext);
    }

    if (mDrawContext.mResultTexture)
    {
        sead::FormatFixedSafeString<32> name(
            "%d_%s", index + 1, MultiFilterUnit::getFilterName(rUnit.getType()).cstr());
    }
}

/**
 * Trims a texture and applies every active filter to it.
 * @param pDrawContext draw context
 * @param rTexture source texture
 * @param rTrimScale scale of the trimmed area
 * @param rTrimCenter center of the trimmed area
 */
void MultiFilter::draw(DrawContext* pDrawContext, const TextureData& rTexture,
                       const sead::Vector2f& rTrimScale, const sead::Vector2f& rTrimCenter) const
{
    *mTrimming.mEnable = true;
    *mTrimming.mScale = rTrimScale;
    *mTrimming.mCenter = rTrimCenter;
    draw(pDrawContext, rTexture);
}

/**
 * Draws the result texture over the whole frame buffer when debug drawing is enabled.
 * @param pDrawContext draw context
 * @param rFrameBuffer frame buffer to draw to
 * @param rViewport viewport to draw with
 */
void MultiFilter::drawDebug(DrawContext* pDrawContext, const sead::LogicalFrameBuffer& rFrameBuffer,
                            const sead::Viewport& rViewport) const
{
    if (!isDrawable_() || !mIsDrawDebug || !mDrawContext.mResultTexture)
    {
        return;
    }

    sead::GraphicsContext context;
    context.setDepthTestEnable(false);
    context.setBlendEnable(false);
    context.apply(pDrawContext);
    rViewport.apply(pDrawContext, rFrameBuffer);

    TextureSampler sampler(*mDrawContext.mResultTexture);
    const sead::Vector2f& rVirtualSize = rFrameBuffer.getVirtualSize();
    const sead::BoundBox2f& rPhysicalArea = rFrameBuffer.getPhysicalArea();
    sead::Vector2f scale(rVirtualSize.x / rPhysicalArea.getSizeX(),
                         rVirtualSize.y / rPhysicalArea.getSizeY());
    ImageFilter2D::drawTexture(pDrawContext, sampler, rViewport, scale, sead::Vector2f::zero);
}

/**
 * Generates the host IO messages of the filter chain.
 * @param pContext host IO context
 */
void MultiFilter::genMessage(sead::hostio::Context* pContext)
{
    genMessageIO(pContext, 0xf);

    if (mDebugTexturePage)
    {
        mDebugTexturePage->genMessagePage(pContext, this);
    }

    if (mActiveUnits.size() > 0)
    {
        {
            sead::FormatFixedSafeString<64> layout("Layout=Grid, NumRow=%d, NumCol=6",
                                                   mActiveUnits.size());
        }

        s32 index = 1;

        for (auto it = mActiveUnits.begin(); it != mActiveUnits.end(); ++it)
        {
            {
                sead::FormatFixedSafeString<8> label("%d : ", index);

                for (s32 type = 0; type < MultiFilterUnit::cFilterType_Num; type++)
                {
                    sead::SafeString item = MultiFilterUnit::getFilterLabel(
                        static_cast<MultiFilterUnit::FilterType>(type));
                }
            }

            {
                sead::SafeString filterLabel = MultiFilterUnit::getFilterLabel((*it)->getType());
                sead::FormatFixedSafeString<128> meta("Comment= Deletes %s", filterLabel.cstr());
            }

            if (auto* pNext = mActiveUnits.next(*it))
            {
                const s32 swapIndex = index + 1;
                sead::SafeString filterLabel =
                    MultiFilterUnit::getFilterLabel(pNext->mData->getType());
                sead::FormatFixedSafeString<128> meta("Comment=Filter%d : %sと順序を入れ替えます",
                                                      swapIndex, filterLabel.cstr());
            }

            if (auto* pPrev = mActiveUnits.prev(*it))
            {
                const s32 swapIndex = index - 1;
                sead::SafeString filterLabel =
                    MultiFilterUnit::getFilterLabel(pPrev->mData->getType());
                sead::FormatFixedSafeString<128> meta("Comment=Filter%d : %sと順序を入れ替えます",
                                                      swapIndex, filterLabel.cstr());
            }

            index++;
        }

        mResultSamplerLinear.genMessageParameter(pContext, mResultSamplerLinear.getMeta());
    }

    for (s32 type = 0; type < MultiFilterUnit::cFilterType_Num; type++)
    {
        sead::SafeString item =
            MultiFilterUnit::getFilterLabel(static_cast<MultiFilterUnit::FilterType>(type));
    }

    s32 index = 1;

    for (auto it = mActiveUnits.begin(); it != mActiveUnits.end(); ++it)
    {
        sead::SafeString filterLabel = MultiFilterUnit::getFilterLabel((*it)->getType());
        sead::FormatFixedSafeString<128> label("%d : %s", index, filterLabel.cstr());
        label.cstr();
        index++;
    }
}

/**
 * Handles the host IO events that add, remove, reorder or replace filter units.
 * @param pEvent property event
 */
void MultiFilter::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    listenPropertyEventIO(this, pEvent);

    const uintptr_t addType = getEventId(pEvent) - 30000;

    if (addType < MultiFilterUnit::cFilterType_Num)
    {
        auto* pNode = mFreeUnits.mBuffer[addType].popFront();

        if (pNode)
        {
            pNode->mList = nullptr;
            pNode->mData->activate();
        }

        return;
    }

    if (getEventId(pEvent) == 90000)
    {
        inactivateAll();
        return;
    }

    for (auto it = mActiveUnits.begin(); it != mActiveUnits.end(); ++it)
    {
        const u32 base = (*it)->getId() << 16;

        if (getEventId(pEvent) == static_cast<u32>(base + 50000))
        {
            (*it)->inactivate();
            return;
        }

        if (getEventId(pEvent) == static_cast<u32>(base + 40000))
        {
            sead::TListNode<MultiFilterUnit*>* pNode = nullptr;

            for (s32 type = 0; type < MultiFilterUnit::cFilterType_Num; type++)
            {
                pNode = mFreeUnits[type].popFront();

                if (pNode)
                {
                    break;
                }
            }

            if (pNode)
            {
                pNode->mList = nullptr;
                pNode->mData->activate();
                mActiveUnits.moveBefore(*it, pNode);
                return;
            }
        }
        else if (getEventId(pEvent) == static_cast<u32>(base + 60000))
        {
            if (auto* pPrev = mActiveUnits.prev(*it))
            {
                mActiveUnits.moveBefore(pPrev, *it);
                mSelectId = base;
            }

            return;
        }
        else if (getEventId(pEvent) == static_cast<u32>(base + 70000))
        {
            if (auto* pNext = mActiveUnits.next(*it))
            {
                mActiveUnits.moveAfter(pNext, *it);
                mSelectId = base;
            }

            return;
        }
        else if (getEventId(pEvent) == static_cast<u32>(base + 80000))
        {
            const s32 type = **reinterpret_cast<const s32* const*>(
                reinterpret_cast<uintptr_t>(pEvent) + 0x30);
            auto* pNode = mFreeUnits[type].popFront();

            if (!pNode)
            {
                return;
            }

            pNode->mList = nullptr;
            pNode->mData->activate();
            mActiveUnits.moveBefore(*it, pNode);
            (*it)->inactivate();
            return;
        }
    }

    for (auto it = mActiveUnits.begin(); it != mActiveUnits.end(); ++it)
    {
        (*it)->listenPropertyEvent(pEvent);
    }
}

/**
 * Activates a free unit of a filter type.
 * @param type filter type of the unit
 * @return the list node of the activated unit, or nullptr if none is free
 */
sead::TListNode<MultiFilterUnit*>* MultiFilter::addFilter_(MultiFilterUnit::FilterType type)
{
    auto* pNode = mFreeUnits[type].popFront();

    if (pNode)
    {
        pNode->mList = nullptr;
        pNode->mData->activate();
    }

    return pNode;
}

/**
 * Deactivates every active unit.
 */
void MultiFilter::inactivateAll()
{
    for (auto& rNode : mActiveUnits.robustRange())
    {
        rNode.mData->inactivate();
    }
}

/**
 * Numbers the active units in chain order before saving.
 * @return always true
 */
bool MultiFilter::preWrite_() const
{
    s32 index = 0;

    for (auto it = mActiveUnits.begin(); it != mActiveUnits.end(); ++it)
    {
        (*it)->setSaveIndex(index);
        index++;
    }

    return true;
}

/**
 * Rebuilds the filter chain from the loaded active flags and save indices.
 */
void MultiFilter::postRead_()
{
    for (auto& rNode : mActiveUnits.robustRange())
    {
        if (!rNode.mData->isActive())
        {
            rNode.mData->inactivate();
        }
    }

    for (s32 type = 0; type < MultiFilterUnit::cFilterType_Num; type++)
    {
        for (auto& rNode : mFreeUnits[type].robustRange())
        {
            if (rNode.mData->isActive())
            {
                rNode.mData->activate();
            }
        }
    }

    mActiveUnits.sort(0, compareSaveIndex);
}

}  // namespace agl::utl
