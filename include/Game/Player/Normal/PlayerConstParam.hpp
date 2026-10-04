#pragma once

#include <basis/seadTypes.h>

namespace al {
    class ByamlIter;
}

/// Defines a tuning value: a member and a virtual getter that asks the override
/// parameter set instead while one is active.
#define PLAYER_CONST_PARAM(Type, Name)                                  \
    virtual Type get##Name() const {                                     \
        if (mIsOverride) {                                               \
            return mOverrideParam->get##Name();                          \
        }                                                                \
        return m##Name;                                                  \
    }

/// The player's tuning values (speeds, jump heights, frame counts, ...), read from
/// PlayerConst.byml. Another set can override it (e.g. for Bowser's Fury's modes).
class PlayerConstParam {
public:
    PlayerConstParam();
    PlayerConstParam(const al::ByamlIter&);

    PLAYER_CONST_PARAM(f32, Gravity)
    PLAYER_CONST_PARAM(f32, CenterHeight)
    PLAYER_CONST_PARAM(f32, BodyRadius)
    PLAYER_CONST_PARAM(f32, CollectInfoRadiusAddition)
    PLAYER_CONST_PARAM(f32, SnapGroundMaxLength)
    PLAYER_CONST_PARAM(f32, SnapWallMaxLength)
    PLAYER_CONST_PARAM(f32, StickRoundThreshold)
    PLAYER_CONST_PARAM(f32, HeightCheckLength)
    PLAYER_CONST_PARAM(f32, CutVelLimit)
    PLAYER_CONST_PARAM(f32, CutVelRate)
    PLAYER_CONST_PARAM(s32, ThrowInvalidationFrames)
    PLAYER_CONST_PARAM(f32, Tall)
    PLAYER_CONST_PARAM(f32, ChestRadius)
    PLAYER_CONST_PARAM(f32, DashCheckRadius)
    PLAYER_CONST_PARAM(f32, ShadowCheckLength)
    PLAYER_CONST_PARAM(f32, ShadowLengthMax)
    PLAYER_CONST_PARAM(s32, PivotFrame)
    PLAYER_CONST_PARAM(f32, PivotDegree)
    PLAYER_CONST_PARAM(f32, NormalMaxSpeed)
    PLAYER_CONST_PARAM(f32, DashMaxSpeed)
    PLAYER_CONST_PARAM(f32, SuperDashSpeed)
    PLAYER_CONST_PARAM(s32, SuperDashTimer)
    PLAYER_CONST_PARAM(s32, SuperDashTimerMini)
    PLAYER_CONST_PARAM(s32, SuperDashTimerFire)
    PLAYER_CONST_PARAM(s32, SuperDashTimerClimb)
    PLAYER_CONST_PARAM(s32, SuperDashTimerRaccoonDog)
    PLAYER_CONST_PARAM(s32, SuperDashTimerBoomerang)
    PLAYER_CONST_PARAM(s32, SuperDashTimerRaccoonDogWhite)
    PLAYER_CONST_PARAM(f32, SuperDashStartAnimRate)
    PLAYER_CONST_PARAM(s32, SuperDashStartAnimFrame)
    PLAYER_CONST_PARAM(s32, BrakeFrame)
    PLAYER_CONST_PARAM(s32, DashBrakeFrame)
    PLAYER_CONST_PARAM(s32, StickOnBrakeFrame)
    PLAYER_CONST_PARAM(s32, BrakeFrameOnIce)
    PLAYER_CONST_PARAM(s32, AccelFrame)
    PLAYER_CONST_PARAM(s32, DashAccelFrame)
    PLAYER_CONST_PARAM(f32, RoundLimitDegreeMax)
    PLAYER_CONST_PARAM(f32, RoundLimitDegreeMin)
    PLAYER_CONST_PARAM(f32, RunAnimRateMax)
    PLAYER_CONST_PARAM(f32, GiantRunAnimRateMax)
    PLAYER_CONST_PARAM(f32, ShortAnimRateEff)
    PLAYER_CONST_PARAM(s32, DashStartFrame)
    PLAYER_CONST_PARAM(s32, ModifiedDashStartFrame)
    PLAYER_CONST_PARAM(s32, DashStartBlendFrame)
    PLAYER_CONST_PARAM(s32, DashInputSuccessFrame)
    PLAYER_CONST_PARAM(f32, DownHillAccelStartDegree)
    PLAYER_CONST_PARAM(f32, DownHillAccelEndDegree)
    PLAYER_CONST_PARAM(f32, DownHillAccelAddRate)
    PLAYER_CONST_PARAM(f32, DashPanelSpeed)
    PLAYER_CONST_PARAM(f32, ModifiedDashPanelSpeed)
    PLAYER_CONST_PARAM(f32, DashPanelOverRate)
    PLAYER_CONST_PARAM(s32, DashPanelTimer)
    PLAYER_CONST_PARAM(f32, FlingPoleSpeed)
    PLAYER_CONST_PARAM(s32, GroundOffFrame)
    PLAYER_CONST_PARAM(s32, ClimbToGroundMoveFrame)
    PLAYER_CONST_PARAM(f32, TiltMaxDegree)
    PLAYER_CONST_PARAM(f32, TiltBlendRate)
    PLAYER_CONST_PARAM(f32, TiltMaxFrontAngle)
    PLAYER_CONST_PARAM(f32, HoldingTiltMaxFrontAngle)
    PLAYER_CONST_PARAM(f32, TiltStartSpeed)
    PLAYER_CONST_PARAM(f32, TiltEndSpeed)
    PLAYER_CONST_PARAM(f32, ClimbRunAnimRateEff)
    PLAYER_CONST_PARAM(f32, PanelDashAnimRate)
    PLAYER_CONST_PARAM(f32, ModifiedPanelDashAnimRate)
    PLAYER_CONST_PARAM(f32, SlopeMaxSpeedScale)
    PLAYER_CONST_PARAM(f32, MaxSpeedScale)
    PLAYER_CONST_PARAM(f32, DashSignAnimRate)
    PLAYER_CONST_PARAM(f32, DashSignMaxLoop)
    PLAYER_CONST_PARAM(f32, DashSignMaxSpeed)
    PLAYER_CONST_PARAM(s32, DashSignAnimFrameMax)
    PLAYER_CONST_PARAM(f32, JumpDirRotLimit)
    PLAYER_CONST_PARAM(f32, JumpPowLow)
    PLAYER_CONST_PARAM(f32, JumpPow)
    PLAYER_CONST_PARAM(s32, JumpPowCountMax)
    PLAYER_CONST_PARAM(f32, JumpExtensionGravityRate)
    PLAYER_CONST_PARAM(f32, JumpSideVelRate)
    PLAYER_CONST_PARAM(f32, JumpFrontBrakeRate)
    PLAYER_CONST_PARAM(f32, JumpRotBlendRate)
    PLAYER_CONST_PARAM(s32, JumpAccelFrame)
    PLAYER_CONST_PARAM(s32, ReleaseAccelFrame)
    PLAYER_CONST_PARAM(s32, JumpAccelAddFrame)
    PLAYER_CONST_PARAM(f32, JumpCancelBrakeRate)
    PLAYER_CONST_PARAM(f32, JumpCancelMinSpeed)
    PLAYER_CONST_PARAM(s32, ContinuousJumpTimer)
    PLAYER_CONST_PARAM(s32, ContinuousJumpCount)
    PLAYER_CONST_PARAM(f32, FallSpeedMax)
    PLAYER_CONST_PARAM(f32, DashJumpAddition)
    PLAYER_CONST_PARAM(f32, PunchReflectPower)
    PLAYER_CONST_PARAM(s32, GlideInhibitFrameAfterPunch)
    PLAYER_CONST_PARAM(f32, WalkMinSpeedRate)
    PLAYER_CONST_PARAM(s32, JumpHVelSamplingNum)
    PLAYER_CONST_PARAM(f32, FollowFrontDamper)
    PLAYER_CONST_PARAM(f32, FollowBackDamper)
    PLAYER_CONST_PARAM(s32, FlightDurationCount)
    PLAYER_CONST_PARAM(f32, FlightDurationRotBlendRate)
    PLAYER_CONST_PARAM(s32, FlightDurationJumpStartInhibitFrame)
    PLAYER_CONST_PARAM(f32, FlightDurationSideDamper)
    PLAYER_CONST_PARAM(f32, RaccoonDogFallSpeedMax)
    PLAYER_CONST_PARAM(f32, RaccoonDogFallGravityRate)
    PLAYER_CONST_PARAM(f32, RaccoonDogFallGravityAdd)
    PLAYER_CONST_PARAM(f32, RaccoonDogFirstFallDamper)
    PLAYER_CONST_PARAM(f32, RaccoonDogRotBlendRate)
    PLAYER_CONST_PARAM(f32, RaccoonDogFallSideDamper)
    PLAYER_CONST_PARAM(f32, TrampleJumpGravity)
    PLAYER_CONST_PARAM(f32, TrampleJump)
    PLAYER_CONST_PARAM(f32, TrampleJumpSideVelRate)
    PLAYER_CONST_PARAM(f32, TrampleHighJumpGravity)
    PLAYER_CONST_PARAM(f32, TrampleHighJump)
    PLAYER_CONST_PARAM(f32, TrampleHipDropGravity)
    PLAYER_CONST_PARAM(f32, TrampleHipDropJump)
    PLAYER_CONST_PARAM(f32, RisingTrampleJumpGravity)
    PLAYER_CONST_PARAM(f32, RisingTrampleJump)
    PLAYER_CONST_PARAM(f32, RisingTrampleHighJumpGravity)
    PLAYER_CONST_PARAM(f32, RisingTrampleHighJump)
    PLAYER_CONST_PARAM(f32, RisingTrampleHVelBrakeRate)
    PLAYER_CONST_PARAM(f32, PunchedJumpGravity)
    PLAYER_CONST_PARAM(f32, PunchedJump)
    PLAYER_CONST_PARAM(f32, RisingPunchedHVelBrakeRate)
    PLAYER_CONST_PARAM(f32, TossedJumpGravity)
    PLAYER_CONST_PARAM(f32, TossedJump)
    PLAYER_CONST_PARAM(f32, TossedHighJumpGravity)
    PLAYER_CONST_PARAM(f32, TossedHighJump)
    PLAYER_CONST_PARAM(f32, TossedHipDropGravity)
    PLAYER_CONST_PARAM(f32, TossedHipDropJump)
    PLAYER_CONST_PARAM(f32, RisingTossedJumpGravity)
    PLAYER_CONST_PARAM(f32, RisingTossedJump)
    PLAYER_CONST_PARAM(f32, RisingTossedHighJumpGravity)
    PLAYER_CONST_PARAM(f32, RisingTossedHighJump)
    PLAYER_CONST_PARAM(f32, RisingTossedHVelBrakeRate)
    PLAYER_CONST_PARAM(f32, SquatBrakeEndSpeed)
    PLAYER_CONST_PARAM(f32, SquatShiftSpeedRate)
    PLAYER_CONST_PARAM(f32, SquatAccelRate)
    PLAYER_CONST_PARAM(s32, SquatNoBrakeFrame)
    PLAYER_CONST_PARAM(f32, SquatBrakeRate)
    PLAYER_CONST_PARAM(f32, SquatBrakeRateOnSkate)
    PLAYER_CONST_PARAM(f32, SquatBrakeSideAccel)
    PLAYER_CONST_PARAM(f32, SquatBrakeSideBrakeRate)
    PLAYER_CONST_PARAM(f32, SquatBrakeSideBrakeRateOnSkate)
    PLAYER_CONST_PARAM(f32, SquatBrakeSideMaxSpeedRate)
    PLAYER_CONST_PARAM(f32, SquatWalkSpeed)
    PLAYER_CONST_PARAM(f32, SquatWalkFrontVecBlend)
    PLAYER_CONST_PARAM(s32, SquatEnergyAccelFrame)
    PLAYER_CONST_PARAM(f32, SquatJumpGravity)
    PLAYER_CONST_PARAM(f32, SquatJumpPow)
    PLAYER_CONST_PARAM(f32, SquatHighJumpPow)
    PLAYER_CONST_PARAM(f32, SquatJumpBackPow)
    PLAYER_CONST_PARAM(f32, SquatJumpTramplePow)
    PLAYER_CONST_PARAM(f32, HipDropSpeed)
    PLAYER_CONST_PARAM(s32, HipDropLandCancelFrame)
    PLAYER_CONST_PARAM(f32, HipDropHeight)
    PLAYER_CONST_PARAM(s32, HipDropMsgInterval)
    PLAYER_CONST_PARAM(f32, HipDropKnockDownRadiusMin)
    PLAYER_CONST_PARAM(f32, HipDropKnockDownRadiusMax)
    PLAYER_CONST_PARAM(f32, HipDropStartAnimRate)
    PLAYER_CONST_PARAM(s32, HipDropKnockDownFrame)
    PLAYER_CONST_PARAM(f32, HipDropJumpPow)
    PLAYER_CONST_PARAM(s32, HipDropJumpPowCountMax)
    PLAYER_CONST_PARAM(s32, HipDropJumpPermitBeginFrame)
    PLAYER_CONST_PARAM(s32, HipDropJumpPermitEndFrame)
    PLAYER_CONST_PARAM(f32, WallHeightLowLimit)
    PLAYER_CONST_PARAM(f32, WallGravity)
    PLAYER_CONST_PARAM(f32, WallMaxSpeed)
    PLAYER_CONST_PARAM(f32, WallApartFrame)
    PLAYER_CONST_PARAM(f32, WallSnapDistance)
    PLAYER_CONST_PARAM(f32, WallSlideMaxSpeed)
    PLAYER_CONST_PARAM(f32, WallSlideAccel)
    PLAYER_CONST_PARAM(s32, WallInhibitAfterPunch)
    PLAYER_CONST_PARAM(f32, WallJumpGravity)
    PLAYER_CONST_PARAM(f32, WallJumpHSpeed)
    PLAYER_CONST_PARAM(f32, WallJumpPow)
    PLAYER_CONST_PARAM(s32, WallJumpInvalidateInputFrame)
    PLAYER_CONST_PARAM(s32, WallJumpDirEffectiveFrame)
    PLAYER_CONST_PARAM(f32, WallJumpDirLimit)
    PLAYER_CONST_PARAM(f32, WallJumpLimitPlay)
    PLAYER_CONST_PARAM(s32, WallClimbReadyFrame)
    PLAYER_CONST_PARAM(s32, WallClimbReadyWaitFrame)
    PLAYER_CONST_PARAM(s32, WallClimbReadyFromSlopeFrame)
    PLAYER_CONST_PARAM(f32, WallClimbAccel)
    PLAYER_CONST_PARAM(f32, WallClimbMaxSpeed)
    PLAYER_CONST_PARAM(f32, WallClimbDashMaxSpeed)
    PLAYER_CONST_PARAM(s32, WallClimbFrame)
    PLAYER_CONST_PARAM(s32, WallClimbFrame1)
    PLAYER_CONST_PARAM(s32, WallClimbFrame2)
    PLAYER_CONST_PARAM(s32, WallClimbDashFrame)
    PLAYER_CONST_PARAM(s32, WallClimbDashFrame1)
    PLAYER_CONST_PARAM(s32, WallClimbDashFrame2)
    PLAYER_CONST_PARAM(s32, WallClimbStopFrame)
    PLAYER_CONST_PARAM(f32, WallClimbBrakeRate)
    PLAYER_CONST_PARAM(s32, WallClimbNoWallFrame)
    PLAYER_CONST_PARAM(f32, WallClimbMaxSideSpeed)
    PLAYER_CONST_PARAM(f32, WallClimbDashMaxSideSpeed)
    PLAYER_CONST_PARAM(f32, WallClimbSideAccel)
    PLAYER_CONST_PARAM(f32, WallClimbRetainThreshold)
    PLAYER_CONST_PARAM(f32, WallClimbInvalidSideMoveDegree)
    PLAYER_CONST_PARAM(f32, WallClimbDashAnimRate)
    PLAYER_CONST_PARAM(f32, WallClimbNormalAnimRate)
    PLAYER_CONST_PARAM(s32, WallClimbJumpPowCountMax)
    PLAYER_CONST_PARAM(f32, WallClimbJumpDashAddition)
    PLAYER_CONST_PARAM(f32, WallClimbJumpPowLow)
    PLAYER_CONST_PARAM(f32, WallClimbJumpPow)
    PLAYER_CONST_PARAM(f32, WallClimbSlideGravity)
    PLAYER_CONST_PARAM(f32, WallClimbSlideMaxSpeed)
    PLAYER_CONST_PARAM(f32, WallClimbSlideSideAccel)
    PLAYER_CONST_PARAM(f32, WallClimbSlideSideMaxSpeed)
    PLAYER_CONST_PARAM(f32, WallClimbSlideStartBrakeRate)
    PLAYER_CONST_PARAM(s32, ClimbAirStopFrame)
    PLAYER_CONST_PARAM(f32, ClimbAirStopRotateVelMax)
    PLAYER_CONST_PARAM(f32, LongJumpSuccessSpeed)
    PLAYER_CONST_PARAM(f32, LongJumpSpeedMin)
    PLAYER_CONST_PARAM(f32, LongJumpBrake)
    PLAYER_CONST_PARAM(f32, LongJumpSideAccel)
    PLAYER_CONST_PARAM(s32, LongJumpCancelFrame)
    PLAYER_CONST_PARAM(s32, LongJumpFastSuccessFrame)
    PLAYER_CONST_PARAM(f32, LongJumpFastGravity)
    PLAYER_CONST_PARAM(f32, LongJumpFastJumpPow)
    PLAYER_CONST_PARAM(f32, LongJumpFastSpeed)
    PLAYER_CONST_PARAM(f32, LongJumpSlowGravity)
    PLAYER_CONST_PARAM(f32, LongJumpSlowJumpPow)
    PLAYER_CONST_PARAM(f32, LongJumpSlowSpeed)
    PLAYER_CONST_PARAM(s32, DashBrakeCommandFrame)
    PLAYER_CONST_PARAM(s32, DashBrakeActionFrame)
    PLAYER_CONST_PARAM(f32, DashBrakeSpeed)
    PLAYER_CONST_PARAM(f32, TurnJumpGravity)
    PLAYER_CONST_PARAM(f32, TurnJumpPow)
    PLAYER_CONST_PARAM(f32, TurnJumpVelH)
    PLAYER_CONST_PARAM(f32, TurnJumpBrake)
    PLAYER_CONST_PARAM(f32, TurnJumpAccel)
    PLAYER_CONST_PARAM(f32, TurnJumpSideAccel)
    PLAYER_CONST_PARAM(s32, TurnJumpToFlightDurationFrame)
    PLAYER_CONST_PARAM(f32, WaitRollingMinSpeed)
    PLAYER_CONST_PARAM(s32, WaitRollingNoBrakeFrame)
    PLAYER_CONST_PARAM(f32, WaitRollingBrakeRate)
    PLAYER_CONST_PARAM(f32, WaitRollingSideBrakeRate)
    PLAYER_CONST_PARAM(f32, WaitRollingSideAccel)
    PLAYER_CONST_PARAM(f32, WaitRollingSideMaxSpeed)
    PLAYER_CONST_PARAM(f32, NormalRollingMinSpeed)
    PLAYER_CONST_PARAM(s32, NormalRollingNoBrakeFrame)
    PLAYER_CONST_PARAM(f32, NormalRollingBrakeRate)
    PLAYER_CONST_PARAM(f32, NormalRollingSideBrakeRate)
    PLAYER_CONST_PARAM(f32, NormalRollingSideAccel)
    PLAYER_CONST_PARAM(f32, NormalRollingSideMaxSpeed)
    PLAYER_CONST_PARAM(f32, AirRollingMinSpeed)
    PLAYER_CONST_PARAM(s32, AirRollingNoBrakeFrame)
    PLAYER_CONST_PARAM(f32, AirRollingBrakeRate)
    PLAYER_CONST_PARAM(f32, AirRollingSideBrakeRate)
    PLAYER_CONST_PARAM(f32, AirRollingSideAccel)
    PLAYER_CONST_PARAM(f32, AirRollingSideMaxSpeed)
    PLAYER_CONST_PARAM(f32, AirRollingJumpPow)
    PLAYER_CONST_PARAM(f32, AirRollingGravity)
    PLAYER_CONST_PARAM(f32, RollingMinSpeed)
    PLAYER_CONST_PARAM(s32, RollingNoBrakeFrame)
    PLAYER_CONST_PARAM(f32, RollingBrakeRate)
    PLAYER_CONST_PARAM(f32, RollingSideBrakeRate)
    PLAYER_CONST_PARAM(f32, RollingSideAccel)
    PLAYER_CONST_PARAM(f32, RollingSideMaxSpeed)
    PLAYER_CONST_PARAM(f32, RaccoonDogWaitRollingMinSpeed)
    PLAYER_CONST_PARAM(s32, RaccoonDogWaitRollingNoBrakeFrame)
    PLAYER_CONST_PARAM(f32, RaccoonDogWaitRollingBrakeRate)
    PLAYER_CONST_PARAM(f32, RaccoonDogWaitRollingSideBrakeRate)
    PLAYER_CONST_PARAM(f32, RaccoonDogWaitRollingSideAccel)
    PLAYER_CONST_PARAM(f32, RaccoonDogWaitRollingSideMaxSpeed)
    PLAYER_CONST_PARAM(f32, RaccoonDogNormalRollingMinSpeed)
    PLAYER_CONST_PARAM(s32, RaccoonDogNormalRollingNoBrakeFrame)
    PLAYER_CONST_PARAM(f32, RaccoonDogNormalRollingBrakeRate)
    PLAYER_CONST_PARAM(f32, RaccoonDogNormalRollingSideBrakeRate)
    PLAYER_CONST_PARAM(f32, RaccoonDogNormalRollingSideAccel)
    PLAYER_CONST_PARAM(f32, RaccoonDogNormalRollingSideMaxSpeed)
    PLAYER_CONST_PARAM(f32, RaccoonDogDashRollingMinSpeed)
    PLAYER_CONST_PARAM(s32, RaccoonDogDashRollingNoBrakeFrame)
    PLAYER_CONST_PARAM(f32, RaccoonDogDashRollingBrakeRate)
    PLAYER_CONST_PARAM(f32, RaccoonDogDashRollingSideBrakeRate)
    PLAYER_CONST_PARAM(f32, RaccoonDogDashRollingSideAccel)
    PLAYER_CONST_PARAM(f32, RaccoonDogDashRollingSideMaxSpeed)
    PLAYER_CONST_PARAM(f32, RollingTramplePow)
    PLAYER_CONST_PARAM(f32, WaitRollingAttackJumpGravity)
    PLAYER_CONST_PARAM(f32, WaitRollingAttackJumpPow)
    PLAYER_CONST_PARAM(f32, WaitRollingAttackVelH)
    PLAYER_CONST_PARAM(f32, NormalRollingAttackJumpGravity)
    PLAYER_CONST_PARAM(f32, NormalRollingAttackJumpPow)
    PLAYER_CONST_PARAM(f32, NormalRollingAttackVelH)
    PLAYER_CONST_PARAM(f32, RollingAttackJumpGravity)
    PLAYER_CONST_PARAM(f32, RollingAttackJumpPow)
    PLAYER_CONST_PARAM(f32, RollingAttackVelH)
    PLAYER_CONST_PARAM(f32, RaccoonDogWaitRollingAttackJumpGravity)
    PLAYER_CONST_PARAM(f32, RaccoonDogWaitRollingAttackJumpPow)
    PLAYER_CONST_PARAM(f32, RaccoonDogWaitRollingAttackVelH)
    PLAYER_CONST_PARAM(f32, RaccoonDogWaitRollingAttackHighJumpGravity)
    PLAYER_CONST_PARAM(f32, RaccoonDogWaitRollingAttackHighJumpPow)
    PLAYER_CONST_PARAM(f32, RaccoonDogWaitRollingAttackHighVelH)
    PLAYER_CONST_PARAM(f32, CommonRollingAttackSpeedMin)
    PLAYER_CONST_PARAM(f32, CommonRollingAttackBrake)
    PLAYER_CONST_PARAM(f32, CommonRollingAttackSideAccel)
    PLAYER_CONST_PARAM(f32, RollingHitBound)
    PLAYER_CONST_PARAM(s32, WallHitLandCancelFrame)
    PLAYER_CONST_PARAM(s32, DamageInvalidCount)
    PLAYER_CONST_PARAM(s32, DamageCancelFrame)
    PLAYER_CONST_PARAM(s32, InvincibleFrame)
    PLAYER_CONST_PARAM(s32, InvincibleDashFrame)
    PLAYER_CONST_PARAM(f32, InvincibleDashSpeed)
    PLAYER_CONST_PARAM(f32, InvincibleJumpPow)
    PLAYER_CONST_PARAM(s32, InvincibleJumpPowCountMax)
    PLAYER_CONST_PARAM(s32, TailAttackStart)
    PLAYER_CONST_PARAM(s32, TailAttackFrame)
    PLAYER_CONST_PARAM(s32, TailAttackInterval)
    PLAYER_CONST_PARAM(f32, StandSwimRisePower)
    PLAYER_CONST_PARAM(f32, StandSwimRiseSpeedMax)
    PLAYER_CONST_PARAM(f32, StandSwimGravity)
    PLAYER_CONST_PARAM(f32, StandSwimFallSpeedMax)
    PLAYER_CONST_PARAM(f32, StandSwimHorizontalFloorDashAccel)
    PLAYER_CONST_PARAM(f32, StandSwimHorizontalFloorDashSpeedMax)
    PLAYER_CONST_PARAM(f32, StandSwimHorizontalFloorAccel)
    PLAYER_CONST_PARAM(f32, StandSwimHorizontalFloorSpeedMax)
    PLAYER_CONST_PARAM(f32, NoSinkSwimHorizontalHighAccel)
    PLAYER_CONST_PARAM(f32, NoSinkSwimHorizontalHighInputMin)
    PLAYER_CONST_PARAM(f32, NoSinkSwimHorizontalHighSpeedMax)
    PLAYER_CONST_PARAM(f32, NoSinkSwimHorizontalHighSpeedMin)
    PLAYER_CONST_PARAM(f32, StandSwimHorizontalHighAccel)
    PLAYER_CONST_PARAM(f32, StandSwimHorizontalHighSpeedMax)
    PLAYER_CONST_PARAM(f32, StandSwimHorizontalLowAccel)
    PLAYER_CONST_PARAM(f32, StandSwimHorizontalLowSpeedMax)
    PLAYER_CONST_PARAM(f32, StandSwimHorizontalBrakeRate)
    PLAYER_CONST_PARAM(s32, StandSwimHighAccelPermitFrame)
    PLAYER_CONST_PARAM(f32, StandSwimForwardBentDegree)
    PLAYER_CONST_PARAM(f32, StandSwimForwardBentBlend)
    PLAYER_CONST_PARAM(f32, StandSwimFlowFieldBlend)
    PLAYER_CONST_PARAM(f32, StandSwimRotSpeed)
    PLAYER_CONST_PARAM(f32, StandSwimSurfaceRotSpeed)
    PLAYER_CONST_PARAM(f32, StandSwimSurfaceRotSpeedNoMovement)
    PLAYER_CONST_PARAM(f32, StandSwimWalkAnimMinRate)
    PLAYER_CONST_PARAM(f32, StandSwimWalkAnimMaxRate)
    PLAYER_CONST_PARAM(f32, StandSwimWalkMaxSpeed)
    PLAYER_CONST_PARAM(s32, StandSwimPaddleAnimInterval)
    PLAYER_CONST_PARAM(s32, StandSwimPaddleAnimRateIntervalMax)
    PLAYER_CONST_PARAM(s32, StandSwimPaddleAnimRateIntervalMin)
    PLAYER_CONST_PARAM(f32, StandSwimPaddleAnimMaxRate)
    PLAYER_CONST_PARAM(f32, SwimHRotSpeed)
    PLAYER_CONST_PARAM(f32, SwimVRotSpeed)
    PLAYER_CONST_PARAM(f32, SwimPaddleAccel)
    PLAYER_CONST_PARAM(f32, SwimPaddleSpeedMax)
    PLAYER_CONST_PARAM(s32, SwimPaddleFrame)
    PLAYER_CONST_PARAM(f32, SwimKickAccel)
    PLAYER_CONST_PARAM(f32, SwimKickSpeedMax)
    PLAYER_CONST_PARAM(f32, SwimKickBrake)
    PLAYER_CONST_PARAM(f32, SwimBrake)
    PLAYER_CONST_PARAM(f32, SwimSideBrake)
    PLAYER_CONST_PARAM(s32, StandSwimFromDiveTimer)
    PLAYER_CONST_PARAM(f32, StandSwimFromDiveRisePower)
    PLAYER_CONST_PARAM(f32, StandSwimFromDiveRisePowerClimb)
    PLAYER_CONST_PARAM(f32, SwimDiveStartSpeed)
    PLAYER_CONST_PARAM(f32, SwimDiveBrake)
    PLAYER_CONST_PARAM(f32, SwimDiveEndSpeed)
    PLAYER_CONST_PARAM(s32, SwimDiveLandCount)
    PLAYER_CONST_PARAM(s32, SwimDiveLandCancelFrame)
    PLAYER_CONST_PARAM(s32, SwimDiveButtonValidFrame)
    PLAYER_CONST_PARAM(f32, DiveStartSpeed)
    PLAYER_CONST_PARAM(f32, DiveBrake)
    PLAYER_CONST_PARAM(f32, DiveBrakeSingleMode)
    PLAYER_CONST_PARAM(f32, DiveEndSpeed)
    PLAYER_CONST_PARAM(f32, StandSwimTramplePower)
    PLAYER_CONST_PARAM(f32, DiveTramplePower)
    PLAYER_CONST_PARAM(f32, DiveTrampleCancelFrame)
    PLAYER_CONST_PARAM(f32, SwimSurfaceStartDist)
    PLAYER_CONST_PARAM(f32, SwimSurfaceEndDist)
    PLAYER_CONST_PARAM(f32, SwimSurfaceStartDistShort)
    PLAYER_CONST_PARAM(f32, SwimSurfaceEndDistShort)
    PLAYER_CONST_PARAM(f32, SwimSurfaceVelDamper)
    PLAYER_CONST_PARAM(f32, SwimSurfaceGravity)
    PLAYER_CONST_PARAM(s32, SwimSurfaceValidDamperFrame)
    PLAYER_CONST_PARAM(s32, SwimSurfaceDamperLerpFrame)
    PLAYER_CONST_PARAM(f32, SwimSurfaceBaseHeight)
    PLAYER_CONST_PARAM(f32, SwimSurfaceBaseHeightShort)
    PLAYER_CONST_PARAM(f32, SwimSurfaceSpring)
    PLAYER_CONST_PARAM(f32, SwimSurfaceVerticalOffset)
    PLAYER_CONST_PARAM(f32, SwimSurfacePivotRate)
    PLAYER_CONST_PARAM(f32, SwimSurfacePivotCancelAngle)
    PLAYER_CONST_PARAM(f32, SwimSurfaceSpeedThreshold)
    PLAYER_CONST_PARAM(s32, SwimSurfacePivotCounter)
    PLAYER_CONST_PARAM(f32, SwimSurfaceTiltDuringPivotMaxDegree)
    PLAYER_CONST_PARAM(f32, SwimSurfaceTiltMaxDegree)
    PLAYER_CONST_PARAM(f32, SwimSurfaceTiltMaxFrontAngle)
    PLAYER_CONST_PARAM(f32, SwimSurfaceClimbAnimationRate)
    PLAYER_CONST_PARAM(f32, SwimSurfaceSpringForSurfaceSwim)
    PLAYER_CONST_PARAM(f32, SwimSurfaceSpringForSurfaceSwimClimb)
    PLAYER_CONST_PARAM(f32, SwimJumpPow)
    PLAYER_CONST_PARAM(s32, SwimSquatInhibitFrame)
    PLAYER_CONST_PARAM(f32, PropellerRisePow)
    PLAYER_CONST_PARAM(s32, PropellerPowSustain)
    PLAYER_CONST_PARAM(s32, PropellerPowSustainMin)
    PLAYER_CONST_PARAM(s32, PropellerPowRelease)
    PLAYER_CONST_PARAM(f32, PropellerBeforeDropGravity)
    PLAYER_CONST_PARAM(f32, PropellerRiseGravity)
    PLAYER_CONST_PARAM(f32, PropellerAfterDropGravity)
    PLAYER_CONST_PARAM(f32, PropellerFallSpeedMax)
    PLAYER_CONST_PARAM(f32, PropellerButtonOffFallSpeedMax)
    PLAYER_CONST_PARAM(f32, PropellerEngineBrakeVel)
    PLAYER_CONST_PARAM(f32, PropellerEngineBrakeRate)
    PLAYER_CONST_PARAM(f32, PropellerEngineBrakeEndVel)
    PLAYER_CONST_PARAM(f32, PropellerRotBlendRate)
    PLAYER_CONST_PARAM(f32, PropellerSideDamper)
    PLAYER_CONST_PARAM(f32, PropellerStickOffBrakeRate)
    PLAYER_CONST_PARAM(f32, LongFallDistance)
    PLAYER_CONST_PARAM(s32, StatueFallStartFrame)
    PLAYER_CONST_PARAM(s32, StatueLandFrame)
    PLAYER_CONST_PARAM(s32, StatueEndFrame)
    PLAYER_CONST_PARAM(s32, StatueEndAnimStep)
    PLAYER_CONST_PARAM(f32, StatueFallSpeedInWater)
    PLAYER_CONST_PARAM(f32, SlideSlopeAngle)
    PLAYER_CONST_PARAM(f32, SlideSlopeEndAngle)
    PLAYER_CONST_PARAM(f32, SlideEndSpeed)
    PLAYER_CONST_PARAM(f32, SlideAccel)
    PLAYER_CONST_PARAM(f32, SlideMaxSpeed)
    PLAYER_CONST_PARAM(f32, SlideSideBrake)
    PLAYER_CONST_PARAM(f32, SlideSideAccel)
    PLAYER_CONST_PARAM(f32, SlideSideMaxSpeed)
    PLAYER_CONST_PARAM(f32, SlideSideAccelOnLevelLand)
    PLAYER_CONST_PARAM(f32, SlideSideMaxSpeedOnLevelLand)
    PLAYER_CONST_PARAM(f32, SlideBrake)
    PLAYER_CONST_PARAM(f32, ForceSlideBrake)
    PLAYER_CONST_PARAM(f32, SlidePostureBlendRate)
    PLAYER_CONST_PARAM(f32, ForceSlideSpeed)
    PLAYER_CONST_PARAM(f32, ForceSlideSpeedUpRate)
    PLAYER_CONST_PARAM(f32, SlideTiltBlendRate)
    PLAYER_CONST_PARAM(f32, SlideTiltMaxDegree)
    PLAYER_CONST_PARAM(s32, SlideInvalidFrame)
    PLAYER_CONST_PARAM(f32, ForceSlideMaxSpeed)
    PLAYER_CONST_PARAM(s32, SlideFallCancelFrame)
    PLAYER_CONST_PARAM(f32, SlideJumpHVelScale)
    PLAYER_CONST_PARAM(s32, HoldShakeInterval)
    PLAYER_CONST_PARAM(s32, HoldThrowFrontTiming)
    PLAYER_CONST_PARAM(s32, HoldThrowUpTiming)
    PLAYER_CONST_PARAM(f32, HoldJumpFrontVel)
    PLAYER_CONST_PARAM(f32, HoldJumpUpVel)
    PLAYER_CONST_PARAM(s32, ClimbAttackInterval)
    PLAYER_CONST_PARAM(s32, ClimbAttackWaitInterval)
    PLAYER_CONST_PARAM(s32, ClimbAttackCancelFrame)
    PLAYER_CONST_PARAM(s32, ClimbAttackSensorOnFrame)
    PLAYER_CONST_PARAM(f32, ClimbBodyAttackFrontVel)
    PLAYER_CONST_PARAM(f32, ClimbBodyAttackDownVel)
    PLAYER_CONST_PARAM(s32, ClimbBodyAttackFrame)
    PLAYER_CONST_PARAM(f32, ClimbBodyAttackGravity)
    PLAYER_CONST_PARAM(f32, ClimbBodyAttackFallSpeedMax)
    PLAYER_CONST_PARAM(f32, ClimbBodyAttackHBrakeRate)
    PLAYER_CONST_PARAM(f32, ClimbBodyAttackSideAccel)
    PLAYER_CONST_PARAM(f32, ClimbBodyAttackSideMoveDist)
    PLAYER_CONST_PARAM(f32, SinkSandMoveMaxSpeed)
    PLAYER_CONST_PARAM(f32, SinkSandMoveMaxDashSpeed)
    PLAYER_CONST_PARAM(s32, SinkSandInvalidFrameInJump)
    PLAYER_CONST_PARAM(f32, PushedBrakeRate)
    PLAYER_CONST_PARAM(f32, PushedBrakeMaxRate)
    PLAYER_CONST_PARAM(f32, PushedJumpCancelSpeed)
    PLAYER_CONST_PARAM(s32, GroundSpinFrame)
    PLAYER_CONST_PARAM(f32, GroundSpinAccel)
    PLAYER_CONST_PARAM(f32, GroundSpinBrake)
    PLAYER_CONST_PARAM(f32, GroundSpinVelMax)
    PLAYER_CONST_PARAM(f32, SpinJumpGravity)
    PLAYER_CONST_PARAM(f32, SpinJumpPow)
    PLAYER_CONST_PARAM(s32, SpinAttackInterval)
    PLAYER_CONST_PARAM(s32, SpinAttackCancelFrame)
    PLAYER_CONST_PARAM(s32, SpinAttackSensorOnFrame)
    PLAYER_CONST_PARAM(f32, SpinAttackJumpPow)
    PLAYER_CONST_PARAM(f32, SpinAttackJumpGravity)
    PLAYER_CONST_PARAM(f32, SpinAttackGroundBrake)
    PLAYER_CONST_PARAM(f32, SkateJumpGravity)
    PLAYER_CONST_PARAM(f32, SkateJumpPowLow)
    PLAYER_CONST_PARAM(f32, SkateJumpPow)
    PLAYER_CONST_PARAM(s32, SkateJumpPowCountMax)
    PLAYER_CONST_PARAM(f32, SkateJumpThreshold)
    PLAYER_CONST_PARAM(s32, CoopHipDropFrame)
    PLAYER_CONST_PARAM(f32, CoopHipDropRadiusMin)
    PLAYER_CONST_PARAM(f32, CoopHipDropRadius)
    PLAYER_CONST_PARAM(s32, GiantHipDropFrame)
    PLAYER_CONST_PARAM(f32, GiantHipDropRadiusMin)
    PLAYER_CONST_PARAM(f32, GiantHipDropRadiusMax)
    PLAYER_CONST_PARAM(f32, KnockDownVelH)
    PLAYER_CONST_PARAM(f32, KnockDownVelV)
    PLAYER_CONST_PARAM(s32, KnockDownCancelFrame)
    PLAYER_CONST_PARAM(f32, ReflectJumpGravity)
    PLAYER_CONST_PARAM(f32, ReflectJump)
    PLAYER_CONST_PARAM(f32, RisingReflectJumpHVelBrakeRate)
    PLAYER_CONST_PARAM(s32, TossCancelFrame)
    PLAYER_CONST_PARAM(s32, ManekinekoFallStartFrame)
    PLAYER_CONST_PARAM(s32, ManekinekoLandFrame)
    PLAYER_CONST_PARAM(s32, ManekinekoEndNoticeFrame)
    PLAYER_CONST_PARAM(s32, ManekinekoEndFrame)
    PLAYER_CONST_PARAM(s32, ManekinekoCancelFrame)
    PLAYER_CONST_PARAM(f32, ManekinekoFallSpeedInWater)
    PLAYER_CONST_PARAM(s32, GroomingMaxInterval)
    PLAYER_CONST_PARAM(s32, GroomingMinInterval)
    PLAYER_CONST_PARAM(s32, SePropellerBeginStep)
    PLAYER_CONST_PARAM(f32, SeFootNoteNormalVolMul)
    PLAYER_CONST_PARAM(f32, SeFootNoteNormalPitDec)
    PLAYER_CONST_PARAM(f32, SeFootNoteDashVolAdd)
    PLAYER_CONST_PARAM(f32, SeFootNoteDashPitAdd)
    PLAYER_CONST_PARAM(f32, GigaCommonAnimRate)
    PLAYER_CONST_PARAM(f32, GigaMiniRunAnimRateMax)
    PLAYER_CONST_PARAM(f32, GigaMiniDashAnimRateMax)
    PLAYER_CONST_PARAM(f32, GigaSuperRunAnimRateMax)
    PLAYER_CONST_PARAM(f32, GigaSuperDashAnimRateMax)
    PLAYER_CONST_PARAM(f32, GigaClimbRunAnimRateMax)
    PLAYER_CONST_PARAM(f32, GigaClimbDashAnimRateMax)
    PLAYER_CONST_PARAM(f32, GigaNormalMaxSpeed)
    PLAYER_CONST_PARAM(f32, GigaDashMaxSpeed)
    PLAYER_CONST_PARAM(f32, GigaSuperDashSpeed)
    PLAYER_CONST_PARAM(f32, GigaInvincibleDashSpeed)
    PLAYER_CONST_PARAM(s32, GigaAccelFrame)
    PLAYER_CONST_PARAM(f32, GigaSquatWalkSpeed)
    PLAYER_CONST_PARAM(f32, GigaGroundSpinAccel)
    PLAYER_CONST_PARAM(f32, GigaGroundSpinBrake)
    PLAYER_CONST_PARAM(f32, GigaKnockDownVelH)
    PLAYER_CONST_PARAM(f32, GigaKnockDownVelV)
    PLAYER_CONST_PARAM(f32, GigaLeftFootHrTime)
    PLAYER_CONST_PARAM(f32, GigaRightFootHrTime)
    PLAYER_CONST_PARAM(f32, GigaClimbLeftWalkHrTime)
    PLAYER_CONST_PARAM(f32, GigaClimbRightWalkHrTime)
    PLAYER_CONST_PARAM(f32, GigaClimbLeftRunHrTime)
    PLAYER_CONST_PARAM(f32, GigaClimbRightRunHrTime)
    PLAYER_CONST_PARAM(f32, GigaRoundLimitDegreeMax)
    PLAYER_CONST_PARAM(f32, GigaRoundLimitDegreeMin)
    PLAYER_CONST_PARAM(f32, GigaNormalRollingMinSpeed)
    PLAYER_CONST_PARAM(f32, GigaNormalGravityAddition)
    PLAYER_CONST_PARAM(f32, GigaSquatBrakeRate)
    PLAYER_CONST_PARAM(f32, GigaGravity)
    PLAYER_CONST_PARAM(f32, GigaFallSpeedMax)
    PLAYER_CONST_PARAM(f32, GigaFloatFallSpeedMax)
    PLAYER_CONST_PARAM(f32, GigaJumpPow)
    PLAYER_CONST_PARAM(f32, GigaJumpPowLow)
    PLAYER_CONST_PARAM(s32, GigaJumpPowCountMax)
    PLAYER_CONST_PARAM(f32, GigaJumpCancelBrakeRate)
    PLAYER_CONST_PARAM(f32, GigaJumpCancelMinSpeed)
    PLAYER_CONST_PARAM(f32, GigaHipDropSpeed)
    PLAYER_CONST_PARAM(f32, GigaHipDropAnimRate)
    PLAYER_CONST_PARAM(f32, GigaHipDropJumpPow)
    PLAYER_CONST_PARAM(f32, GigaTrampleJump)
    PLAYER_CONST_PARAM(f32, GigaLongJumpSlowSpeed)
    PLAYER_CONST_PARAM(f32, GigaLongJumpFastSpeed)
    PLAYER_CONST_PARAM(f32, GigaLongJumpSlowJumpPow)
    PLAYER_CONST_PARAM(f32, GigaLongJumpFastJumpPow)
    PLAYER_CONST_PARAM(f32, GigaLongJumpSlowGravity)
    PLAYER_CONST_PARAM(f32, GigaLongJumpFastGravity)
    PLAYER_CONST_PARAM(f32, GigaSquatJumpGravity)
    PLAYER_CONST_PARAM(f32, GigaSquatJumpPow)
    PLAYER_CONST_PARAM(f32, GigaSquatHighJumpPow)
    PLAYER_CONST_PARAM(f32, GigaSquatJumpBackPow)
    PLAYER_CONST_PARAM(f32, GigaSpinJumpGravity)
    PLAYER_CONST_PARAM(f32, GigaSpinJumpPow)
    PLAYER_CONST_PARAM(f32, GigaNormalRollingAttackJumpGravity)
    PLAYER_CONST_PARAM(f32, GigaNormalRollingAttackJumpPow)
    PLAYER_CONST_PARAM(f32, GigaNormalRollingAttackVelH)
    PLAYER_CONST_PARAM(f32, GigaWallJumpHSpeed)
    PLAYER_CONST_PARAM(f32, GigaLongJumpBrake)
    PLAYER_CONST_PARAM(f32, GigaLongJumpSpeedMin)
    PLAYER_CONST_PARAM(f32, GigaLongJumpSideAccel)
    PLAYER_CONST_PARAM(s32, GigaLandFrame)
    PLAYER_CONST_PARAM(f32, GigaWallClimbMaxSpeed)
    PLAYER_CONST_PARAM(f32, GigaWallClimbDashMaxSpeed)
    PLAYER_CONST_PARAM(f32, GigaWallClimbAccel)
    PLAYER_CONST_PARAM(f32, GigaWallClimbMaxSideSpeed)
    PLAYER_CONST_PARAM(f32, GigaWallClimbDashMaxSideSpeed)
    PLAYER_CONST_PARAM(f32, GigaWallClimbSideAccel)
    PLAYER_CONST_PARAM(f32, GigaWallSnapDistance)
    PLAYER_CONST_PARAM(f32, GigaWallClimbJumpPowLow)
    PLAYER_CONST_PARAM(f32, GigaWallClimbJumpPow)
    PLAYER_CONST_PARAM(f32, GigaWallClimbSlideGravity)
    PLAYER_CONST_PARAM(f32, GigaWallClimbSlideMaxSpeed)
    PLAYER_CONST_PARAM(f32, GigaWallClimbSlideSideAccel)
    PLAYER_CONST_PARAM(f32, GigaWallClimbSlideSideMaxSpeed)
    PLAYER_CONST_PARAM(f32, FlashRangeAttackLengthOffset)
    PLAYER_CONST_PARAM(f32, FlashRangeAttackDegreeScale)
    PLAYER_CONST_PARAM(f32, FlashRangeAttackHeightMax)
    PLAYER_CONST_PARAM(f32, FlashRangeAttackHeightMin)
    PLAYER_CONST_PARAM(s32, IsEnableHeadLightOfx)
    PLAYER_CONST_PARAM(f32, HeadLightOfxOffsetY)
    PLAYER_CONST_PARAM(f32, HeadLightOfxOffsetZ)
    PLAYER_CONST_PARAM(f32, HeadLightPrePassPointLightRadiusScale)
    PLAYER_CONST_PARAM(f32, HeadLightPrePassPointLightOffsetY)
    PLAYER_CONST_PARAM(f32, HeadLightPrePassPointLightOffsetZ)

    /// Make the getters ask the override parameter set (or stop doing so).
    void setOverride(bool isOverride) { mIsOverride = isOverride; }

private:
    f32 mGravity;  // 0x8
    f32 mCenterHeight;  // 0xc
    f32 mBodyRadius;  // 0x10
    f32 mCollectInfoRadiusAddition;  // 0x14
    f32 mSnapGroundMaxLength;  // 0x18
    f32 mSnapWallMaxLength;  // 0x1c
    f32 mStickRoundThreshold;  // 0x20
    f32 mHeightCheckLength;  // 0x24
    f32 mCutVelLimit;  // 0x28
    f32 mCutVelRate;  // 0x2c
    s32 mThrowInvalidationFrames;  // 0x30
    f32 mTall;  // 0x34
    f32 mChestRadius;  // 0x38
    f32 mDashCheckRadius;  // 0x3c
    f32 mShadowCheckLength;  // 0x40
    f32 mShadowLengthMax;  // 0x44
    s32 mPivotFrame;  // 0x48
    f32 mPivotDegree;  // 0x4c
    f32 mNormalMaxSpeed;  // 0x50
    f32 mDashMaxSpeed;  // 0x54
    f32 mSuperDashSpeed;  // 0x58
    s32 mSuperDashTimer;  // 0x5c
    s32 mSuperDashTimerMini;  // 0x60
    s32 mSuperDashTimerFire;  // 0x64
    s32 mSuperDashTimerClimb;  // 0x68
    s32 mSuperDashTimerRaccoonDog;  // 0x6c
    s32 mSuperDashTimerBoomerang;  // 0x70
    s32 mSuperDashTimerRaccoonDogWhite;  // 0x74
    f32 mSuperDashStartAnimRate;  // 0x78
    s32 mSuperDashStartAnimFrame;  // 0x7c
    s32 mBrakeFrame;  // 0x80
    s32 mDashBrakeFrame;  // 0x84
    s32 mStickOnBrakeFrame;  // 0x88
    s32 mBrakeFrameOnIce;  // 0x8c
    s32 mAccelFrame;  // 0x90
    s32 mDashAccelFrame;  // 0x94
    f32 mRoundLimitDegreeMax;  // 0x98
    f32 mRoundLimitDegreeMin;  // 0x9c
    f32 mRunAnimRateMax;  // 0xa0
    f32 mGiantRunAnimRateMax;  // 0xa4
    f32 mShortAnimRateEff;  // 0xa8
    s32 mDashStartFrame;  // 0xac
    s32 mModifiedDashStartFrame;  // 0xb0
    s32 mDashStartBlendFrame;  // 0xb4
    s32 mDashInputSuccessFrame;  // 0xb8
    f32 mDownHillAccelStartDegree;  // 0xbc
    f32 mDownHillAccelEndDegree;  // 0xc0
    f32 mDownHillAccelAddRate;  // 0xc4
    f32 mDashPanelSpeed;  // 0xc8
    f32 mModifiedDashPanelSpeed;  // 0xcc
    f32 mDashPanelOverRate;  // 0xd0
    s32 mDashPanelTimer;  // 0xd4
    f32 mFlingPoleSpeed;  // 0xd8
    s32 mGroundOffFrame;  // 0xdc
    s32 mClimbToGroundMoveFrame;  // 0xe0
    f32 mTiltMaxDegree;  // 0xe4
    f32 mTiltBlendRate;  // 0xe8
    f32 mTiltMaxFrontAngle;  // 0xec
    f32 mHoldingTiltMaxFrontAngle;  // 0xf0
    f32 mTiltStartSpeed;  // 0xf4
    f32 mTiltEndSpeed;  // 0xf8
    f32 mClimbRunAnimRateEff;  // 0xfc
    f32 mPanelDashAnimRate;  // 0x100
    f32 mModifiedPanelDashAnimRate;  // 0x104
    f32 mSlopeMaxSpeedScale;  // 0x108
    f32 mMaxSpeedScale;  // 0x10c
    f32 mDashSignAnimRate;  // 0x110
    f32 mDashSignMaxLoop;  // 0x114
    f32 mDashSignMaxSpeed;  // 0x118
    s32 mDashSignAnimFrameMax;  // 0x11c
    f32 mJumpDirRotLimit;  // 0x120
    f32 mJumpPowLow;  // 0x124
    f32 mJumpPow;  // 0x128
    s32 mJumpPowCountMax;  // 0x12c
    f32 mJumpExtensionGravityRate;  // 0x130
    f32 mJumpSideVelRate;  // 0x134
    f32 mJumpFrontBrakeRate;  // 0x138
    f32 mJumpRotBlendRate;  // 0x13c
    s32 mJumpAccelFrame;  // 0x140
    s32 mReleaseAccelFrame;  // 0x144
    s32 mJumpAccelAddFrame;  // 0x148
    f32 mJumpCancelBrakeRate;  // 0x14c
    f32 mJumpCancelMinSpeed;  // 0x150
    s32 mContinuousJumpTimer;  // 0x154
    s32 mContinuousJumpCount;  // 0x158
    f32 mFallSpeedMax;  // 0x15c
    f32 mDashJumpAddition;  // 0x160
    f32 mPunchReflectPower;  // 0x164
    s32 mGlideInhibitFrameAfterPunch;  // 0x168
    f32 mWalkMinSpeedRate;  // 0x16c
    s32 mJumpHVelSamplingNum;  // 0x170
    f32 mFollowFrontDamper;  // 0x174
    f32 mFollowBackDamper;  // 0x178
    s32 mFlightDurationCount;  // 0x17c
    f32 mFlightDurationRotBlendRate;  // 0x180
    s32 mFlightDurationJumpStartInhibitFrame;  // 0x184
    f32 mFlightDurationSideDamper;  // 0x188
    f32 mRaccoonDogFallSpeedMax;  // 0x18c
    f32 mRaccoonDogFallGravityRate;  // 0x190
    f32 mRaccoonDogFallGravityAdd;  // 0x194
    f32 mRaccoonDogFirstFallDamper;  // 0x198
    f32 mRaccoonDogRotBlendRate;  // 0x19c
    f32 mRaccoonDogFallSideDamper;  // 0x1a0
    f32 mTrampleJumpGravity;  // 0x1a4
    f32 mTrampleJump;  // 0x1a8
    f32 mTrampleJumpSideVelRate;  // 0x1ac
    f32 mTrampleHighJumpGravity;  // 0x1b0
    f32 mTrampleHighJump;  // 0x1b4
    f32 mTrampleHipDropGravity;  // 0x1b8
    f32 mTrampleHipDropJump;  // 0x1bc
    f32 mRisingTrampleJumpGravity;  // 0x1c0
    f32 mRisingTrampleJump;  // 0x1c4
    f32 mRisingTrampleHighJumpGravity;  // 0x1c8
    f32 mRisingTrampleHighJump;  // 0x1cc
    f32 mRisingTrampleHVelBrakeRate;  // 0x1d0
    f32 mPunchedJumpGravity;  // 0x1d4
    f32 mPunchedJump;  // 0x1d8
    f32 mRisingPunchedHVelBrakeRate;  // 0x1dc
    f32 mTossedJumpGravity;  // 0x1e0
    f32 mTossedJump;  // 0x1e4
    f32 mTossedHighJumpGravity;  // 0x1e8
    f32 mTossedHighJump;  // 0x1ec
    f32 mTossedHipDropGravity;  // 0x1f0
    f32 mTossedHipDropJump;  // 0x1f4
    f32 mRisingTossedJumpGravity;  // 0x1f8
    f32 mRisingTossedJump;  // 0x1fc
    f32 mRisingTossedHighJumpGravity;  // 0x200
    f32 mRisingTossedHighJump;  // 0x204
    f32 mRisingTossedHVelBrakeRate;  // 0x208
    f32 mSquatBrakeEndSpeed;  // 0x20c
    f32 mSquatShiftSpeedRate;  // 0x210
    f32 mSquatAccelRate;  // 0x214
    s32 mSquatNoBrakeFrame;  // 0x218
    f32 mSquatBrakeRate;  // 0x21c
    f32 mSquatBrakeRateOnSkate;  // 0x220
    f32 mSquatBrakeSideAccel;  // 0x224
    f32 mSquatBrakeSideBrakeRate;  // 0x228
    f32 mSquatBrakeSideBrakeRateOnSkate;  // 0x22c
    f32 mSquatBrakeSideMaxSpeedRate;  // 0x230
    f32 mSquatWalkSpeed;  // 0x234
    f32 mSquatWalkFrontVecBlend;  // 0x238
    s32 mSquatEnergyAccelFrame;  // 0x23c
    f32 mSquatJumpGravity;  // 0x240
    f32 mSquatJumpPow;  // 0x244
    f32 mSquatHighJumpPow;  // 0x248
    f32 mSquatJumpBackPow;  // 0x24c
    f32 mSquatJumpTramplePow;  // 0x250
    f32 mHipDropSpeed;  // 0x254
    s32 mHipDropLandCancelFrame;  // 0x258
    f32 mHipDropHeight;  // 0x25c
    s32 mHipDropMsgInterval;  // 0x260
    f32 mHipDropKnockDownRadiusMin;  // 0x264
    f32 mHipDropKnockDownRadiusMax;  // 0x268
    f32 mHipDropStartAnimRate;  // 0x26c
    s32 mHipDropKnockDownFrame;  // 0x270
    f32 mHipDropJumpPow;  // 0x274
    s32 mHipDropJumpPowCountMax;  // 0x278
    s32 mHipDropJumpPermitBeginFrame;  // 0x27c
    s32 mHipDropJumpPermitEndFrame;  // 0x280
    f32 mWallHeightLowLimit;  // 0x284
    f32 mWallGravity;  // 0x288
    f32 mWallMaxSpeed;  // 0x28c
    f32 mWallApartFrame;  // 0x290
    f32 mWallSnapDistance;  // 0x294
    f32 mWallSlideMaxSpeed;  // 0x298
    f32 mWallSlideAccel;  // 0x29c
    s32 mWallInhibitAfterPunch;  // 0x2a0
    f32 mWallJumpGravity;  // 0x2a4
    f32 mWallJumpHSpeed;  // 0x2a8
    f32 mWallJumpPow;  // 0x2ac
    s32 mWallJumpInvalidateInputFrame;  // 0x2b0
    s32 mWallJumpDirEffectiveFrame;  // 0x2b4
    f32 mWallJumpDirLimit;  // 0x2b8
    f32 mWallJumpLimitPlay;  // 0x2bc
    s32 mWallClimbReadyFrame;  // 0x2c0
    s32 mWallClimbReadyWaitFrame;  // 0x2c4
    s32 mWallClimbReadyFromSlopeFrame;  // 0x2c8
    f32 mWallClimbAccel;  // 0x2cc
    f32 mWallClimbMaxSpeed;  // 0x2d0
    f32 mWallClimbDashMaxSpeed;  // 0x2d4
    s32 mWallClimbFrame;  // 0x2d8
    s32 mWallClimbFrame1;  // 0x2dc
    s32 mWallClimbFrame2;  // 0x2e0
    s32 mWallClimbDashFrame;  // 0x2e4
    s32 mWallClimbDashFrame1;  // 0x2e8
    s32 mWallClimbDashFrame2;  // 0x2ec
    s32 mWallClimbStopFrame;  // 0x2f0
    f32 mWallClimbBrakeRate;  // 0x2f4
    s32 mWallClimbNoWallFrame;  // 0x2f8
    f32 mWallClimbMaxSideSpeed;  // 0x2fc
    f32 mWallClimbDashMaxSideSpeed;  // 0x300
    f32 mWallClimbSideAccel;  // 0x304
    f32 mWallClimbRetainThreshold;  // 0x308
    f32 mWallClimbInvalidSideMoveDegree;  // 0x30c
    f32 mWallClimbDashAnimRate;  // 0x310
    f32 mWallClimbNormalAnimRate;  // 0x314
    s32 mWallClimbJumpPowCountMax;  // 0x318
    f32 mWallClimbJumpDashAddition;  // 0x31c
    f32 mWallClimbJumpPowLow;  // 0x320
    f32 mWallClimbJumpPow;  // 0x324
    f32 mWallClimbSlideGravity;  // 0x328
    f32 mWallClimbSlideMaxSpeed;  // 0x32c
    f32 mWallClimbSlideSideAccel;  // 0x330
    f32 mWallClimbSlideSideMaxSpeed;  // 0x334
    f32 mWallClimbSlideStartBrakeRate;  // 0x338
    s32 mClimbAirStopFrame;  // 0x33c
    f32 mClimbAirStopRotateVelMax;  // 0x340
    f32 mLongJumpSuccessSpeed;  // 0x344
    f32 mLongJumpSpeedMin;  // 0x348
    f32 mLongJumpBrake;  // 0x34c
    f32 mLongJumpSideAccel;  // 0x350
    s32 mLongJumpCancelFrame;  // 0x354
    s32 mLongJumpFastSuccessFrame;  // 0x358
    f32 mLongJumpFastGravity;  // 0x35c
    f32 mLongJumpFastJumpPow;  // 0x360
    f32 mLongJumpFastSpeed;  // 0x364
    f32 mLongJumpSlowGravity;  // 0x368
    f32 mLongJumpSlowJumpPow;  // 0x36c
    f32 mLongJumpSlowSpeed;  // 0x370
    s32 mDashBrakeCommandFrame;  // 0x374
    s32 mDashBrakeActionFrame;  // 0x378
    f32 mDashBrakeSpeed;  // 0x37c
    f32 mTurnJumpGravity;  // 0x380
    f32 mTurnJumpPow;  // 0x384
    f32 mTurnJumpVelH;  // 0x388
    f32 mTurnJumpBrake;  // 0x38c
    f32 mTurnJumpAccel;  // 0x390
    f32 mTurnJumpSideAccel;  // 0x394
    s32 mTurnJumpToFlightDurationFrame;  // 0x398
    f32 mWaitRollingMinSpeed;  // 0x39c
    s32 mWaitRollingNoBrakeFrame;  // 0x3a0
    f32 mWaitRollingBrakeRate;  // 0x3a4
    f32 mWaitRollingSideBrakeRate;  // 0x3a8
    f32 mWaitRollingSideAccel;  // 0x3ac
    f32 mWaitRollingSideMaxSpeed;  // 0x3b0
    f32 mNormalRollingMinSpeed;  // 0x3b4
    s32 mNormalRollingNoBrakeFrame;  // 0x3b8
    f32 mNormalRollingBrakeRate;  // 0x3bc
    f32 mNormalRollingSideBrakeRate;  // 0x3c0
    f32 mNormalRollingSideAccel;  // 0x3c4
    f32 mNormalRollingSideMaxSpeed;  // 0x3c8
    f32 mAirRollingMinSpeed;  // 0x3cc
    s32 mAirRollingNoBrakeFrame;  // 0x3d0
    f32 mAirRollingBrakeRate;  // 0x3d4
    f32 mAirRollingSideBrakeRate;  // 0x3d8
    f32 mAirRollingSideAccel;  // 0x3dc
    f32 mAirRollingSideMaxSpeed;  // 0x3e0
    f32 mAirRollingJumpPow;  // 0x3e4
    f32 mAirRollingGravity;  // 0x3e8
    f32 mRollingMinSpeed;  // 0x3ec
    s32 mRollingNoBrakeFrame;  // 0x3f0
    f32 mRollingBrakeRate;  // 0x3f4
    f32 mRollingSideBrakeRate;  // 0x3f8
    f32 mRollingSideAccel;  // 0x3fc
    f32 mRollingSideMaxSpeed;  // 0x400
    f32 mRaccoonDogWaitRollingMinSpeed;  // 0x404
    s32 mRaccoonDogWaitRollingNoBrakeFrame;  // 0x408
    f32 mRaccoonDogWaitRollingBrakeRate;  // 0x40c
    f32 mRaccoonDogWaitRollingSideBrakeRate;  // 0x410
    f32 mRaccoonDogWaitRollingSideAccel;  // 0x414
    f32 mRaccoonDogWaitRollingSideMaxSpeed;  // 0x418
    f32 mRaccoonDogNormalRollingMinSpeed;  // 0x41c
    s32 mRaccoonDogNormalRollingNoBrakeFrame;  // 0x420
    f32 mRaccoonDogNormalRollingBrakeRate;  // 0x424
    f32 mRaccoonDogNormalRollingSideBrakeRate;  // 0x428
    f32 mRaccoonDogNormalRollingSideAccel;  // 0x42c
    f32 mRaccoonDogNormalRollingSideMaxSpeed;  // 0x430
    f32 mRaccoonDogDashRollingMinSpeed;  // 0x434
    s32 mRaccoonDogDashRollingNoBrakeFrame;  // 0x438
    f32 mRaccoonDogDashRollingBrakeRate;  // 0x43c
    f32 mRaccoonDogDashRollingSideBrakeRate;  // 0x440
    f32 mRaccoonDogDashRollingSideAccel;  // 0x444
    f32 mRaccoonDogDashRollingSideMaxSpeed;  // 0x448
    f32 mRollingTramplePow;  // 0x44c
    f32 mWaitRollingAttackJumpGravity;  // 0x450
    f32 mWaitRollingAttackJumpPow;  // 0x454
    f32 mWaitRollingAttackVelH;  // 0x458
    f32 mNormalRollingAttackJumpGravity;  // 0x45c
    f32 mNormalRollingAttackJumpPow;  // 0x460
    f32 mNormalRollingAttackVelH;  // 0x464
    f32 mRollingAttackJumpGravity;  // 0x468
    f32 mRollingAttackJumpPow;  // 0x46c
    f32 mRollingAttackVelH;  // 0x470
    f32 mRaccoonDogWaitRollingAttackJumpGravity;  // 0x474
    f32 mRaccoonDogWaitRollingAttackJumpPow;  // 0x478
    f32 mRaccoonDogWaitRollingAttackVelH;  // 0x47c
    f32 mRaccoonDogWaitRollingAttackHighJumpGravity;  // 0x480
    f32 mRaccoonDogWaitRollingAttackHighJumpPow;  // 0x484
    f32 mRaccoonDogWaitRollingAttackHighVelH;  // 0x488
    f32 mCommonRollingAttackSpeedMin;  // 0x48c
    f32 mCommonRollingAttackBrake;  // 0x490
    f32 mCommonRollingAttackSideAccel;  // 0x494
    f32 mRollingHitBound;  // 0x498
    s32 mWallHitLandCancelFrame;  // 0x49c
    s32 mDamageInvalidCount;  // 0x4a0
    s32 mDamageCancelFrame;  // 0x4a4
    s32 mInvincibleFrame;  // 0x4a8
    s32 mInvincibleDashFrame;  // 0x4ac
    f32 mInvincibleDashSpeed;  // 0x4b0
    f32 mInvincibleJumpPow;  // 0x4b4
    s32 mInvincibleJumpPowCountMax;  // 0x4b8
    s32 mTailAttackStart;  // 0x4bc
    s32 mTailAttackFrame;  // 0x4c0
    s32 mTailAttackInterval;  // 0x4c4
    f32 mStandSwimRisePower;  // 0x4c8
    f32 mStandSwimRiseSpeedMax;  // 0x4cc
    f32 mStandSwimGravity;  // 0x4d0
    f32 mStandSwimFallSpeedMax;  // 0x4d4
    f32 mStandSwimHorizontalFloorDashAccel;  // 0x4d8
    f32 mStandSwimHorizontalFloorDashSpeedMax;  // 0x4dc
    f32 mStandSwimHorizontalFloorAccel;  // 0x4e0
    f32 mStandSwimHorizontalFloorSpeedMax;  // 0x4e4
    f32 mNoSinkSwimHorizontalHighAccel;  // 0x4e8
    f32 mNoSinkSwimHorizontalHighInputMin;  // 0x4ec
    f32 mNoSinkSwimHorizontalHighSpeedMax;  // 0x4f0
    f32 mNoSinkSwimHorizontalHighSpeedMin;  // 0x4f4
    f32 mStandSwimHorizontalHighAccel;  // 0x4f8
    f32 mStandSwimHorizontalHighSpeedMax;  // 0x4fc
    f32 mStandSwimHorizontalLowAccel;  // 0x500
    f32 mStandSwimHorizontalLowSpeedMax;  // 0x504
    f32 mStandSwimHorizontalBrakeRate;  // 0x508
    s32 mStandSwimHighAccelPermitFrame;  // 0x50c
    f32 mStandSwimForwardBentDegree;  // 0x510
    f32 mStandSwimForwardBentBlend;  // 0x514
    f32 mStandSwimFlowFieldBlend;  // 0x518
    f32 mStandSwimRotSpeed;  // 0x51c
    f32 mStandSwimSurfaceRotSpeed;  // 0x520
    f32 mStandSwimSurfaceRotSpeedNoMovement;  // 0x524
    f32 mStandSwimWalkAnimMinRate;  // 0x528
    f32 mStandSwimWalkAnimMaxRate;  // 0x52c
    f32 mStandSwimWalkMaxSpeed;  // 0x530
    s32 mStandSwimPaddleAnimInterval;  // 0x534
    s32 mStandSwimPaddleAnimRateIntervalMax;  // 0x538
    s32 mStandSwimPaddleAnimRateIntervalMin;  // 0x53c
    f32 mStandSwimPaddleAnimMaxRate;  // 0x540
    f32 mSwimHRotSpeed;  // 0x544
    f32 mSwimVRotSpeed;  // 0x548
    f32 mSwimPaddleAccel;  // 0x54c
    f32 mSwimPaddleSpeedMax;  // 0x550
    s32 mSwimPaddleFrame;  // 0x554
    f32 mSwimKickAccel;  // 0x558
    f32 mSwimKickSpeedMax;  // 0x55c
    f32 mSwimKickBrake;  // 0x560
    f32 mSwimBrake;  // 0x564
    f32 mSwimSideBrake;  // 0x568
    s32 mStandSwimFromDiveTimer;  // 0x56c
    f32 mStandSwimFromDiveRisePower;  // 0x570
    f32 mStandSwimFromDiveRisePowerClimb;  // 0x574
    f32 mSwimDiveStartSpeed;  // 0x578
    f32 mSwimDiveBrake;  // 0x57c
    f32 mSwimDiveEndSpeed;  // 0x580
    s32 mSwimDiveLandCount;  // 0x584
    s32 mSwimDiveLandCancelFrame;  // 0x588
    s32 mSwimDiveButtonValidFrame;  // 0x58c
    f32 mDiveStartSpeed;  // 0x590
    f32 mDiveBrake;  // 0x594
    f32 mDiveBrakeSingleMode;  // 0x598
    f32 mDiveEndSpeed;  // 0x59c
    f32 mStandSwimTramplePower;  // 0x5a0
    f32 mDiveTramplePower;  // 0x5a4
    f32 mDiveTrampleCancelFrame;  // 0x5a8
    f32 mSwimSurfaceStartDist;  // 0x5ac
    f32 mSwimSurfaceEndDist;  // 0x5b0
    f32 mSwimSurfaceStartDistShort;  // 0x5b4
    f32 mSwimSurfaceEndDistShort;  // 0x5b8
    f32 mSwimSurfaceVelDamper;  // 0x5bc
    f32 mSwimSurfaceGravity;  // 0x5c0
    s32 mSwimSurfaceValidDamperFrame;  // 0x5c4
    s32 mSwimSurfaceDamperLerpFrame;  // 0x5c8
    f32 mSwimSurfaceBaseHeight;  // 0x5cc
    f32 mSwimSurfaceBaseHeightShort;  // 0x5d0
    f32 mSwimSurfaceSpring;  // 0x5d4
    f32 mSwimSurfaceVerticalOffset;  // 0x5d8
    f32 mSwimSurfacePivotRate;  // 0x5dc
    f32 mSwimSurfacePivotCancelAngle;  // 0x5e0
    f32 mSwimSurfaceSpeedThreshold;  // 0x5e4
    s32 mSwimSurfacePivotCounter;  // 0x5e8
    f32 mSwimSurfaceTiltDuringPivotMaxDegree;  // 0x5ec
    f32 mSwimSurfaceTiltMaxDegree;  // 0x5f0
    f32 mSwimSurfaceTiltMaxFrontAngle;  // 0x5f4
    f32 mSwimSurfaceClimbAnimationRate;  // 0x5f8
    f32 mSwimSurfaceSpringForSurfaceSwim;  // 0x5fc
    f32 mSwimSurfaceSpringForSurfaceSwimClimb;  // 0x600
    f32 mSwimJumpPow;  // 0x604
    s32 mSwimSquatInhibitFrame;  // 0x608
    f32 mPropellerRisePow;  // 0x60c
    s32 mPropellerPowSustain;  // 0x610
    s32 mPropellerPowSustainMin;  // 0x614
    s32 mPropellerPowRelease;  // 0x618
    f32 mPropellerBeforeDropGravity;  // 0x61c
    f32 mPropellerRiseGravity;  // 0x620
    f32 mPropellerAfterDropGravity;  // 0x624
    f32 mPropellerFallSpeedMax;  // 0x628
    f32 mPropellerButtonOffFallSpeedMax;  // 0x62c
    f32 mPropellerEngineBrakeVel;  // 0x630
    f32 mPropellerEngineBrakeRate;  // 0x634
    f32 mPropellerEngineBrakeEndVel;  // 0x638
    f32 mPropellerRotBlendRate;  // 0x63c
    f32 mPropellerSideDamper;  // 0x640
    f32 mPropellerStickOffBrakeRate;  // 0x644
    f32 mLongFallDistance;  // 0x648
    s32 mStatueFallStartFrame;  // 0x64c
    s32 mStatueLandFrame;  // 0x650
    s32 mStatueEndFrame;  // 0x654
    s32 mStatueEndAnimStep;  // 0x658
    f32 mStatueFallSpeedInWater;  // 0x65c
    f32 mSlideSlopeAngle;  // 0x660
    f32 mSlideSlopeEndAngle;  // 0x664
    f32 mSlideEndSpeed;  // 0x668
    f32 mSlideAccel;  // 0x66c
    f32 mSlideMaxSpeed;  // 0x670
    f32 mSlideSideBrake;  // 0x674
    f32 mSlideSideAccel;  // 0x678
    f32 mSlideSideMaxSpeed;  // 0x67c
    f32 mSlideSideAccelOnLevelLand;  // 0x680
    f32 mSlideSideMaxSpeedOnLevelLand;  // 0x684
    f32 mSlideBrake;  // 0x688
    f32 mForceSlideBrake;  // 0x68c
    f32 mSlidePostureBlendRate;  // 0x690
    f32 mForceSlideSpeed;  // 0x694
    f32 mForceSlideSpeedUpRate;  // 0x698
    f32 mSlideTiltBlendRate;  // 0x69c
    f32 mSlideTiltMaxDegree;  // 0x6a0
    s32 mSlideInvalidFrame;  // 0x6a4
    f32 mForceSlideMaxSpeed;  // 0x6a8
    s32 mSlideFallCancelFrame;  // 0x6ac
    f32 mSlideJumpHVelScale;  // 0x6b0
    s32 mHoldShakeInterval;  // 0x6b4
    s32 mHoldThrowFrontTiming;  // 0x6b8
    s32 mHoldThrowUpTiming;  // 0x6bc
    f32 mHoldJumpFrontVel;  // 0x6c0
    f32 mHoldJumpUpVel;  // 0x6c4
    s32 mClimbAttackInterval;  // 0x6c8
    s32 mClimbAttackWaitInterval;  // 0x6cc
    s32 mClimbAttackCancelFrame;  // 0x6d0
    s32 mClimbAttackSensorOnFrame;  // 0x6d4
    f32 mClimbBodyAttackFrontVel;  // 0x6d8
    f32 mClimbBodyAttackDownVel;  // 0x6dc
    s32 mClimbBodyAttackFrame;  // 0x6e0
    f32 mClimbBodyAttackGravity;  // 0x6e4
    f32 mClimbBodyAttackFallSpeedMax;  // 0x6e8
    f32 mClimbBodyAttackHBrakeRate;  // 0x6ec
    f32 mClimbBodyAttackSideAccel;  // 0x6f0
    f32 mClimbBodyAttackSideMoveDist;  // 0x6f4
    f32 mSinkSandMoveMaxSpeed;  // 0x6f8
    f32 mSinkSandMoveMaxDashSpeed;  // 0x6fc
    s32 mSinkSandInvalidFrameInJump;  // 0x700
    f32 mPushedBrakeRate;  // 0x704
    f32 mPushedBrakeMaxRate;  // 0x708
    f32 mPushedJumpCancelSpeed;  // 0x70c
    s32 mGroundSpinFrame;  // 0x710
    f32 mGroundSpinAccel;  // 0x714
    f32 mGroundSpinBrake;  // 0x718
    f32 mGroundSpinVelMax;  // 0x71c
    f32 mSpinJumpGravity;  // 0x720
    f32 mSpinJumpPow;  // 0x724
    s32 mSpinAttackInterval;  // 0x728
    s32 mSpinAttackCancelFrame;  // 0x72c
    s32 mSpinAttackSensorOnFrame;  // 0x730
    f32 mSpinAttackJumpPow;  // 0x734
    f32 mSpinAttackJumpGravity;  // 0x738
    f32 mSpinAttackGroundBrake;  // 0x73c
    f32 mSkateJumpGravity;  // 0x740
    f32 mSkateJumpPowLow;  // 0x744
    f32 mSkateJumpPow;  // 0x748
    s32 mSkateJumpPowCountMax;  // 0x74c
    f32 mSkateJumpThreshold;  // 0x750
    s32 mCoopHipDropFrame;  // 0x754
    f32 mCoopHipDropRadiusMin;  // 0x758
    f32 mCoopHipDropRadius;  // 0x75c
    s32 mGiantHipDropFrame;  // 0x760
    f32 mGiantHipDropRadiusMin;  // 0x764
    f32 mGiantHipDropRadiusMax;  // 0x768
    f32 mKnockDownVelH;  // 0x76c
    f32 mKnockDownVelV;  // 0x770
    s32 mKnockDownCancelFrame;  // 0x774
    f32 mReflectJumpGravity;  // 0x778
    f32 mReflectJump;  // 0x77c
    f32 mRisingReflectJumpHVelBrakeRate;  // 0x780
    s32 mTossCancelFrame;  // 0x784
    s32 mManekinekoFallStartFrame;  // 0x788
    s32 mManekinekoLandFrame;  // 0x78c
    s32 mManekinekoEndNoticeFrame;  // 0x790
    s32 mManekinekoEndFrame;  // 0x794
    s32 mManekinekoCancelFrame;  // 0x798
    f32 mManekinekoFallSpeedInWater;  // 0x79c
    s32 mGroomingMaxInterval;  // 0x7a0
    s32 mGroomingMinInterval;  // 0x7a4
    s32 mSePropellerBeginStep;  // 0x7a8
    f32 mSeFootNoteNormalVolMul;  // 0x7ac
    f32 mSeFootNoteNormalPitDec;  // 0x7b0
    f32 mSeFootNoteDashVolAdd;  // 0x7b4
    f32 mSeFootNoteDashPitAdd;  // 0x7b8
    f32 mGigaCommonAnimRate;  // 0x7bc
    f32 mGigaMiniRunAnimRateMax;  // 0x7c0
    f32 mGigaMiniDashAnimRateMax;  // 0x7c4
    f32 mGigaSuperRunAnimRateMax;  // 0x7c8
    f32 mGigaSuperDashAnimRateMax;  // 0x7cc
    f32 mGigaClimbRunAnimRateMax;  // 0x7d0
    f32 mGigaClimbDashAnimRateMax;  // 0x7d4
    f32 mGigaNormalMaxSpeed;  // 0x7d8
    f32 mGigaDashMaxSpeed;  // 0x7dc
    f32 mGigaSuperDashSpeed;  // 0x7e0
    f32 mGigaInvincibleDashSpeed;  // 0x7e4
    s32 mGigaAccelFrame;  // 0x7e8
    f32 mGigaSquatWalkSpeed;  // 0x7ec
    f32 mGigaGroundSpinAccel;  // 0x7f0
    f32 mGigaGroundSpinBrake;  // 0x7f4
    f32 mGigaKnockDownVelH;  // 0x7f8
    f32 mGigaKnockDownVelV;  // 0x7fc
    f32 mGigaLeftFootHrTime;  // 0x800
    f32 mGigaRightFootHrTime;  // 0x804
    f32 mGigaClimbLeftWalkHrTime;  // 0x808
    f32 mGigaClimbRightWalkHrTime;  // 0x80c
    f32 mGigaClimbLeftRunHrTime;  // 0x810
    f32 mGigaClimbRightRunHrTime;  // 0x814
    f32 mGigaRoundLimitDegreeMax;  // 0x818
    f32 mGigaRoundLimitDegreeMin;  // 0x81c
    f32 mGigaNormalRollingMinSpeed;  // 0x820
    f32 mGigaNormalGravityAddition;  // 0x824
    f32 mGigaSquatBrakeRate;  // 0x828
    f32 mGigaGravity;  // 0x82c
    f32 mGigaFallSpeedMax;  // 0x830
    f32 mGigaFloatFallSpeedMax;  // 0x834
    f32 mGigaJumpPow;  // 0x838
    f32 mGigaJumpPowLow;  // 0x83c
    s32 mGigaJumpPowCountMax;  // 0x840
    f32 mGigaJumpCancelBrakeRate;  // 0x844
    f32 mGigaJumpCancelMinSpeed;  // 0x848
    f32 mGigaHipDropSpeed;  // 0x84c
    f32 mGigaHipDropAnimRate;  // 0x850
    f32 mGigaHipDropJumpPow;  // 0x854
    f32 mGigaTrampleJump;  // 0x858
    f32 mGigaLongJumpSlowSpeed;  // 0x85c
    f32 mGigaLongJumpFastSpeed;  // 0x860
    f32 mGigaLongJumpSlowJumpPow;  // 0x864
    f32 mGigaLongJumpFastJumpPow;  // 0x868
    f32 mGigaLongJumpSlowGravity;  // 0x86c
    f32 mGigaLongJumpFastGravity;  // 0x870
    f32 mGigaSquatJumpGravity;  // 0x874
    f32 mGigaSquatJumpPow;  // 0x878
    f32 mGigaSquatHighJumpPow;  // 0x87c
    f32 mGigaSquatJumpBackPow;  // 0x880
    f32 mGigaSpinJumpGravity;  // 0x884
    f32 mGigaSpinJumpPow;  // 0x888
    f32 mGigaNormalRollingAttackJumpGravity;  // 0x88c
    f32 mGigaNormalRollingAttackJumpPow;  // 0x890
    f32 mGigaNormalRollingAttackVelH;  // 0x894
    f32 mGigaWallJumpHSpeed;  // 0x898
    f32 mGigaLongJumpBrake;  // 0x89c
    f32 mGigaLongJumpSpeedMin;  // 0x8a0
    f32 mGigaLongJumpSideAccel;  // 0x8a4
    s32 mGigaLandFrame;  // 0x8a8
    f32 mGigaWallClimbMaxSpeed;  // 0x8ac
    f32 mGigaWallClimbDashMaxSpeed;  // 0x8b0
    f32 mGigaWallClimbAccel;  // 0x8b4
    f32 mGigaWallClimbMaxSideSpeed;  // 0x8b8
    f32 mGigaWallClimbDashMaxSideSpeed;  // 0x8bc
    f32 mGigaWallClimbSideAccel;  // 0x8c0
    f32 mGigaWallSnapDistance;  // 0x8c4
    f32 mGigaWallClimbJumpPowLow;  // 0x8c8
    f32 mGigaWallClimbJumpPow;  // 0x8cc
    f32 mGigaWallClimbSlideGravity;  // 0x8d0
    f32 mGigaWallClimbSlideMaxSpeed;  // 0x8d4
    f32 mGigaWallClimbSlideSideAccel;  // 0x8d8
    f32 mGigaWallClimbSlideSideMaxSpeed;  // 0x8dc
    f32 mFlashRangeAttackLengthOffset;  // 0x8e0
    f32 mFlashRangeAttackDegreeScale;  // 0x8e4
    f32 mFlashRangeAttackHeightMax;  // 0x8e8
    f32 mFlashRangeAttackHeightMin;  // 0x8ec
    s32 mIsEnableHeadLightOfx;  // 0x8f0
    f32 mHeadLightOfxOffsetY;  // 0x8f4
    f32 mHeadLightOfxOffsetZ;  // 0x8f8
    f32 mHeadLightPrePassPointLightRadiusScale;  // 0x8fc
    f32 mHeadLightPrePassPointLightOffsetY;  // 0x900
    f32 mHeadLightPrePassPointLightOffsetZ;  // 0x904
    const PlayerConstParam* mOverrideParam;  // 0x908
    bool mIsOverride;                        // 0x910
};

#undef PLAYER_CONST_PARAM
