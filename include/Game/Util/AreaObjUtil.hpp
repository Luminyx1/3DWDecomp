#pragma once

#include <basis/seadTypes.h>
#include <math/seadVector.h>
#include <prim/seadEnum.h>

namespace al {
    class AreaObj;
    class AreaObjDirector;
    class AreaObjGroup;
    class IUseAreaObj;
    class LiveActor;
    class PlayerHolder;
};

namespace rc {
    // clang-format off
    SEAD_ENUM(AreaObjType,
        AudioEffectChangeArea, AudioListenerParamArea, AudioSituationArea, BalanceTruckBreakArea,
        BgmChangeArea, BgmRegionChangeArea, BgmStartArea, BgmStopArea, BobsledGoalArea,
        BombBoundBreakArea, BowserWaitArea, BugFixBalanceTruckArea, CameraArea,
        CameraAreaActivatedAllPlayer, CameraIconDisplayArea, CameraInSwitchOnArea, CameraNoRotateArea,
        CameraRestrictedArea, CameraStopLookAtArea, CameraWallArea, ClearKillArea, ClippingFarArea,
        CoinBlowConnectGroundArea, CoinFallBoundArea, DeathArea, DepthOfFieldArea,
        DisappearDoorKeyArea, DisasterModeArea, DisasterSpikeArea, DiveArea, EnemyWalkPlaneArea,
        FireBallSafeArea, ForceFallArea, ForceSquatArea, FrameOutCtrlArea, GraphicsArea,
        GhostPresentBoxHideArea, GuideAppearArea, HeadgearWidenStickXSnapArea, HeightMapArea,
        IgnoreForceDashArea, InkArea, InvalidateClippingArea, KeepSkateShoeArea,
        KinopioBrigadeWaveArea, LaserSafeArea, MapDisableArea, MaterialCodeArea, MirrorArea,
        NeedleRollerBreakArea, NeonArea, NoInkPuddleArea, NoDeathArea, NoRainArea, NPCAvoidArea,
        OceanSpikeSafeArea, OffCollideArea, PanelNotePaintRoomCheckArea, PlayerAlongWallArea,
        PlayerAllInCheckArea, PlayerBattleArea, PlayerControlOffArea, PlayerFurOffArea,
        PlayerInclinedControlArea, PlayerNoAirFollowArea, PlayerRestrictedPlane,
        PlayerWaterDoubleCheckArea, PlayerWidenStickXSnapArea, RaidonNoInputArea,
        RaidonFallTriggeredArea , RaidonUIArea, RaidonStartArea, RaidonDisappearArea,
        RaidonCameraHeightLimitArea, SePlayArea, SinkSandArea, SkipperTrampolineInvalidateArea,
        StickFixArea, StickSnapOffArea, SwitchKeepOnArea, SwitchOnArea, TorpedoSpikeRestrictionArea,
        ViewCtrlArea, WaterArea, WaterFallArea, WaterFlowArea, NoWaterArea, WorldClipArea, IslandArea,
        WorldDetectArea, PlayerCollisionYPlaneArea, PlayerCollisionPartsGatherArea,
        PlayerCeilingCheckOffArea, PlayerBugFixAreaForceReleaseIfNotSpace, ForceClipViewCtrlArea
    )
    // clang-format on

    al::AreaObjGroup* tryFindAreaObjGroup(const al::IUseAreaObj* pAreaUser, AreaObjType type);
    void initAreaObjIndex(al::AreaObjDirector* pDirector);
    s32 getAreaObjIndex(AreaObjType type);
    s32 tryFindSingleAreaObj(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                             const sead::Vector3f& rPos, al::AreaObj** pAreaObj);
    al::AreaObj* tryFindAreaObj(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                                const sead::Vector3f& rPos);
    al::AreaObj* tryFindAreaObj(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                                const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
                                sead::Vector3f* pHitPos, sead::Vector3f* pNormal);
    bool isInAreaObj(const al::IUseAreaObj* pAreaUser, AreaObjType type, const sead::Vector3f& rPos);
    bool isInAreaObj(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                     const sead::Vector3f& rStart, const sead::Vector3f& rEnd,
                     sead::Vector3f* pHitPos, sead::Vector3f* pNormal);
    bool isInAreaObj(const al::LiveActor* pActor, AreaObjType type);
    bool isInAreaObjPlayerOne(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                              const al::PlayerHolder* pPlayerHolder);
    bool isInAreaObjPlayerAll(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                              const al::PlayerHolder* pPlayerHolder);
    bool isInDeathArea(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
    bool isInDeathArea(const al::LiveActor* pActor);
    bool isInDeathAreaWithNoDeathCheck(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
    bool isInDeathAreaWithNoDeathCheck(const al::LiveActor* pActor);
    bool isInWaterArea(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
    bool isInWaterArea(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rStart,
                       const sead::Vector3f& rEnd, sead::Vector3f* pHitPos,
                       sead::Vector3f* pNormal);
    bool isInWaterArea(const al::LiveActor* pActor);
    bool isInWaterArea(const al::LiveActor* pActor, f32 offsetY);
    bool isInWaterAreaNoSink(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
    bool isInWaterAreaNoSink(const al::LiveActor* pActor);
    f32 calcWaterSinkDepth(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
    f32 calcWaterSinkDepth(const al::LiveActor* pActor);
    bool isInPlayerControlOffArea(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
    bool isInPlayerControlOffArea(const al::LiveActor* pActor);
    bool isInAreaObjInGroup(const al::IUseAreaObj* pAreaUser, AreaObjType type,
                            const sead::Vector3f& rPos);
    bool isInNoRainArea(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos,
                        bool isCheckFlag);
    bool isInPlessieTunnel(const al::IUseAreaObj* pAreaUser, const sead::Vector3f& rPos);
    bool isInPlessieChaseV2SpecialCamera(const al::LiveActor* pActor, const sead::Vector3f& rPos);
    void updateMaterialCodeWater(al::LiveActor* pActor);
};
