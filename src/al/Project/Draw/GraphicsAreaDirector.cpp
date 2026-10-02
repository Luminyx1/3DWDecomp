#include "Project/Draw/GraphicsAreaDirector.hpp"

#include <gfx/seadCamera.h>
#include <utility/aglResParameter.h>

#include "Library/Draw/GraphicsSystemInfo.hpp"
#include "Library/Nerve/NerveSetup.hpp"
#include "Library/Nerve/NerveUtil.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"
#include "Library/Play/Placement/PlacementId.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Project/AreaObj/AreaObjGroup.hpp"
#include "Project/AreaObj/AreaObjUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Camera/Info/SceneCameraInfo.hpp"

// The nerve keeper stores the director itself (not its IUseNerve base) as the keeper user.
#define GRAPHICS_AREA_NERVE_DECL(Action, ActionFunc)                                               \
    class GraphicsAreaDirectorNrv##Action : public al::Nerve {                                     \
    public:                                                                                        \
        void execute(al::NerveKeeper* pKeeper) const override {                                    \
            reinterpret_cast<al::GraphicsAreaDirector*>(pKeeper->mKeeperUser)->exe##ActionFunc();  \
        }                                                                                          \
    };

namespace {
using namespace al;

GRAPHICS_AREA_NERVE_DECL(Init, Wait)
GRAPHICS_AREA_NERVE_DECL(CancelLerp, Wait)
GRAPHICS_AREA_NERVE_DECL(ChangeImmediate, Wait)
GRAPHICS_AREA_NERVE_DECL(Wait, Wait)
GRAPHICS_AREA_NERVE_DECL(Lerp, Lerp)
GRAPHICS_AREA_NERVE_DECL(LerpPause, LerpPause)

GraphicsAreaDirectorNrvInit NrvGraphicsAreaDirectorInit;
GraphicsAreaDirectorNrvCancelLerp NrvGraphicsAreaDirectorCancelLerp;
GraphicsAreaDirectorNrvChangeImmediate NrvGraphicsAreaDirectorChangeImmediate;
GraphicsAreaDirectorNrvWait NrvGraphicsAreaDirectorWait;
GraphicsAreaDirectorNrvLerp NrvGraphicsAreaDirectorLerp;
GraphicsAreaDirectorNrvLerpPause NrvGraphicsAreaDirectorLerpPause;

// Parameter names and labels, indexed by GraphicsAreaParamType.
const char* sParamTypeNames[GraphicsAreaParamType::size()] = {
    "bloom",
    "directional_light",
    "cube_map_capture_point",
    "exposure",
    "depth_shadow",
    "alpha_mask_projection",
    "graphics_stress",
    "mirror_rendering",
    "ssao",
    "color_correction",
    "god_ray",
    "flare_filter",
    "fog",
    "y_fog",
    "light_streak",
    "hdr_compose",
    "ssii",
    "sdw_mask",
    "atmos_scatter",
    "skybox",
    "water",
};

const char* sParamTypeLabels[GraphicsAreaParamType::size()] = {
    "Bloom",
    "Directionl Light",
    "Cube Map Sample Points",
    "Exposure",
    "Depth Shadow",
    "Alpha Mask Projection",
    "Graphics Load",
    "Mirror Rendering",
    "SSAO",
    "Color Collection",
    "God Ray",
    "Flare Filter",
    "Fog",
    "Y Fog",
    "Light Streak",
    "HDR Compose",
    "SSII",
    "Shadow Mask",
    "Atmospheric Scattering",
    "SkyBox",
    "Water",
};

/**
 * Gets a graphics area by its index in the area group.
 * @param pAreaUser Area object user.
 * @param index Area index.
 * @return The graphics area, or nullptr.
 */
AreaObj* tryGetGraphicsArea(const IUseAreaObj* pAreaUser, s32 index) {
    AreaObjGroup* group = tryFindAreaObjGroup(pAreaUser, "GraphicsArea");

    if (group == nullptr) {
        return nullptr;
    }

    return group->getAreaObj(index);
}

/**
 * Checks whether the director is switching parameters, either by interpolation or immediately.
 * @param pDirector Graphics area director.
 * @return Whether the parameters are changing.
 */
bool isChangingParam(const GraphicsAreaDirector* pDirector) {
    return isNerve(pDirector, &NrvGraphicsAreaDirectorLerp) ||
           isNerve(pDirector, &NrvGraphicsAreaDirectorChangeImmediate);
}

/**
 * Checks whether the director started switching parameters this frame.
 * @param pDirector Graphics area director.
 * @return Whether the parameter change just started.
 */
bool isStartChangingParam(const GraphicsAreaDirector* pDirector) {
    return isChangingParam(pDirector) && (isNewNerve(pDirector) || isFirstStep(pDirector));
}
}  // namespace

namespace al {

/**
 * Creates an empty area parameter state.
 */
CurrentGraphicsAreaParam::CurrentGraphicsAreaParam() = default;

/**
 * Creates the parameters of a graphics area, named after its placement id.
 * @param pAreaObj Graphics area.
 */
GraphicsAreaInfo::GraphicsAreaInfo(const AreaObj* pAreaObj) : mName(""), mAreaObj(pAreaObj) {
    PlacementId placementId;
    tryGetPlacementID(&placementId, pAreaObj->getPlacementInfo());

    if (placementId.mUnitConfigName != nullptr) {
        mName.format("%s_%s_%s", "graphics_area_info", placementId.mPlacementID,
                     placementId.mUnitConfigName);
    } else {
        mName.format("%s_%s", "graphics_area_info", placementId.mPlacementID);
    }

    mLerpStep = new agl::utl::Parameter<s32>(0, "lerp_step", "Lerp Step", "Min=0,Max=10000", this);
    mIsUsePrevLerpStep = new agl::utl::Parameter<bool>(
        false, "is_use_prev_lerp_step", "Use source interpolation time when switching areas", "",
        this);
    mIsForceCameraAreaFindMode = new agl::utl::Parameter<bool>(
        false, "force_camera_area_find_mode", "Force camera area find mode", "", this);

    mParamNames.allocBuffer(GraphicsAreaParamType::size(), nullptr);

    for (s32 i = 0; i < mParamNames.capacity(); i++) {
        ParamNameParameter* paramName =
            new ParamNameParameter(sead::FixedSafeString<32>(sead::SafeString("")),
                                   sParamTypeNames[i], sParamTypeLabels[i], this);
        mParamNames.pushBack(paramName);
    }
}

/**
 * Creates the director without any graphics areas.
 * @param pSystemInfo Graphics system info.
 */
GraphicsAreaDirector::GraphicsAreaDirector(const GraphicsSystemInfo* pSystemInfo)
    : mSystemInfo(pSystemInfo),
      mNerveKeeper(new NerveKeeper(this, &NrvGraphicsAreaDirectorInit, 0)) {}

/**
 * Sets the objects used to look up the active graphics area.
 * @param pAreaObjDirector Area object director.
 * @param pCameraInfo Scene camera info.
 * @param pPlayerHolder Player holder.
 */
void GraphicsAreaDirector::init(AreaObjDirector* pAreaObjDirector,
                                const SceneCameraInfo* pCameraInfo,
                                const PlayerHolder* pPlayerHolder) {
    mAreaObjDirector = pAreaObjDirector;
    mCameraInfo = pCameraInfo;
    mPlayerHolder = pPlayerHolder;
    mIsLockArea = false;
}

/**
 * Sets the stage the area parameter list is loaded for.
 * @param pStageName Stage name.
 * @param scenarioNo Scenario number, or 0 for the common list.
 */
void GraphicsAreaDirector::setStageName(const char* pStageName, s32 scenarioNo) {
    mStageName = pStageName;
    mScenarioNo = scenarioNo;
}

/**
 * Creates the info for every graphics area and loads the area parameter list of the stage.
 */
void GraphicsAreaDirector::initAfterPlacement() {
    if (mStageName == nullptr) {
        return;
    }

    AreaObjGroup* group = tryFindAreaObjGroup(this, "GraphicsArea");

    if (group == nullptr || group->getSize() == 0) {
        return;
    }

    mInfoNum = getGraphicsAreaNum();
    mInfos = new GraphicsAreaInfo*[mInfoNum];

    for (s32 i = 0; i < mInfoNum; i++) {
        GraphicsAreaInfo* info = new GraphicsAreaInfo(tryGetGraphicsArea(this, i));
        mInfos[i] = info;
        addObj(mInfos[i], mInfos[i]->getName());
    }

    StringTmp<256> fileName("");

    if (mScenarioNo != 0) {
        fileName.format("%s%d%s", "AreaParamList", mScenarioNo, ".baglapl");
    } else {
        fileName.format("%s%s", "AreaParamList", ".baglapl");
    }

    if (tryFindStageParameterFileDesign(mStageName, fileName, 1) == nullptr) {
        fileName.format("%s%s", "AreaParamList", ".baglapl");
    }

    const void* file = tryFindStageParameterFileDesign(mStageName, fileName, 1);

    if (file != nullptr) {
        agl::utl::ResParameterArchive archive(file);
        applyResParameterArchive(archive);
    }
}

/**
 * Gets the number of graphics areas placed in the stage.
 * @return Number of graphics areas.
 */
s32 GraphicsAreaDirector::getGraphicsAreaNum() const {
    AreaObjGroup* group = tryFindAreaObjGroup(this, "GraphicsArea");

    if (group == nullptr) {
        return 0;
    }

    return group->getSize();
}

/**
 * Finishes initialization.
 */
void GraphicsAreaDirector::endInit() {}

/**
 * Looks up the graphics area of the current target and starts switching when it changed.
 */
void GraphicsAreaDirector::update() {
    if (mInfos == nullptr) {
        return;
    }

    mNerveKeeper->update();

    AreaObj* area = mCurrentArea;

    if (static_cast<s32>(mSystemInfo->getAreaTarget()) == GraphicsAreaTarget::Player) {
        area = tryFindAreaObjPlayerOne(this, "GraphicsArea", mPlayerHolder);
    } else if (static_cast<s32>(mSystemInfo->getAreaTarget()) == GraphicsAreaTarget::CameraPos) {
        sead::Vector3f pos = mCameraInfo->mLookAtCamera->getPos();
        area = tryFindAreaObj(this, "GraphicsArea", pos);
    } else if (static_cast<s32>(mSystemInfo->getAreaTarget()) ==
               GraphicsAreaTarget::CameraLookAt) {
        sead::Vector3f pos = mCameraInfo->mLookAtCamera->getAt();
        area = tryFindAreaObj(this, "GraphicsArea", pos);
    }

    if (mSystemInfo->mIsEnableForceCameraAreaFind) {
        sead::Vector3f pos = mCameraInfo->mLookAtCamera->getPos();
        AreaObj* cameraArea = tryFindAreaObj(this, "GraphicsArea", pos);
        GraphicsAreaInfo* cameraInfo = findInfo(cameraArea);
        GraphicsAreaInfo* areaInfo = findInfo(area);

        if ((mCurrentInfo != nullptr && mCurrentInfo->isForceCameraAreaFindMode()) ||
            (areaInfo != nullptr && areaInfo->isForceCameraAreaFindMode()) ||
            (cameraInfo != nullptr && cameraInfo->isForceCameraAreaFindMode())) {
            area = cameraArea;
        }
    }

    if (mIsLockArea || area == mCurrentArea) {
        if (isNerve(this, &NrvGraphicsAreaDirectorCancelLerp)) {
            setNerve(this, &NrvGraphicsAreaDirectorChangeImmediate);
        }

        return;
    }

    mCurrentArea = area;
    mPrevInfo = mCurrentInfo;
    mCurrentInfo = findInfo(area);

    if (isNerve(this, &NrvGraphicsAreaDirectorInit) && mPrevInfo == nullptr) {
        setNerve(this, &NrvGraphicsAreaDirectorWait);
    } else if (isNerve(this, &NrvGraphicsAreaDirectorCancelLerp)) {
        setNerve(this, &NrvGraphicsAreaDirectorChangeImmediate);
    } else {
        mLerpFrame = 0;
        setNerve(this, &NrvGraphicsAreaDirectorLerp);
    }
}

/**
 * Finds the info of a graphics area.
 * @param pAreaObj Graphics area.
 * @return The info of the area, or nullptr.
 */
GraphicsAreaInfo* GraphicsAreaDirector::findInfo(const AreaObj* pAreaObj) const {
    if (pAreaObj == nullptr || mInfos == nullptr) {
        return nullptr;
    }

    for (s32 i = 0; i < mInfoNum; i++) {
        if (mInfos[i]->getAreaObj() == pAreaObj) {
            return mInfos[i];
        }
    }

    return nullptr;
}

/**
 * Gets the parameter names and the interpolation state for a parameter type.
 * @param pParam Output parameter state.
 * @param type Parameter type.
 */
void GraphicsAreaDirector::getCurrentGraphicsAreaParam(CurrentGraphicsAreaParam* pParam,
                                                       GraphicsAreaParamType type) const {
    pParam->mParamName = nullptr;
    pParam->mPrevParamName = nullptr;
    pParam->mRate = 0.0f;
    pParam->_14 = 0;

    // Checked through a pointer copy: the original converts it to IUseNerve with a null check.
    const GraphicsAreaDirector* director = this;

    if (isNerve(director, &NrvGraphicsAreaDirectorLerp)) {
        pParam->mIsLerp = true;
    } else {
        pParam->mIsLerp = isNerve(director, &NrvGraphicsAreaDirectorChangeImmediate);
    }

    pParam->mIsNoParam = isStartChangingParam(director);
    pParam->mPriority = -2;

    if (mCurrentArea != nullptr) {
        pParam->mPriority = mCurrentArea->getPriority();
    }

    if (isNerve(this, &NrvGraphicsAreaDirectorLerp) ||
        isNerve(this, &NrvGraphicsAreaDirectorLerpPause)) {
        pParam->mParamName =
            mCurrentInfo != nullptr ? mCurrentInfo->getParamName(type) : nullptr;
        pParam->mPrevParamName = mPrevInfo != nullptr ? mPrevInfo->getParamName(type) : nullptr;

        s32 lerpStep;

        if (mCurrentInfo == nullptr ||
            (mPrevInfo != nullptr && mPrevInfo->isUsePrevLerpStep())) {
            lerpStep = mLerpStep != -1 ? mLerpStep : mPrevInfo->getLerpStep();
        } else {
            lerpStep = mLerpStep != -1 ? mLerpStep : mCurrentInfo->getLerpStep();
        }

        pParam->_14 = lerpStep;

        f32 rate = mLerpRate;

        if (rate < 0.0f || rate > 1.0f) {
            if (mLerpStep == -1) {
                rate = calcNerveRate(this, lerpStep);
            } else {
                rate = sead::Mathf::clamp(static_cast<f32>(mLerpFrame) / mLerpStep, 0.0f, 1.0f);
            }
        }

        pParam->mRate = rate;
    } else if (mCurrentInfo != nullptr) {
        pParam->mParamName = mCurrentInfo->getParamName(type);
        pParam->mRate = 1.0f;
    }
}

/**
 * Finds the info of the graphics area at a position.
 * @param rTrans Position.
 * @return The info of the area, or nullptr.
 */
GraphicsAreaInfo* GraphicsAreaDirector::getGraphicsAreaInfoByTrans(
    const sead::Vector3f& rTrans) const {
    return findInfo(tryFindAreaObj(this, "GraphicsArea", rTrans));
}

/**
 * Finds the info of a graphics area by its index in the area group.
 * @param index Area index.
 * @return The info of the area, or nullptr.
 */
GraphicsAreaInfo* GraphicsAreaDirector::getGraphicsAreaInfoByIndex(s32 index) const {
    return findInfo(tryGetGraphicsArea(this, index));
}

/**
 * Checks whether the parameters are being interpolated.
 * @return Whether the director is interpolating.
 */
bool GraphicsAreaDirector::isLerp() const {
    return isNerve(this, &NrvGraphicsAreaDirectorLerp);
}

/**
 * Cancels the interpolation, so the next area change is applied immediately.
 */
void GraphicsAreaDirector::cancelLerp() {
    setNerve(this, &NrvGraphicsAreaDirectorCancelLerp);
}

/**
 * Waits for an area change. An immediate change ends after one frame.
 */
void GraphicsAreaDirector::exeWait() {
    if (isNerve(this, &NrvGraphicsAreaDirectorChangeImmediate) && isGreaterEqualStep(this, 1)) {
        setNerve(this, &NrvGraphicsAreaDirectorWait);
    }
}

/**
 * Keeps the interpolation paused until it is resumed.
 */
void GraphicsAreaDirector::exeLerpPause() {
    if (!mIsLerpPaused) {
        setNerve(this, &NrvGraphicsAreaDirectorLerp);
    }
}

/**
 * Advances the interpolation between the previous and the current area.
 */
void GraphicsAreaDirector::exeLerp() {
    if (mIsLerpPaused) {
        mPausedLerpFrame = mLerpFrame;
        setNerve(this, &NrvGraphicsAreaDirectorLerpPause);
        return;
    }

    s32 lerpStep = mLerpStep;

    if (lerpStep < 0) {
        if (mCurrentInfo != nullptr) {
            lerpStep = mCurrentInfo->getLerpStep();
        } else if (mPrevInfo != nullptr) {
            lerpStep = mPrevInfo->getLerpStep();
        } else {
            lerpStep = 0;
        }

        if (mPrevInfo != nullptr && mPrevInfo->isUsePrevLerpStep()) {
            lerpStep = mPrevInfo->getLerpStep();
        }
    }

    if (mLerpFrame++ >= lerpStep) {
        setNerve(this, &NrvGraphicsAreaDirectorWait);
    }
}

}  // namespace al
