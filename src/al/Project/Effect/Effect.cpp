#include "Project/Effect/Effect.hpp"

#include <nn/util/util_MatrixApi.h>
#include <nn/util/util_VectorApi.h>
#include <nn/vfx/EmitterSet.h>

#include "Library/Effect/EffectSystem.hpp"
#include "Library/Effect/EffectSystemInfo.hpp"
#include "Library/Effect/EmitterSetResourceInfoHolder.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Math/MatrixUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Effect/EffectCameraHolder.hpp"
#include "Project/Effect/EffectEmitter.hpp"
#include "Project/Effect/EffectInfo.hpp"

#include <ptcl/seadPtclSystem.h>

namespace al {
namespace {
bool isEqualStringOrBothNull(const char* pStr1, const char* pStr2) {
    if (pStr1 == nullptr && pStr2 == nullptr) {
        return true;
    }

    if (pStr1 == nullptr || pStr2 == nullptr) {
        return false;
    }

    return isEqualString(pStr1, pStr2);
}

void loadMatrix(nn::util::Matrix4x3fType* pOut, const sead::Matrix34f& rMtx) {
    nn::util::MatrixLoad(pOut, reinterpret_cast<const nn::util::FloatColumnMajor4x3&>(rMtx));
}

void loadVector(nn::util::Vector3fType* pOut, const sead::Vector3f& rVec) {
    nn::util::VectorLoad(pOut, reinterpret_cast<const nn::util::Float3&>(rVec));
}

void setEmitterSetMtx(nn::vfx::EmitterSet* pEmitterSet, const sead::Matrix34f& rMtx,
                      const sead::Vector3f& rScale) {
    nn::util::Matrix4x3fType mtx;
    loadMatrix(&mtx, rMtx);
    nn::util::Vector3fType scale;
    loadVector(&scale, rScale);
    pEmitterSet->SetMatrixAndScale(mtx, scale);
}

void getEmitterSetMtx(sead::Matrix34f* pOut, const nn::vfx::EmitterSet* pEmitterSet) {
    nn::util::MatrixStore(reinterpret_cast<nn::util::FloatColumnMajor4x3*>(pOut),
                          pEmitterSet->GetMatrixRt());
}

const sead::Vector3f& toVector3f(const nn::util::Vector3fType& rVec) {
    return reinterpret_cast<const sead::Vector3f&>(rVec);
}

const sead::Vector3f& toVector3f(const float32x4_t& rVec) {
    return reinterpret_cast<const sead::Vector3f&>(rVec);
}

void getEmitterSetRtMtx(sead::Matrix34f* pOut, const nn::vfx::EmitterSet* pEmitterSet) {
    const nn::util::Matrix4x3fType& rt = pEmitterSet->GetMatrixRt();
    pOut->setBase(0, toVector3f(rt._m.val[0]));
    pOut->setBase(1, toVector3f(rt._m.val[1]));
    pOut->setBase(2, toVector3f(rt._m.val[2]));
}

void addOffsetTrans(sead::Vector3f* pTrans, const EffectEmitParam* pParam,
                    const sead::Matrix34f& rMtx) {
    sead::Vector3f offset;
    offset.setMul(rMtx, pParam->mOffsetTrans);
    *pTrans += offset;
}

void calcEmitterScale(sead::Vector3f* pScale, const EffectEmitParam* pParam,
                      const sead::Vector3f* pScalePtr, JointMtxPtr mtxPtr, bool isFollowScale) {
    if (isFollowScale) {
        if (pScalePtr != nullptr && pParam->mJointName == nullptr) {
            *pScale = *pScalePtr;
        } else if (mtxPtr.isValid()) {
            mtxPtr.calcMtxScale(pScale);
        } else {
            pScale->set(sead::Vector3f::ones);
        }
    } else {
        pScale->set(sead::Vector3f::ones);
    }

    *pScale *= pParam->mScale;
}

/**
 * Calculates the emitter set position from the camera, the followed position or the joint.
 * @param pTrans Receives the position.
 * @param pParam Emit parameters.
 * @param pPos Followed position.
 * @param mtxPtr Followed joint matrix.
 * @param pCameraHolder Camera holder.
 * @param isKeepScale Whether the joint matrix keeps its scale for the offset.
 */
void calcEmitterTrans(sead::Vector3f* pTrans, const EffectEmitParam* pParam,
                      const sead::Vector3f* pPos, JointMtxPtr mtxPtr,
                      EffectCameraHolder* pCameraHolder, bool isKeepScale) {
    if (pParam->mIsFollowCamera) {
        *pTrans = pCameraHolder->getCameraPos();
    } else if (!mtxPtr.isValid() || pParam->mIsIgnoreJoint) {
        pTrans->set(*pPos);
    } else {
        mtxPtr.getTranslation(pTrans);
    }

    if (!pParam->mIsAddOffsetTrans) {
        return;
    }

    sead::Matrix34f mtx;

    if (pParam->mIsFollowCamera) {
        mtx.setInverse(*pCameraHolder->getViewMtxPtr());
        mtx.setTranslation(sead::Vector3f::zero);
    } else if (mtxPtr.isValid()) {
        mtxPtr.copyTo(&mtx);

        if (!isKeepScale) {
            normalizeMtxScale(&mtx, mtx);
        }

        mtx.setTranslation(sead::Vector3f::zero);
    } else {
        mtx.makeIdentity();
    }

    addOffsetTrans(pTrans, pParam, mtx);
}

void resetMtxRotate(sead::Matrix34f* pMtx, const JointMtxPtr& rMtxPtr) {
    rMtxPtr.copyTo(pMtx);
    pMtx->setTranslation(sead::Vector3f::zero);
}

/**
 * Calculates the emitter set matrix from the joint, billboard and keep-front settings.
 * @param pMtx Receives the matrix.
 * @param pParam Emit parameters.
 * @param pTrans Emitter set position.
 * @param mtxPtr Followed joint matrix.
 * @param pCameraHolder Camera holder.
 * @param isFollowRotate Whether to follow the joint rotation.
 */
void calcEmitterMtx(sead::Matrix34f* pMtx, const EffectEmitParam* pParam,
                    const sead::Vector3f* pTrans, JointMtxPtr mtxPtr,
                    EffectCameraHolder* pCameraHolder, bool isFollowRotate) {
    if (mtxPtr.isValid() && isFollowRotate) {
        mtxPtr.copyTo(pMtx);
        tryNormalizeMtxScaleOrIdentity(pMtx, *pMtx);
        pMtx->setTranslation(sead::Vector3f::zero);
    } else {
        pMtx->makeIdentity();
    }

    if (pParam->mIsBillboard) {
        if (!pCameraHolder->tryMakeBillboardMtx(pMtx, *pTrans)) {
            resetMtxRotate(pMtx, mtxPtr);
        }
    } else if (pParam->mIsYBillboard) {
        if (!pCameraHolder->tryMakeYBillboardMtx(pMtx, *pTrans)) {
            resetMtxRotate(pMtx, mtxPtr);
        }
    }

    if (pParam->mIsFollowCamera && isFollowRotate) {
        if (!pCameraHolder->tryMakeCameraFrontMtx(pMtx)) {
            resetMtxRotate(pMtx, mtxPtr);
        }
    }

    if (pParam->mIsKeepFrontX || pParam->mIsKeepFrontY || pParam->mIsKeepFrontZ) {
        u8 keepNum = pParam->mIsKeepFrontX + pParam->mIsKeepFrontY + pParam->mIsKeepFrontZ;

        if (keepNum >= 2) {
            pMtx->makeIdentity();
        } else if (pParam->mIsKeepFrontX) {
            sead::Vector3f up = pMtx->getBase(1);

            if (isNearZero(up.y, 0.001f) && isNearZero(up.z, 0.001f)) {
                pMtx->makeIdentity();
            } else {
                makeMtxSideUp(pMtx, sead::Vector3f::ex, up);
            }
        } else if (pParam->mIsKeepFrontY) {
            sead::Vector3f front = pMtx->getBase(2);

            if (isNearZero(front.x, 0.001f) && isNearZero(front.z, 0.001f)) {
                pMtx->makeIdentity();
            } else {
                makeMtxUpFront(pMtx, sead::Vector3f::ey, front);
            }
        } else {
            sead::Vector3f up = pMtx->getBase(1);

            if (isNearZero(up.x, 0.001f) && isNearZero(up.y, 0.001f)) {
                pMtx->makeIdentity();
            } else {
                makeMtxFrontUp(pMtx, sead::Vector3f::ez, up);
            }
        }
    }

    if (pParam->mIsAddOffsetRotate) {
        sead::Vector3f rotate(sead::Mathf::deg2rad(pParam->mOffsetRotate.x),
                              sead::Mathf::deg2rad(pParam->mOffsetRotate.y),
                              sead::Mathf::deg2rad(pParam->mOffsetRotate.z));
        sead::Matrix34f rotateMtx;
        rotateMtx.makeR(rotate);
        pMtx->setMul(*pMtx, rotateMtx);
    }

    pMtx->setTranslation(*pTrans);
}

__attribute__((noinline)) void createEmitter(EffectEmitter* pEmitter, const EffectEmitParam* pParam,
                                             const sead::Vector3f* pPos,
                                             const sead::Vector3f* pScalePtr, JointMtxPtr mtxPtr,
                                             EffectCameraHolder* pCameraHolder, u64 userData,
                                             bool isPauseForceCalc) {
    s32 forceCalcFrame = pParam->mForceCalcFrame;

    if (isPauseForceCalc && pEmitter->isLoopOrInfinity() && forceCalcFrame == 0) {
        forceCalcFrame = EffectSystem::getPauseForceCalcFrame();
    }

    if (!mtxPtr.isValid() || pParam->mIsIgnoreJoint) {
        pEmitter->createEmitter(nullptr, pPos, pParam->mGroupId, forceCalcFrame, userData);
    } else {
        pEmitter->createEmitter(&mtxPtr, nullptr, pParam->mGroupId, forceCalcFrame, userData);
    }

    if (!pEmitter->isActive()) {
        return;
    }

    nn::vfx::EmitterSet* emitterSet = pEmitter->getHandle()->GetEmitterSet();

    if (!pParam->mIsBillboard && !pParam->mIsYBillboard && !pParam->mIsFollowCamera &&
        !pParam->mIsAddOffsetTrans && !pParam->mIsAddOffsetRotate &&
        !pParam->mIsFollowRotateOnEmit && !pParam->mIsFollowScaleOnEmit) {
        return;
    }

    sead::Vector3f trans;
    calcEmitterTrans(&trans, pParam, pPos, mtxPtr, pCameraHolder, pParam->mIsFollowScaleOnEmit);
    sead::Vector3f scale;
    sead::Matrix34f mtx;
    calcEmitterMtx(&mtx, pParam, &trans, mtxPtr, pCameraHolder, pParam->mIsFollowRotateOnEmit);

    calcEmitterScale(&scale, pParam, pScalePtr, mtxPtr, pParam->mIsFollowScaleOnEmit);
    setEmitterSetMtx(emitterSet, mtx, scale);
}

bool isExistMaterial(const Effect* pEffect, const sead::SafeString& rMaterialName) {
    for (s32 i = 0; i < pEffect->getEmitterNum(); i++) {
        const char* name = pEffect->getEmitter(i)->getResourceInfo()->mMaterialName;

        if (name != nullptr && isEqualString(name, rMaterialName)) {
            return true;
        }
    }

    return false;
}

bool isExistMaterial(const Effect* pEffect, const char* pMaterialName) {
    for (s32 i = 0; i < pEffect->getEmitterNum(); i++) {
        const char* name = pEffect->getEmitter(i)->getResourceInfo()->mMaterialName;

        if (name != nullptr && isEqualString(name, pMaterialName)) {
            return true;
        }
    }

    return false;
}

StringTmp<64> makeMaterialName(const Effect* pEffect, const char* pMaterialCode,
                               const bool (&rIsPrefix)[4]) {
    const char* materialCode = pMaterialCode != nullptr ? pMaterialCode : "";

    for (s32 i = 0; i < 4; i++) {
        if (rIsPrefix[i]) {
            StringTmp<64> name("%s%s", EffectPrefixType::text(i), materialCode);

            if (isExistMaterial(pEffect, name)) {
                return name;
            }
        }
    }

    for (s32 i = 0; i < 4; i++) {
        if (rIsPrefix[i]) {
            const char* prefix = EffectPrefixType::text(i);

            if (isExistMaterial(pEffect, prefix)) {
                return StringTmp<64>(prefix);
            }
        }
    }

    if (isExistMaterial(pEffect, pMaterialCode)) {
        return StringTmp<64>(pMaterialCode);
    }

    return StringTmp<64>("");
}

}  // namespace

/**
 * Creates an effect and its emitters from effect info.
 * @param pSystemInfo Effect system info.
 * @param pInfo Effect info.
 * @param pTrans Translation the effect follows.
 * @param pScale Scale the effect follows.
 * @param pMtx Matrix the effect follows when the effect has no joint.
 * @param userData User data given to the emitter sets.
 */
Effect::Effect(const EffectSystemInfo* pSystemInfo, const EffectInfo* pInfo, const sead::Vector3f* pTrans,
               const sead::Vector3f* pScale, const sead::Matrix34f* pMtx, u64 userData)
    : mName(pInfo->mName), mEmitters(nullptr), mEmitterNum(0),
      mEffectSystem(pSystemInfo->getEffectSystem()), mEffectInfo(pInfo), mPosPtr(pTrans),
      mScalePtr(pScale), mViewMtxPtr(nullptr), mCameraHolder(nullptr), mIsEmitted(false),
      mIsActorClip(false), mIsFarClip(false), _b8(0), mUserData(userData) {
    if (mEffectInfo->mParam.mJointName == nullptr) {
        mJointMtxPtr.set(pMtx);
    }

    mEmitterNum = pInfo->mEmitInfoNum;
    mEmitters = new EffectEmitter*[mEmitterNum];

    for (s32 i = 0; i < mEmitterNum; i++) {
        EffectResourceInfo* resourceInfo = &pInfo->mEmitInfos[i];
        mEmitters[i] = new EffectEmitter(pSystemInfo, resourceInfo, mEffectInfo->mParam.mHandleNum);
    }
}

/**
 * Sets the camera holder and caches its view matrix.
 * @param pCameraHolder Camera holder.
 */
void Effect::setCameraHolder(EffectCameraHolder* pCameraHolder) {
    mCameraHolder = pCameraHolder;
    mViewMtxPtr = pCameraHolder->getViewMtxPtr();
}

/**
 * Makes the effect follow a position.
 * @param pPos Position to follow.
 */
void Effect::setPosPtr(const sead::Vector3f* pPos) {
    mPosPtr = pPos;
    mJointMtxPtr.setNull();
    mEffectInfo->mParam.mIsSetPosPtr = true;
}

/**
 * Makes the effect follow a matrix.
 * @param pMtx Matrix to follow.
 */
void Effect::setMtxPtr(const sead::Matrix34f* pMtx) {
    mJointMtxPtr.set(pMtx);
    mPosPtr = nullptr;
    mEffectInfo->mParam.mIsSetMtxPtr = true;
}

bool Effect::update() {
    if (mEffectInfo->mParam.mFarClipDistance > 0.0f && mViewMtxPtr != nullptr) {
        sead::Vector3f cameraPos = mCameraHolder->getCameraPos();

        EffectEmitter* emitter = nullptr;
        bool isActive = false;

        for (s32 i = 0; i < mEmitterNum; i++) {
            emitter = mEmitters[i];
            isActive = emitter->isActive();

            if (isActive) {
                break;
            }
        }

        if (emitter == nullptr || !isActive) {
            emitter = mEmitters[0];
        }

        if (emitter != nullptr) {
            const EffectEmitParam* param = &mEffectInfo->mParam;
            bool isFollowScaleOnEmit = param->mIsFollowScaleOnEmit;
            JointMtxPtr mtxPtr = mJointMtxPtr;
            bool isFarClip = false;

            if (!param->mIsFollowCamera && !param->mIsIgnoreJoint) {
                sead::Vector3f trans;
                mtxPtr.getTranslation(&trans);

                if (param->mIsAddOffsetTrans) {
                    sead::Matrix34f mtx;

                    if (mtxPtr.isValid()) {
                        mtxPtr.copyTo(&mtx);

                        if (!isFollowScaleOnEmit) {
                            normalizeMtxScale(&mtx, mtx);
                        }

                        mtx.setTranslation(sead::Vector3f::zero);
                    } else {
                        mtx.makeIdentity();
                    }

                    addOffsetTrans(&trans, param, mtx);
                }

                f32 farClipDistance = param->mFarClipDistance * mCameraHolder->calcFarClipRateByFovy();
                isFarClip = farClipDistance * farClipDistance < (cameraPos - trans).squaredLength();
            }

            setFarClip(isFarClip);
        }
    }

    bool isUpdated = false;

    for (s32 i = 0; i < mEmitterNum; i++) {
        EffectEmitter* emitter = mEmitters[i];

        if (!emitter->isActive()) {
            continue;
        }

        const EffectEmitParam* param = &mEffectInfo->mParam;
        const sead::Vector3f* posPtr = mPosPtr;
        const sead::Vector3f* scalePtr = mScalePtr;
        JointMtxPtr mtxPtr = mJointMtxPtr;
        EffectCameraHolder* cameraHolder = mCameraHolder;

        bool isFollowTrans;
        bool isFollowRotate;
        bool isFollowScale;

        if (emitter->isFirstFrame()) {
            emitter->resetFirstFrame();
            isFollowTrans = param->mIsFollowTrans ||
                            (param->mIsFollowTransOnEmit && !param->mIsIgnoreJoint);
            isFollowRotate = param->mIsFollowRotate || param->mIsFollowRotateOnEmit;
            isFollowScale = param->mIsFollowScale || param->mIsFollowScaleOnEmit;
        } else {
            isFollowTrans = param->mIsFollowTrans;
            isFollowRotate = param->mIsFollowRotate;
            isFollowScale = param->mIsFollowScale;
        }

        bool isKeepScale = param->mIsFollowScale || param->mIsFollowScaleOnEmit;
        nn::vfx::EmitterSet* emitterSet = emitter->getHandle()->GetEmitterSet();

        if (param->mIsBillboard || param->mIsYBillboard || param->mIsFollowCamera || isFollowRotate) {
            sead::Vector3f scale(1.0f, 1.0f, 1.0f);
            sead::Vector3f trans;

            if (isFollowTrans) {
                calcEmitterTrans(&trans, param, posPtr, mtxPtr, cameraHolder, isKeepScale);
            } else {
                trans.set(toVector3f(emitterSet->GetMatrixRt()._m.val[3]));
            }

            sead::Matrix34f mtx;
            calcEmitterMtx(&mtx, param, &trans, mtxPtr, cameraHolder, isFollowRotate);

            calcEmitterScale(&scale, param, scalePtr, mtxPtr, isFollowScale);

            if (param->mIsFollowCameraFovy) {
                scale *= cameraHolder->calcFollowScaleByFovy();
            }

            setEmitterSetMtx(emitterSet, mtx, scale);
        } else if (isFollowScale || isFollowTrans || param->mIsFollowCameraFovy) {
            sead::Vector3f scale(1.0f, 1.0f, 1.0f);
            sead::Matrix34f mtx;
            getEmitterSetRtMtx(&mtx, emitterSet);
            sead::Vector3f trans;

            if (isFollowTrans) {
                calcEmitterTrans(&trans, param, posPtr, mtxPtr, cameraHolder, isKeepScale);
                mtx.setTranslation(trans);
            } else {
                mtx.setTranslation(toVector3f(emitterSet->GetMatrixRt()._m.val[3]));
            }

            if (isFollowScale) {
                calcEmitterScale(&scale, param, scalePtr, mtxPtr, isFollowScale);
            } else if (!param->mIsFollowCameraFovy) {
                const sead::Vector3f& scaleForCalc = toVector3f(emitterSet->GetParticleScaleForCalc());
                const sead::Vector3f& particleScale = toVector3f(emitterSet->GetParticleScale());
                scale.set(scaleForCalc.x / particleScale.x, scaleForCalc.y / particleScale.y,
                          scaleForCalc.z / particleScale.z);
            }

            if (param->mIsFollowCameraFovy) {
                scale *= cameraHolder->calcFollowScaleByFovy();
            }

            setEmitterSetMtx(emitterSet, mtx, scale);
        } else {
            continue;
        }

        isUpdated = true;
    }

    return isUpdated;
}

/**
 * Stops or resumes the effect when it gets out of or back into the far clip distance.
 * @param isFarClip Whether the effect is far clipped.
 */
void Effect::setFarClip(bool isFarClip) {
    if (mIsFarClip == isFarClip) {
        return;
    }

    mIsFarClip = isFarClip;
    setStopCalcAndDraw(mIsActorClip || mIsFarClip);
}

/**
 * Switches the emitters to the material matching the material code and prefixes.
 * @param pMaterialCode Material code.
 * @param rIsPrefix Which material prefixes are active.
 */
void Effect::tryUpdateMaterial(const char* pMaterialCode, const bool (&rIsPrefix)[4]) {
    StringTmp<64> materialName = makeMaterialName(this, pMaterialCode, rIsPrefix);

    if (isEqualStringOrBothNull(mMaterialName.cstr(), materialName.cstr())) {
        return;
    }

    for (s32 i = 0; i < mEmitterNum; i++) {
        EffectEmitter* emitter = mEmitters[i];

        if (isEqualStringOrBothNull(emitter->getResourceInfo()->mMaterialName, mMaterialName.cstr()) &&
            emitter->isActive() && !emitter->getHandle()->GetEmitterSet()->IsFadeRequest() &&
            emitter->isLoopOrInfinity()) {
            emitter->tryDeleteEmitter(false);
        }
    }

    if (mIsEmitted) {
        for (s32 i = 0; i < mEmitterNum; i++) {
            EffectEmitter* emitter = mEmitters[i];

            if (isEqualStringOrBothNull(emitter->getResourceInfo()->mMaterialName, materialName.cstr()) &&
                emitter->isLoopOrInfinity()) {
                emitEmitter(emitter, nullptr);
            }
        }
    }

    mMaterialName = materialName;
}

/**
 * Creates the emitter set of an emitter unless the effect is clipped.
 * @param pEmitter Emitter to emit.
 * @param pPos Position to emit at, or nullptr to use the followed position.
 */
void Effect::emitEmitter(EffectEmitter* pEmitter, const sead::Vector3f* pPos) {
    if (mIsActorClip || mIsFarClip) {
        return;
    }

    bool isStopCalc = mEffectSystem->isStopCalc();
    JointMtxPtr mtxPtr = mJointMtxPtr;
    createEmitter(pEmitter, &mEffectInfo->mParam, pPos != nullptr ? pPos : mPosPtr, mScalePtr, mtxPtr,
                  mCameraHolder, mUserData, isStopCalc);

    if (!pEmitter->getHandle()->IsValid()) {
        return;
    }

    const EffectEmitParam* param = &mEffectInfo->mParam;
    nn::vfx::EmitterSet* emitterSet = pEmitter->getHandle()->GetEmitterSet();

    if (param->mParticleScale != 1.0f) {
        nn::util::Vector3fType scale;
        nn::util::VectorSet(&scale, param->mParticleScale, param->mParticleScale, param->mParticleScale);
        emitterSet->SetParticleScale(scale);
    }

    f32 emitRatio = param->mEmitRatio;

    if (emitRatio != 1.0f) {
        for (s32 i = 0; i < emitterSet->GetEmitterNum(); i++) {
            emitterSet->SetEmissionRatioScale(emitRatio);
        }
    }

    if (param->mIsSetColor) {
        nn::util::Vector4fType color;
        color._v = float32x4_t{param->mColor.r, param->mColor.g, param->mColor.b, param->mColor.a};
        emitterSet->SetColor(color);
    }
}

/**
 * Emits the emitters of the current material.
 * @param pPos Position to emit at, or nullptr to use the followed position.
 * @param isCurrentMaterial Whether to also clear the first frame flag.
 * @return Whether an emitter was emitted.
 */
bool Effect::emitEmitters(const sead::Vector3f* pPos, bool isCurrentMaterial) {
    return tryEmitEmitters(pPos, isCurrentMaterial);
}

/**
 * Emits the emitters of the current material that may emit.
 * @param pPos Position to emit at, or nullptr to use the followed position.
 * @param isCurrentMaterial Whether to also clear the first frame flag.
 * @return Whether an emitter was emitted.
 */
bool Effect::tryEmitEmitters(const sead::Vector3f* pPos, bool isCurrentMaterial) {
    bool isEmitted = false;

    for (s32 i = 0; i < mEmitterNum; i++) {
        EffectEmitter* emitter = mEmitters[i];

        if (isEqualStringOrBothNull(emitter->getResourceInfo()->mMaterialName, mMaterialName.cstr()) &&
            emitter->isEnableEmit()) {
            emitEmitter(emitter, pPos);

            if (isCurrentMaterial) {
                emitter->resetFirstFrame();
            }

            isEmitted = true;
        }

        if (emitter->isLoopOrInfinity() || mEffectInfo->mParam.mIsBillboard ||
            mEffectInfo->mParam.mIsYBillboard || mEffectInfo->mParam.mIsFollowCamera) {
            mIsEmitted = true;
        }
    }

    return isEmitted;
}

/**
 * Emits an emitter if it may emit.
 * @param pEmitter Emitter to emit.
 * @param pPos Position to emit at, or nullptr to use the followed position.
 * @return Whether the emitter was emitted.
 */
bool Effect::tryEmitEmitter(EffectEmitter* pEmitter, const sead::Vector3f* pPos) {
    if (!pEmitter->isEnableEmit()) {
        return false;
    }

    emitEmitter(pEmitter, pPos);
    return true;
}

/**
 * Fades out all emitters.
 * @return Whether any emitter set was alive.
 */
bool Effect::tryDeleteEmitters() {
    bool isDeleted = false;

    for (s32 i = 0; i < mEmitterNum; i++) {
        isDeleted |= mEmitters[i]->tryDeleteEmitter(false);
    }

    mIsEmitted = false;
    return isDeleted;
}

/**
 * Fades out looping emitters, or all emitters for one time fade effects.
 */
void Effect::deleteAndClearEmitter() {
    for (s32 i = 0; i < mEmitterNum; i++) {
        EffectEmitter* emitter = mEmitters[i];

        if (emitter->isLoopOrInfinity() || mEffectInfo->mParam.mIsOneTimeFade) {
            emitter->tryDeleteEmitter(false);
        }
    }

    mIsEmitted = false;
}

/**
 * Checks whether one time emitters fade out on delete.
 * @return Whether the effect is a one time fade effect.
 */
bool Effect::isOneTimeFade() const {
    return mEffectInfo->mParam.mIsOneTimeFade;
}

/**
 * Kills all emitters and their particles.
 */
void Effect::tryKillEmitterAndParticleAll() {
    for (s32 i = 0; i < mEmitterNum; i++) {
        mEmitters[i]->tryDeleteEmitter(true);
    }

    mIsEmitted = false;
}

/**
 * Stops or resumes calculation and drawing.
 * @param isStop Whether to stop.
 */
void Effect::setStopCalcAndDraw_CAFE(bool isStop) {
    setStopCalcAndDraw(isStop);
}

void Effect::setStopCalcAndDraw(bool isStop) {
    if (!mEffectInfo->mParam.mIsReEmitOnClip) {
        for (s32 i = 0; i < mEmitterNum; i++) {
            EffectEmitter* emitter = mEmitters[i];

            if (!emitter->isActive() || emitter->getResourceInfo()->mEmitterSetResourceInfo->mIsInfinity) {
                continue;
            }

            emitter->setStopCalcAndDraw(isStop);
        }

        return;
    }

    if (!mIsEmitted) {
        return;
    }

    for (s32 i = 0; i < mEmitterNum; i++) {
        EffectEmitter* emitter = mEmitters[i];

        if (isStop) {
            if (emitter->isLoopOrInfinity() && emitter->isActive()) {
                emitter->tryDeleteEmitter(false);
            }
        } else if (emitter->isLoopOrInfinity() &&
                   isEqualStringOrBothNull(emitter->getResourceInfo()->mMaterialName,
                                           mMaterialName.cstr()) &&
                   emitter->isEnableEmit()) {
            emitEmitter(emitter, nullptr);
        }
    }
}

/**
 * Stops or resumes calculation and drawing of all active emitters that are not infinite.
 * @param isStop Whether to stop.
 */
void Effect::forceSetStopCalcAndDraw(bool isStop) {
    for (s32 i = 0; i < mEmitterNum; i++) {
        EffectEmitter* emitter = mEmitters[i];

        if (emitter->isActive() && !emitter->getResourceInfo()->mEmitterSetResourceInfo->mIsInfinity) {
            emitter->setStopCalcAndDraw(isStop);
        }
    }
}

/**
 * Stops or resumes the effect when its actor gets clipped.
 * @param isClip Whether the actor is clipped.
 */
void Effect::setActorClip(bool isClip) {
    mIsActorClip = isClip;
    setStopCalcAndDraw(mIsActorClip || mIsFarClip);
}

/**
 * Enables or disables drawing of all active emitters that are not infinite.
 * @param isEnable Whether to draw.
 */
void Effect::setEnableDraw(bool isEnable) {
    for (s32 i = 0; i < mEmitterNum; i++) {
        EffectEmitter* emitter = mEmitters[i];

        if (emitter->isActive() && !emitter->getResourceInfo()->mEmitterSetResourceInfo->mIsInfinity) {
            emitter->setEnableDraw(isEnable);
        }
    }
}

/**
 * Sets the emission ratio of all active emitter sets.
 * @param ratio Emission ratio scale.
 */
void Effect::setEmitRatio(f32 ratio) {
    for (s32 i = 0; i < mEmitterNum; i++) {
        sead::ptcl::Handle* handle = mEmitters[i]->getHandle();

        if (!handle->IsValid()) {
            continue;
        }

        nn::vfx::EmitterSet* emitterSet = handle->GetEmitterSet();

        for (s32 j = 0; j < emitterSet->GetEmitterNum(); j++) {
            emitterSet->SetEmissionRatioScale(ratio);
        }
    }
}

/**
 * Sets the emitter scale of all active emitter sets.
 * @param rScale Emitter scale.
 */
void Effect::setEmitterScale(const sead::Vector3f& rScale) {
    for (s32 i = 0; i < mEmitterNum; i++) {
        sead::ptcl::Handle* handle = mEmitters[i]->getHandle();

        if (handle->IsValid()) {
            nn::vfx::EmitterSet* emitterSet = handle->GetEmitterSet();
            nn::util::Vector3fType scale;
            loadVector(&scale, rScale);
            emitterSet->SetEmitterScale(scale);
        }
    }
}

/**
 * Sets the overall scale of all active emitter sets.
 * @param rScale Scale.
 */
void Effect::setEmitterAllScale(const sead::Vector3f& rScale) {
    for (s32 i = 0; i < mEmitterNum; i++) {
        sead::ptcl::Handle* handle = mEmitters[i]->getHandle();

        if (handle->IsValid()) {
            nn::vfx::EmitterSet* emitterSet = handle->GetEmitterSet();
            sead::Matrix34f mtx;
            getEmitterSetMtx(&mtx, emitterSet);
            setEmitterSetMtx(emitterSet, mtx, rScale);
        }
    }
}

/**
 * Sets the emitter volume scale of all active emitter sets.
 * @param rScale Emitter volume scale.
 */
void Effect::setEmitterVolumeScale(const sead::Vector3f& rScale) {
    for (s32 i = 0; i < mEmitterNum; i++) {
        sead::ptcl::Handle* handle = mEmitters[i]->getHandle();

        if (handle->IsValid()) {
            nn::vfx::EmitterSet* emitterSet = handle->GetEmitterSet();
            nn::util::Vector3fType scale;
            loadVector(&scale, rScale);
            emitterSet->SetEmitterVolumeScale(scale);
        }
    }
}

/**
 * Sets a uniform particle scale for all active emitter sets.
 * @param scale Particle scale.
 */
void Effect::setParticleScale(f32 scale) {
    for (s32 i = 0; i < mEmitterNum; i++) {
        sead::ptcl::Handle* handle = mEmitters[i]->getHandle();

        if (handle->IsValid()) {
            nn::util::Vector3fType particleScale;
            nn::util::VectorSet(&particleScale, scale, scale, scale);
            handle->GetEmitterSet()->SetParticleScale(particleScale);
        }
    }
}

/**
 * Sets the particle scale of all active emitter sets.
 * @param rScale Particle scale.
 */
void Effect::setParticleScale(const sead::Vector3f& rScale) {
    for (s32 i = 0; i < mEmitterNum; i++) {
        sead::ptcl::Handle* handle = mEmitters[i]->getHandle();

        if (handle->IsValid()) {
            nn::vfx::EmitterSet* emitterSet = handle->GetEmitterSet();
            nn::util::Vector3fType scale;
            loadVector(&scale, rScale);
            emitterSet->SetParticleScale(scale);
        }
    }
}

/**
 * Sets the particle alpha of all active emitter sets.
 * @param alpha Particle alpha.
 */
void Effect::setParticleAlpha(f32 alpha) {
    for (s32 i = 0; i < mEmitterNum; i++) {
        sead::ptcl::Handle* handle = mEmitters[i]->getHandle();

        if (handle->IsValid()) {
            handle->GetEmitterSet()->SetAlpha(alpha);
        }
    }
}

/**
 * Sets the particle color of all active emitter sets.
 * @param rColor Particle color.
 */
void Effect::setParticleColor(const sead::Color4f& rColor) {
    for (s32 i = 0; i < mEmitterNum; i++) {
        sead::ptcl::Handle* handle = mEmitters[i]->getHandle();

        if (handle->IsValid()) {
            nn::vfx::EmitterSet* emitterSet = handle->GetEmitterSet();
            emitterSet->SetColor(rColor.r, rColor.g, rColor.b);
            emitterSet->SetAlpha(rColor.a);
        }
    }
}

/**
 * Sets the particle life scale of all active emitter sets.
 * @param scale Particle life scale.
 */
void Effect::setParticleLifeScale(f32 scale) {
    for (s32 i = 0; i < mEmitterNum; i++) {
        sead::ptcl::Handle* handle = mEmitters[i]->getHandle();

        if (handle->IsValid()) {
            handle->GetEmitterSet()->SetParticleLifeScale(scale);
        }
    }
}

/**
 * Sets the two emitter colors of all active emitter sets.
 * @param rColor0 First emitter color.
 * @param rColor1 Second emitter color.
 */
void Effect::setEmitterColors(const sead::Color4f& rColor0, const sead::Color4f& rColor1) {
    for (s32 i = 0; i < mEmitterNum; i++) {
        sead::ptcl::Handle* handle = mEmitters[i]->getHandle();

        if (handle->IsValid()) {
            nn::vfx::EmitterSet* emitterSet = handle->GetEmitterSet();
            nn::util::Vector4fType color;
            color._v = vld1q_f32(&rColor0.r);
            emitterSet->SetEmitterColor0(color);
            color._v = vld1q_f32(&rColor1.r);
            emitterSet->SetEmitterColor1(color);
        }
    }
}

/**
 * Gets the cached camera view matrix.
 * @return View matrix pointer.
 */
const sead::Matrix34f* Effect::getViewMtxPtr() const {
    return mViewMtxPtr;
}

/**
 * Checks whether any emitter loops or lives forever.
 * @return Whether any emitter loops or lives forever.
 */
bool Effect::isLoopOrInfinity() const {
    for (s32 i = 0; i < mEmitterNum; i++) {
        if (mEmitters[i]->isLoopOrInfinity()) {
            return true;
        }
    }

    return false;
}

/**
 * Checks whether any emitter set is alive.
 * @return Whether any emitter set is alive.
 */
bool Effect::isEmitterActive() const {
    for (s32 i = 0; i < mEmitterNum; i++) {
        if (mEmitters[i]->isActive()) {
            return true;
        }
    }

    return false;
}

/**
 * Checks whether any emitter set is alive or a looping effect was emitted.
 * @return Whether the effect is active.
 */
bool Effect::isEmitterActiveFully() const {
    for (s32 i = 0; i < mEmitterNum; i++) {
        if (mEmitters[i]->isActive()) {
            return true;
        }
    }

    return mIsEmitted;
}
}  // namespace al
