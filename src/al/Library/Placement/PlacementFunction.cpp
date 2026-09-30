#include "Library/Play/Placement/PlacementFunction.hpp"

#include <math/seadMathCalcCommon.h>

#include "Library/Actor/ActorInitInfo.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Play/Placement/PlacementInfo.hpp"
#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/AreaObj/AreaObj.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Checks whether placement info references placement data.
 * @param rInfo placement info
 * @return true if valid
 */
bool isValidInfo(const PlacementInfo& rInfo) {
    return rInfo.getPlacementIter().isValid();
}

/**
 * Checks whether an actor was created from placement data.
 * @param rInfo actor init info
 * @return true if placed
 */
bool isPlaced(const ActorInitInfo& rInfo) {
    return isValidInfo(*rInfo.mPlacementInfo);
}

/**
 * Gets the object name (UnitConfigName) of an actor.
 * @param pName output name
 * @param rInfo actor init info
 */
void getObjectName(const char** pName, const ActorInitInfo& rInfo) {
    getObjectName(pName, *rInfo.mPlacementInfo);
}

/**
 * Gets the object name (UnitConfigName) of a placement.
 * @param pName output name
 * @param rInfo placement info
 */
void getObjectName(const char** pName, const PlacementInfo& rInfo) {
    tryGetObjectName(pName, rInfo);
}

/**
 * Gets the object name (UnitConfigName) of a placement.
 * @param pName output name
 * @param rInfo placement info
 * @return true if the name exists
 */
bool tryGetObjectName(const char** pName, const PlacementInfo& rInfo) {
    return tryGetStringArg(pName, rInfo, "UnitConfigName");
}

/**
 * Gets the object name (UnitConfigName) of an actor.
 * @param pName output name
 * @param rInfo actor init info
 * @return true if the name exists
 */
bool tryGetObjectName(const char** pName, const ActorInitInfo& rInfo) {
    return tryGetObjectName(pName, *rInfo.mPlacementInfo);
}

/**
 * Gets a non-empty string argument.
 * @param pArg output string
 * @param rInfo placement info
 * @param pKey argument key
 * @return true if the argument exists and is not empty
 */
bool tryGetStringArg(const char** pArg, const PlacementInfo& rInfo, const char* pKey) {
    const char* str = "";

    if (!rInfo.getPlacementIter().tryGetStringByKey(&str, pKey) || isEqualString("", str)) {
        return false;
    }

    *pArg = str;
    return true;
}

/**
 * Checks the object name of an actor.
 * @param rInfo actor init info
 * @param pName name to compare with
 * @return true if the names are equal
 */
bool isObjectName(const ActorInitInfo& rInfo, const char* pName) {
    return isObjectName(*rInfo.mPlacementInfo, pName);
}

/**
 * Checks the object name of a placement.
 * @param rInfo placement info
 * @param pName name to compare with
 * @return true if the names are equal
 */
bool isObjectName(const PlacementInfo& rInfo, const char* pName) {
    const char* name;
    return tryGetObjectName(&name, rInfo) && isEqualString(name, pName);
}

/**
 * Checks whether the object name of an actor contains a string.
 * @param rInfo actor init info
 * @param pName string to look for
 * @return true if found
 */
bool isObjectNameSubStr(const ActorInitInfo& rInfo, const char* pName) {
    return isObjectNameSubStr(*rInfo.mPlacementInfo, pName);
}

/**
 * Checks whether the object name of a placement contains a string.
 * @param rInfo placement info
 * @param pName string to look for
 * @return true if found
 */
bool isObjectNameSubStr(const PlacementInfo& rInfo, const char* pName) {
    const char* name;
    return tryGetObjectName(&name, rInfo) && isEqualSubString(name, pName);
}

/**
 * Gets the class name (UnitConfig/ParameterConfigName) of an actor.
 * @param pName output name
 * @param rInfo actor init info
 * @return true if the name exists
 */
bool tryGetClassName(const char** pName, const ActorInitInfo& rInfo) {
    return tryGetClassName(pName, *rInfo.mPlacementInfo);
}

/**
 * Gets the class name (UnitConfig/ParameterConfigName) of a placement.
 * @param pName output name
 * @param rInfo placement info
 * @return true if the name exists
 */
bool tryGetClassName(const char** pName, const PlacementInfo& rInfo) {
    PlacementInfo unitConfig;

    if (!tryGetPlacementInfoByKey(&unitConfig, rInfo, "UnitConfig")) {
        return false;
    }

    return tryGetStringArg(pName, unitConfig, "ParameterConfigName");
}

/**
 * Gets the child placement info stored under a key.
 * @param pOut output placement info
 * @param rInfo placement info
 * @param pKey key name
 * @return true if the key exists
 */
bool tryGetPlacementInfoByKey(PlacementInfo* pOut, const PlacementInfo& rInfo, const char* pKey) {
    ByamlIter iter;

    if (!rInfo.getPlacementIter().tryGetIterByKey(&iter, pKey)) {
        return false;
    }

    pOut->set(iter, rInfo.getZoneIter(), rInfo._20, rInfo._28);
    return true;
}

/**
 * Gets the class name of an actor.
 * @param pName output name
 * @param rInfo actor init info
 */
void getClassName(const char** pName, const ActorInitInfo& rInfo) {
    getClassName(pName, *rInfo.mPlacementInfo);
}

/**
 * Gets the class name of a placement.
 * @param pName output name
 * @param rInfo placement info
 */
void getClassName(const char** pName, const PlacementInfo& rInfo) {
    tryGetClassName(pName, rInfo);
}

/**
 * Checks the class name of an actor.
 * @param rInfo actor init info
 * @param pName name to compare with
 * @return true if the names are equal
 */
bool isClassName(const ActorInitInfo& rInfo, const char* pName) {
    return isClassName(*rInfo.mPlacementInfo, pName);
}

/**
 * Checks the class name of a placement.
 * @param rInfo placement info
 * @param pName name to compare with
 * @return true if the names are equal
 */
bool isClassName(const PlacementInfo& rInfo, const char* pName) {
    const char* className = nullptr;
    return tryGetClassName(&className, rInfo) && isEqualString(className, pName);
}

/**
 * Gets the display name of an actor.
 * @param pName output name
 * @param rInfo actor init info
 */
void getDisplayName(const char** pName, const ActorInitInfo& rInfo) {
    getDisplayName(pName, *rInfo.mPlacementInfo);
}

/**
 * Gets the display name of an actor.
 * @param pName output name
 * @param rInfo actor init info
 * @return true if the name exists
 */
bool tryGetDisplayName(const char** pName, const ActorInitInfo& rInfo) {
    return tryGetDisplayName(pName, *rInfo.mPlacementInfo);
}

/**
 * Gets the display name of a placement.
 * @param pName output name
 * @param rInfo placement info
 */
void getDisplayName(const char** pName, const PlacementInfo& rInfo) {
    tryGetDisplayName(pName, rInfo);
}

/**
 * Gets the display name (UnitConfig/DisplayName) of a placement.
 * @param pName output name
 * @param rInfo placement info
 * @return true if the name exists
 */
bool tryGetDisplayName(const char** pName, const PlacementInfo& rInfo) {
    PlacementInfo unitConfig;

    if (!tryGetPlacementInfoByKey(&unitConfig, rInfo, "UnitConfig")) {
        return false;
    }

    return tryGetStringArg(pName, unitConfig, "DisplayName");
}

/**
 * Gets the placement target file (UnitConfig/PlacementTargetFile) of a placement.
 * @param pFile output file name
 * @param rInfo placement info
 */
void getPlacementTargetFile(const char** pFile, const PlacementInfo& rInfo) {
    PlacementInfo unitConfig;
    getPlacementInfoByKey(&unitConfig, rInfo, "UnitConfig");
    tryGetStringArg(pFile, unitConfig, "PlacementTargetFile");
}

/**
 * Gets the world translation of an actor.
 * @param pTrans output translation
 * @param rInfo actor init info
 * @return true if the translation exists
 */
bool tryGetTrans(sead::Vector3f* pTrans, const ActorInitInfo& rInfo) {
    return tryGetTrans(pTrans, *rInfo.mPlacementInfo);
}

/**
 * Gets the world translation of a placement.
 * @param pTrans output translation
 * @param rInfo placement info
 * @return true if the translation exists
 */
bool tryGetTrans(sead::Vector3f* pTrans, const PlacementInfo& rInfo) {
    if (!tryGetArgV3f(pTrans, rInfo, "Translate")) {
        return false;
    }

    multZoneMtx(pTrans, rInfo);
    return true;
}

/**
 * Transforms a position from zone space to world space.
 * @param pTrans position to transform
 * @param rInfo placement info
 */
void multZoneMtx(sead::Vector3f* pTrans, const PlacementInfo& rInfo) {
    sead::Matrix34f mtx;

    if (tryGetZoneMatrixTR(&mtx, rInfo)) {
        pTrans->mul(mtx);
    }
}

/**
 * Gets the world translation of a placement.
 * @param pTrans output translation
 * @param rInfo placement info
 */
void getTrans(sead::Vector3f* pTrans, const PlacementInfo& rInfo) {
    tryGetTrans(pTrans, rInfo);
}

/**
 * Gets the world rotation of an actor.
 * @param pRotate output rotation in degrees
 * @param rInfo actor init info
 * @return true if the rotation exists
 */
bool tryGetRotate(sead::Vector3f* pRotate, const ActorInitInfo& rInfo) {
    return tryGetRotate(pRotate, *rInfo.mPlacementInfo);
}

/**
 * Gets the world rotation of a placement.
 * @param pRotate output rotation in degrees
 * @param rInfo placement info
 * @return true if the rotation exists
 */
bool tryGetRotate(sead::Vector3f* pRotate, const PlacementInfo& rInfo) {
    if (!tryGetArgV3f(pRotate, rInfo, "Rotate")) {
        return false;
    }

    sead::Matrix34f zoneMtx;

    if (tryGetZoneMatrixTR(&zoneMtx, rInfo)) {
        sead::Matrix34f rotateMtx;
        sead::Vector3f rotate = {sead::Mathf::deg2rad(pRotate->x), sead::Mathf::deg2rad(pRotate->y),
                                 sead::Mathf::deg2rad(pRotate->z)};
        rotateMtx.makeRT(rotate, sead::Vector3f::zero);
        sead::Matrix34f mtx;
        mtx.setMul(zoneMtx, rotateMtx);

        f32 absSin = sead::Mathf::abs(mtx.m[2][0]);

        if (1.0f - absSin < sead::Mathf::epsilon() * 10) {
            pRotate->x = 0.0f;
            pRotate->y = (mtx.m[2][0] / absSin) * (-sead::Mathf::pi() / 2);
            pRotate->z = std::atan2(-mtx.m[0][1], -(mtx.m[2][0] * mtx.m[0][2]));
        } else {
            pRotate->x = std::atan2(mtx.m[2][1], mtx.m[2][2]);
            pRotate->y = std::asin(-mtx.m[2][0]);
            pRotate->z = std::atan2(mtx.m[1][0], mtx.m[0][0]);
        }

        pRotate->set(sead::Mathf::rad2deg(pRotate->x), sead::Mathf::rad2deg(pRotate->y),
                     sead::Mathf::rad2deg(pRotate->z));
    }

    return true;
}

/**
 * Gets the zone transform of a placement, including the transforms of parent zones.
 * @param pMtx output matrix
 * @param rInfo placement info
 * @return true if the placement is in a zone
 */
bool tryGetZoneMatrixTR(sead::Matrix34f* pMtx, const PlacementInfo& rInfo) {
    ByamlIter zone = rInfo.getZoneIter();

    if (!zone.isValid()) {
        return false;
    }

    sead::Vector3f trans = sead::Vector3f::zero;

    if (!tryGetByamlV3f(&trans, zone, "Translate")) {
        return false;
    }

    sead::Vector3f rotate = sead::Vector3f::zero;

    if (!tryGetByamlV3f(&rotate, zone, "Rotate")) {
        return false;
    }

    pMtx->makeRT({sead::Mathf::deg2rad(rotate.x), sead::Mathf::deg2rad(rotate.y),
                  sead::Mathf::deg2rad(rotate.z)},
                 trans);

    if (rInfo._20) {
        sead::Matrix34f parentMtx;

        if (tryGetZoneMatrixTR(&parentMtx, *rInfo._20)) {
            pMtx->setMul(parentMtx, *pMtx);
        }
    }

    return true;
}

/**
 * Gets the rotation of an actor with the zone's Y rotation added.
 * @param pRotate output rotation in degrees
 * @param rInfo actor init info
 * @return true if the rotation exists
 */
bool tryGetRotate_ParentY(sead::Vector3f* pRotate, const ActorInitInfo& rInfo) {
    return tryGetRotate_ParentY(pRotate, *rInfo.mPlacementInfo);
}

/**
 * Gets the rotation of a placement with the zone's Y rotation added.
 * @param pRotate output rotation in degrees
 * @param rInfo placement info
 * @return true if the rotation exists
 */
bool tryGetRotate_ParentY(sead::Vector3f* pRotate, const PlacementInfo& rInfo) {
    if (!tryGetArgV3f(pRotate, rInfo, "Rotate")) {
        return false;
    }

    sead::Matrix34f zoneMtx;

    if (tryGetZoneMatrixTR(&zoneMtx, rInfo)) {
        sead::Vector3f zoneRotate;

        if (tryGetZoneR(&zoneRotate, rInfo)) {
            pRotate->y += zoneRotate.y;
        }
    }

    return true;
}

/**
 * Gets the rotation of the zone of a placement.
 * @param pRotate output rotation in degrees
 * @param rInfo placement info
 * @return true if the placement is in a zone
 */
bool tryGetZoneR(sead::Vector3f* pRotate, const PlacementInfo& rInfo) {
    ByamlIter zone = rInfo.getZoneIter();

    if (!zone.isValid()) {
        return false;
    }

    sead::Vector3f rotate = sead::Vector3f::zero;

    if (!tryGetByamlV3f(&rotate, zone, "Rotate")) {
        return false;
    }

    pRotate->x = rotate.x;
    pRotate->y = rotate.y;
    pRotate->z = rotate.z;
    return true;
}

/**
 * Gets the world rotation of a placement.
 * @param pRotate output rotation in degrees
 * @param rInfo placement info
 */
void getRotate(sead::Vector3f* pRotate, const PlacementInfo& rInfo) {
    tryGetRotate(pRotate, rInfo);
}

/**
 * Gets the world rotation of an actor as a quaternion.
 * @param pQuat output rotation
 * @param rInfo actor init info
 * @return true if the rotation exists
 */
bool tryGetQuat(sead::Quatf* pQuat, const ActorInitInfo& rInfo) {
    return tryGetQuat(pQuat, *rInfo.mPlacementInfo);
}

/**
 * Gets the world rotation of a placement as a quaternion.
 * @param pQuat output rotation, identity if missing
 * @param rInfo placement info
 * @return true if the rotation exists
 */
bool tryGetQuat(sead::Quatf* pQuat, const PlacementInfo& rInfo) {
    sead::Vector3f rotate = sead::Vector3f::zero;

    if (!tryGetRotate(&rotate, rInfo)) {
        *pQuat = sead::Quatf::unit;
        return false;
    }

    pQuat->setRPY(sead::Mathf::deg2rad(rotate.x), sead::Mathf::deg2rad(rotate.y),
                  sead::Mathf::deg2rad(rotate.z));
    return true;
}

/**
 * Gets the world rotation of a placement as a quaternion.
 * @param pQuat output rotation
 * @param rInfo placement info
 */
void getQuat(sead::Quatf* pQuat, const PlacementInfo& rInfo) {
    tryGetQuat(pQuat, rInfo);
}

/**
 * Gets the scale of an actor.
 * @param pScale output scale
 * @param rInfo actor init info
 * @return true if the scale exists
 */
bool tryGetScale(sead::Vector3f* pScale, const ActorInitInfo& rInfo) {
    return tryGetScale(pScale, *rInfo.mPlacementInfo);
}

/**
 * Gets the scale of a placement.
 * @param pScale output scale
 * @param rInfo placement info
 * @return true if the scale exists
 */
bool tryGetScale(sead::Vector3f* pScale, const PlacementInfo& rInfo) {
    return tryGetArgV3f(pScale, rInfo, "Scale");
}

/**
 * Gets the side axis of an actor.
 * @param pSide output axis
 * @param rInfo actor init info
 * @return true if the rotation exists
 */
bool tryGetSide(sead::Vector3f* pSide, const ActorInitInfo& rInfo) {
    return tryGetSide(pSide, *rInfo.mPlacementInfo);
}

/**
 * Gets the side axis of a placement.
 * @param pSide output axis
 * @param rInfo placement info
 * @return true if the rotation exists
 */
bool tryGetSide(sead::Vector3f* pSide, const PlacementInfo& rInfo) {
    sead::Quatf quat = sead::Quatf::unit;

    if (!tryGetQuat(&quat, rInfo)) {
        return false;
    }

    calcQuatSide(pSide, quat);
    return true;
}

/**
 * Gets the up axis of an actor.
 * @param pUp output axis
 * @param rInfo actor init info
 * @return true if the rotation exists
 */
bool tryGetUp(sead::Vector3f* pUp, const ActorInitInfo& rInfo) {
    return tryGetUp(pUp, *rInfo.mPlacementInfo);
}

/**
 * Gets the up axis of a placement.
 * @param pUp output axis
 * @param rInfo placement info
 * @return true if the rotation exists
 */
bool tryGetUp(sead::Vector3f* pUp, const PlacementInfo& rInfo) {
    sead::Quatf quat = sead::Quatf::unit;

    if (!tryGetQuat(&quat, rInfo)) {
        return false;
    }

    calcQuatUp(pUp, quat);
    return true;
}

/**
 * Gets the front axis of an actor.
 * @param pFront output axis
 * @param rInfo actor init info
 * @return true if the rotation exists
 */
bool tryGetFront(sead::Vector3f* pFront, const ActorInitInfo& rInfo) {
    return tryGetFront(pFront, *rInfo.mPlacementInfo);
}

/**
 * Gets the front axis of a placement.
 * @param pFront output axis
 * @param rInfo placement info
 * @return true if the rotation exists
 */
bool tryGetFront(sead::Vector3f* pFront, const PlacementInfo& rInfo) {
    sead::Quatf quat = sead::Quatf::unit;

    if (!tryGetQuat(&quat, rInfo)) {
        return false;
    }

    calcQuatFront(pFront, quat);
    return true;
}

/**
 * Gets a local axis of an actor.
 * @param pDir output axis
 * @param rInfo actor init info
 * @param axis axis index (0 = side, 1 = up, 2 = front)
 * @return true if the rotation exists
 */
bool tryGetLocalAxis(sead::Vector3f* pDir, const ActorInitInfo& rInfo, s32 axis) {
    return tryGetLocalAxis(pDir, *rInfo.mPlacementInfo, axis);
}

/**
 * Gets a local axis of a placement.
 * @param pDir output axis
 * @param rInfo placement info
 * @param axis axis index (0 = side, 1 = up, 2 = front)
 * @return true if the rotation exists
 */
bool tryGetLocalAxis(sead::Vector3f* pDir, const PlacementInfo& rInfo, s32 axis) {
    switch (static_cast<Axis>(axis + 1)) {
    case Axis::X:
        return tryGetSide(pDir, rInfo);
    case Axis::Y:
        return tryGetUp(pDir, rInfo);
    case Axis::Z:
        return tryGetFront(pDir, rInfo);
    default:
        return false;
    }
}

/**
 * Gets a signed local axis of an actor.
 * @param pDir output axis
 * @param rInfo actor init info
 * @param axis signed axis (1 = side, 2 = up, 3 = front, negative to invert)
 * @return true if the axis is valid
 */
bool tryGetLocalSignAxis(sead::Vector3f* pDir, const ActorInitInfo& rInfo, s32 axis) {
    return tryGetLocalSignAxis(pDir, *rInfo.mPlacementInfo, axis);
}

/**
 * Gets a signed local axis of a placement.
 * @param pDir output axis
 * @param rInfo placement info
 * @param axis signed axis (1 = side, 2 = up, 3 = front, negative to invert)
 * @return true if the axis is valid
 */
bool tryGetLocalSignAxis(sead::Vector3f* pDir, const PlacementInfo& rInfo, s32 axis) {
    switch (static_cast<Axis>(sead::Mathi::abs(axis))) {
    case Axis::X:
        tryGetSide(pDir, rInfo);
        break;
    case Axis::Y:
        tryGetUp(pDir, rInfo);
        break;
    case Axis::Z:
        tryGetFront(pDir, rInfo);
        break;
    default:
        return false;
    }

    if (axis < 0) {
        *pDir *= -1;
    }

    return true;
}

/**
 * Gets the world transform of an actor.
 * @param pMtx output matrix
 * @param rInfo actor init info
 * @return true if translation and rotation exist
 */
bool tryGetMatrixTR(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo) {
    return tryGetMatrixTR(pMtx, *rInfo.mPlacementInfo);
}

/**
 * Gets the world transform of a placement.
 * @param pMtx output matrix
 * @param rInfo placement info
 * @return true if translation and rotation exist
 */
bool tryGetMatrixTR(sead::Matrix34f* pMtx, const PlacementInfo& rInfo) {
    sead::Vector3f trans = sead::Vector3f::zero;
    sead::Vector3f rotate = sead::Vector3f::zero;

    if (!tryGetTrans(&trans, rInfo)) {
        return false;
    }

    if (!tryGetRotate(&rotate, rInfo)) {
        return false;
    }

    pMtx->makeRT({sead::Mathf::deg2rad(rotate.x), sead::Mathf::deg2rad(rotate.y),
                  sead::Mathf::deg2rad(rotate.z)},
                 trans);
    return true;
}

/**
 * Gets the world transform of an actor including scale.
 * @param pMtx output matrix
 * @param rInfo actor init info
 * @return true if translation, rotation and scale exist
 */
bool tryGetMatrixTRS(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo) {
    return tryGetMatrixTRS(pMtx, *rInfo.mPlacementInfo);
}

/**
 * Gets the world transform of a placement including scale.
 * @param pMtx output matrix
 * @param rInfo placement info
 * @return true if translation, rotation and scale exist
 */
bool tryGetMatrixTRS(sead::Matrix34f* pMtx, const PlacementInfo& rInfo) {
    sead::Vector3f trans = sead::Vector3f::zero;
    sead::Vector3f rotate = sead::Vector3f::zero;
    sead::Vector3f scale = sead::Vector3f::ones;

    if (!tryGetTrans(&trans, rInfo)) {
        return false;
    }

    if (!tryGetRotate(&rotate, rInfo)) {
        return false;
    }

    if (!tryGetScale(&scale, rInfo)) {
        return false;
    }

    pMtx->makeSRT(scale,
                  {sead::Mathf::deg2rad(rotate.x), sead::Mathf::deg2rad(rotate.y),
                   sead::Mathf::deg2rad(rotate.z)},
                  trans);
    return true;
}

/**
 * Gets the inverse world transform of an actor.
 * @param pMtx output matrix
 * @param rInfo actor init info
 * @return true if translation and rotation exist
 */
bool tryGetInvertMatrixTR(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo) {
    return tryGetInvertMatrixTR(pMtx, *rInfo.mPlacementInfo);
}

/**
 * Gets the inverse world transform of a placement.
 * @param pMtx output matrix
 * @param rInfo placement info
 * @return true if translation and rotation exist
 */
bool tryGetInvertMatrixTR(sead::Matrix34f* pMtx, const PlacementInfo& rInfo) {
    sead::Matrix34f mtx;

    if (!tryGetMatrixTR(&mtx, rInfo)) {
        return false;
    }

    pMtx->setInverse(mtx);
    return true;
}

/**
 * Multiplies the transform of a placement by the transform of its parent.
 * @param pMtx output matrix
 * @param rInfo placement info
 * @param rParentInfo parent placement info
 */
void calcMatrixMultParent(sead::Matrix34f* pMtx, const PlacementInfo& rInfo,
                          const PlacementInfo& rParentInfo) {
    sead::Matrix34f mtx;
    mtx.makeIdentity();
    tryGetMatrixTR(&mtx, rInfo);
    sead::Matrix34f parentMtx;
    parentMtx.makeIdentity();
    tryGetMatrixTR(&parentMtx, rParentInfo);
    pMtx->setMul(parentMtx, mtx);
}

/**
 * Multiplies the transform of an actor by the transform of its parent.
 * @param pMtx output matrix
 * @param rInfo actor init info
 * @param rParentInfo parent actor init info
 */
void calcMatrixMultParent(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo,
                          const ActorInitInfo& rParentInfo) {
    calcMatrixMultParent(pMtx, *rInfo.mPlacementInfo, *rParentInfo.mPlacementInfo);
}

/**
 * Gets an int argument.
 * @param pArg output value
 * @param rInfo actor init info
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetArg(s32* pArg, const ActorInitInfo& rInfo, const char* pKey) {
    return tryGetArg(pArg, *rInfo.mPlacementInfo, pKey);
}

/**
 * Gets an int argument.
 * @param pArg output value
 * @param rInfo placement info
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetArg(s32* pArg, const PlacementInfo& rInfo, const char* pKey) {
    return rInfo.getPlacementIter().tryGetIntByKey(pArg, pKey);
}

/**
 * Gets a float argument.
 * @param pArg output value
 * @param rInfo actor init info
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetArg(f32* pArg, const ActorInitInfo& rInfo, const char* pKey) {
    return tryGetArg(pArg, *rInfo.mPlacementInfo, pKey);
}

/**
 * Gets a float argument.
 * @param pArg output value
 * @param rInfo placement info
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetArg(f32* pArg, const PlacementInfo& rInfo, const char* pKey) {
    return rInfo.getPlacementIter().tryGetFloatByKey(pArg, pKey);
}

/**
 * Gets a bool argument.
 * @param pArg output value
 * @param rInfo actor init info
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetArg(bool* pArg, const ActorInitInfo& rInfo, const char* pKey) {
    return tryGetArg(pArg, *rInfo.mPlacementInfo, pKey);
}

/**
 * Gets a bool argument.
 * @param pArg output value
 * @param rInfo placement info
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetArg(bool* pArg, const PlacementInfo& rInfo, const char* pKey) {
    return rInfo.getPlacementIter().tryGetBoolByKey(pArg, pKey);
}

/**
 * Gets an int argument.
 * @param pArg output value
 * @param rInfo actor init info
 * @param pKey argument key
 */
void getArg(s32* pArg, const ActorInitInfo& rInfo, const char* pKey) {
    getArg(pArg, *rInfo.mPlacementInfo, pKey);
}

/**
 * Gets an int argument.
 * @param pArg output value
 * @param rInfo placement info
 * @param pKey argument key
 */
void getArg(s32* pArg, const PlacementInfo& rInfo, const char* pKey) {
    tryGetArg(pArg, rInfo, pKey);
}

/**
 * Gets a float argument.
 * @param pArg output value
 * @param rInfo actor init info
 * @param pKey argument key
 */
void getArg(f32* pArg, const ActorInitInfo& rInfo, const char* pKey) {
    getArg(pArg, *rInfo.mPlacementInfo, pKey);
}

/**
 * Gets a float argument.
 * @param pArg output value
 * @param rInfo placement info
 * @param pKey argument key
 */
void getArg(f32* pArg, const PlacementInfo& rInfo, const char* pKey) {
    tryGetArg(pArg, rInfo, pKey);
}

/**
 * Gets a bool argument.
 * @param pArg output value
 * @param rInfo actor init info
 * @param pKey argument key
 */
void getArg(bool* pArg, const ActorInitInfo& rInfo, const char* pKey) {
    getArg(pArg, *rInfo.mPlacementInfo, pKey);
}

/**
 * Gets a bool argument.
 * @param pArg output value
 * @param rInfo placement info
 * @param pKey argument key
 */
void getArg(bool* pArg, const PlacementInfo& rInfo, const char* pKey) {
    tryGetArg(pArg, rInfo, pKey);
}

/**
 * Gets a non-empty string argument.
 * @param pArg output string
 * @param rInfo actor init info
 * @param pKey argument key
 */
void getStringArg(const char** pArg, const ActorInitInfo& rInfo, const char* pKey) {
    getStringArg(pArg, *rInfo.mPlacementInfo, pKey);
}

/**
 * Gets a non-empty string argument.
 * @param pArg output string
 * @param rInfo placement info
 * @param pKey argument key
 */
void getStringArg(const char** pArg, const PlacementInfo& rInfo, const char* pKey) {
    tryGetStringArg(pArg, rInfo, pKey);
}

/**
 * Gets a non-empty string argument of an area.
 * @param pArg output string
 * @param rInfo area init info
 * @param pKey argument key
 */
void getStringArg(const char** pArg, const AreaInitInfo& rInfo, const char* pKey) {
    getStringArg(pArg, rInfo.mPlacementInfo, pKey);
}

/**
 * Gets a non-empty string argument.
 * @param pArg output string
 * @param rInfo actor init info
 * @param pKey argument key
 * @return true if the argument exists and is not empty
 */
bool tryGetStringArg(const char** pArg, const ActorInitInfo& rInfo, const char* pKey) {
    return tryGetStringArg(pArg, *rInfo.mPlacementInfo, pKey);
}

/**
 * Gets a non-empty string argument of an area.
 * @param pArg output string
 * @param rInfo area init info
 * @param pKey argument key
 * @return true if the argument exists and is not empty
 */
bool tryGetStringArg(const char** pArg, const AreaInitInfo& rInfo, const char* pKey) {
    return tryGetStringArg(pArg, rInfo.mPlacementInfo, pKey);
}

/**
 * Gets a vector argument.
 * @param pArg output vector
 * @param rInfo actor init info
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetArgV2f(sead::Vector2f* pArg, const ActorInitInfo& rInfo, const char* pKey) {
    return tryGetArgV2f(pArg, *rInfo.mPlacementInfo, pKey);
}

/**
 * Gets a vector argument.
 * @param pArg output vector
 * @param rInfo placement info
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetArgV2f(sead::Vector2f* pArg, const PlacementInfo& rInfo, const char* pKey) {
    return tryGetByamlV2f(pArg, rInfo.getPlacementIter(), pKey);
}

/**
 * Gets a vector argument.
 * @param pArg output vector
 * @param rInfo actor init info
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetArgV3f(sead::Vector3f* pArg, const ActorInitInfo& rInfo, const char* pKey) {
    return tryGetArgV3f(pArg, *rInfo.mPlacementInfo, pKey);
}

/**
 * Gets a vector argument.
 * @param pArg output vector
 * @param rInfo placement info
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetArgV3f(sead::Vector3f* pArg, const PlacementInfo& rInfo, const char* pKey) {
    return tryGetByamlV3f(pArg, rInfo.getPlacementIter(), pKey);
}

/**
 * Gets a color argument.
 * @param pArg output color
 * @param rInfo actor init info
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetArgColor(sead::Color4f* pArg, const ActorInitInfo& rInfo, const char* pKey) {
    return tryGetArgColor(pArg, *rInfo.mPlacementInfo, pKey);
}

/**
 * Gets a color argument.
 * @param pArg output color
 * @param rInfo placement info
 * @param pKey argument key
 * @return true if the argument exists
 */
bool tryGetArgColor(sead::Color4f* pArg, const PlacementInfo& rInfo, const char* pKey) {
    return tryGetByamlColor(pArg, rInfo.getPlacementIter(), pKey);
}

/**
 * Gets the number of entries of a placement node.
 * @param rInfo placement info
 * @return entry count
 */
s32 getCountPlacementInfo(const PlacementInfo& rInfo) {
    return rInfo.getPlacementIter().getSize();
}

/**
 * Gets the child placement info stored under a key.
 * @param pOut output placement info
 * @param rInfo placement info
 * @param pKey key name
 */
void getPlacementInfoByKey(PlacementInfo* pOut, const PlacementInfo& rInfo, const char* pKey) {
    tryGetPlacementInfoByKey(pOut, rInfo, pKey);
}

/**
 * Gets the child placement info at an index.
 * @param pOut output placement info
 * @param rInfo placement info
 * @param index entry index
 */
void getPlacementInfoByIndex(PlacementInfo* pOut, const PlacementInfo& rInfo, s32 index) {
    tryGetPlacementInfoByIndex(pOut, rInfo, index);
}

/**
 * Gets the child placement info at an index.
 * @param pOut output placement info
 * @param rInfo placement info
 * @param index entry index
 * @return true if the entry exists
 */
bool tryGetPlacementInfoByIndex(PlacementInfo* pOut, const PlacementInfo& rInfo, s32 index) {
    ByamlIter iter;

    if (!rInfo.getPlacementIter().tryGetIterByIndex(&iter, index)) {
        return false;
    }

    pOut->set(iter, rInfo.getZoneIter(), rInfo._20, rInfo._28);
    return true;
}

/**
 * Gets the child placement info and its key name at an index.
 * @param pOut output placement info
 * @param pKey output key name
 * @param rInfo placement info
 * @param index entry index
 */
void getPlacementInfoAndKeyNameByIndex(PlacementInfo* pOut, const char** pKey,
                                       const PlacementInfo& rInfo, s32 index) {
    tryGetPlacementInfoAndKeyNameByIndex(pOut, pKey, rInfo, index);
}

/**
 * Gets the child placement info and its key name at an index.
 * @param pOut output placement info
 * @param pKey output key name
 * @param rInfo placement info
 * @param index entry index
 * @return true if the entry exists
 */
bool tryGetPlacementInfoAndKeyNameByIndex(PlacementInfo* pOut, const char** pKey,
                                          const PlacementInfo& rInfo, s32 index) {
    ByamlIter iter;

    if (!rInfo.getPlacementIter().tryGetIterAndKeyNameByIndex(&iter, pKey, index)) {
        return false;
    }

    pOut->set(iter, rInfo.getZoneIter(), rInfo._20, rInfo._28);
    return true;
}

/**
 * Gets the layer of an actor.
 * @param rInfo actor init info
 * @return layer id, or -1 if unknown
 */
s32 tryGetLayerID(const ActorInitInfo& rInfo) {
    return tryGetLayerID(rInfo.mPlacementInfo->getPlacementIter());
}

/**
 * Gets the layer of a placement.
 * @param rInfo placement info
 * @return layer id, or -1 if unknown
 */
s32 tryGetLayerID(const PlacementInfo& rInfo) {
    return tryGetLayerID(rInfo.getPlacementIter());
}

/**
 * Gets the layer of placement data from its LayerConfigName.
 * @param rIter placement data iterator
 * @return layer id, or -1 if unknown
 */
s32 tryGetLayerID(const ByamlIter& rIter) {
    const char* name;

    if (!rIter.tryGetStringByKey(&name, "LayerConfigName")) {
        return -1;
    }

    LayerId layer;

    for (s32 i = 0; i < LayerId::size(); i++) {
        layer = i;

        if (isEqualString(name, LayerId::text(i))) {
            return layer;
        }
    }

    return -1;
}

/**
 * Gets the layer of the zone of the root placement.
 * @param rInfo placement info
 * @return layer id, or -1 if unknown
 */
s32 tryGetLayerIDbyParents(const PlacementInfo& rInfo) {
    const PlacementInfo* info = &rInfo;

    while (info->_20) {
        info = info->_20;
    }

    return tryGetLayerID(info->getZoneIter());
}

/**
 * Reads the placement id of an actor.
 * @param pId output id
 * @param rInfo actor init info
 * @return true if the actor has an id
 */
bool tryGetPlacementID(PlacementId* pId, const ActorInitInfo& rInfo) {
    return tryGetPlacementID(pId, *rInfo.mPlacementInfo);
}

/**
 * Reads the placement id of a placement.
 * @param pId output id
 * @param rInfo placement info
 * @return true if the placement has an id
 */
bool tryGetPlacementID(PlacementId* pId, const PlacementInfo& rInfo) {
    return pId->init(rInfo);
}

/**
 * Reads the placement id of an actor.
 * @param pId output id
 * @param rInfo actor init info
 */
void getPlacementId(PlacementId* pId, const ActorInitInfo& rInfo) {
    getPlacementId(pId, *rInfo.mPlacementInfo);
}

/**
 * Reads the placement id of a placement.
 * @param pId output id
 * @param rInfo placement info
 */
void getPlacementId(PlacementId* pId, const PlacementInfo& rInfo) {
    tryGetPlacementID(pId, rInfo);
}

/**
 * Compares two placement ids.
 * @param rId first id
 * @param rOther second id
 * @return true if both ids refer to the same object
 */
bool isEqualPlacementID(const PlacementId& rId, const PlacementId& rOther) {
    return PlacementId::isEqual(rId, rOther);
}

/**
 * Compares the placement ids of two placements.
 * @param rInfo first placement info
 * @param rOther second placement info
 * @return true if both have ids that refer to the same object
 */
bool isEqualPlacementID(const PlacementInfo& rInfo, const PlacementInfo& rOther) {
    PlacementId id;

    if (!tryGetPlacementID(&id, rInfo)) {
        return false;
    }

    PlacementId otherId;

    if (!tryGetPlacementID(&otherId, rOther)) {
        return false;
    }

    return isEqualPlacementID(id, otherId);
}

/**
 * Checks whether an actor has a rail link.
 * @param rInfo actor init info
 * @return true if a rail is linked
 */
bool isExistRail(const ActorInitInfo& rInfo) {
    PlacementInfo railInfo;
    return tryGetRailIter(&railInfo, *rInfo.mPlacementInfo);
}

/**
 * Gets the rail linked to a placement.
 * @param pRailInfo output rail placement info
 * @param rInfo placement info
 * @return true if a rail is linked
 */
bool tryGetRailIter(PlacementInfo* pRailInfo, const PlacementInfo& rInfo) {
    if (!tryGetLinksInfo(pRailInfo, rInfo, "Rail")) {
        return false;
    }

    return pRailInfo->getPlacementIter().isTypeContainer();
}

/**
 * Gets the first placement linked under a link name.
 * @param pOut output placement info
 * @param rInfo placement info
 * @param pLinkName link name
 * @return true if the link exists
 */
bool tryGetLinksInfo(PlacementInfo* pOut, const PlacementInfo& rInfo, const char* pLinkName) {
    PlacementInfo links;

    if (!tryGetPlacementInfoByKey(&links, rInfo, "Links")) {
        return false;
    }

    PlacementInfo link;

    if (!tryGetPlacementInfoByKey(&link, links, pLinkName)) {
        return false;
    }

    if (!tryGetPlacementInfoByIndex(pOut, link, 0)) {
        return false;
    }

    return true;
}

/**
 * Gets the rail with move parameters linked to a placement.
 * @param pRailInfo output rail placement info
 * @param rInfo placement info
 * @return true if the rail exists
 */
bool tryGetMoveParameterRailIter(PlacementInfo* pRailInfo, const PlacementInfo& rInfo) {
    if (!tryGetLinksInfo(pRailInfo, rInfo, "RailWithMoveParameter")) {
        return false;
    }

    return pRailInfo->getPlacementIter().isTypeContainer();
}

/**
 * Gets the position of a rail point.
 * @param pPos output position
 * @param rInfo rail point placement info
 * @return true if the position exists
 */
bool tryGetRailPointPos(sead::Vector3f* pPos, const PlacementInfo& rInfo) {
    return tryGetTrans(pPos, rInfo);
}

/**
 * Gets the previous control point of a rail point.
 * @param pPos output position
 * @param rInfo rail point placement info
 */
void getRailPointHandlePrev(sead::Vector3f* pPos, const PlacementInfo& rInfo) {
    tryGetRailPointHandlePrev(pPos, rInfo);
}

/**
 * Gets the previous control point of a rail point.
 * @param pPos output position
 * @param rInfo rail point placement info
 * @return true if the control point exists
 */
bool tryGetRailPointHandlePrev(sead::Vector3f* pPos, const PlacementInfo& rInfo) {
    PlacementInfo controlPoints;

    if (!tryGetPlacementInfoByKey(&controlPoints, rInfo, "ControlPoints")) {
        return false;
    }

    PlacementInfo controlPoint;

    if (!tryGetPlacementInfoByIndex(&controlPoint, controlPoints, 0)) {
        return false;
    }

    if (!tryGetByamlV3f(pPos, controlPoint.getPlacementIter())) {
        return false;
    }

    multZoneMtx(pPos, rInfo);
    return true;
}

/**
 * Gets the next control point of a rail point.
 * @param pPos output position
 * @param rInfo rail point placement info
 */
void getRailPointHandleNext(sead::Vector3f* pPos, const PlacementInfo& rInfo) {
    tryGetRailPointHandleNext(pPos, rInfo);
}

/**
 * Gets the next control point of a rail point.
 * @param pPos output position
 * @param rInfo rail point placement info
 * @return true if the control point exists
 */
bool tryGetRailPointHandleNext(sead::Vector3f* pPos, const PlacementInfo& rInfo) {
    PlacementInfo controlPoints;

    if (!tryGetPlacementInfoByKey(&controlPoints, rInfo, "ControlPoints")) {
        return false;
    }

    PlacementInfo controlPoint;

    if (!tryGetPlacementInfoByIndex(&controlPoint, controlPoints, 1)) {
        return false;
    }

    if (!tryGetByamlV3f(pPos, controlPoint.getPlacementIter())) {
        return false;
    }

    multZoneMtx(pPos, rInfo);
    return true;
}

/**
 * Counts the placements linked under a link name.
 * @param rInfo actor init info
 * @param pLinkName link name
 * @return number of linked placements
 */
s32 calcLinkChildNum(const ActorInitInfo& rInfo, const char* pLinkName) {
    return calcLinkChildNum(*rInfo.mPlacementInfo, pLinkName);
}

/**
 * Counts the placements linked under a link name.
 * @param rInfo placement info
 * @param pLinkName link name
 * @return number of linked placements
 */
s32 calcLinkChildNum(const PlacementInfo& rInfo, const char* pLinkName) {
    PlacementInfo links;
    PlacementInfo link;

    if (!tryGetPlacementInfoByKey(&links, rInfo, "Links")) {
        return 0;
    }

    if (!tryGetPlacementInfoByKey(&link, links, pLinkName)) {
        return 0;
    }

    return link.getPlacementIter().getSize();
}

/**
 * Counts how deep a link name is nested through first children.
 * @param rInfo actor init info
 * @param pLinkName link name
 * @return nesting depth
 */
s32 calcLinkNestNum(const ActorInitInfo& rInfo, const char* pLinkName) {
    return calcLinkNestNum(*rInfo.mPlacementInfo, pLinkName);
}

/**
 * Counts how deep a link name is nested through first children.
 * @param rInfo placement info
 * @param pLinkName link name
 * @return nesting depth
 */
s32 calcLinkNestNum(const PlacementInfo& rInfo, const char* pLinkName) {
    PlacementInfo links;

    if (!tryGetPlacementInfoByKey(&links, rInfo, "Links")) {
        return 0;
    }

    PlacementInfo link = links;
    s32 depth = 0;

    while (tryGetPlacementInfoByKey(&link, links, pLinkName) &&
           link.getPlacementIter().getSize() != 0) {
        PlacementInfo item;
        getPlacementInfoByIndex(&item, link, 0);
        getPlacementInfoByKey(&links, item, "Links");
        depth++;
    }

    return depth;
}

/**
 * Gets the first placement linked under a link name.
 * @param pOut output placement info
 * @param rInfo placement info
 * @param pLinkName link name
 */
void getLinksInfo(PlacementInfo* pOut, const PlacementInfo& rInfo, const char* pLinkName) {
    getLinksInfoByIndex(pOut, rInfo, pLinkName, 0);
}

/**
 * Gets a placement linked under a link name.
 * @param pOut output placement info
 * @param rInfo placement info
 * @param pLinkName link name
 * @param index link index
 */
void getLinksInfoByIndex(PlacementInfo* pOut, const PlacementInfo& rInfo, const char* pLinkName,
                         s32 index) {
    PlacementInfo links;

    if (!tryGetPlacementInfoByKey(&links, rInfo, "Links")) {
        return;
    }

    PlacementInfo link;

    if (!tryGetPlacementInfoByKey(&link, links, pLinkName)) {
        return;
    }

    getPlacementInfoByIndex(pOut, link, index);
}

/**
 * Gets a placement linked to an actor under a link name.
 * @param pOut output placement info
 * @param rInfo actor init info
 * @param pLinkName link name
 * @param index link index
 */
void getLinksInfoByIndex(PlacementInfo* pOut, const ActorInitInfo& rInfo, const char* pLinkName,
                         s32 index) {
    getLinksInfoByIndex(pOut, *rInfo.mPlacementInfo, pLinkName, index);
}

/**
 * Gets the first placement linked to an actor under a link name.
 * @param pOut output placement info
 * @param rInfo actor init info
 * @param pLinkName link name
 * @return true if the link exists
 */
bool tryGetLinksInfo(PlacementInfo* pOut, const ActorInitInfo& rInfo, const char* pLinkName) {
    return tryGetLinksInfo(pOut, *rInfo.mPlacementInfo, pLinkName);
}

/**
 * Gets the transform of the first placement linked to an actor.
 * @param pMtx output matrix
 * @param rInfo actor init info
 * @param pLinkName link name
 */
void getLinksMatrix(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo, const char* pLinkName) {
    getLinksMatrixByIndex(pMtx, rInfo, pLinkName, 0);
}

/**
 * Gets the transform of a placement linked to an actor.
 * @param pMtx output matrix
 * @param rInfo actor init info
 * @param pLinkName link name
 * @param index link index
 */
void getLinksMatrixByIndex(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo,
                           const char* pLinkName, s32 index) {
    PlacementInfo info;
    getLinksInfoByIndex(&info, rInfo, pLinkName, index);
    tryGetMatrixTR(pMtx, info);
}

/**
 * Gets the translation and rotation of the first linked placement.
 * @param pTrans output translation
 * @param pRotate output rotation in degrees
 * @param rInfo placement info
 * @param pLinkName link name
 */
void getLinkTR(sead::Vector3f* pTrans, sead::Vector3f* pRotate, const PlacementInfo& rInfo,
               const char* pLinkName) {
    PlacementInfo info;
    getLinksInfo(&info, rInfo, pLinkName);
    getTrans(pTrans, info);
    getRotate(pRotate, info);
}

/**
 * Gets the translation and rotation of the first placement linked to an actor.
 * @param pTrans output translation
 * @param pRotate output rotation in degrees
 * @param rInfo actor init info
 * @param pLinkName link name
 */
void getLinkTR(sead::Vector3f* pTrans, sead::Vector3f* pRotate, const ActorInitInfo& rInfo,
               const char* pLinkName) {
    getLinkTR(pTrans, pRotate, *rInfo.mPlacementInfo, pLinkName);
}

/**
 * Gets the translation and rotation of the first placement linked to an area.
 * @param pTrans output translation
 * @param pRotate output rotation in degrees
 * @param rInfo area init info
 * @param pLinkName link name
 */
void getLinkTR(sead::Vector3f* pTrans, sead::Vector3f* pRotate, const AreaInitInfo& rInfo,
               const char* pLinkName) {
    getLinkTR(pTrans, pRotate, rInfo.mPlacementInfo, pLinkName);
}

/**
 * Gets the rotation and translation of the first placement linked to an actor.
 * @param pQuat output rotation, may be null
 * @param pTrans output translation, may be null
 * @param rInfo actor init info
 * @param pLinkName link name
 * @return true if the link and all requested values exist
 */
bool tryGetLinksQT(sead::Quatf* pQuat, sead::Vector3f* pTrans, const ActorInitInfo& rInfo,
                   const char* pLinkName) {
    PlacementInfo info;

    if (!tryGetLinksInfo(&info, rInfo, pLinkName)) {
        return false;
    }

    bool result = true;

    if (pQuat) {
        result &= tryGetQuat(pQuat, info);
    }

    if (pTrans) {
        result &= tryGetTrans(pTrans, info);
    }

    return result;
}

/**
 * Gets the rotation, translation and scale of the first placement linked to an actor.
 * @param pQuat output rotation, may be null
 * @param pTrans output translation, may be null
 * @param pScale output scale, may be null
 * @param rInfo actor init info
 * @param pLinkName link name
 * @return true if the link and all requested values exist
 */
bool tryGetLinksQTS(sead::Quatf* pQuat, sead::Vector3f* pTrans, sead::Vector3f* pScale,
                    const ActorInitInfo& rInfo, const char* pLinkName) {
    PlacementInfo info;

    if (!tryGetLinksInfo(&info, rInfo, pLinkName)) {
        return false;
    }

    bool result = true;

    if (pQuat) {
        result &= tryGetQuat(pQuat, info);
    }

    if (pTrans) {
        result &= tryGetTrans(pTrans, info);
    }

    if (pScale) {
        result &= tryGetScale(pScale, info);
    }

    return result;
}

/**
 * Gets the transform including scale of the first placement linked to an actor.
 * @param pMtx output matrix
 * @param rInfo actor init info
 * @param pLinkName link name
 * @return true if the link and its transform exist
 */
bool tryGetLinksMatrixTRS(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo,
                          const char* pLinkName) {
    PlacementInfo info;

    if (!tryGetLinksInfo(&info, *rInfo.mPlacementInfo, pLinkName)) {
        return false;
    }

    return tryGetMatrixTRS(pMtx, info);
}

/**
 * Gets the translation of the first placement linked to an actor.
 * @param pTrans output translation
 * @param rInfo actor init info
 * @param pLinkName link name
 * @return true if the link and the translation exist
 */
bool tryGetLinksTrans(sead::Vector3f* pTrans, const ActorInitInfo& rInfo, const char* pLinkName) {
    return tryGetLinksQT(nullptr, pTrans, rInfo, pLinkName);
}

/**
 * Gets the rotation of the first placement linked to an actor.
 * @param pQuat output rotation
 * @param rInfo actor init info
 * @param pLinkName link name
 * @return true if the link and the rotation exist
 */
bool tryGetLinksQuat(sead::Quatf* pQuat, const ActorInitInfo& rInfo, const char* pLinkName) {
    return tryGetLinksQT(pQuat, nullptr, rInfo, pLinkName);
}

/**
 * Gets the translation of a placement linked to an actor.
 * @param pTrans output translation
 * @param rInfo actor init info
 * @param pLinkName link name
 * @param index link index
 */
void getChildLinkT(sead::Vector3f* pTrans, const ActorInitInfo& rInfo, const char* pLinkName,
                   s32 index) {
    PlacementInfo info;
    getLinksInfoByIndex(&info, rInfo, pLinkName, index);
    getTrans(pTrans, info);
}

/**
 * Gets the translation and rotation of a placement linked to an actor.
 * @param pTrans output translation
 * @param pRotate output rotation in degrees
 * @param rInfo actor init info
 * @param pLinkName link name
 * @param index link index
 */
void getChildLinkTR(sead::Vector3f* pTrans, sead::Vector3f* pRotate, const ActorInitInfo& rInfo,
                    const char* pLinkName, s32 index) {
    PlacementInfo info;
    getLinksInfoByIndex(&info, rInfo, pLinkName, index);
    getTrans(pTrans, info);
    getRotate(pRotate, info);
}

/**
 * Gets the translation and front axis of a placement linked to an actor.
 * @param pTrans output translation
 * @param pFront output front axis, ez if missing
 * @param rInfo actor init info
 * @param pLinkName link name
 * @param index link index
 */
void getChildLinkTF(sead::Vector3f* pTrans, sead::Vector3f* pFront, const ActorInitInfo& rInfo,
                    const char* pLinkName, s32 index) {
    PlacementInfo info;
    getLinksInfoByIndex(&info, rInfo, pLinkName, index);
    getTrans(pTrans, info);

    if (!tryGetFront(pFront, info)) {
        *pFront = sead::Vector3f::ez;
    }
}

/**
 * Gets the translation and rotation of a placement linked to an actor.
 * @param pTrans output translation
 * @param pQuat output rotation
 * @param rInfo actor init info
 * @param pLinkName link name
 * @param index link index
 */
void getChildLinkTQ(sead::Vector3f* pTrans, sead::Quatf* pQuat, const ActorInitInfo& rInfo,
                    const char* pLinkName, s32 index) {
    PlacementInfo info;
    getLinksInfoByIndex(&info, rInfo, pLinkName, index);
    getTrans(pTrans, info);
    getQuat(pQuat, info);
}

/**
 * Counts the link names matching a pattern.
 * @param rInfo placement info
 * @param pMatch link name pattern
 * @return number of matching links
 */
s32 calcMatchNameLinkCount(const PlacementInfo& rInfo, const char* pMatch) {
    PlacementInfo links;

    if (!tryGetPlacementInfoByKey(&links, rInfo, "Links")) {
        return 0;
    }

    s32 size = links.getPlacementIter().getSize();
    s32 count = 0;

    for (s32 i = 0; i < size; i++) {
        PlacementInfo item;
        const char* key = nullptr;
        getPlacementInfoAndKeyNameByIndex(&item, &key, links, i);

        if (isMatchString(key, MatchStr(pMatch))) {
            count++;
        }
    }

    return count;
}

/**
 * Counts the links whose first placement has a class name.
 * @param rInfo placement info
 * @param pClassName class name
 * @return number of matching links
 */
s32 calcLinkCountClassName(const PlacementInfo& rInfo, const char* pClassName) {
    PlacementInfo links;

    if (!tryGetPlacementInfoByKey(&links, rInfo, "Links")) {
        return 0;
    }

    s32 size = links.getPlacementIter().getSize();
    s32 count = 0;

    for (s32 i = 0; i < size; i++) {
        PlacementInfo item;
        getPlacementInfoByIndex(&item, links, i);
        PlacementInfo first;
        getPlacementInfoByIndex(&first, item, 0);

        const char* className = nullptr;

        if (tryGetClassName(&className, first) && isEqualString(className, pClassName)) {
            count++;
        }
    }

    return count;
}

/**
 * Gets the zone transform of an actor.
 * @param pMtx output matrix
 * @param rInfo actor init info
 * @return true if the actor is in a zone
 */
bool tryGetZoneMatrixTR(sead::Matrix34f* pMtx, const ActorInitInfo& rInfo) {
    return tryGetZoneMatrixTR(pMtx, *rInfo.mPlacementInfo);
}

/**
 * Gets the id of the zone of a placement.
 * @param pId output zone id
 * @param rInfo placement info
 * @return true if the placement is in a zone with an id
 */
bool tryGetZoneID(s32* pId, const PlacementInfo& rInfo) {
    ByamlIter zone = rInfo.getZoneIter();

    if (!zone.isValid()) {
        return false;
    }

    return tryGetByamlS32(pId, rInfo.getZoneIter(), "ZoneId");
}

/**
 * Gets the display offset of an actor rotated into world space.
 * @param pOffset output offset
 * @param rInfo actor init info
 * @return true if the offset and the actor transform exist
 */
bool tryGetDisplayOffset(sead::Vector3f* pOffset, const ActorInitInfo& rInfo) {
    PlacementInfo unitConfig;

    if (!tryGetPlacementInfoByKey(&unitConfig, *rInfo.mPlacementInfo, "UnitConfig")) {
        return false;
    }

    if (!tryGetArgV3f(pOffset, unitConfig, "DisplayTranslate")) {
        return false;
    }

    sead::Matrix34f mtx = sead::Matrix34f::ident;

    if (!tryGetMatrixTR(&mtx, *rInfo.mPlacementInfo)) {
        return false;
    }

    pOffset->rotate(mtx);

    if (!isSingleMode(rInfo)) {
        sead::Matrix34f zoneMtx = sead::Matrix34f::ident;

        if (tryGetZoneMatrixTR(&zoneMtx, *rInfo.mPlacementInfo)) {
            pOffset->rotate(zoneMtx);
        }
    }

    return true;
}

/**
 * Gets the display scale of an actor.
 * @param pScale output scale
 * @param rInfo actor init info
 * @return true if the scale exists
 */
bool tryGetDisplayScale(sead::Vector3f* pScale, const ActorInitInfo& rInfo) {
    PlacementInfo unitConfig;
    getPlacementInfoByKey(&unitConfig, *rInfo.mPlacementInfo, "UnitConfig");
    return tryGetArgV3f(pScale, unitConfig, "DisplayScale");
}

/**
 * Checks whether the actor is placed in single player mode.
 * @param rInfo actor init info
 * @return true in single player mode
 */
bool isSingleMode(const ActorInitInfo& rInfo) {
    return rInfo.mActorSceneInfo.isSingleMode;
}

/**
 * Checks whether a link has more than a number of placements.
 * @param rInfo placement info
 * @param pLinkName link name
 * @param index link index to check
 * @return true if the link index exists
 */
bool isExistLinkChild(const PlacementInfo& rInfo, const char* pLinkName, s32 index) {
    return calcLinkChildNum(rInfo, pLinkName) > index;
}
}  // namespace al

namespace alPlacementFunction {
/**
 * Gets the camera id argument of an actor.
 * @param rInfo actor init info
 * @return camera id, or -1 if missing
 */
s32 getCameraId(const al::ActorInitInfo& rInfo) {
    s32 id = -1;

    if (!al::tryGetArg(&id, rInfo, "CameraId")) {
        return -1;
    }

    return id;
}

/**
 * Gets the placement id of the first placement linked to an actor.
 * @param pId output id
 * @param rInfo actor init info
 * @param pLinkName link name
 * @return true if the link exists and has an id
 */
bool getLinkGroupId(al::PlacementId* pId, const al::ActorInitInfo& rInfo, const char* pLinkName) {
    al::PlacementInfo info;

    if (al::tryGetLinksInfo(&info, rInfo, pLinkName) && al::tryGetPlacementID(pId, info)) {
        return true;
    }

    return false;
}

/**
 * Checks whether an actor has a linked group.
 * @param rInfo actor init info
 * @param pLinkName link name
 * @return true if the link exists and has an id
 */
bool isEnableLinkGroupId(const al::ActorInitInfo& rInfo, const char* pLinkName) {
    al::PlacementId id;
    return getLinkGroupId(&id, rInfo, pLinkName);
}

/**
 * Checks whether an actor belongs to a clipping group.
 * @param rInfo actor init info
 * @return true if the actor has a GroupClipping link
 */
bool isEnableGroupClipping(const al::ActorInitInfo& rInfo) {
    return isEnableLinkGroupId(rInfo, "GroupClipping");
}

/**
 * Gets the clipping group id of an actor.
 * @param pId output id
 * @param rInfo actor init info
 * @return true if the actor has a clipping group
 */
bool getClippingGroupId(al::PlacementId* pId, const al::ActorInitInfo& rInfo) {
    return getLinkGroupId(pId, rInfo, "GroupClipping");
}

/**
 * Gets the clipping view group id of a placement.
 * @param pId output id
 * @param rInfo placement info
 * @return true if the placement has a view group
 */
bool getClippingViewId(al::PlacementId* pId, const al::PlacementInfo& rInfo) {
    al::PlacementInfo info;

    if (al::tryGetLinksInfo(&info, rInfo, "ViewGroup") && al::tryGetPlacementID(pId, info)) {
        return true;
    }

    return false;
}

/**
 * Gets the clipping view group id of an actor.
 * @param pId output id
 * @param rInfo actor init info
 * @return true if the actor has a view group
 */
bool getClippingViewId(al::PlacementId* pId, const al::ActorInitInfo& rInfo) {
    return getClippingViewId(pId, *rInfo.mPlacementInfo);
}

/**
 * Gets the model name of an actor.
 * @param pName output name
 * @param rInfo actor init info
 */
void getModelName(const char** pName, const al::ActorInitInfo& rInfo) {
    getModelName(pName, *rInfo.mPlacementInfo);
}

/**
 * Gets the model name of a placement.
 * @param pName output name
 * @param rInfo placement info
 */
void getModelName(const char** pName, const al::PlacementInfo& rInfo) {
    tryGetModelName(pName, rInfo);
}

/**
 * Gets the model name (ModelName) of a placement.
 * @param pName output name
 * @param rInfo placement info
 * @return true if a name exists
 */
bool tryGetModelName(const char** pName, const al::PlacementInfo& rInfo) {
    return al::tryGetStringArg(pName, rInfo, "ModelName");
}

/**
 * Gets the model name of an actor.
 * @param pName output name
 * @param rInfo actor init info
 * @return true if a name exists
 */
bool tryGetModelName(const char** pName, const al::ActorInitInfo& rInfo) {
    return tryGetModelName(pName, *rInfo.mPlacementInfo);
}
}  // namespace alPlacementFunction
