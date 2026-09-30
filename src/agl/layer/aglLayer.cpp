#include "layer/aglLayer.h"

#include <controller/seadController.h>
#include <gfx/seadGraphicsContext.h>
#include <hostio/seadHostIONodeEvent.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <math/seadMathCalcCommon.h>
#include <random/seadGlobalRandom.h>

#include "common/aglDrawContext.h"
#include "common/aglRenderBuffer.h"
#include "detail/aglPrivateResource.h"
#include "detail/aglRootNode.h"
#include "detail/aglSeadUtil.h"
#include "layer/aglDrawMethod.h"
#include "layer/aglRenderDisplay.h"
#include "layer/aglRenderInfo.h"
#include "layer/aglRenderer.h"
#include "utility/aglDevTools.h"
#include "utility/aglParameter.h"
#include "utility/aglParameterIO.h"
#include "utility/aglParameterObj.h"

namespace agl::lyr {

namespace {

class CameraIO : public utl::IParameterIO {
public:
    CameraIO()
        : utl::IParameterIO("aglcam", 0),
          mPos(sead::Vector3f::ez, "pos", "位置", &mObj),
          mAt(sead::Vector3f::zero, "at", "注視点", &mObj),
          mUp(sead::Vector3f::ey, "up", "アップ", &mObj),
          mFovy(0.7853982f, "fovy", "画角", &mObj),
          mAtOffset(sead::Vector3f::zero, "at_offset", "注視点のオフセット", &mObj)
    {
        addObj(&mObj, "cam");
    }

    utl::IParameterObj mObj;
    utl::Parameter<sead::Vector3f> mPos;
    utl::Parameter<sead::Vector3f> mAt;
    utl::Parameter<sead::Vector3f> mUp;
    utl::Parameter<f32> mFovy;
    utl::Parameter<sead::Vector3f> mAtOffset;
};

}  // namespace

sead::DirectCamera Layer::sCameraIdentity;
sead::OrthoProjection Layer::sProjectionIdentity;

/**
 * Constructs a layer with default settings.
 */
Layer::Layer()
{
    mJobDraw = nullptr;
    mJobSubDraw = nullptr;
    mJobGPUCalc = nullptr;
    mDebugInfo = nullptr;
    sCameraIdentity.updateViewMatrix();
}

/**
 * Unregisters the layer and frees its render steps and debug information.
 */
Layer::~Layer()
{
    mRenderer->removeLayer(this);
    mRenderStep.freeBuffer();

    if (mDebugInfo)
    {
        mDebugInfo->mInfoText.freeBuffer();
    }

    delete mDebugInfo;
}

/**
 * Allocates the render steps and the debug information.
 * @param pHeap heap to allocate from
 */
void Layer::initialize_(sead::Heap* pHeap)
{
    mRenderStep.tryAllocBuffer(getRenderStepNum(), pHeap);

    for (auto it = mRenderStep.begin(), itEnd = mRenderStep.end(); it != itEnd; ++it)
    {
        getRenderStepName(it.getIndex());
    }

    initializeImpl(pHeap);

    sead::Heap* pDebugHeap = detail::PrivateResource::instance()->getDebugHeap();

    if (pDebugHeap)
    {
        mDebugInfo = new (pDebugHeap) DebugInfo();
        mDebugInfo->mInfoText.tryAllocBuffer(2, pDebugHeap);
        updateDebugInfo_(3);
        mDebugInfo->mText = sead::SafeString::cEmptyString.getStringTop();
    }
}

/**
 * Updates the debug text describing the camera and the projection.
 * @param flag 1 to update the camera text, 2 to update the projection text
 */
void Layer::updateDebugInfo_(u32 flag)
{
    if (flag & 1)
    {
        const sead::Camera* pCamera = mCamera ? mCamera : &sCameraIdentity;

        if (mDebugFlag.isOn(1 << 3))
        {
            pCamera = &mDebugInfo->mCamera;
        }

        sead::Vector3f pos;
        sead::Vector3f up;
        sead::Vector3f at;
        pCamera->getWorldPosByMatrix(&pos);
        pCamera->getUpVectorByMatrix(&up);

        if (sead::IsDerivedFrom<sead::LookAtCamera>(pCamera))
        {
            at = static_cast<const sead::LookAtCamera*>(pCamera)->getAt();
        }
        else
        {
            pCamera->getLookVectorByMatrix(&at);
            at = pos - at * getDebugCameraAtDist();
        }

        mDebugInfo->mInfoText(0).format(
            "Position  ( %f, %f, %f )\nAttention ( %f, %f, %f )\nUp Vector ( %f, %f, %f )", pos.x,
            pos.y, pos.z, at.x, at.y, at.z, up.x, up.y, up.z);
    }

    if (flag & 2)
    {
        const sead::Projection* pProjection = getRenderProjection();

        if (sead::IsDerivedFrom<sead::PerspectiveProjection>(pProjection))
        {
            mDebugInfo->mInfoText[1].format("PespectiveProjection");
        }
        else if (sead::IsDerivedFrom<sead::FrustumProjection>(pProjection))
        {
            mDebugInfo->mInfoText[1].format("FrustumProjection");
        }
        else if (sead::IsDerivedFrom<sead::OrthoProjection>(pProjection))
        {
            mDebugInfo->mInfoText[1].format("OrthoProjection");
        }
        else if (sead::IsDerivedFrom<sead::DirectProjection>(pProjection))
        {
            mDebugInfo->mInfoText[1].format("DirectProjection");
        }
        else
        {
            mDebugInfo->mInfoText[1].format("Unknown\n");
        }

        mDebugInfo->mInfoText[1].append("\n");

        f32 near;
        f32 far;
        f32 aspect;
        f32 fovy;
        sead::Vector2f offset;
        detail::SeadUtil::getNearFarAspectFovy(*pProjection, &near, &far, &aspect, &fovy,
                                               &offset);
        sead::FormatFixedSafeString<1024> str("Near:%f Far:%f\nAspect:%f Fovy:%f Offset:%f %f",
                                              near, far, aspect, sead::Mathf::rad2deg(fovy),
                                              offset.x, offset.y);
        mDebugInfo->mInfoText[1].append(str);
    }
}

/**
 * Copies the current camera to the debug camera.
 */
void Layer::copyCurrentCameraToDebugCamera_()
{
    if (!mCamera)
    {
        return;
    }

    sead::Vector3f pos;
    sead::Vector3f at;
    sead::Vector3f up;
    mCamera->getWorldPosByMatrix(&pos);
    mCamera->getUpVectorByMatrix(&up);

    if (const sead::LookAtCamera* pLookAtCamera = sead::DynamicCast<const sead::LookAtCamera>(mCamera))
    {
        at = pLookAtCamera->getAt();
        resetBoundDebugCameraAt();
    }
    else
    {
        mCamera->getLookVectorByMatrix(&at);
        at = pos - at * getDebugCameraAtDist();
        resetBoundDebugCameraAt();
    }

    if (!mDebugInfo)
    {
        return;
    }

    setBoundDebugCameraAt(at);

    if (!mDebugInfo)
    {
        return;
    }

    mDebugInfo->mDebugCamera.setPos(pos);
    mDebugInfo->mDebugCamera.setAt(at);
    mDebugInfo->mDebugCamera.setUp(up);
    mDebugInfo->mDebugCamera.normalizeUp();
    mDebugInfo->mDebugCamera.updateViewMatrix();
    mDebugInfo->mCamera = mDebugInfo->mDebugCamera;
}

/**
 * Gets the distance between the debug camera and its attention point.
 * @return distance, or 0 without debug information
 */
f32 Layer::getDebugCameraAtDist() const
{
    if (mDebugInfo)
    {
        return mDebugInfo->mAtDist;
    }

    return 0.0f;
}

/**
 * Resets the point the debug camera attention is bound to.
 */
void Layer::resetBoundDebugCameraAt()
{
    mDebugFlag.set(1 << 4);

    if (mDebugInfo)
    {
        mDebugInfo->mBoundAt = sead::Vector3f::zero;
    }
}

/**
 * Sets the point the debug camera attention is bound to.
 * @param rAt attention point
 */
void Layer::setBoundDebugCameraAt(const sead::Vector3f& rAt)
{
    if (mDebugInfo)
    {
        mDebugInfo->mBoundAt = rAt;
    }
}

/**
 * Copies the current projection parameters to the debug projection.
 */
void Layer::copyCurrentProjectionToDebugProjection_()
{
    if (!mProjection)
    {
        return;
    }

    f32 near;
    f32 far;
    f32 aspect;
    f32 fovy;
    sead::Vector2f offset;
    detail::SeadUtil::getNearFarAspectFovy(*mProjection, &near, &far, &aspect, &fovy, &offset);

    if (mDebugInfo)
    {
        setDebugFovyDeg(sead::Mathf::rad2deg(fovy));
        setDebugNear(near);
        setDebugFar(far);
        mDebugInfo->mOffset = offset;
        mDebugInfo->mAspect = aspect;
    }
}

/**
 * Adds a draw method to a render step.
 * @param renderStep render step index
 * @param pMethod draw method to add
 * @return the draw method, or nullptr if it was already registered
 */
DrawMethod* Layer::pushBackDrawMethod(u32 renderStep, DrawMethod* pMethod)
{
    return mRenderStep[renderStep].pushBack(pMethod) ? pMethod : nullptr;
}

/**
 * Adds a draw method to every render step.
 * @param pMethod draw method to add
 * @return the draw method, or nullptr if it was not added to any render step
 */
DrawMethod* Layer::pushBackDrawMethod(DrawMethod* pMethod)
{
    bool isPushed = false;

    for (auto it = mRenderStep.begin(), itEnd = mRenderStep.end(); it != itEnd; ++it)
    {
        isPushed |= pushBackDrawMethod(it.getIndex(), pMethod) != nullptr;
    }

    return isPushed ? pMethod : nullptr;
}

/**
 * Removes every draw method bound to an object from all render steps.
 * @param pObject object the draw methods are bound to
 * @return number of removed draw methods
 */
s32 Layer::removeDrawMethodByObject(const void* pObject)
{
    s32 count = 0;

    for (auto& rStep : mRenderStep)
    {
        count += rStep.removeByObject(pObject);
    }

    return count;
}

/**
 * Removes a draw method from all render steps.
 * @param pMethod draw method to remove
 * @return number of removed draw methods
 */
s32 Layer::removeDrawMethod(const DrawMethod* pMethod)
{
    s32 count = 0;

    for (auto& rStep : mRenderStep)
    {
        count += rStep.remove(pMethod);
    }

    return count;
}

/**
 * Removes a draw method from a render step.
 * @param renderStep render step index
 * @param pMethod draw method to remove
 * @return number of removed draw methods
 */
s32 Layer::removeDrawMethod(u32 renderStep, const DrawMethod* pMethod)
{
    return mRenderStep[renderStep].remove(pMethod);
}

/**
 * Removes all draw methods.
 */
void Layer::clearDrawMethod()
{
    for (auto& rStep : mRenderStep)
    {
        rStep.clear();
    }
}

/**
 * Gets the camera used to draw the layer.
 * @return camera
 */
const sead::Camera* Layer::getRenderCamera() const
{
    if (mDebugFlag.isOn(1 << 1) && (mDebugFlag & 0x21) != 0 && mDebugInfo)
    {
        return &mDebugInfo->mCamera;
    }

    return mCamera ? mCamera : &sCameraIdentity;
}

/**
 * Gets the projection used to draw the layer.
 * @return projection
 */
const sead::Projection* Layer::getRenderProjection() const
{
    if ((mDebugDrawFlag & 2) && mDebugInfo)
    {
        return mDebugInfo->mProjection;
    }

    return mProjection ? mProjection : &sProjectionIdentity;
}

/**
 * Gets the logical frame buffer of the display the layer is drawn on.
 * @return logical frame buffer
 */
sead::LogicalFrameBuffer* Layer::getLogicalFrameBuffer() const
{
    return mRenderer->mDisplay[mDisplayTypeOverride == -1 ? mDisplayType : mDisplayTypeOverride]
        ->getLogicalFrameBuffer();
}

/**
 * Records the largest display list size of the layer.
 * @param size size of the last display list
 */
void Layer::setLastDisplayListSize(u64 size) const
{
    if (mLastDisplayListSize < size)
    {
        mLastDisplayListSize = size;
    }
}

/**
 * Enables or disables the layer.
 * @param enable whether the layer is enabled
 */
void Layer::setEnable(bool enable)
{
    if (isEnable() == enable)
    {
        return;
    }

    mRenderer->lockLayerList_();
    mFlag.change(cFlag_Enable, enable);
    mFlag.set(cFlag_ListDirty);
    mRenderer->unlockLayerList_();
}

/**
 * Shows or hides the layer.
 * @param visible whether the layer is visible
 */
void Layer::setVisible(bool visible)
{
    if (isVisible() == visible)
    {
        return;
    }

    mRenderer->lockLayerList_();
    mFlag.change(cFlag_Visible, visible);
    mRenderer->unlockLayerList_();
}

/**
 * Sets the display the layer is drawn on.
 * @param displayType display type
 */
void Layer::setDisplayType(s32 displayType)
{
    if (mDisplayType == displayType)
    {
        return;
    }

    mRenderer->lockLayerList_();
    mDisplayType = displayType;
    mFlag.set(cFlag_ListDirty);
    mRenderer->unlockLayerList_();
}

/**
 * Updates the debug camera, the viewport and the render steps.
 * @param pController controller operating the debug camera, or nullptr
 * @param controllerIndex camera control type
 * @param isDebugCameraTwist whether the debug camera twist is applied
 */
void Layer::calc_(const sead::Controller* pController, s32 controllerIndex,
                  bool isDebugCameraTwist)
{
    if (mDebugInfo)
    {
        if (mDebugInfo->mMessageTimer != 0)
        {
            mFlag.change(1 << 4, mDebugInfo->mMessageTimer % 20 > 9);

            if (--mDebugInfo->mMessageTimer == 0)
            {
                mFlag.reset(1 << 4);
            }
        }

        if (mFlag.isOn(cFlag_Initialized))
        {
            DebugInfo* pDebugInfo = mDebugInfo;
            sead::FormatFixedSafeString<1024> meta(
                "%s%sOrder = %d", pDebugInfo->mText.cstr(),
                mDebugInfo->mText.isEmpty() ? "" : ", ", ~mLayerIndex);
            detail::RootNode::setNodeMeta(this, meta);
            mFlag.reset(cFlag_Initialized);
        }

        if (mDebugFlag.isOn(1 << 1))
        {
            if (mDebugFlag.isOn(1 << 0))
            {
                if (pController)
                {
                    u32 hold = pController->getHoldMask();
                    u32 trig = pController->getTrigMask();
                    u32 repeat = pController->getRepeatMask();

                    if (((trig & 0x4000) && (hold & 0x2000)) ||
                        ((trig & 0x2000) && (hold & 0x4000)))
                    {
                        mDebugInfo->mTwist = 0.0f;
                        mDebugInfo->mFlag.set(1);
                    }
                    else if (((hold >> 14) & 1) == ((hold & 0x2000) >> 13))
                    {
                        if ((hold & 0x6000) == 0)
                        {
                            mDebugInfo->mFlag.reset(1);
                        }
                    }
                    else if (!mDebugInfo->mFlag.isOn(1))
                    {
                        u32 pressed = repeat | trig;
                        f32 delta = (pressed & 0x4000) ? -sead::Mathf::deg2rad(1.0f) : 0.0f;
                        delta = (pressed & 0x2000) ? sead::Mathf::deg2rad(1.0f) : delta;
                        f32 twist = delta + mDebugInfo->mTwist;
                        twist -= sead::Mathf::floor(twist * (1.0f / (2 * sead::Mathf::pi()))) *
                                 (2 * sead::Mathf::pi());
                        mDebugInfo->mTwist = twist < 2 * sead::Mathf::pi() ? twist : 0.0f;
                    }

                    f32 twist = isDebugCameraTwist ? mDebugInfo->mTwist : 0.0f;

                    if (controllerIndex == 1)
                    {
                        utl::DevTools::controlCamera(&mDebugInfo->mDebugCamera, *pController,
                                                     twist, utl::DevTools::CameraControlType(1));
                    }
                    else if (controllerIndex == 0)
                    {
                        utl::DevTools::controlCamera(&mDebugInfo->mDebugCamera, *pController,
                                                     twist, utl::DevTools::CameraControlType(0));
                    }
                }

                utl::DevTools::controlCameraPointer(
                    &mDebugInfo->mDebugCamera,
                    reinterpret_cast<utl::DevTools::PoiningControlBuffer*>(
                        mDebugInfo->mPointerBuffer),
                    mDisplayViewport);

                if (mDebugFlag.isOn(1 << 4))
                {
                    sead::LookAtCamera& rCamera = mDebugInfo->mDebugCamera;
                    rCamera.setPos(rCamera.getPos() - rCamera.getAt());
                    rCamera.setAt(sead::Vector3f::zero);
                }

                if (mDebugFlag.isOn(1 << 8))
                {
                    DebugInfo* pInfo = mDebugInfo;
                    sead::Random* pRandom = sead::GlobalRandom::instance();
                    f32 x = pInfo->mRandomRange * pRandom->getF32();
                    f32 y = mDebugInfo->mRandomRange * pRandom->getF32();
                    f32 z = mDebugInfo->mRandomRange * pRandom->getF32();
                    pInfo->mDebugCamera.setPos(sead::Vector3f(x, y, z));
                    sead::Vector3f dir(pRandom->getF32() - 0.5f, pRandom->getF32() - 0.5f,
                                       pRandom->getF32() - 0.5f);
                    dir.normalize();
                    mDebugInfo->mDebugCamera.setAt(mDebugInfo->mDebugCamera.getPos() + dir);
                }

                mDebugInfo->mDebugCamera.updateViewMatrix();
                mDebugInfo->mCamera.setAt(mDebugInfo->mDebugCamera.getAt() +
                                          mDebugInfo->mBoundAt);
                mDebugInfo->mCamera.setPos(mDebugInfo->mDebugCamera.getPos() +
                                           mDebugInfo->mBoundAt);
                DebugInfo* pInfo = mDebugInfo;
                pInfo->mCamera.setUp(pInfo->mDebugCamera.getUp());
                pInfo->mCamera.getUp().normalize();
                mDebugInfo->mCamera.updateViewMatrix();
            }

            if (mDebugFlag.isOn(1 << 5))
            {
                sead::LookAtCamera* pCamera = sead::DynamicCast<sead::LookAtCamera>(mCamera);

                if (pCamera)
                {
                    f32 rotY = sead::Mathf::deg2rad(mDebugInfo->mRotateXDeg);
                    f32 rotX = sead::Mathf::deg2rad(mDebugInfo->mRotateYDeg);
                    f32 cosY = std::cos(rotY);
                    f32 sinY = std::sin(rotY);
                    f32 x = cosY * std::sin(rotX);
                    f32 z = cosY * std::cos(rotX);
                    static_cast<sead::Camera&>(mDebugInfo->mCamera) = *pCamera;
                    mDebugInfo->mCamera.setPos(pCamera->getPos());
                    mDebugInfo->mCamera.setAt(pCamera->getAt());
                    mDebugInfo->mCamera.setUp(pCamera->getUp());
                    mDebugInfo->mCamera.setAt(mDebugInfo->mRotateCenter);
                    f32 dist = mDebugInfo->mRotateDist;
                    mDebugInfo->mCamera.setPos(sead::Vector3f(
                        mDebugInfo->mRotateCenter.x + x * dist,
                        sinY * dist + mDebugInfo->mRotateCenter.y,
                        z * dist + mDebugInfo->mRotateCenter.z));
                    mDebugInfo->mCamera.updateViewMatrix();
                    pCamera->setAt(mDebugInfo->mCamera.getAt());
                    pCamera->setPos(mDebugInfo->mCamera.getPos());
                    pCamera->setUp(mDebugInfo->mCamera.getUp());
                    pCamera->getUp().normalize();
                    pCamera->updateViewMatrix();
                }
            }
        }

        if (mProjection)
        {
            if (!(mDebugDrawFlag & 2))
            {
                f32 near;
                f32 far;
                f32 aspect;
                f32 fovy;
                sead::Vector2f offset;
                detail::SeadUtil::getNearFarAspectFovy(*mProjection, &near, &far, &aspect, &fovy,
                                                       &offset);
                if (mDebugInfo)
                {
                    setDebugFovyDeg(sead::Mathf::rad2deg(fovy));
                    setDebugNear(near);
                    setDebugFar(far);
                    mDebugInfo->mOffset = offset;
                    mDebugInfo->mAspect = aspect;
                }
            }
            else if (mProjection->getProjectionType() == 1)
            {
                auto* pOrtho = sead::DynamicCast<sead::OrthoProjection>(mProjection);
                mDebugInfo->mOrthoProjection = *pOrtho;
                mDebugInfo->mOrthoProjection.setNear(mDebugInfo->mNear);
                mDebugInfo->mOrthoProjection.setFar(mDebugInfo->mFar);
                mDebugInfo->mProjection = &mDebugInfo->mOrthoProjection;
            }
            else
            {
                auto* pPerspective = sead::DynamicCast<sead::PerspectiveProjection>(mProjection);

                if (pPerspective)
                {
                    pPerspective = sead::DynamicCast<sead::PerspectiveProjection>(mProjection);
                    mDebugInfo->mPerspectiveProjection = *pPerspective;
                    mDebugInfo->mPerspectiveProjection.setNear(mDebugInfo->mNear);
                    mDebugInfo->mPerspectiveProjection.setFar(mDebugInfo->mFar);
                    mDebugInfo->mPerspectiveProjection.setFovy(
                        sead::Mathf::deg2rad(mDebugInfo->mFovyDeg));
                    mDebugInfo->mPerspectiveProjection.setOffset(mDebugInfo->mOffset);
                    mDebugInfo->mPerspectiveProjection.setAspect(mDebugInfo->mAspect);

                    if (mDebugDrawFlag & 4)
                    {
                        pPerspective->setNear(mDebugInfo->mPerspectiveProjection.getNear());
                        pPerspective->setFar(mDebugInfo->mPerspectiveProjection.getFar());
                        pPerspective->setFovy(mDebugInfo->mPerspectiveProjection.getFovy());
                        pPerspective->setOffset(mDebugInfo->mPerspectiveProjection.getOffsetDirect());
                    }

                    mDebugInfo->mProjection = &mDebugInfo->mPerspectiveProjection;
                }
                else
                {
                    mDebugInfo->mDirectProjection.setProjectionMatrix(
                        mProjection->getProjectionMatrix(), sead::Graphics::DevicePosture(0));
                    mDebugInfo->mProjection = &mDebugInfo->mDirectProjection;
                }
            }
        }

        mDebugFlag.reset(1 << 4);

        if (mFlag.isOn(1 << 3))
        {
            updateDebugInfo_(3);
            mFlag.reset(1 << 3);
        }
        else
        {
            if (mDebugFlag.isOn(1 << 2) && (mDebugInfo->mFrame & 3) == 0)
            {
                updateDebugInfo_(1);
            }

            if ((mDebugDrawFlag & 1) && (mDebugInfo->mFrame & 3) == 2)
            {
                updateDebugInfo_(2);
            }
        }

        mDebugInfo->mFrame++;

        if (mFlag.isOn(1 << 9))
        {
            mDisplayViewport.setMin(mDebugInfo->mViewportPos);
            mDisplayViewport.setMax(mDebugInfo->mViewportPos + mDebugInfo->mViewportSize);
        }
        else
        {
            mDebugInfo->mViewportPos.x = mDisplayViewport.getMin().x;
            mDebugInfo->mViewportPos.y = mDisplayViewport.getMin().y;
            mDebugInfo->mViewportSize.x = mDisplayViewport.getSizeX();
            mDebugInfo->mViewportSize.y = mDisplayViewport.getSizeY();
        }
    }

    mViewport = mDisplayViewport;

    if (_96 != 0)
    {
        static const f32 cScale[] = {1.0f, 0.5f, 0.25f, 0.125f, 0.0625f, 0.03125f};
        f32 scale = cScale[_96] * 0.5f;
        sead::Vector2f center((mDisplayViewport.getMax().x + mDisplayViewport.getMin().x) * 0.5f,
                              (mDisplayViewport.getMax().y + mDisplayViewport.getMin().y) * 0.5f);
        f32 halfX = scale * (mDisplayViewport.getMax().x - mDisplayViewport.getMin().x);
        f32 halfY = scale * (mDisplayViewport.getMax().y - mDisplayViewport.getMin().y);
        mViewport.set(center.x - halfX, center.y - halfY, center.x + halfX, center.y + halfY);
    }

    for (auto& rStep : mRenderStep)
    {
        rStep.calc();
    }

    calcImpl();
}

/**
 * Notifies the layer that the display lists were recorded.
 */
void Layer::postCalcCommand_()
{
    postCalcCommandImpl();
}

/**
 * Checks whether the layer is drawn.
 * @return true if the layer is enabled, visible and not blinking
 */
bool Layer::isRenderingEnabled() const
{
    if (!mFlag.isOnAll(cFlag_Visible | cFlag_Enable))
    {
        return false;
    }

    if (isForceInvisible())
    {
        return false;
    }

    return !mFlag.isOn(1 << 4);
}

/**
 * Clears the frame buffer with the layer clear settings.
 * @param rInfo render information
 */
void Layer::clearColor_(const RenderInfo& rInfo) const
{
    if (isForceDisableClear())
    {
        return;
    }

    if (mClearFlag == 0)
    {
        return;
    }

    const RenderBuffer* pFrameBuffer = rInfo.mFrameBuffer;

    if (!pFrameBuffer)
    {
        return;
    }

    pFrameBuffer->fastClear(rInfo.getDrawContext(), 0,
                            (mClearFlag & 1) | ((mClearFlag >> 1) & 2), mClearColor, mClearDepth,
                            0, *rInfo.getViewport(), true);
}

/**
 * Draws a render step.
 * @param rInfo render information containing the render step index
 */
void Layer::drawRenderStep_(const RenderInfo& rInfo) const
{
    const RenderStep& rStep = mRenderStep[rInfo.getRenderStep()];

    if (!rStep.isEnable())
    {
        return;
    }

    preDrawRenderStepImpl(rInfo);

    for (auto it = rStep.getDrawMethods().begin(), itEnd = rStep.getDrawMethods().end();
         it != itEnd; ++it)
    {
        preDrawRenderStepMethodImpl(rInfo, *it);
        it->invoke(rInfo);
        postDrawRenderStepMethodImpl(rInfo, *it);
    }

    postDrawRenderStepImpl(rInfo);
}

/**
 * Draws the frustum and attention point axis of the layer camera while the debug camera is used.
 * @param pDrawContext draw context
 */
void Layer::drawDebugCamera(DrawContext* pDrawContext) const
{
    if (!(mDebugInfo && mDebugFlag.isOn(1 << 1) && (mDebugFlag & 0x21) != 0 && mCamera &&
          (mDebugFlag & 0xc0) != 0))
    {
        return;
    }

    const sead::Vector3f& rAt = mDebugInfo->mCamera.getAt();
    sead::Matrix34f mtx(1.0f, 0.0f, 0.0f, rAt.x, 0.0f, 1.0f, 0.0f, rAt.y, 0.0f, 0.0f, 1.0f, rAt.z);

    for (s32 i = 0; i < 2; i++)
    {
        sead::GraphicsContext context;
        context.setColorMask(0xf);
        context.setDepthEnable(true, false);
        context.setDepthFunc(i == 0 ? 4 : 5);
        context.apply(pDrawContext);

        f32 alpha = i == 0 ? 1.0f : mDebugInfo->_384;

        if (mDebugFlag.isOn(1 << 6))
        {
            sead::Color4f color(1.0f, 1.0f, 1.0f, alpha);
            utl::DevTools::drawCameraAndFrustum(pDrawContext, *mCamera, *mProjection,
                                                getRenderCamera()->getMatrix(),
                                                getRenderProjection()->getProjectionMatrix(), true,
                                                color, color, mDebugInfo->_380);
        }

        if (mDebugFlag.isOn(1 << 7))
        {
            utl::DevTools::beginDrawImm(pDrawContext, getRenderCamera()->getMatrix(),
                                        getRenderProjection()->getProjectionMatrix());
            utl::DevTools::drawAxisImm(pDrawContext, mtx, utl::DevTools::calcScale(1.0f), 1.0f,
                                       alpha);
        }
    }
}

/**
 * Draws the debug information of the layer (no-op in release builds).
 * @param rInfo render information
 */
void Layer::drawDebugInfo_(const RenderInfo& rInfo) const {}

/**
 * Sets the distance between the debug camera and its attention point.
 * @param dist distance
 */
void Layer::setDebugCameraAtDist(f32 dist)
{
    if (mDebugInfo)
    {
        mDebugInfo->mAtDist = dist;
    }
}

/**
 * Sets the twist of the debug camera.
 * @param twist twist in radians
 */
void Layer::setDebugCameraTwist(f32 twist)
{
    if (mDebugInfo)
    {
        mDebugInfo->mTwist = twist;
    }
}

/**
 * Gets the twist of the debug camera.
 * @return twist in radians, or 0 without debug information
 */
f32 Layer::getDebugCameraTwist()
{
    if (mDebugInfo)
    {
        return mDebugInfo->mTwist;
    }

    return 0.0f;
}

/**
 * Gets the debug camera.
 * @return debug camera, or nullptr without debug information
 */
sead::LookAtCamera* Layer::getDebugCameraPtr()
{
    return mDebugInfo ? &mDebugInfo->mDebugCamera : nullptr;
}

/**
 * Sets the near clip distance of the debug projection.
 * @param near near clip distance
 */
void Layer::setDebugNear(f32 near)
{
    if (mDebugInfo)
    {
        mDebugInfo->mNear = near;
    }
}

/**
 * Sets the far clip distance of the debug projection.
 * @param far far clip distance
 */
void Layer::setDebugFar(f32 far)
{
    if (mDebugInfo)
    {
        mDebugInfo->mFar = far;
    }
}

/**
 * Sets the vertical field of view of the debug projection.
 * @param fovyDeg field of view in degrees
 */
void Layer::setDebugFovyDeg(f32 fovyDeg)
{
    if (mDebugInfo)
    {
        mDebugInfo->mFovyDeg = fovyDeg;
    }
}

/**
 * Generates the host I/O messages of the layer.
 * @param pContext host I/O context
 */
void Layer::genMessage(sead::hostio::Context* pContext)
{
    {
        sead::FormatFixedSafeString<1024> str("規定：(%s)",
                                              Renderer::getDisplayName(mDisplayType).cstr());
    }

    {
        sead::SafeString name = Renderer::getDisplayName(0);
    }

    {
        sead::SafeString name = Renderer::getDisplayName(1);
    }

    {
        sead::FormatFixedSafeString<1024> str("ID:%d", mLayerIndex);
    }

    {
        sead::FormatFixedSafeString<1024> str("GroupHeader=Save/load debug camera, Dir=X");
    }

    genMessageCamera(pContext);

    f32 min;

    if (mProjection)
    {
        static const f32 cMin[] = {-1000.0f, 0.1f};
        min = cMin[mProjection->getProjectionType() == 0];
    }
    else
    {
        min = -1000.0f;
    }

    {
        auto str = utl::DevTools::getStringMinMax(min, 1000.0f);
    }

    {
        auto str = utl::DevTools::getStringMinMax(min, 1000.0f);
    }

    {
        auto str = utl::DevTools::getStringMinMax(0.0f, 200.0f);
    }

    {
        auto str = utl::DevTools::getStringMinMax(0.0f, 200.0f);
    }

    {
        sead::FormatFixedSafeString<1024> str(
            "pos  ( %.1f, %.1f )\nsize ( %.1f  %.1f )", mDisplayViewport.getMin().x,
            mDisplayViewport.getMin().y, mDisplayViewport.getSizeX(), mDisplayViewport.getSizeY());
    }

    {
        sead::FormatFixedSafeString<1024> str("Max download size used:%d[byte]",
                                              mLastDisplayListSize);
    }
}

/**
 * Generates the host I/O messages of the debug camera.
 * @param pContext host I/O context
 */
void Layer::genMessageCamera(sead::hostio::Context* pContext)
{
    {
        auto str = utl::DevTools::getStringMinMax(0.01f, 1.0f);
    }

    {
        sead::FormatFixedSafeString<1024> str("GroupHeader=デバッグ, IsEnable=%s",
                                              mCamera ? "true" : "false");
    }

    {
        auto str = utl::DevTools::getStringMinMax(0.01f, 10.0f);
    }

    const char* pIsEnable = mDebugFlag.isOn(1 << 5) ? "true" : "false";
    {
        sead::FormatFixedSafeString<1024> str("GroupHeader=observation point,IsEnable=%s",
                                              pIsEnable);
    }

    {
        auto str = utl::DevTools::getStringMinMax(-100.0f, 100.0f);
    }

    {
        auto str = utl::DevTools::getStringMinMax(-100.0f, 100.0f);
    }

    {
        auto str = utl::DevTools::getStringMinMax(-100.0f, 100.0f);
    }

    {
        sead::FormatFixedSafeString<1024> str("GroupHeader= position and angle, IsEnable=%s",
                                              pIsEnable);
    }

    {
        auto str = utl::DevTools::getStringMinMax(0.0f, 100.0f);
    }

    {
        auto str = utl::DevTools::getStringMinMax(0.0f, 100.0f);
    }
}

/**
 * Generates the host I/O messages of the debug projection.
 * @param pContext host I/O context
 */
void Layer::genMessageProjection(sead::hostio::Context* pContext)
{
    f32 min;

    if (mProjection)
    {
        static const f32 cMin[] = {-1000.0f, 0.1f};
        min = cMin[mProjection->getProjectionType() == 0];
    }
    else
    {
        min = -1000.0f;
    }

    {
        auto str = utl::DevTools::getStringMinMax(min, 1000.0f);
    }

    {
        auto str = utl::DevTools::getStringMinMax(min, 1000.0f);
    }
}

/**
 * Handles a host I/O property event.
 * @param pEvent property event
 */
void Layer::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    switch (reinterpret_cast<uintptr_t>(pEvent->getId()))
    {
    case 10000:
        mFlag.change(1 << 11, !mFlag.isOn(1 << 11));
        mFlag.set(cFlag_ListDirty);
        break;
    case 10001:
        updateDebugInfo_(1);
        break;
    case 10004:
    {
        CameraIO io;
        io.mPos = mDebugInfo->mDebugCamera.getPos();
        io.mAt = mDebugInfo->mDebugCamera.getAt();
        io.mUp = mDebugInfo->mDebugCamera.getUp();
        io.mFovy = mDebugInfo->mFovyDeg;
        io.mAtOffset = mDebugInfo->mBoundAt;
        io.save(sead::SafeString::cEmptyString, 0x2000000);
        break;
    }
    case 10005:
    {
        CameraIO io;
        io.load(sead::SafeString::cEmptyString, false);
        mDebugInfo->mDebugCamera.setPos(*io.mPos);
        mDebugInfo->mDebugCamera.setAt(*io.mAt);
        DebugInfo* pInfo = mDebugInfo;
        pInfo->mDebugCamera.setUp(*io.mUp);
        pInfo->mDebugCamera.getUp().normalize();
        mDebugInfo->mDebugCamera.updateViewMatrix();
        mDebugInfo->mFovyDeg = *io.mFovy;
        mDebugInfo->mBoundAt = *io.mAtOffset;
        mDebugFlag.reset(1 << 4);
        break;
    }
    case 10006:
    {
        CameraIO io;
        io.load(sead::SafeString::cEmptyString, false);
        mDebugInfo->mDebugCamera.setPos(*io.mPos);
        mDebugInfo->mDebugCamera.setAt(*io.mAt);
        DebugInfo* pInfo = mDebugInfo;
        pInfo->mDebugCamera.setUp(*io.mUp);
        pInfo->mDebugCamera.getUp().normalize();
        mDebugInfo->mDebugCamera.updateViewMatrix();
        mDebugInfo->mFovyDeg = *io.mFovy;
        mDebugFlag.reset(1 << 4);
        break;
    }
    case 10007:
        mDebugInfo->mAspect = mDisplayViewport.getSizeX() / mDisplayViewport.getSizeY();
        break;
    }
}

/**
 * Handles a host I/O property event of the camera node (no-op in release builds).
 * @param pReflexible node the event was sent to
 * @param pEvent property event
 */
void Layer::listenPropertyEventCamera(sead::hostio::Reflexible* pReflexible,
                                      const sead::hostio::PropertyEvent* pEvent)
{
}

/**
 * Handles a host I/O property event of the projection node (no-op in release builds).
 * @param pReflexible node the event was sent to
 * @param pEvent property event
 */
void Layer::listenPropertyEventProjection(sead::hostio::Reflexible* pReflexible,
                                          const sead::hostio::PropertyEvent* pEvent)
{
}

/**
 * Handles a host I/O node event.
 * @param pEvent node event
 */
void Layer::listenNodeEvent(const sead::hostio::NodeEvent* pEvent)
{
    u32 id = pEvent->getId();

    if (id == 0)
    {
        mDebugInfo->mMessageTimer = 60;
    }
    else if (id == reinterpret_cast<uintptr_t>(&mDisplayTypeOverride))
    {
        mFlag.set(cFlag_ListDirty);
    }
}

/**
 * Constructs the debug information with default debug camera and projection settings.
 */
Layer::DebugInfo::DebugInfo()
    : mMessageTimer(0), mAtDist(utl::DevTools::calcScale(1.0f)), mTwist(0.0f),
      mBoundAt(sead::Vector3f::zero), mNear(utl::DevTools::calcScale(0.01f)),
      mFar(utl::DevTools::calcScale(1000.0f)), mFovyDeg(45.0f), mOffset(0.0f, 0.0f),
      mAspect(16.0f / 9.0f), mProjection(&mPerspectiveProjection), mViewportPos(0.0f, 0.0f),
      mViewportSize(1.0f, 1.0f), _380(utl::DevTools::calcScale(0.1f)), _384(0.1f),
      mRotateCenter(0.0f, 0.0f, 0.0f), mRotateDist(utl::DevTools::calcScale(10.0f)),
      mRotateYDeg(0.0f), mRotateXDeg(0.0f), mFrame(0),
      mRandomRange(utl::DevTools::calcScale(10.0f)), mFlag(0)
{
    for (auto& rByte : mPointerBuffer)
    {
        rByte = 0;
    }

    mCamera.setPos(sead::Vector3f::zero);
    mCamera.setAt(-sead::Vector3f::ez);
    mCamera.updateViewMatrix();
    mDebugCamera = mCamera;
}

}  // namespace agl::lyr
