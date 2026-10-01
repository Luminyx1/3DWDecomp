#include "Library/Shadow/ShadowKeeper.hpp"

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Connector/MtxConnector.hpp"
#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Shadow/ShadowMaskCastOvalCylinder.hpp"
#include "Library/Shadow/ShadowMaskCube.hpp"
#include "Library/Shadow/ShadowMaskCylinder.hpp"
#include "Library/Shadow/ShadowMaskSphere.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Library/Yaml/MacroUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Stage/ShadowMaskCastInterpolateCube.hpp"

namespace al {
bool isSingleMode(const ActorInitInfo& rInfo);
bool isExistJoint(const LiveActor* pActor, const char* pJointName);
void attachMtxConnectorToJoint(MtxConnector* pConnector, const LiveActor* pActor,
                               const char* pJointName);
void attachMtxConnectorToActor(MtxConnector* pConnector, const LiveActor* pActor,
                               const sead::Matrix34f* pMtx);
}  // namespace al

template void alYamlMacroUtil::YamlParamGroup::setParamPtr<u8>(const char*, u8*);

namespace {

void attachMtxConnector(al::MtxConnector* pConnector, al::LiveActor* pActor,
                        const char* pJointName) {
    if (pJointName != nullptr && !al::isEqualString(pJointName, "") && al::isExistJoint(pActor, pJointName)) {
        al::attachMtxConnectorToJoint(pConnector, pActor, pJointName);
        return;
    }

    al::attachMtxConnectorToActor(pConnector, pActor, nullptr);
}

template <typename T>
void pushMask(al::ShadowMaskArray& rArray, T* pMask) {
    pMask->createMtxConnector();
    rArray.pushBack(pMask);
}

}  // namespace

namespace al {

/**
 * Reads the common shadow mask parameters.
 * @param rIter Parameter iterator.
 */
void ShadowKeeper::ShadowMaskBaseInfo::readIter(const ByamlIter& rIter) {
    setPtr();
    np_ShadowMaskCommon::ShadowMaskCommon.readParam(rIter);
}

/**
 * Constructs an empty shadow keeper.
 * @param isIgnoreShadowMaskYaml Whether the shadow mask parameters are ignored.
 */
ShadowKeeper::ShadowKeeper(bool isIgnoreShadowMaskYaml)
    : mHostActor(nullptr), mIsIgnoreShadowMaskYaml(isIgnoreShadowMaskYaml) {}

/**
 * Destroys the shadow keeper and all shadow masks.
 */
ShadowKeeper::~ShadowKeeper() {
    while (!mMaskArray.isEmpty()) {
        delete mMaskArray.popBack();
    }

    mMaskArray.freeBuffer();
}

/**
 * Allocates the shadow mask array.
 * @param maskNum Maximum number of shadow masks.
 */
void ShadowKeeper::initShadowMaskNum(s32 maskNum) {
    mMaskArray.allocBuffer(maskNum, nullptr);
}

/**
 * Creates the shadow masks described by the shadow mask parameters.
 * @param pActor Host actor.
 * @param rInfo Actor init info.
 * @param rIter Shadow parameter iterator.
 * @return Whether shadow masks were created.
 */
bool ShadowKeeper::init(LiveActor* pActor, const ActorInitInfo& rInfo, const ByamlIter& rIter) {
    if (mIsIgnoreShadowMaskYaml) {
        return false;
    }

    mHostActor = pActor;

    ByamlIter arrayIter;
    rIter.tryGetIterByKey(&arrayIter, "ShadowMaskArray");

    if (!arrayIter.isValid() || !arrayIter.isTypeArray()) {
        goto fail;
    }

    {

    bool isSingle = isSingleMode(rInfo);
    s32 maskNum = arrayIter.getSize();
    mMaskArray.allocBuffer(maskNum, nullptr);

    for (s32 i = 0; i < maskNum; i++) {
        ByamlIter maskIter;
        arrayIter.tryGetIterByIndex(&maskIter, i);

        ShadowMaskBaseInfo info;
        info.setPtr();
        np_ShadowMaskCommon::ShadowMaskCommon.readParam(maskIter);

        if (info.mName == nullptr) {
            continue;
        }

        const char* typeName = info.mShadowMaskType;

        if (typeName == nullptr || isEqualString(typeName, ShadowMaskType::text(ShadowMaskType::None))) {
            continue;
        }

        ShadowMaskBase* mask;
        s32 type;

        if (isEqualString(typeName, ShadowMaskType::text(ShadowMaskType::Sphere))) {
            auto* sphere = new ShadowMaskSphere(info.mName);
            pushMask(mMaskArray, sphere);
            auto& group = np_ShadowMaskSphereParam::ShadowMaskSphereParam;
            group.readyToSetPtr();
            group.setParamPtr("Scale", &sphere->mScale);
            group.setParamPtr("Exp", &sphere->mExp);
            group.setParamPtr("IsEnableCollisionCheck", &sphere->mIsEnableCollisionCheck);
            group.setParamPtr("CollisionCheckLength", &sphere->mCollisionCheckLength);
            group.readParam(maskIter);
            mask = sphere;
            type = ShadowMaskType::Sphere;
        } else if (isEqualString(typeName,
                                 ShadowMaskType::text(ShadowMaskType::Cylinder))) {
            auto* cylinder = new ShadowMaskCylinder(info.mName);
            pushMask(mMaskArray, cylinder);
            tryGetByamlV3f(&cylinder->mScale, maskIter, "ScaleXYZ");
            tryGetByamlF32(&cylinder->mScale.x, maskIter, "ScaleXZ");
            tryGetByamlF32(&cylinder->mScale.y, maskIter, "ScaleY");
            cylinder->mDropLength = cylinder->mScale.y;
            auto& group = np_ShadowMaskCylinderParam::ShadowMaskCylinderParam;
            group.readyToSetPtr();
            group.setParamPtr("Radius", &cylinder->mScale.x);
            group.setParamPtr("ExpXZ", &cylinder->mExpXZ);
            group.setParamPtr("ExpY", &cylinder->mExpY);
            group.setParamPtr("DistYBase", &cylinder->mDistYBase);
            group.readParam(maskIter);
            mask = cylinder;
            type = ShadowMaskType::Cylinder;
        } else if (isEqualString(typeName, ShadowMaskType::text(ShadowMaskType::Cube))) {
            auto* cube = new ShadowMaskCube(info.mName);
            pushMask(mMaskArray, cube);
            cube->mDropLength = cube->mScale.y;
            auto& group = np_ShadowMaskCubeParam::ShadowMaskCubeParam;
            group.readyToSetPtr();
            group.setParamPtr("Scale", &cube->mScale);
            group.setParamPtr("Exp", &cube->mExp);
            group.setParamPtr("DistYBase", &cube->mDistYBase);
            group.setParamPtr("TextureBaseName", &cube->mTextureBaseName);
            group.setParamPtr("TextureFixedScale", &cube->mTextureFixedScale);
            group.readParam(maskIter);
            cube->tryInitTexture(cube->mTextureBaseName);
            mask = cube;
            type = ShadowMaskType::Cube;
        } else if (isEqualString(typeName,
                                 ShadowMaskType::text(ShadowMaskType::CastOvalCylinder))) {
            auto* cylinder = new ShadowMaskCastOvalCylinder(info.mName);
            pushMask(mMaskArray, cylinder);
            tryGetByamlF32(&cylinder->mExpXZ, maskIter, "Exp");
            tryGetByamlF32(&cylinder->mDropLength, maskIter, "CastLength");
            auto& group = np_ShadowMaskCastOvalCylinderParam::ShadowMaskCastOvalCylinderParam;
            group.readyToSetPtr();
            group.setParamPtr("Scale", &cylinder->mScale);
            group.setParamPtr("ExpXZ", &cylinder->mExpXZ);
            group.setParamPtr("ExpY", &cylinder->mExpY);
            group.setParamPtr("DistYBase", &cylinder->mDistYBase);
            group.readParam(maskIter);
            mask = cylinder;
            type = ShadowMaskType::CastOvalCylinder;
        } else if (isEqualString(typeName,
                                 ShadowMaskType::text(ShadowMaskType::CastInterpolateCube))) {
            auto* cube = new ShadowMaskCastInterpolateCube(info.mName);
            pushMask(mMaskArray, cube);
            mask = cube;
            type = ShadowMaskType::CastInterpolateCube;
        } else {
            continue;
        }

        mask->readParam(maskIter);
        mask->_ea = isSingle;
        mask->setHost(pActor);
        mask->declare(mask->getDrawCategory());
        attachMtxConnector(mask->getMtxConnector(), pActor, info.mActorJointName);

        if (type == ShadowMaskType::CastInterpolateCube) {
            auto* cube = static_cast<ShadowMaskCastInterpolateCube*>(mask);
            attachMtxConnector(cube->getTargetMtxConnector(), pActor,
                               cube->getTargetJointName().cstr());
        }
    }

    for (s32 i = 0; i < mMaskArray.size(); i++) {
        ShadowMaskBase* mask = mMaskArray.at(i);

        if (isEqualString(mask->mSetHeightEvenTargetName.cstr(), "")) {
            continue;
        }

        const char* targetName = mask->mSetHeightEvenTargetName.cstr();
        ShadowMaskBase* target = nullptr;

        for (auto it = mMaskArray.begin(); it != mMaskArray.end(); ++it) {
            if (isEqualString(it->mName, targetName)) {
                target = &*it;
                break;
            }
        }

        mask->mHeightEvenTarget = target;

        if (target != nullptr) {
            target->_e8 = true;
        }
    }

    if (!mIsIgnoreShadowMaskYaml) {
        show();
    }

    return true;
    }

fail:
    mMaskArray.allocBuffer(1, nullptr);
    return false;
}

/**
 * Finds a shadow mask by name.
 * @param pName Name of the shadow mask.
 * @return Shadow mask, or nullptr if not found.
 */
ShadowMaskBase* ShadowKeeper::findShadowMask(const char* pName) const {
    for (auto it = mMaskArray.begin(); it != mMaskArray.end(); ++it) {
        if (isEqualString(it->mName, pName)) {
            return &*it;
        }
    }

    return nullptr;
}

}  // namespace al
