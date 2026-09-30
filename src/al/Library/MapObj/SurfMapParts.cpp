#include "Library/MapObj/SurfMapParts.hpp"

#include "Library/LiveActor/ActorAreaFunction.hpp"
#include "Library/LiveActor/Util/ActorInitUtil.hpp"
#include "Library/LiveActor/Util/ActorMapUtil.hpp"
#include "Library/LiveActor/Util/ActorPoseUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Project/Collision/CollisionPartsFilterBase.hpp"
#include "Project/Collision/CollisionPartsTriangle.hpp"
#include "Project/Collision/CollisionUtil.hpp"

namespace {
using namespace al;

NERVE_DECL(SurfMapParts, Wait)
NERVES_MAKE_NOSTRUCT(SurfMapParts, Wait)
}  // namespace

namespace al {
/**
 * Constructs a map part floating on a surface.
 * @param pName actor name
 */
SurfMapParts::SurfMapParts(const char* pName) : LiveActor(pName) {}

/**
 * Initializes the map part.
 * @param rInfo actor init info
 */
void SurfMapParts::init(const ActorInitInfo& rInfo) {
    initActorPoseTQSV(this);
    initMapPartsActor(this, rInfo, nullptr, 0);
    initNerve(this, &NrvSurfMapPartsWait, 0);
    registerAreaHostMtx(this, rInfo);
    tryGetArg(&mIsEnableSlope, rInfo, "IsEnableSlope");
    mStartQuat = getQuat(this);
    mStartTrans = getTrans(this);
    calcQuatUp(&mUpDir, mStartQuat);
    mCollisionPartsFilter = new CollisionPartsFilterActor(this);
    trySyncStageSwitchAppear(this);
}

/**
 * Follows the surface below.
 */
void SurfMapParts::exeWait() {
    sead::Vector3f hitPos;
    Triangle triangle;
    sead::Vector3f trans = getTrans(this);

    if (alCollisionUtil::getFirstPolyOnArrow(
            this, &hitPos, &triangle, getTrans(this) + mCheckOffset * sead::Vector3f::ey * 0.5f,
            -mCheckOffset * sead::Vector3f::ey, mCollisionPartsFilter, nullptr)) {
        setTrans(this, trans * 0.9f + hitPos * 0.1f);

        if (mIsEnableSlope) {
            sead::Quatf quat;
            sead::Vector3f normal = *triangle.getNormal(0);
            turnQuatYDirRate(&quat, getQuat(this), normal, 0.1f);
            calcQuatUp(&mUpDir, quat);
        }
    }

    if (mIsEnableSlope) {
        turnQuatYDirRate(getQuatPtr(this), mStartQuat, mUpDir, 1.0f);
    }
}
}  // namespace al
