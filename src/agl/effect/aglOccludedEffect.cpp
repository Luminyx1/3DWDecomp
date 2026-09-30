#include "effect/aglOccludedEffect.h"

#include <gfx/seadCamera.h>
#include <gfx/seadGraphicsContext.h>
#include <gfx/seadProjection.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadSafeString.h>
#include <xml/seadXmlElement.h>
#include <xml/seadXmlUtil.h>

#include "common/aglRenderBuffer.h"
#include "common/aglShaderProgram.h"
#include "detail/aglRootNode.h"
#include "detail/aglShaderHolder.h"
#include "effect/aglOccludedEffectMgr.h"
#include "environment/aglEnvObjBuffer.h"
#include "environment/aglEnvObjMgr.h"
#include "postfx/aglPostFxUtil.h"
#include "utility/aglDevTools.h"

namespace agl::fx
{

namespace
{

inline void genMessageDummy(sead::hostio::Context* pContext, const sead::SafeString& rLabel) {}

inline void genMessageDummy(sead::hostio::Context* pContext, const sead::SafeString& rLabel,
                            const sead::SafeString& rMeta)
{
}

inline bool isQuarterTexture(const OccludedEffectMgr* pMgr, s32 index)
{
    return index >= 0 && *pMgr->mTextureInfo.unsafeAt(index)->mPlacement.mIsQuarter;
}

inline void setUniformFloat(const UniformBlock& rBlock, s32 index, f32 value)
{
    rBlock.setData(index, &value, 0, 1);
}

inline void mulMtx22(sead::Vector2f* pOut, const sead::Matrix22f& rMtx, const sead::Vector2f& rVec)
{
    f32 x = rMtx.m[0][0] * rVec.x + rMtx.m[0][1] * rVec.y;
    f32 y = rMtx.m[1][0] * rVec.x + rMtx.m[1][1] * rVec.y;
    pOut->set(x, y);
}

inline void rotateVec(sead::Vector2f* pVec, f32 angle)
{
    f32 s = std::sin(angle);
    f32 c = std::cos(angle);
    pVec->set(c * pVec->x - s * pVec->y, s * pVec->x + c * pVec->y);
}

inline f32 calcEdgeRate(f32 pos, f32 edge)
{
    if (pos == 1.0f && edge == 1.0f)
    {
        return 0.0f;
    }

    if (edge < 1.0f)
    {
        return sead::Mathf::clamp((pos - edge) / (1.0f - edge), 0.0f, 1.0f);
    }

    return sead::Mathf::clamp((pos - 1.0f) / (edge - 1.0f), 0.0f, 1.0f);
}

}  // namespace

/**
 * Constructs the preset and registers its common parameters.
 */
OfxBase::PresetBase::PresetBase()
    : mPosition(sead::Vector3f::zero, "Position", "World位置", this),
      mRadius(40.0f, "Radius", "サイズ", this), mCoreRadius(5.0f, "CoreRadius", "コアサイズ", this),
      mVerticesBias(1.0f, "VerticesBias", "頂点の偏り", this),
      mDepthOffset(0.0f, "DepthOffset", "深度オフセット", this),
      mScrEdgeSize(sead::Vector2f(1.0f, 1.0f), "ScrEdgeSize", "フレームアウト位置", this),
      mDirection(sead::Vector3f(0.0f, 0.0f, 1.0f), "Direction", "向き", this),
      mPseudoOccl(false, "PseudoOccl", "画面外疑似遮蔽判定", this),
      mIsFixPosX(false, "IsFixPosX", "X軸位置のカメラ相対にする", this),
      mIsFixPosY(false, "IsFixPosY", "X軸位置のカメラ相対にする", this),
      mIsFixPosZ(false, "IsFixPosZ", "X軸位置のカメラ相対にする", this)
{
}

/**
 * Binds the preset to its manager, names it and runs the type-specific initialization.
 * @param rArg creation arguments
 * @param pMgr owning manager
 * @param index preset index used for the default name
 * @param unused unused
 * @param pHeap heap used for allocations
 */
void OfxBase::PresetBase::initializeOfx(const CreateArg& rArg, OccludedEffectMgr* pMgr, s32 index,
                                        s32 unused, sead::Heap* pHeap)
{
    mMgr = pMgr;
    mPresetName.initializeParameter(sead::FormatFixedSafeString<16>("default %d", index),
                                    "PresetName", "プリセット名", this);
    initializeOfxImpl_(rArg, pHeap);
}

/**
 * Looks up the manager type index of this preset class.
 * @return type index, or -1 if the class is not registered
 */
s32 OfxBase::PresetBase::getOfxTypeID() const
{
    return mMgr->getCreateArg().searchPresetType(getTypeID());
}

/**
 * Handles host IO edits of the preset and propagates them to the instances using it.
 * @param pEvent property event
 */
void OfxBase::PresetBase::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    if ((pEvent->getType() & 2) == 0)
    {
        const void* id = pEvent->getId();
        if (id < &mIsFixPosX && id >= &mPresetName)
        {
        }
        else if (id < &mIsDebugDraw + 1 && id >= &mIsDebugDraw)
        {
            mMgr->setInstanceDebugDrawAll(getOfxTypeID(), *mPresetName, mIsDebugDraw);
        }
        else if (id < &mDebugColorType + 1 && id >= &mDebugColorType)
        {
            switch (mDebugColorType)
            {
            case 0:
                mMgr->setInstanceDebugDrawColorAll(getOfxTypeID(), *mPresetName,
                                                   sead::Color4f::cWhite, sead::Color4f::cBlue,
                                                   sead::Color4f::cRed);
                break;
            case 1:
                mMgr->setInstanceDebugDrawColorAll(getOfxTypeID(), *mPresetName,
                                                   sead::Color4f::cBlack, sead::Color4f::cYellow,
                                                   sead::Color4f::cGreen);
                break;
            case 2:
                mMgr->setInstanceDebugDrawColorAll(getOfxTypeID(), *mPresetName,
                                                   sead::Color4f::cBlue, sead::Color4f::cRed,
                                                   sead::Color4f::cWhite);
                break;
            case 3:
                mMgr->setInstanceDebugDrawColorAll(getOfxTypeID(), *mPresetName,
                                                   sead::Color4f::cRed, sead::Color4f::cBlack,
                                                   sead::Color4f::cYellow);
                break;
            }
        }
    }

    if (pEvent->getId() == reinterpret_cast<const void*>(1000))
    {
        mMgr->setPresetDirty();
    }
    else if (pEvent->getId() == reinterpret_cast<const void*>(1001))
    {
        if (mCopySrcIndex >= 0 && mCopySrcIndex < mMgr->getCreateArg().mTotalPresetNum)
        {
            sead::FixedSafeString<32> name(*mPresetName);
            copy(*mMgr->getCreateArg().getPreset(mCopySrcIndex));
            mPresetName->copy(name);
            mMgr->updateInstancePresetAll(getOfxTypeID(), *mPresetName);
        }
    }

    if (mIsSyncInstance)
    {
        for (utl::ParameterBase* p = getParamListHead(); p != nullptr; p = p->getNext())
        {
            if (pEvent->getId() == p->ptr())
            {
                mMgr->setInstanceParameterAll(getOfxTypeID(), *mPresetName, *p);
                break;
            }
        }
    }
}

/**
 * Copies the parameters of another preset, keeping the fixed position axes of this one.
 * @param rOther object to copy from
 */
void OfxBase::PresetBase::copy(const PresetBase& rOther)
{
    const utl::ParameterBase* pSrc = rOther.getParamListHead();
    utl::ParameterBase* pDst = getParamListHead();
    mPresetName->copy(*rOther.mPresetName);
    sead::BaseVec3<f32> pos = *mPosition;
    if (getTypeID() == rOther.getTypeID())
    {
        for (; pSrc != nullptr; pSrc = pSrc->getNext(), pDst = pDst->getNext())
        {
            pDst->copyUnsafe(*pSrc);
        }
    }
    else
    {
        for (; pSrc != nullptr; pSrc = pSrc->getNext())
        {
            for (utl::ParameterBase* p = pDst; p != nullptr; p = p->getNext())
            {
                if (p->copy(*pSrc))
                {
                    break;
                }
            }
        }
    }

    static_cast<sead::BaseVec3<f32>&>(*mPosition) = pos;
}

/**
 * Generates the host IO message of the preset.
 * @param pContext host IO context
 */
void OfxBase::PresetBase::genMessage(sead::hostio::Context* pContext)
{
    if (mMgr == nullptr)
    {
        return;
    }

    s32 num = mMgr->getCreateArg().mTotalPresetNum;
    for (s32 i = 0; i < num; i++)
    {
        PresetBase* pPreset = mMgr->getCreateArg().getPreset(i);
        if (pPreset == this || pPreset == nullptr)
        {
            continue;
        }

        sead::FormatFixedSafeString<256> label("%s / %s",
                                               env::EnvObj::getTypeData(pPreset->getTypeID()).mName,
                                               pPreset->mPresetName->cstr());
    }

    mPosition.genMessageParameter(pContext, mPosition.getMeta());
    mCoreRadius.genMessageParameter(pContext, utl::DevTools::getStringMinMax(0.0f, 10.0f));
    mRadius.genMessageParameter(pContext, utl::DevTools::getStringMinMax(0.0f, 10.0f));
    mVerticesBias.genMessageParameter(pContext, utl::DevTools::getStringMinMax(0.0f, 5.0f));
    mDepthOffset.genMessageParameter(pContext, mDepthOffset.getMeta());
    mScrEdgeSize.genMessageParameter(pContext, "Min = 0, Max = 2");
}

/**
 * Writes the preset parameters as a child element.
 * @param index unused
 * @param pElement XML element to write into
 * @param pHeap heap used for allocations
 */
void OfxBase::PresetBase::writeToXML(s32 index, sead::XmlElement* pElement, sead::Heap* pHeap) const
{
    sead::XmlElement* pChild =
        sead::XmlUtil::createBackChildAndSetupElement(pElement, "preset", "", pHeap);
    pChild->expandAttributeList(2, pHeap);
    pChild->addAttribute("idx", sead::FormatFixedSafeString<16>("%d", index), pHeap);
    pChild->addAttribute("name", *mPresetName, pHeap);
    utl::IParameterObj::writeToXML(pChild, pHeap);
}

OfxBase::OfxBase() = default;

/**
 * Frees the per-view contexts.
 */
OfxBase::~OfxBase()
{
    mContext.freeBuffer();
}

/**
 * Does nothing; effects are initialized through initializeOfx.
 * @param viewNum number of views
 * @param pHeap heap used for allocations
 */
void OfxBase::initialize(s32 viewNum, sead::Heap* pHeap) {}

/**
 * Sets up names, contexts, the owned preset and the occlusion renderer of the effect.
 * @param rArg creation arguments
 * @param ofxType effect type index
 * @param pPreset initial preset, may be null
 * @param pHeap heap used for allocations
 * @param pMgr owning manager
 */
void OfxBase::initializeOfx(const CreateArg& rArg, s32 ofxType, const PresetBase* pPreset,
                            sead::Heap* pHeap, OccludedEffectMgr* pMgr)
{
    mOfxName.format("%s%d", getTypeData(getTypeID()).mName, mIndex);
    mOfxLabel.format("%s %d", getEnvObjName().cstr(), mIndex);
    mMgr = pMgr;
    detail::RootNode::setNodeMeta(this, "Icon=EFFECT");
    mPreset = pPreset;
    if (pPreset != nullptr)
    {
        mPresetName = *pPreset->mPresetName;
    }
    else
    {
        mPresetName = sead::SafeString::cEmptyString;
    }

    mContext.tryAllocBuffer(rArg.mViewNum, pHeap, 0x20);

    PresetBase::CreateArg arg;
    arg.mViewNum = rArg.mViewNum;
    mOwnPreset = sead::DynamicCast<PresetBase>(
        getTypeData(mMgr->getCreateArg().mPresetTypeId[ofxType]).mCreateFunc(pHeap));
    initializePreset(mOwnPreset, arg, 0, pHeap);
    mOcclusionRenderer.initialize(rArg.mViewNum, pHeap);
    mDebugColor0 = sead::Color4f::cWhite;
    mDebugColor1 = sead::Color4f::cBlue;
    mDebugColor2 = sead::Color4f::cRed;
    initializeImpl_(rArg, pHeap);
}

/**
 * Initializes a preset if its class matches the preset type of this effect.
 * @param pPreset preset to initialize
 * @param rArg creation arguments
 * @param index preset index
 * @param pHeap heap used for allocations
 */
void OfxBase::initializePreset(PresetBase* pPreset, const PresetBase::CreateArg& rArg, s32 index,
                               sead::Heap* pHeap)
{
    s32 type = getOfxTypeID();
    if (pPreset->getTypeID() == mMgr->getCreateArg().mPresetTypeId[type])
    {
        pPreset->initializeOfx(rArg, mMgr, index, 0, pHeap);
    }
}

/**
 * Looks up the manager type index of this effect class.
 * @return type index, or -1 if the class is not registered
 */
s32 OfxBase::getOfxTypeID() const
{
    return mMgr->getCreateArg().searchOfxType(getTypeID());
}

/**
 * Reloads pending presets and updates the occlusion renderer from the owned preset.
 */
void OfxBase::calc()
{
    if (mFlag.isOn(0x20))
    {
        mFlag.reset(0x20);
        loadPresetByName(mPresetName, false);
    }

    if (mFlag.isOn(0x40))
    {
        mFlag.reset(0x40);
        loadPresetByIndex(mPresetIndex, false);
    }

    if (!isEnable())
    {
        return;
    }

    mOcclusionRenderer.mOffset = *getPreset_<PresetBase>()->mPosition;
    mOcclusionRenderer.mSize = *getPreset_<PresetBase>()->mRadius;
    mOcclusionRenderer.mSampleSize = *getPreset_<PresetBase>()->mCoreRadius;
    mOcclusionRenderer.mPower = *getPreset_<PresetBase>()->mVerticesBias;
    mOcclusionRenderer.mThreshold = *getPreset_<PresetBase>()->mDepthOffset;
    mOcclusionRenderer.mAutoDirection = *getPreset_<PresetBase>()->mPseudoOccl;
    mOcclusionRenderer.calc();
    updateImpl_();
}

/**
 * Selects the preset with the given name and copies it into the owned preset.
 * @param rName preset name
 * @param force unused
 * @return whether the preset was found
 */
bool OfxBase::loadPresetByName(const sead::SafeString& rName, bool force)
{
    if (&mPresetName != &rName)
    {
        mPresetName.copy(rName);
    }

    const PresetBase* pPreset = mMgr->searchPresetByName(getOfxTypeID(), rName);
    mPreset = pPreset;
    if (pPreset == nullptr)
    {
        mPresetIndex = -1;
        return false;
    }

    OccludedEffectMgr* pMgr = mMgr;
    s32 type = getOfxTypeID();
    mPresetIndex = pMgr->getCreateArg().searchPresetTypeIndex(pPreset, type);
    mOwnPreset->copy(*mPreset);
    return true;
}

/**
 * Selects the preset at the given index and copies it into the owned preset.
 * @param index preset index within the type
 * @param force unused
 * @return whether the preset was found
 */
bool OfxBase::loadPresetByIndex(s32 index, bool force)
{
    mPresetIndex = index;
    mPreset = mMgr->getPresetByIndex(getOfxTypeID(), index);
    if (mPreset == nullptr)
    {
        mPresetName = sead::SafeString::cEmptyString;
        return false;
    }

    mPresetName = *mPreset->mPresetName;
    mOwnPreset->copy(*mPreset);
    return true;
}

void OfxBase::calcContext(s32 viewIndex, const sead::Matrix34f& rViewMtx,
                          const sead::Matrix44f& rProjMtx, f32 near, f32 far, f32 fovy, f32 aspect,
                          const sead::Vector2f& rOffset)
{
    if (!isEnable())
    {
        return;
    }

    Context& rContext = mContext[viewIndex];
    rContext.mViewFrustumCulling.update(rViewMtx, rProjMtx, near, far, fovy, aspect, rOffset);

    const sead::Matrix34f rView = rContext.mViewFrustumCulling.getViewMtx();
    sead::Vector3f camPos(-rView(0, 3), -rView(1, 3), -rView(2, 3));
    sead::Vector3f offset;
    offset.x = *getPreset_<PresetBase>()->mIsFixPosX ?
                   rView(0, 0) * camPos.x + rView(1, 0) * camPos.y + rView(2, 0) * camPos.z :
                   0.0f;
    offset.y = *getPreset_<PresetBase>()->mIsFixPosY ?
                   rView(0, 1) * camPos.x + rView(1, 1) * camPos.y + rView(2, 1) * camPos.z :
                   0.0f;
    offset.z = *getPreset_<PresetBase>()->mIsFixPosZ ?
                   rView(0, 2) * camPos.x + rView(1, 2) * camPos.y + rView(2, 2) * camPos.z :
                   0.0f;
    mOcclusionRenderer.mContext[viewIndex].mOffset = offset;

    sead::Vector3f pos = *getPreset_<PresetBase>()->mPosition;
    OcclusionRenderer::CalcResult result;
    mOcclusionRenderer.calcContext(viewIndex, rViewMtx, rProjMtx, near, far, fovy, aspect, rOffset,
                                   &result);
    rContext.mIsVisible = true;
    rContext.mScreenPos.set(result.mScreenPos.x, result.mScreenPos.y);

    f32 edgeX = getPreset_<PresetBase>()->mScrEdgeSize->x;
    f32 edgeY = getPreset_<PresetBase>()->mScrEdgeSize->y;
    f32 absX = sead::Mathf::abs(result.mScreenPos.x);
    f32 absY = sead::Mathf::abs(result.mScreenPos.y);
    if (result.mScreenPos.z < 0.0f || result.mScreenPos.z > 1.0f || absX > edgeX + result.mDepth ||
        absY > edgeY + result.mDepth)
    {
        rContext.mIsVisible = false;
        return;
    }

    rContext.mWorldMtx.makeT(offset + pos);
    const cull::ViewFrustumCulling& rCulling = mContext[viewIndex].mViewFrustumCulling;
    rContext.mWorldViewMtx.setMul(rCulling.getViewMtx(), rContext.mWorldMtx);
    rContext.mWorldViewProjMtx.setMul(rCulling.getProjMtx(), rContext.mWorldViewMtx);

    rContext.mEdgeRateX = calcEdgeRate(absX, edgeX);
    rContext.mEdgeRateY = calcEdgeRate(absY, edgeY);

    sead::Vector3f center;
    center.setMul(rContext.mWorldViewMtx, sead::Vector3f::zero);
    sead::Vector3f dir;
    dir.setMul(rContext.mWorldViewMtx, *getPreset_<PresetBase>()->mDirection);
    dir -= center;
    dir.normalize();
    sead::Vector3f eye = -center;
    eye.normalize();
    f32 dot = dir.dot(eye);
    sead::Vector3f cross;
    cross.setCross(dir, eye);
    rContext.mAngle = sead::Mathf::atan2(cross.length(), dot);

    calcContextImpl_(viewIndex, rContext, rContext);
}

/**
 * Updates GPU resources of an enabled effect.
 */
void OfxBase::updateGPU()
{
    if (!isEnable())
    {
        return;
    }

    mOcclusionRenderer.updateGPU();
    updateGPUImpl_();
}

/**
 * Updates per-view GPU resources of an enabled, visible effect.
 * @param viewIndex view index
 * @param rRenderBuffer render buffer to draw into
 */
void OfxBase::updateViewGPU(s32 viewIndex, const RenderBuffer& rRenderBuffer)
{
    if (!isEnable() || !mContext[viewIndex].mIsVisible)
    {
        return;
    }

    mOcclusionRenderer.updateViewGPU(viewIndex, rRenderBuffer);
    updateViewGPUImpl_(viewIndex, mContext[viewIndex], mContext[viewIndex]);
}

/**
 * Draws the occlusion test and then the effect for a visible view.
 * @param pDrawContext draw context that receives the commands
 * @param viewIndex view index
 * @param rRenderBuffer render buffer to draw into
 * @param rViewport viewport to draw with
 * @param rDepth scene depth target
 */
void OfxBase::draw(DrawContext* pDrawContext, s32 viewIndex, const RenderBuffer& rRenderBuffer,
                   const sead::Viewport& rViewport, const RenderTargetDepth& rDepth) const
{
    if (!isEnable() || !mContext[viewIndex].mIsVisible)
    {
        return;
    }

    mOcclusionRenderer.draw(pDrawContext, viewIndex, rDepth);
    drawImpl_(pDrawContext, viewIndex, rRenderBuffer, rViewport, mContext[viewIndex],
              mOcclusionRenderer);
    mOcclusionRenderer.release(pDrawContext, viewIndex);
}

/**
 * Draws the occlusion debug shapes, the direction line and the screen edge frame.
 * @param pDrawContext draw context that receives the commands
 * @param viewIndex view index
 */
void OfxBase::drawDebugOfx(DrawContext* pDrawContext, s32 viewIndex) const
{
    if (!isEnable() || !mContext[viewIndex].mIsVisible)
    {
        return;
    }

    if (!mFlag.isOn(0x88))
    {
        return;
    }

    mFlag.reset(0x80);

    mOcclusionRenderer.drawDebug(pDrawContext, viewIndex, mDebugColor0, mDebugColor1, mDebugColor2);
    const cull::ViewFrustumCulling& rCulling = mContext[viewIndex].mViewFrustumCulling;
    utl::DevTools::beginDrawImm(pDrawContext, rCulling.getViewMtx(), rCulling.getProjMtx());

    {
        sead::Vector3f dir = *getPreset_<PresetBase>()->mDirection;
        dir.normalize();
        sead::Vector3f start =
            *getPreset_<PresetBase>()->mPosition + mOcclusionRenderer.mContext[viewIndex].mOffset;
        sead::Vector3f end = *getPreset_<PresetBase>()->mPosition +
                             mOcclusionRenderer.mContext[viewIndex].mOffset +
                             dir * *getPreset_<PresetBase>()->mRadius;
        utl::DevTools::drawLineImm(pDrawContext, start, end, mDebugColor2, 1.0f);
    }

    sead::OrthoProjection projection;
    sead::LookAtCamera camera(sead::Vector3f::zero, sead::Vector3f(0.0f, 0.0f, 1.0f),
                              sead::Vector3f::ey);
    utl::DevTools::beginDrawImm(pDrawContext, camera.getMatrix(), projection.getProjectionMatrix());

    f32 halfX = getPreset_<PresetBase>()->mScrEdgeSize->x * 0.5f;
    f32 edgeY = getPreset_<PresetBase>()->mScrEdgeSize->y;
    f32 top = edgeY * 0.5f;
    utl::DevTools::drawLineImm(pDrawContext, sead::Vector3f(-halfX, top, 0.0f),
                               sead::Vector3f(halfX, top, 0.0f), sead::Color4f::cBlue, 1.0f);
    f32 bottom = edgeY * -0.5f;
    utl::DevTools::drawLineImm(pDrawContext, sead::Vector3f(-halfX, bottom, 0.0f),
                               sead::Vector3f(halfX, bottom, 0.0f), sead::Color4f::cBlue, 1.0f);
    utl::DevTools::drawLineImm(pDrawContext, sead::Vector3f(-halfX, top, 0.0f),
                               sead::Vector3f(-halfX, bottom, 0.0f), sead::Color4f::cRed, 1.0f);
    utl::DevTools::drawLineImm(pDrawContext, sead::Vector3f(halfX, top, 0.0f),
                               sead::Vector3f(halfX, bottom, 0.0f), sead::Color4f::cRed, 1.0f);
}

/**
 * Returns the occlusion rate measured for a view.
 * @param viewIndex view index
 * @return occlusion rate
 */
f32 OfxBase::getOcclusionRate(s32 viewIndex) const
{
    return mOcclusionRenderer.getOcclusionRate(viewIndex);
}

/**
 * Enables or disables the effect and marks the manager menu dirty on change.
 * @param enable whether to enable
 */
void OfxBase::setEnable(bool enable)
{
    if (isEnable() != enable)
    {
        mMgr->setDirty();
    }

    EnvObj::setEnable(enable);
}

/**
 * Handles host IO edits of the effect.
 * @param pEvent property event
 */
void OfxBase::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    sead::BitFlag16* pFlag = &mFlag;
    if ((pEvent->getType() & 2) == 0)
    {
        const void* id = pEvent->getId();
        if (id < pFlag + 1 && id >= pFlag)
        {
            mMgr->setDirty();
        }
    }

    if (pEvent->getId() == reinterpret_cast<const void*>(1000))
    {
        pFlag->set(0x20);
    }

    listenPropertyEventImpl_(pEvent);
    mOcclusionRenderer.listenPropertyEvent(pEvent);
}

/**
 * Generates the host IO message of the effect.
 * @param pContext host IO context
 */
void OfxBase::genMessage(sead::hostio::Context* pContext)
{
    genMessageImpl_(pContext);
}

/**
 * Generates the short host IO message used in the instance menu (empty in release).
 * @param pContext host IO context
 */
void OfxBase::genMessageSimple(sead::hostio::Context* pContext) {}

/**
 * Handles host IO edits made in the instance menu.
 * @param pEvent property event
 */
void OfxBase::listenPropertyEventSimple(const sead::hostio::PropertyEvent* pEvent)
{
    if ((pEvent->getType() & 2) == 0)
    {
        const void* id = pEvent->getId();
        if (id < &mFlag + 1 && id >= &mFlag)
        {
            mMgr->setDirty();
        }
    }
}

/**
 * Constructs an empty per-view lens flare context.
 * @param index view index
 */
OfxLensFlare::ContextLensFlare::ContextLensFlare(s32 index) : mIndex(index) {}

/**
 * Constructs the parameters of one flare element.
 * @param index element index used in the parameter names
 * @param pObj parameter object that owns the parameters
 */
OfxLensFlare::Preset::PresetElement::PresetElement(s32 index, utl::IParameterObj* pObj)
    : mTextureIdx(-1, sead::FormatFixedSafeString<256>("TextureIdx%d", index), "テクスチャ番号",
                  pObj),
      mTexture2Idx(-1, sead::FormatFixedSafeString<256>("Texture2Idx%d", index), "テクスチャ番号2",
                   pObj),
      mPosition(0.0f, sead::FormatFixedSafeString<256>("Position%d", index), "位置", pObj),
      mRotate(0.0f, sead::FormatFixedSafeString<256>("Rotate%d", index), "角度", pObj),
      mIntensity(1.0f, sead::FormatFixedSafeString<256>("Intensity%d", index), "強度", pObj),
      mIsSizeZoom(false, sead::FormatFixedSafeString<256>("IsSizeZoom%d", index), "サイズを拡縮",
                  pObj),
      mSize(sead::Vector2f(0.1f, 0.1f), sead::FormatFixedSafeString<256>("Size%d", index), "サイズ",
            pObj),
      mColor(sead::Color4f::cWhite, sead::FormatFixedSafeString<256>("Color%d", index), "色", pObj),
      mBlendMode(0, sead::FormatFixedSafeString<256>("BlendMode%d", index), "合成モード", pObj),
      mIsEnableDraw(false, sead::FormatFixedSafeString<256>("IsEnableDraw%d", index), "有効", pObj),
      mIsEnableRotate(false, sead::FormatFixedSafeString<256>("IsEnableRotate%d", index),
                      "ライト方向に回転", pObj),
      mIsEnableRotateInv(false, sead::FormatFixedSafeString<256>("IsEnableRotateInv%d", index),
                         "ライトと逆方向に回転", pObj),
      mIsEnableOccludedScaling(false,
                               sead::FormatFixedSafeString<256>("IsEnableOccludedScaling%d", index),
                               "遮蔽率でスケール", pObj),
      mIsEnableOccludedDec(true, sead::FormatFixedSafeString<256>("IsEnableOccludedDec%d", index),
                           "遮蔽率でテクスチャカラーを減算", pObj),
      mIsEnableOccludedAlpha(true,
                             sead::FormatFixedSafeString<256>("IsEnableOccludedAlpha%d", index),
                             "遮蔽率で透明にする", pObj),
      mIsEnableRotatePos(false, sead::FormatFixedSafeString<256>("IsEnableRotatePos%d", index),
                         "位置依存回転", pObj),
      mRotatePosRate(1.0f, sead::FormatFixedSafeString<256>("RotatePosRate%d", index),
                     "位置依存回転の勢い", pObj),
      mIsEnableEdgeScaling(false, sead::FormatFixedSafeString<256>("IsEnableEdgeScaling%d", index),
                           "フレームアウトでスケーリングする", pObj),
      mEdgeScaleRate(sead::Vector2f(1.0f, 1.0f),
                     sead::FormatFixedSafeString<256>("EdgeScaleRate%d", index),
                     "フレームアウトスケーリングの度合い", pObj),
      mCenterPosScalingRate(0.0f, sead::FormatFixedSafeString<256>("CenterPosScalingRate%d", index),
                            "中心からの距離でスケーリングする度合い", pObj),
      mCenterPosScalingPow(2.0f, sead::FormatFixedSafeString<256>("CenterPosScalingPow%d", index),
                           "中心からの距離でスケーリングする按配", pObj),
      mCenterPosAlphaRate(0.0f, sead::FormatFixedSafeString<256>("CenterPosAlphaRate%d", index),
                          "中心からの距離でAlphaする度合い", pObj),
      mCenterPosAlphaPow(2.0f, sead::FormatFixedSafeString<256>("CenterPosAlphaPow%d", index),
                         "中心からの距離でAlphaする按配", pObj),
      mIsEnableOctagon(false, sead::FormatFixedSafeString<256>("IsEnableOctagon%d", index),
                       "八角形ポリゴンを使う", pObj),
      mIsEnableAngleOcclusion(false,
                              sead::FormatFixedSafeString<256>("IsEnableAngleOcclusion%d", index),
                              "角度遮蔽", pObj),
      mAngleCenter(0.0f, sead::FormatFixedSafeString<256>("AngleCenter%d", index), "中心角度",
                   pObj),
      mAngleWidth(0.5f, sead::FormatFixedSafeString<256>("AngleWidth%d", index), "開き角度", pObj),
      mAnglePower(1.0f, sead::FormatFixedSafeString<256>("AnglePower%d", index), "角度遮蔽のカーブ",
                  pObj),
      mIndex(index)
{
}

/**
 * Copies all parameter values of another element.
 * @param rOther object to copy from
 */
void OfxLensFlare::Preset::PresetElement::copy(const PresetElement& rOther)
{
    *mTextureIdx = *rOther.mTextureIdx;
    *mTexture2Idx = *rOther.mTexture2Idx;
    *mPosition = *rOther.mPosition;
    *mRotate = *rOther.mRotate;
    *mIntensity = *rOther.mIntensity;
    *mIsSizeZoom = *rOther.mIsSizeZoom;
    *mSize = *rOther.mSize;
    *mColor = *rOther.mColor;
    *mBlendMode = *rOther.mBlendMode;
    *mIsEnableRotate = *rOther.mIsEnableRotate;
    *mIsEnableRotateInv = *rOther.mIsEnableRotateInv;
    *mIsEnableOccludedScaling = *rOther.mIsEnableOccludedScaling;
    *mIsEnableOccludedDec = *rOther.mIsEnableOccludedDec;
    *mIsEnableOccludedAlpha = *rOther.mIsEnableOccludedAlpha;
    *mIsEnableRotatePos = *rOther.mIsEnableRotatePos;
    *mRotatePosRate = *rOther.mRotatePosRate;
    *mIsEnableEdgeScaling = *rOther.mIsEnableEdgeScaling;
    *mEdgeScaleRate = *rOther.mEdgeScaleRate;
    *mCenterPosScalingRate = *rOther.mCenterPosScalingRate;
    *mCenterPosScalingPow = *rOther.mCenterPosScalingPow;
    *mCenterPosAlphaRate = *rOther.mCenterPosAlphaRate;
    *mCenterPosAlphaPow = *rOther.mCenterPosAlphaPow;
    *mIsEnableOctagon = *rOther.mIsEnableOctagon;
    *mIsEnableAngleOcclusion = *rOther.mIsEnableAngleOcclusion;
    *mAngleCenter = *rOther.mAngleCenter;
    *mAngleWidth = *rOther.mAngleWidth;
    *mAnglePower = *rOther.mAnglePower;
}

/**
 * Constructs the per-view parameters of a preset.
 * @param index view index used in the parameter names
 * @param pObj parameter object that owns the parameters
 */
OfxLensFlare::Preset::PresetContext::PresetContext(s32 index, utl::IParameterObj* pObj)
    : mCenterPos(sead::Vector2f::zero, sead::FormatFixedSafeString<256>("CenterPos%d", index),
                 "視点毎のレンズ中心位置", pObj),
      mBaseAxis(0, sead::FormatFixedSafeString<256>("BaseAxis%d", index), "ビルボード向きの基準軸",
                pObj),
      mIndex(index)
{
}

/**
 * Constructs the lens flare preset parameters.
 */
OfxLensFlare::Preset::Preset()
    : mSizeBaseScale(100.0f, "SizeBaseScale", "サイズを奥行で拡縮する場合の基準スケール", this),
      mCoreOcclusionType(0, "CoreOcclusionType", "コア遮蔽率の扱い", this),
      mScrEdgeType(0, "ScrEdgeType", "画面端の扱い", this),
      mScrEdgePow(1.0f, "ScrEdgePow", "画面端率の具合", this),
      mScrEdgeFlash(10.0f, "ScrEdgeFlash", "画面端でフラッシュする強さ", this)
{
}

/**
 * Deletes the per-view and per-element parameters.
 */
OfxLensFlare::Preset::~Preset()
{
    for (s32 i = 0; i < mPresetContext.size(); i++)
    {
        delete mPresetContext[i];
    }

    mPresetContext.freeBuffer();
    for (s32 i = 0; i < mElement.size(); i++)
    {
        delete mElement[i];
    }

    mElement.freeBuffer();
}

/**
 * Creates the element and per-view parameter sets.
 * @param rArg creation arguments
 * @param pHeap heap used for allocations
 */
void OfxLensFlare::Preset::initializeOfxImpl_(const CreateArg& rArg, sead::Heap* pHeap)
{
    s32 elementNum = getMaxElementNum();
    mElement.allocBuffer(elementNum, pHeap);
    for (s32 i = 0; i < elementNum; i++)
    {
        mElement.pushBack(new (pHeap) PresetElement(i, this));
    }

    mPresetContext.allocBuffer(rArg.mViewNum, pHeap);
    for (s32 i = 0; i < rArg.mViewNum; i++)
    {
        mPresetContext.pushBack(new (pHeap) PresetContext(i, this));
    }
}

void OfxLensFlare::Preset::listenPropertyEventImpl_(const sead::hostio::PropertyEvent* pEvent)
{
    uintptr_t id = reinterpret_cast<uintptr_t>(pEvent->getId());
    if (id - 1100 < 100)
    {
        PresetElement* pDst = searchElement_(static_cast<s32>(id) - 1100);
        if (pDst == nullptr)
        {
            return;
        }

        PresetElement* pSrc = searchElement_(pDst->mOrder);
        if (pSrc == nullptr)
        {
            return;
        }

        pDst->copy(*pSrc);
    }
}

void OfxLensFlare::Preset::genMessageImpl_(sead::hostio::Context* pContext)
{
    mSizeBaseScale.genMessageParameter(pContext, utl::DevTools::getStringMinMax(0.01f, 100.0f));

    for (auto it = mPresetContext.begin(), end = mPresetContext.end(); it != end; ++it)
    {
        genMessageDummy(pContext, sead::FormatFixedSafeString<256>(
                                      "GroupHeader = perspective %d, Layout = UniformGrid, "
                                      "NumCol = 2, NumRow = 1",
                                      it->mIndex));
    }

    for (auto it = mElement.begin(), end = mElement.end(); it != end; ++it)
    {
        genMessageDummy(pContext, sead::FormatFixedSafeString<16>("%d", it->mIndex),
                        sead::FormatFixedSafeString<16>("Order=%d", it->mIndex + 1));
        it->mIsEnableDraw.genMessageParameter(pContext, it->mIsEnableDraw.getMeta());
        if (mElement.size() > 1)
        {
            for (auto it2 = mElement.begin(), end2 = mElement.end(); it2 != end2; ++it2)
            {
                if (it2->mIndex == it->mIndex)
                {
                    continue;
                }

                if (it->mOrder == it->mIndex)
                {
                    it->mOrder = it2->mIndex;
                }

                genMessageDummy(pContext, sead::FormatFixedSafeString<16>("%d", it2->mIndex));
            }
        }

        genMessageDummy(pContext, utl::DevTools::getStringMinMax(-0.2f, 2.0f));
        mMgr->genMessageTextureSelect(pContext, &*it->mTextureIdx, "Texture");
        mMgr->genMessageTextureSelect(
            pContext, &*it->mTexture2Idx,
            "Texture 2 (it is multiplied by by a position dependence turn)");
        genMessageDummy(pContext, utl::DevTools::getStringMinMax(0.0f, 0.2f));
    }
}

/**
 * Constructs an element without sampler overrides.
 * @param index element index
 */
OfxLensFlare::Element::Element(s32 index) : mSampler(nullptr), mSampler2(nullptr), mIndex(index) {}

/**
 * Constructs the lens flare effect.
 */
OfxLensFlare::OfxLensFlare()
{
    detail::RootNode::setNodeMeta(this, "Icon=EFFECT");
}

/**
 * Frees the per-view contexts, the elements and the vertex attributes.
 */
OfxLensFlare::~OfxLensFlare()
{
    for (auto it = mContextLensFlare.begin(), end = mContextLensFlare.end(); it != end; ++it)
    {
        it->mInstance.freeBuffer();
        it->mUniformBlock.freeBuffer();
        delete &*it;
    }

    mContextLensFlare.freeBuffer();
    for (s32 i = 0; i < mElement.capacity(); i++)
    {
        delete mElement[i];
    }

    mElement.freeBuffer();
    mVertexAttrQuad.destroy();
    mVertexAttrQuadDouble.destroy();
    mVertexAttrOctagon.destroy();
    mVertexAttrOctagonDouble.destroy();
}

/**
 * Returns the occlusion rate combined with the core occlusion rate as configured by the preset.
 * @param viewIndex view index
 * @return occlusion rate
 */
f32 OfxLensFlare::getOcclusionRate(s32 viewIndex) const
{
    f32 rate = mOcclusionRenderer.getOcclusionRate(viewIndex);
    f32 coreRate = mOcclusionRenderer.getCoreOcclusionRate(viewIndex);
    switch (*getPreset_<Preset>()->mCoreOcclusionType)
    {
    case 1:
        rate *= coreRate;
        break;
    case 2:
        rate = coreRate <= 0.0f ? 0.0f : rate;
        break;
    }

    return rate;
}

void OfxLensFlare::initializeImpl_(const CreateArg& rArg, sead::Heap* pHeap)
{
    mContextLensFlare.allocBuffer(rArg.mViewNum, pHeap);
    for (s32 i = 0; i < mContextLensFlare.capacity(); i++)
    {
        mContextLensFlare.pushBack(new (pHeap) ContextLensFlare(i));
    }

    mMgr->getVertexAttrQuad(&mVertexAttrQuad, pHeap);
    mMgr->getVertexAttrQuadDouble(&mVertexAttrQuadDouble, pHeap);
    mMgr->getVertexAttrOctagon(&mVertexAttrOctagon, pHeap);
    mMgr->getVertexAttrOctagonDouble(&mVertexAttrOctagonDouble, pHeap);

    mOcclusionRenderer.mUseCore = false;
    mOcclusionRenderer.mUseSoft = true;

    mElement.allocBuffer(getMaxElementNum(), pHeap);
    for (s32 i = 0; i < mElement.capacity(); i++)
    {
        mElement.pushBack(new (pHeap) Element(i));
    }

    for (auto it = mContextLensFlare.begin(), end = mContextLensFlare.end(); it != end; ++it)
    {
        it->mInstance.tryAllocBuffer(getMaxElementNum(), pHeap);
        sead::Buffer<UniformBlock>& rBlocks = it->mUniformBlock;
        rBlocks.tryAllocBuffer(getMaxElementNum(), pHeap);
        UniformBlock* pBlocks = rBlocks.getBufferPtr();
        s32 num = rBlocks.size();
        for (s32 i = 0; i != num; i++)
        {
            UniformBlock& rBlock = pBlocks[i];
            if (i == 0)
            {
                rBlock.startDeclare(10, pHeap);
                rBlock.declare(UniformBlock::cType_Vec4, 1);
                rBlock.declare(UniformBlock::cType_Vec2, 1);
                rBlock.declare(UniformBlock::cType_Vec2, 1);
                rBlock.declare(UniformBlock::cType_Vec2, 1);
                rBlock.declare(UniformBlock::cType_Float, 1);
                rBlock.declare(UniformBlock::cType_Float, 1);
                rBlock.declare(UniformBlock::cType_Vec2, 1);
                rBlock.declare(UniformBlock::cType_Vec2, 1);
                rBlock.declare(UniformBlock::cType_Vec2, 2);
                rBlock.declare(UniformBlock::cType_Vec2, 1);
            }
            else
            {
                rBlock.declare(*rBlocks.getBufferPtr());
            }

            rBlock.create(pHeap, 2, 1);
        }
    }
}

/**
 * Flips the uniform buffer index and applies the occlusion options of the preset.
 */
void OfxLensFlare::updateImpl_()
{
    mBufferIndex = 1 - mBufferIndex;
    mOcclusionRenderer.mUseSoft = *getPreset_<Preset>()->mCoreOcclusionType != 0;
    mOcclusionRenderer.mBorderBlack = *getPreset_<Preset>()->mScrEdgeType == 0;
}

void OfxLensFlare::calcContextImpl_(s32 viewIndex, const Context& rContext,
                                    const UpdateViewGPUArg& rArg)
{
    ContextLensFlare* pContext = mContextLensFlare[viewIndex];

    sead::Vector2f dir =
        rArg.mScreenPos - *getPreset_<Preset>()->mPresetContext.unsafeAt(viewIndex)->mCenterPos;
    f32 dist = dir.normalize();

    f32 angle;
    s32 baseAxis = *getPreset_<Preset>()->mPresetContext.unsafeAt(viewIndex)->mBaseAxis;
    if (baseAxis < 1)
    {
        angle = 0.0f;
    }
    else
    {
        sead::Vector3f axis;
        if (baseAxis == 3)
        {
            axis.set(0.0f, 0.0f, 1.0f);
        }
        else if (baseAxis == 2)
        {
            axis.set(0.0f, 1.0f, 0.0f);
        }
        else
        {
            axis.set(1.0f, 0.0f, 0.0f);
        }

        const sead::Matrix34f& rViewMtx = rContext.mViewFrustumCulling.getViewMtx();
        sead::Vector3f viewAxis(
            rViewMtx.m[0][0] * axis.x + rViewMtx.m[0][1] * axis.y + rViewMtx.m[0][2] * axis.z,
            rViewMtx.m[1][0] * axis.x + rViewMtx.m[1][1] * axis.y + rViewMtx.m[1][2] * axis.z,
            0.0f);
        viewAxis.normalize();
        f32 dot = viewAxis.dot(sead::Vector3f::ey);
        sead::Vector3f cross;
        cross.setCross(viewAxis, sead::Vector3f::ey);
        angle = sead::Mathf::atan2(cross.length(), dot);
        if (viewAxis.x > 0.0f)
        {
            angle = -angle;
        }
    }

    f32 rotSin = std::sin(angle);
    f32 rotCos = std::cos(angle);
    sead::Matrix22f rotMtx(rotCos, -rotSin, rotSin, rotCos);
    f32 invSin = std::sin(angle);
    f32 invCos = std::cos(angle);
    sead::Matrix22f invRotMtx(invCos, invSin, -invSin, invCos);

    for (auto it = pContext->mInstance.begin(), end = pContext->mInstance.end(); it != end; ++it)
    {
        s32 index = it.getIndex();
        const Preset::PresetElement* pElement = getPreset_<Preset>()->mElement[index];
        if (!*pElement->mIsEnableDraw)
        {
            continue;
        }

        const Element* pElementCtx = mElement[index];
        ContextLensFlare::Instance& rInstance = *it;

        rInstance.mIntensity = 1.0f;
        if (*pElement->mIsEnableAngleOcclusion)
        {
            f32 diff = sead::Mathf::abs(rArg.mAngle / -sead::Mathf::pi() * 180.0f +
                                        *pElement->mAngleCenter);
            rInstance.mIntensity *=
                std::pow(sead::Mathf::max(1.0f - diff / *pElement->mAngleWidth, 0.0f),
                         *pElement->mAnglePower);
        }

        f32 edgeRate = rArg.mEdgeRateX > rArg.mEdgeRateY ? rArg.mEdgeRateX : rArg.mEdgeRateY;
        edgeRate = std::pow(edgeRate, *getPreset_<Preset>()->mScrEdgePow);
        f32 edgeFlash = *getPreset_<Preset>()->mScrEdgeFlash;
        f32 edgeInv = 1.0f - edgeRate;
        if (*getPreset_<Preset>()->mScrEdgeType == 1)
        {
            rInstance.mIntensity *= edgeInv;
        }

        rInstance.mScale.set(1.0f, 1.0f);
        rInstance.mOffset.set(0.0f, 0.0f);
        if (*pElement->mIsEnableEdgeScaling)
        {
            f32 scale = edgeRate * edgeInv * edgeFlash;
            rInstance.mScale.set(scale * pElement->mEdgeScaleRate->x + 1.0f,
                                 scale * pElement->mEdgeScaleRate->y + 1.0f);
        }

        if (*pElement->mCenterPosScalingRate != 0.0f)
        {
            sead::Vector2f diff =
                rArg.mScreenPos -
                *getPreset_<Preset>()->mPresetContext.unsafeAt(viewIndex)->mCenterPos;
            diff.y /= rContext.mViewFrustumCulling.mAspect;
            f32 rate = std::pow(diff.length(), *pElement->mCenterPosScalingPow);
            rInstance.mScale.x *= rate * *pElement->mCenterPosScalingRate + 1.0f;
            rInstance.mScale.y *= rate * *pElement->mCenterPosScalingRate + 1.0f;
        }

        f32 alpha = 1.0f;
        if (*pElement->mCenterPosAlphaRate != 0.0f)
        {
            sead::Vector2f diff =
                rArg.mScreenPos -
                *getPreset_<Preset>()->mPresetContext.unsafeAt(viewIndex)->mCenterPos;
            diff.y /= rContext.mViewFrustumCulling.mAspect;
            f32 rate = std::pow(diff.length(), *pElement->mCenterPosAlphaPow);
            if (*pElement->mCenterPosAlphaRate > 0.0f)
            {
                alpha =
                    sead::Mathf::clamp(1.0f - rate * *pElement->mCenterPosAlphaRate, 0.0f, 1.0f);
            }
            else
            {
                alpha = sead::Mathf::clamp(1.0f - (rate - 1.0f) * *pElement->mCenterPosAlphaRate,
                                           0.0f, 1.0f);
            }
        }

        sead::Vector2f size(pElement->mSize->x, pElement->mSize->y);
        if (*pElement->mIsSizeZoom)
        {
            sead::Vector3f viewPos;
            viewPos.setMul(rArg.mWorldViewMtx, sead::Vector3f(0.0f, 0.0f, 0.0f));
            f32 scale = *getPreset_<Preset>()->mSizeBaseScale /
                        (viewPos.z * rContext.mViewFrustumCulling.mTanHalfFovy);
            size.x *= scale;
            size.y *= scale;
        }

        f32 intensity = *pElement->mIntensity;
        rInstance.mColor = sead::Color4f(intensity, intensity, intensity, 1.0f);
        rInstance.mColor *= *pElement->mColor;
        rInstance.mColor *= mColor;
        rInstance.mColor.a *= alpha;
        rInstance.mHalfSize.set(size.x * 0.5f, size.y * 0.5f);
        f32 position = *pElement->mPosition;
        rInstance.mPos = rArg.mScreenPos - dist * (dir * position);

        rInstance.mDir.set(0.0f, 1.0f);
        f32 rotate;
        if (*pElement->mIsEnableRotate)
        {
            rInstance.mDir.set(dir.x, dir.y);
            rInstance.mDir.y = dir.y / rContext.mViewFrustumCulling.mAspect;
        }

        rotate = *pElement->mRotate;
        if (!*pElement->mIsEnableRotate)
        {
            rotate += angle;
        }

        rotateVec(&rInstance.mDir, rotate);

        if (*pElement->mIsEnableRotatePos)
        {
            sead::Vector2f pos = rArg.mScreenPos;
            pos.y /= rContext.mViewFrustumCulling.mAspect;
            if (angle != 0.0f)
            {
                mulMtx22(&pos, invRotMtx, pos);
            }

            rotateVec(&rInstance.mDir, (pos.x - pos.y) * *pElement->mRotatePosRate);
        }

        if (*pElement->mIsEnableRotateInv)
        {
            if (angle != 0.0f)
            {
                sead::Vector2f v = rInstance.mDir;
                mulMtx22(&v, invRotMtx, v);
                v.x = -v.x;
                mulMtx22(&rInstance.mDir, rotMtx, v);
            }
            else
            {
                rInstance.mDir.x = -rInstance.mDir.x;
            }
        }

        rInstance.mDir.normalize();

        if (pElementCtx->mSampler == nullptr &&
            !(*pElement->mTexture2Idx >= 0 &&
              mMgr->mTextureInfo.unsafeAt(*pElement->mTexture2Idx)->mIsValid))
        {
            continue;
        }

        sead::Vector2f pos = rArg.mScreenPos;
        pos.y /= rContext.mViewFrustumCulling.mAspect;
        if (angle != 0.0f)
        {
            mulMtx22(&pos, invRotMtx, pos);
        }

        f32 texRotate = (pos.y - pos.x) * *pElement->mRotatePosRate * 2.0f;
        f32 texSin = std::sin(texRotate);
        f32 texCos = std::cos(texRotate);
        sead::Matrix22f texMtx(texCos, -texSin, texSin, texCos);
        if (isQuarterTexture(mMgr, *pElement->mTextureIdx) &&
            !isQuarterTexture(mMgr, *pElement->mTexture2Idx))
        {
            texMtx.setMul(texMtx, sead::Matrix22f(0.5f, 0.0f, 0.0f, 0.5f));
        }
        else if (!isQuarterTexture(mMgr, *pElement->mTextureIdx) &&
                 isQuarterTexture(mMgr, *pElement->mTexture2Idx))
        {
            texMtx.setMul(texMtx, sead::Matrix22f(2.0f, 0.0f, 0.0f, 2.0f));
        }

        rInstance.mTexMtx[0].set(texMtx.m[0][0], texMtx.m[0][1]);
        rInstance.mTexMtx[1].set(texMtx.m[1][0], texMtx.m[1][1]);
        rInstance.mTexScale.set(isQuarterTexture(mMgr, *pElement->mTextureIdx) ? 1.0f : 0.5f,
                                isQuarterTexture(mMgr, *pElement->mTexture2Idx) ? 1.0f : 0.5f);
    }

    for (UniformBlock& rBlock : pContext->mUniformBlock)
    {
        rBlock.setCurrentBufferIndex(mBufferIndex);
    }
}

/**
 * Does nothing.
 */
void OfxLensFlare::updateGPUImpl_() {}

void OfxLensFlare::drawImpl_(DrawContext* pDrawContext, s32 viewIndex,
                             const RenderBuffer& rRenderBuffer, const sead::Viewport& rViewport,
                             const Context& rContext,
                             const OcclusionRenderer& rOcclusionRenderer) const
{
    const ContextLensFlare* pContext = mContextLensFlare[viewIndex];
    rRenderBuffer.bind(pDrawContext);
    rViewport.apply(pDrawContext, rRenderBuffer);
    const ShaderProgram* pProgram = detail::ShaderHolder::instance()->getShaderProgram(
        detail::ShaderHolder::cOccludedEffectLensflare);

    s32 currentBlendMode = 2;
    for (auto it = mElement.begin(), end = mElement.end(); it != end; ++it)
    {
        const Preset::PresetElement* pPresetElement = getPreset_<Preset>()->mElement[it->mIndex];
        if (!*pPresetElement->mIsEnableDraw)
        {
            continue;
        }

        const TextureSampler* pSampler = it->mSampler;
        if (pSampler == nullptr)
        {
            s32 textureIdx = *pPresetElement->mTextureIdx;
            if (textureIdx < 0 || !mMgr->mTextureInfo.unsafeAt(textureIdx)->mIsValid)
            {
                continue;
            }

            pSampler = &mMgr->mTextureInfo[textureIdx]->mSampler;
        }

        const TextureSampler* pSampler2 = it->mSampler2;
        if (pSampler2 == nullptr)
        {
            s32 textureIdx = *pPresetElement->mTexture2Idx;
            if (textureIdx >= 0 && mMgr->mTextureInfo.unsafeAt(textureIdx)->mIsValid)
            {
                pSampler2 = &mMgr->mTextureInfo[textureIdx]->mSampler;
            }
            else
            {
                pSampler2 = nullptr;
            }
        }

        s32 blendMode = *pPresetElement->mBlendMode;
        if (currentBlendMode != blendMode)
        {
            sead::GraphicsContext graphicsContext;
            graphicsContext.setBlendEnable(true);
            graphicsContext.setDepthEnable(false, false);
            graphicsContext.setBlendEquation(0, 1);
            if (blendMode == 0)
            {
                graphicsContext.setBlendFactor(0, 5, 2);
            }
            else if (blendMode == 1)
            {
                graphicsContext.setBlendFactor(0, 5, 6);
            }

            graphicsContext.apply(pDrawContext);
            currentBlendMode = blendMode;
        }

        bool isOccludedScaling = *pPresetElement->mIsEnableOccludedScaling;
        bool isOccludedDec = *pPresetElement->mIsEnableOccludedDec;
        bool isOccludedAlpha = *pPresetElement->mIsEnableOccludedAlpha;
        bool isColored = !(*pPresetElement->mColor == sead::Color4f::cWhite &&
                           *pPresetElement->mIntensity == 1.0f);
        bool hasTexture2 = pSampler2 != nullptr;
        s32 variation = *getPreset_<Preset>()->mCoreOcclusionType << 5 | isOccludedScaling << 4 |
                        isOccludedDec << 3 | isOccludedAlpha << 2 | isColored << 1 | hasTexture2;
        const ShaderProgram* pVariation = pProgram->getVariation(variation);
        pVariation->activate(pDrawContext, true);

        pContext->mUniformBlock[it->mIndex].activate(pDrawContext,
                                                     pVariation->getUniformBlockLocation(0));
        rOcclusionRenderer.mContext[viewIndex].mSub[0].mSampler.activate(
            pDrawContext, pVariation->getSamplerLocation(0), -1, false);
        pSampler->activate(pDrawContext, pVariation->getSamplerLocation(1), -1, false);
        if (pSampler2 != nullptr)
        {
            pSampler2->activate(pDrawContext, pVariation->getSamplerLocation(2), -1, false);
        }

        bool isOctagon = *pPresetElement->mIsEnableOctagon;
        bool isQuarter = isQuarterTexture(mMgr, *pPresetElement->mTextureIdx);
        if (isOctagon)
        {
            if (isQuarter)
            {
                mVertexAttrOctagonDouble.activate(pDrawContext);
                pfx::detail::drawIndexStream(pDrawContext, mMgr->mVtxOctagonDouble.mIndexStream);
            }
            else
            {
                mVertexAttrOctagon.activate(pDrawContext);
                pfx::detail::drawIndexStream(pDrawContext, mMgr->mVtxOctagon.mIndexStream);
            }
        }
        else if (isQuarter)
        {
            mVertexAttrQuadDouble.activate(pDrawContext);
            pfx::detail::drawIndexStream(pDrawContext, mMgr->mVtxQuadDouble.mIndexStream);
        }
        else
        {
            mVertexAttrQuad.activate(pDrawContext);
            pfx::detail::drawIndexStream(pDrawContext, mMgr->mVtxQuad.mIndexStream);
        }
    }
}

void OfxLensFlare::updateViewGPUImpl_(s32 viewIndex, const Context& rContext,
                                      const UpdateViewGPUArg& rArg)
{
    ContextLensFlare* pContext = mContextLensFlare[viewIndex];
    for (auto it = pContext->mUniformBlock.begin(), end = pContext->mUniformBlock.end(); it != end;
         ++it)
    {
        s32 index = it.getIndex();
        if (!*getPreset_<Preset>()->mElement.unsafeAt(index)->mIsEnableDraw)
        {
            continue;
        }

        const UniformBlock& rBlock = *it;
        const ContextLensFlare::Instance& rInstance = pContext->mInstance[index];
        rBlock.dcbz(0);
        setUniformFloat(rBlock, 4, rContext.mViewFrustumCulling.mAspect);
        setUniformFloat(rBlock, 5, rInstance.mIntensity);
        rBlock.setData(6, &rInstance.mScale, 0, 1);
        rBlock.setData(7, &rInstance.mOffset, 0, 1);
        rBlock.setData(0, &rInstance.mColor, 0, 1);
        rBlock.setData(3, &rInstance.mHalfSize, 0, 1);
        rBlock.setData(1, &rInstance.mPos, 0, 1);
        rBlock.setData(2, &rInstance.mDir, 0, 1);
        rBlock.setData(8, &rInstance.mTexMtx[0], 0, 1);
        rBlock.setData(8, &rInstance.mTexMtx[1], 1, 1);
        rBlock.setData(9, &rInstance.mTexScale, 0, 1);
        rBlock.flushCurrentBuffer();
    }
}

/**
 * Does nothing.
 * @param pContext host IO context
 */
void OfxLensFlare::genMessageImpl_(sead::hostio::Context* pContext) {}

/**
 * Does nothing.
 * @param pEvent property event
 */
void OfxLensFlare::listenPropertyEventImpl_(const sead::hostio::PropertyEvent* pEvent) {}

/**
 * Constructs the program-driven lens flare effect.
 */
OfxLensFlareDynamic::OfxLensFlareDynamic()
{
    detail::RootNode::setNodeMeta(this, "Icon=EFFECT");
}

/**
 * Destroys the program-driven lens flare effect.
 */
OfxLensFlareDynamic::~OfxLensFlareDynamic() = default;

/**
 * Selects the parent preset and sets the position and base scale of the flare.
 * @param presetIndex parent preset index
 * @param scale unused scale
 * @param rPos world position
 * @param rSize size (x is used as the base scale)
 * @param rColor unused color
 * @param enable unused
 */
void OfxLensFlareDynamic::setParam(s32 presetIndex, f32 scale, const sead::Vector3f& rPos,
                                   const sead::Vector2f& rSize, const sead::Color4f& rColor,
                                   bool enable)
{
    Preset* pPreset = getPreset_<Preset>();
    if (mParentIndex != presetIndex)
    {
        mParentIndex = presetIndex;
        loadPresetByParentIndex(presetIndex);
    }

    *pPreset->mPosition = rPos;
    *getPreset_<OfxLensFlare::Preset>()->mSizeBaseScale = rSize.x;
}

/**
 * Copies a preset of this effect type into the owned preset.
 * @param index preset index within the type
 * @return whether both presets exist
 */
bool OfxLensFlareDynamic::loadPresetByParentIndex(s32 index)
{
    Preset* pPreset = getPreset_<Preset>();
    OfxBase::PresetBase* pSrc = mMgr->getPresetByIndex(getOfxTypeID(), index);
    if (pPreset == nullptr || pSrc == nullptr)
    {
        return false;
    }

    pPreset->copy(*pSrc);
    return true;
}

const env::TypeInfo* OfxLensFlare::sTypeInfo = env::EnvObj::registClass(
    "OfxLensFlare", "Lens flare", &env::EnvObj::TypeToID<OfxLensFlare>::createInstance,
    env::EnvObj::cMetaInfo_Other, sizeof(OfxLensFlare));
const env::TypeInfo* OfxLensFlare::Preset::sTypeInfo =
    env::EnvObj::registClass("OfxLensFlare::Preset", "Lens flare Preset",
                             &env::EnvObj::TypeToID<OfxLensFlare::Preset>::createInstance,
                             env::EnvObj::cMetaInfo_Other, sizeof(OfxLensFlare::Preset));
const env::TypeInfo* OfxLensFlareDynamic::sTypeInfo =
    env::EnvObj::registClass("OfxLensFlareDynamic", "Lens flare (program designation)",
                             &env::EnvObj::TypeToID<OfxLensFlareDynamic>::createInstance,
                             env::EnvObj::cMetaInfo_Other, sizeof(OfxLensFlareDynamic));
const env::TypeInfo* OfxLensFlareDynamic::Preset::sTypeInfo = env::EnvObj::registClass(
    "OfxLensFlareDynamic::Preset", "Lens flare (program designation) Preset",
    &env::EnvObj::TypeToID<OfxLensFlareDynamic::Preset>::createInstance,
    env::EnvObj::cMetaInfo_Other, sizeof(OfxLensFlareDynamic::Preset));

}  // namespace agl::fx
