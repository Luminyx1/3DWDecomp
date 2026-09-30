#include "Library/Obj/PartsFunction.hpp"

#include "Library/Execute/ExecuteUtil.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/LiveActor/Util/ActorClippingUtil.hpp"
#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Library/LiveActor/Util/ActorModelUtil.hpp"
#include "Library/LiveActor/Util/ActorMovementUtil.hpp"
#include "Library/LiveActor/Util/ActorResourceUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Obj/CollisionObj.hpp"
#include "Library/Obj/PartsModel.hpp"
#include "Library/Shadow/Common/ShadowUtil.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Creates a collision object following a joint of a parent.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pCollisionFileName collision file name
 * @param pHitSensor sensor of the collision
 * @param pJointName joint to follow, or null to follow the base matrix
 * @param pSuffix collision suffix
 * @return collision object
 */
CollisionObj* createCollisionObj(const LiveActor* pParent, const ActorInitInfo& rInfo,
                                 const char* pCollisionFileName, HitSensor* pHitSensor,
                                 const char* pJointName, const char* pSuffix) {
    return new CollisionObj(rInfo, getModelResource(pParent), pCollisionFileName, pHitSensor,
                            pJointName ? getJointMtxPtr(pParent, pJointName) :
                                         pParent->getBaseMtx(),
                            pSuffix);
}

/**
 * Creates a collision object following a matrix.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pCollisionFileName collision file name
 * @param pHitSensor sensor of the collision
 * @param pJointMtx matrix to follow
 * @param pSuffix collision suffix
 * @return collision object
 */
CollisionObj* createCollisionObjMtx(const LiveActor* pParent, const ActorInitInfo& rInfo,
                                    const char* pCollisionFileName, HitSensor* pHitSensor,
                                    const sead::Matrix34f* pJointMtx, const char* pSuffix) {
    return new CollisionObj(rInfo, getModelResource(pParent), pCollisionFileName, pHitSensor,
                            pJointMtx, pSuffix);
}

/**
 * Creates a parts model following a matrix.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pName actor name
 * @param pArchiveName model archive name
 * @param pJointMtx matrix to follow, or null to follow the base matrix
 * @return parts model
 */
PartsModel* createPartsModel(LiveActor* pParent, const ActorInitInfo& rInfo, const char* pName,
                             const char* pArchiveName, const sead::Matrix34f* pJointMtx) {
    PartsModel* partsModel = new PartsModel(pName);

    if (!pJointMtx) {
        pJointMtx = pParent->getBaseMtx();
    }

    partsModel->initPartsMtx(pParent, rInfo, pArchiveName, pJointMtx, false);
    return partsModel;
}

/**
 * Creates a parts model placed by the parent's InitPartsFixInfo.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pName actor name
 * @param pArchiveName model archive name
 * @param pSuffix InitPartsFixInfo suffix
 * @return parts model
 */
PartsModel* createPartsModelFile(LiveActor* pParent, const ActorInitInfo& rInfo,
                                 const char* pName, const char* pArchiveName,
                                 const char* pSuffix) {
    return createPartsModelFileSuffix(pParent, rInfo, pName, pArchiveName, nullptr, pSuffix);
}

/**
 * Creates a parts model placed by the parent's InitPartsFixInfo.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pName actor name
 * @param pArchiveName model archive name
 * @param pArchiveSuffix archive suffix
 * @param pSuffix InitPartsFixInfo suffix
 * @return parts model
 */
PartsModel* createPartsModelFileSuffix(LiveActor* pParent, const ActorInitInfo& rInfo,
                                       const char* pName, const char* pArchiveName,
                                       const char* pArchiveSuffix, const char* pSuffix) {
    PartsModel* partsModel = new PartsModel(pName);
    partsModel->initPartsFixFile(pParent, rInfo, pArchiveName, pArchiveSuffix, pSuffix);
    StringTmp<128>("[PartsModel] %s", partsModel->getName()).cstr();
    return partsModel;
}

/**
 * Creates a parts model placed by the parent's InitPartsFixInfo.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pName actor name
 * @param pArchiveName model archive name
 * @param pSuffix InitPartsFixInfo suffix
 * @return parts model
 */
PartsModel* createSimplePartsModel(LiveActor* pParent, const ActorInitInfo& rInfo,
                                   const char* pName, const char* pArchiveName,
                                   const char* pSuffix) {
    return createPartsModelFileSuffix(pParent, rInfo, pName, pArchiveName, nullptr, pSuffix);
}

/**
 * Creates a parts model placed by the parent's InitPartsFixInfo.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pName actor name
 * @param pArchiveName model archive name
 * @param pArchiveSuffix archive suffix
 * @param pSuffix InitPartsFixInfo suffix
 * @return parts model
 */
PartsModel* createSimplePartsModelSuffix(LiveActor* pParent, const ActorInitInfo& rInfo,
                                         const char* pName, const char* pArchiveName,
                                         const char* pArchiveSuffix, const char* pSuffix) {
    return createPartsModelFileSuffix(pParent, rInfo, pName, pArchiveName, pArchiveSuffix,
                                      pSuffix);
}

/**
 * Creates a parts model with a suffixed archive following a matrix.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pName actor name
 * @param pArchiveName model archive name
 * @param pSuffix archive suffix
 * @param pJointMtx matrix to follow, or null to follow the base matrix
 * @return parts model
 */
PartsModel* createPartsModelSuffix(LiveActor* pParent, const ActorInitInfo& rInfo,
                                   const char* pName, const char* pArchiveName,
                                   const char* pSuffix, const sead::Matrix34f* pJointMtx) {
    PartsModel* partsModel = new PartsModel(pName);

    if (!pJointMtx) {
        pJointMtx = pParent->getBaseMtx();
    }

    partsModel->initPartsSuffix(pParent, rInfo, pArchiveName, pSuffix, pJointMtx, false);
    return partsModel;
}

/**
 * Creates a parts model following a joint.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pName actor name
 * @param pArchiveName model archive name
 * @param pJointName joint to follow
 * @return parts model
 */
PartsModel* createPartsModelJoint(LiveActor* pParent, const ActorInitInfo& rInfo,
                                  const char* pName, const char* pArchiveName,
                                  const char* pJointName) {
    PartsModel* partsModel = new PartsModel(pName);
    partsModel->initPartsMtx(pParent, rInfo, pArchiveName, getJointMtxPtr(pParent, pJointName),
                             false);
    return partsModel;
}

/**
 * Creates a parts model with a suffixed archive following a joint.
 * @param pParent parent actor
 * @param rInfo actor init info
 * @param pName actor name
 * @param pArchiveName model archive name
 * @param pArchiveSuffix archive suffix
 * @param pJointName joint to follow
 * @return parts model
 */
PartsModel* createPartsModelSuffixJoint(LiveActor* pParent, const ActorInitInfo& rInfo,
                                        const char* pName, const char* pArchiveName,
                                        const char* pArchiveSuffix, const char* pJointName) {
    PartsModel* partsModel = new PartsModel(pName);
    partsModel->initPartsSuffix(pParent, rInfo, pArchiveName, pArchiveSuffix,
                                getJointMtxPtr(pParent, pJointName), false);
    return partsModel;
}

/**
 * Appears with a random Y rotation.
 * @param pActor actor
 */
void appearBreakModelRandomRotateY(LiveActor* pActor) {
    pActor->appear();
    addRotateAndRepeatY(pActor, getRandomDegree());
}

/**
 * Hides or shows an actor with its host.
 * @param pIsHidden whether the actor is hidden, updated
 * @param pActor actor
 * @param pHost host actor
 * @param isForceHide whether the actor is always hidden
 * @return whether the actor is visible
 */
bool updateSyncHostVisible(bool* pIsHidden, LiveActor* pActor, const LiveActor* pHost,
                           bool isForceHide) {
    if (isDead(pHost) || isClipped(pHost) || isHideModel(pHost) || isForceHide) {
        if (!*pIsHidden) {
            if (isExistModel(pActor)) {
                alActorSystemFunction::removeFromExecutorDraw(pActor);
            }

            if (isExistShadow(pActor)) {
                hideShadow(pActor);
            }

            *pIsHidden = true;
        }

        return false;
    }

    if (*pIsHidden) {
        if (isExistModel(pActor)) {
            alActorSystemFunction::addToExecutorDraw(pActor);
        }

        if (isExistShadow(pActor)) {
            showShadow(pActor);
        }

        *pIsHidden = false;
    }

    return true;
}

/**
 * Checks if the traced model is randomly rotated.
 * @param pActor actor
 * @return whether the traced model is randomly rotated
 */
bool isTraceModelRandomRotate(const LiveActor* pActor) {
    if (!isExistModelResourceYaml(pActor, "InitTraceModel", nullptr)) {
        return false;
    }

    return tryGetByamlKeyBoolOrFalse(
        ByamlIter(getModelResourceYaml(pActor, "InitTraceModel", nullptr)), "IsRandomRotate");
}
}  // namespace al
