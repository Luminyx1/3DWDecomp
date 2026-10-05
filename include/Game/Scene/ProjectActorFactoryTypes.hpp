#include "MapObj/GraphicsAreaController.hpp"
#include "MapObj/Fury/FlowerCat.hpp"
#include "MapObj/GrassHigh.hpp"
#include "MapObj/CoinCirclePlacement.hpp"
#include "MapObj/Coin.hpp"
#include "MapObj/DashPanel.hpp"
#include "MapObj/BlockTransparent.hpp"
#include "MapObj/BlockBrick.hpp"
#include "MapObj/SuperSkateShoes.hpp"
#include "MapObj/SuperSkateRail.hpp"
#pragma once
#include "MapObj/BoxKiller.hpp"
#include "MapObj/BoxPropeller.hpp"
#include "MapObj/BoxCoin.hpp"
#include "MapObj/BlockRoulette.hpp"
#include "MapObj/CheckpointFlag.hpp"
#include "MapObj/FairyHouseIllustItemWatcher.hpp"
#include "MapObj/Fury/GigaBellPedestal.hpp"
#include "MapObj/Fury/GigaBellManager.hpp"
#include "MapObj/Fury/GigaBell.hpp"
#include "MapObj/NeedleRollerGenerator.hpp"
#include "MapObj/Seaweed.hpp"
#include "MapObj/KinopioBrigadeNpc.hpp"
#include "MapObj/KinopioBrigadeWatcher.hpp"
#include "MapObj/OneSideStep.hpp"
#include "MapObj/OneSideStepGenerator.hpp"
#include "MapObj/KeyMoveLoopLiftGenerator.hpp"
#include "MapObj/KoopaFireBallGenerator.hpp"
#include "MapObj/GoalItem.hpp"
#include "MapObj/CandlestandWatcher.hpp"
#include "MapObj/Candlestand.hpp"
#include "MapObj/ItemBubbleSingleMode.hpp"
#include "MapObj/GreenStar.hpp"
#include "MapObj/Fury/CoinStackMoving.hpp"
#include "MapObj/CoinStack.hpp"

#include "MapObj/AssistSlideMapParts.hpp"
#include "MapObj/AssistSlideMapPartsGroup.hpp"

#include "MapObj/ChikaChikaBlockWatcher.hpp"

#include "Enemy/AllDeadWatcher.hpp"
#include "MapObj/Fury/DisasterBlockDeadWatcher.hpp"

#include "MapObj/WoodBox.hpp"

#include "Enemy/BossBunretsu.hpp"

#include "Enemy/MarchGenerator.hpp"

#include "Enemy/KuriboTower.hpp"

#include "MapObj/DoubleMario.hpp"

#include "Library/LiveActor/LiveActor.hpp"
#include "Library/Scene/ISceneObj.hpp"
#include "Library/Shadow/ShadowMaskBase.hpp"

// Partial actor declarations used by the project factory. Constructor signatures and
// allocation sizes are recovered from the original creation functions; the remaining
// members and virtual interfaces are not reconstructed yet. Replace these declarations
// with the actor's own header as its implementation is recovered.
class ItemBubble;
class CandlestandWatcher;
class CourseSelectMiniature;
namespace alSeFunction { enum DemoType : s32; }


class BalanceTruck : public al::LiveActor {
public:
    explicit BalanceTruck(const char* pName);

private:
    u8 mUnreconstructed[0x84];
};
static_assert(sizeof(BalanceTruck) == 0x1c8);

class Ball : public al::LiveActor {
public:
    explicit Ball(const char* pName);

private:
    u8 mUnreconstructed[0xa4];
};
static_assert(sizeof(Ball) == 0x1e8);

class BallGimmick : public al::LiveActor {
public:
    explicit BallGimmick(const char* pName);

private:
    u8 mUnreconstructed[0x6c];
};
static_assert(sizeof(BallGimmick) == 0x1b0);

class BallSnow : public al::LiveActor {
public:
    explicit BallSnow(const char* pName);

private:
    u8 mUnreconstructed[0x9c];
};
static_assert(sizeof(BallSnow) == 0x1e0);

class BallYarn : public al::LiveActor {
public:
    explicit BallYarn(const char* pName);

private:
    u8 mUnreconstructed[0xac];
};
static_assert(sizeof(BallYarn) == 0x1f0);

class BlockAssistLeaf : public al::LiveActor {
public:
    explicit BlockAssistLeaf(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(BlockAssistLeaf) == 0x158);



class BlockBrickBig : public al::LiveActor {
public:
    explicit BlockBrickBig(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(BlockBrickBig) == 0x170);

class BlockBrickBreakableCourseSelect : public al::LiveActor {
public:
    explicit BlockBrickBreakableCourseSelect(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(BlockBrickBreakableCourseSelect) == 0x158);

class BlockChoiceWatcher : public al::LiveActor {
public:
    explicit BlockChoiceWatcher(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(BlockChoiceWatcher) == 0x158);

class BlockHard : public al::LiveActor {
public:
    explicit BlockHard(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(BlockHard) == 0x168);

class BlockHardLaserOnly : public al::LiveActor {
public:
    explicit BlockHardLaserOnly(const char* pName);

private:
    u8 mUnreconstructed[0x64];
};
static_assert(sizeof(BlockHardLaserOnly) == 0x1a8);

class BlockQuestionCourseSelect : public al::LiveActor {
public:
    explicit BlockQuestionCourseSelect(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(BlockQuestionCourseSelect) == 0x158);


class BlockRailMover : public al::LiveActor {
public:
    explicit BlockRailMover(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(BlockRailMover) == 0x178);

class BlockSlot : public al::LiveActor {
public:
    explicit BlockSlot(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(BlockSlot) == 0x170);

class BlockSwitch : public al::LiveActor {
public:
    explicit BlockSwitch(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(BlockSwitch) == 0x150);



class BobsledDashPanel : public al::LiveActor {
public:
    explicit BobsledDashPanel(const char* pName);

private:
    u8 mUnreconstructed[0x4c];
};
static_assert(sizeof(BobsledDashPanel) == 0x190);

class Bomb : public al::LiveActor {
public:
    explicit Bomb(const char* pName, bool = false);

private:
    u8 mUnreconstructed[0x5c];
};
static_assert(sizeof(Bomb) == 0x1a0);

class BombBound : public al::LiveActor {
public:
    explicit BombBound(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(BombBound) == 0x188);

class BombBoundGenerator : public al::LiveActor {
public:
    explicit BombBoundGenerator(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(BombBoundGenerator) == 0x178);

class BombHei : public al::LiveActor {
public:
    explicit BombHei(const char* pName);

private:
    u8 mUnreconstructed[0x6c];
};
static_assert(sizeof(BombHei) == 0x1b0);

class BombHeiLauncher : public al::LiveActor {
public:
    explicit BombHeiLauncher(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(BombHeiLauncher) == 0x160);

class Bonze : public al::LiveActor {
public:
    explicit Bonze(const char* pName);

private:
    u8 mUnreconstructed[0x74];
};
static_assert(sizeof(Bonze) == 0x1b8);

class BoomerangBros : public al::LiveActor {
public:
    explicit BoomerangBros(const char* pName);

private:
    u8 mUnreconstructed[0x8c];
};
static_assert(sizeof(BoomerangBros) == 0x1d0);

class BoomerangFlower : public al::LiveActor {
public:
    explicit BoomerangFlower(const char* pName, ItemBubble* = nullptr, bool = false);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(BoomerangFlower) == 0x180);



class BossGorobon : public al::LiveActor {
public:
    explicit BossGorobon(const char* pName);

private:
    u8 mUnreconstructed[0xb4];
};
static_assert(sizeof(BossGorobon) == 0x1f8);

class BossWackun : public al::LiveActor {
public:
    explicit BossWackun(const char* pName);

private:
    u8 mUnreconstructed[0x114];
};
static_assert(sizeof(BossWackun) == 0x258);



class BoxKillerLauncher : public al::LiveActor {
public:
    explicit BoxKillerLauncher(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(BoxKillerLauncher) == 0x168);

class BoxKuribo : public al::LiveActor {
public:
    explicit BoxKuribo(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(BoxKuribo) == 0x170);

class BoxLight : public al::LiveActor {
public:
    explicit BoxLight(const char* pName);

private:
    u8 mUnreconstructed[0x7c];
};
static_assert(sizeof(BoxLight) == 0x1c0);

class BreakMapParts : public al::LiveActor {
public:
    explicit BreakMapParts(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(BreakMapParts) == 0x160);

class Bubble : public al::LiveActor {
public:
    explicit Bubble(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(Bubble) == 0x188);

class Bull : public al::LiveActor {
public:
    explicit Bull(const char* pName);

private:
    u8 mUnreconstructed[0x54];
};
static_assert(sizeof(Bull) == 0x198);

class Bunbun : public al::LiveActor {
public:
    explicit Bunbun(const char* pName);

private:
    u8 mUnreconstructed[0x7c];
};
static_assert(sizeof(Bunbun) == 0x1c0);

class Bush : public al::LiveActor {
public:
    explicit Bush(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(Bush) == 0x170);

class Byugo : public al::LiveActor {
public:
    explicit Byugo(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(Byugo) == 0x178);

class CameraInSwitchOnAreaWatcher : public al::LiveActor {
public:
    explicit CameraInSwitchOnAreaWatcher(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(CameraInSwitchOnAreaWatcher) == 0x150);

class CameraLookAtPoint : public al::LiveActor {
public:
    explicit CameraLookAtPoint(const char* pName);

private:
    u8 mUnreconstructed[0x5c];
};
static_assert(sizeof(CameraLookAtPoint) == 0x1a0);

class CameraRailObserver : public al::LiveActor {
public:
    explicit CameraRailObserver(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(CameraRailObserver) == 0x160);

class CameraWall : public al::LiveActor {
public:
    explicit CameraWall(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(CameraWall) == 0x168);


static_assert(sizeof(Candlestand) == 0x180);


static_assert(sizeof(CandlestandWatcher) == 0x150);

class CatGull : public al::LiveActor {
public:
    explicit CatGull(const char* pName);

private:
    u8 mUnreconstructed[0x4c];
};
static_assert(sizeof(CatGull) == 0x190);

class CheckPoint : public al::LiveActor {
public:
    explicit CheckPoint(const char* pName);

private:
    u8 mUnreconstructed[0xb4];
};
static_assert(sizeof(CheckPoint) == 0x1f8);


class ChikuwaBlock : public al::LiveActor {
public:
    explicit ChikuwaBlock(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(ChikuwaBlock) == 0x158);

class ChorobonColony : public al::LiveActor {
public:
    explicit ChorobonColony(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(ChorobonColony) == 0x160);

class ChorobonCubeHolder : public al::LiveActor {
public:
    explicit ChorobonCubeHolder(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(ChorobonCubeHolder) == 0x188);

class ChorobonHolder : public al::LiveActor {
public:
    explicit ChorobonHolder(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(ChorobonHolder) == 0x160);

class ClimbHandle : public al::LiveActor {
public:
    explicit ClimbHandle(const char* pName);

private:
    u8 mUnreconstructed[0x6c];
};
static_assert(sizeof(ClimbHandle) == 0x1b0);

class ClimbHandleMapParts : public al::LiveActor {
public:
    explicit ClimbHandleMapParts(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(ClimbHandleMapParts) == 0x178);

class CloudBonusLauncher : public al::LiveActor {
public:
    explicit CloudBonusLauncher(const char* pName);

private:
    u8 mUnreconstructed[0x154];
};
static_assert(sizeof(CloudBonusLauncher) == 0x298);



class CoinBlowConcentric : public al::LiveActor {
public:
    explicit CoinBlowConcentric(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(CoinBlowConcentric) == 0x180);

class CoinBlowGenerator : public al::LiveActor {
public:
    explicit CoinBlowGenerator(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(CoinBlowGenerator) == 0x168);

class CoinChameleon : public al::LiveActor {
public:
    explicit CoinChameleon(const char* pName);

private:
    u8 mUnreconstructed[0x74];
};
static_assert(sizeof(CoinChameleon) == 0x1b8);

class CoinChameleonCourseSelect : public al::LiveActor {
public:
    explicit CoinChameleonCourseSelect(const char* pName);

private:
    u8 mUnreconstructed[0x4c];
};
static_assert(sizeof(CoinChameleonCourseSelect) == 0x190);



class CoinCollectWatcher : public al::LiveActor {
public:
    explicit CoinCollectWatcher(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(CoinCollectWatcher) == 0x170);

class CoinConcentricCircle : public al::LiveActor {
public:
    explicit CoinConcentricCircle(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(CoinConcentricCircle) == 0x180);

class CoinCourseSelect : public al::LiveActor {
public:
    explicit CoinCourseSelect(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(CoinCourseSelect) == 0x158);

class CoinFallGenerator : public al::LiveActor {
public:
    explicit CoinFallGenerator(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(CoinFallGenerator) == 0x170);

class CoinFalls : public al::LiveActor {
public:
    explicit CoinFalls(const char* pName);

private:
    u8 mUnreconstructed[0x84];
};
static_assert(sizeof(CoinFalls) == 0x1c8);

class CoinLine : public al::LiveActor {
public:
    explicit CoinLine(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(CoinLine) == 0x168);

class CoinRail : public al::LiveActor {
public:
    explicit CoinRail(const char* pName);

private:
    u8 mUnreconstructed[0x54];
};
static_assert(sizeof(CoinRail) == 0x198);

class CoinRedRing : public al::LiveActor {
public:
    explicit CoinRedRing(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(CoinRedRing) == 0x188);

class CoinRing : public al::LiveActor {
public:
    explicit CoinRing(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(CoinRing) == 0x178);



class CollectRingHolder : public al::LiveActor {
public:
    explicit CollectRingHolder(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(CollectRingHolder) == 0x158);

class CollisionSearchObj : public al::LiveActor {
public:
    explicit CollisionSearchObj(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(CollisionSearchObj) == 0x160);

class CourseSelectBgmEventController : public al::LiveActor {
public:
    explicit CourseSelectBgmEventController(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(CourseSelectBgmEventController) == 0x150);

class CourseSelectCloud : public al::LiveActor {
public:
    explicit CourseSelectCloud(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(CourseSelectCloud) == 0x150);

class CourseSelectDokan : public al::LiveActor {
public:
    explicit CourseSelectDokan(const char* pName);

private:
    u8 mUnreconstructed[0x64];
};
static_assert(sizeof(CourseSelectDokan) == 0x1a8);

class CourseSelectMiniature : public al::LiveActor {
public:
    explicit CourseSelectMiniature(const char* pName);

private:
    u8 mUnreconstructed[0xd4];
};
static_assert(sizeof(CourseSelectMiniature) == 0x218);

class CourseSelectNode : public al::LiveActor {
public:
    explicit CourseSelectNode(const char* pName, const CourseSelectMiniature* = nullptr);

private:
    u8 mUnreconstructed[0xac];
};
static_assert(sizeof(CourseSelectNode) == 0x1f0);

class CourseSelectRhythmObject : public al::LiveActor {
public:
    explicit CourseSelectRhythmObject(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(CourseSelectRhythmObject) == 0x158);

class CourseSelectRocket : public al::LiveActor {
public:
    explicit CourseSelectRocket(const char* pName);

private:
    u8 mUnreconstructed[0xcc];
};
static_assert(sizeof(CourseSelectRocket) == 0x210);

class CourseSelectRouteDokan : public al::LiveActor {
public:
    explicit CourseSelectRouteDokan(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(CourseSelectRouteDokan) == 0x160);

class CourseSelectWall : public al::LiveActor {
public:
    explicit CourseSelectWall(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(CourseSelectWall) == 0x170);

class CourseSelectWorldWarpDokan : public al::LiveActor {
public:
    explicit CourseSelectWorldWarpDokan(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(CourseSelectWorldWarpDokan) == 0x158);

class Crab : public al::LiveActor {
public:
    explicit Crab(const char* pName);

private:
    u8 mUnreconstructed[0x4c];
};
static_assert(sizeof(Crab) == 0x190);

class Crawler : public al::LiveActor {
public:
    explicit Crawler(const char* pName);

private:
    u8 mUnreconstructed[0x54];
};
static_assert(sizeof(Crawler) == 0x198);

class CrawlerGenerator : public al::LiveActor {
public:
    explicit CrawlerGenerator(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(CrawlerGenerator) == 0x168);

class CubeMapController : public al::LiveActor {
public:
    explicit CubeMapController(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(CubeMapController) == 0x160);

class DarkBowser : public al::LiveActor {
public:
    explicit DarkBowser(const char* pName);

private:
    u8 mUnreconstructed[0x464];
};
static_assert(sizeof(DarkBowser) == 0x5a8);

class DarkBowserLaserIndicator : public al::LiveActor {
public:
    explicit DarkBowserLaserIndicator(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(DarkBowserLaserIndicator) == 0x168);



class DestructableMapParts : public al::LiveActor {
public:
    explicit DestructableMapParts(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(DestructableMapParts) == 0x160);

class DisasterFixMapParts : public al::LiveActor {
public:
    explicit DisasterFixMapParts(const char* pName);

private:
    u8 mUnreconstructed[0x15c];
};
static_assert(sizeof(DisasterFixMapParts) == 0x2a0);

class DemoEventGateKeeperChecker : public al::LiveActor {
public:
    explicit DemoEventGateKeeperChecker(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(DemoEventGateKeeperChecker) == 0x180);

class DemoAnimatic : public al::LiveActor {
public:
    explicit DemoAnimatic(const char* pName, alSeFunction::DemoType = static_cast<alSeFunction::DemoType>(5));

private:
    u8 mUnreconstructed[0x224];
};
static_assert(sizeof(DemoAnimatic) == 0x368);

class DemoCutscene : public al::LiveActor {
public:
    explicit DemoCutscene(const char* pName, alSeFunction::DemoType = static_cast<alSeFunction::DemoType>(5));

private:
    u8 mUnreconstructed[0x224];
};
static_assert(sizeof(DemoCutscene) == 0x368);

class DemoKoopaW7 : public al::LiveActor {
public:
    explicit DemoKoopaW7(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(DemoKoopaW7) == 0x168);

class DemoObjBase : public al::LiveActor {
public:
    explicit DemoObjBase(const char* pName);

private:
    u8 mUnreconstructed[0x1bc];
};
static_assert(sizeof(DemoObjBase) == 0x300);

class DemoOpeningSwitch : public al::LiveActor {
public:
    explicit DemoOpeningSwitch(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(DemoOpeningSwitch) == 0x148);

class DemoTimerStageSwitchController : public al::LiveActor {
public:
    explicit DemoTimerStageSwitchController(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(DemoTimerStageSwitchController) == 0x158);

class DisasterSpike : public al::LiveActor {
public:
    explicit DisasterSpike(const char* pName);

private:
    u8 mUnreconstructed[0x1d4];
};
static_assert(sizeof(DisasterSpike) == 0x318);

class DisasterSpikeBouncy : public al::LiveActor {
public:
    explicit DisasterSpikeBouncy(const char* pName);

private:
    u8 mUnreconstructed[0x1fc];
};
static_assert(sizeof(DisasterSpikeBouncy) == 0x340);

class DoorKey : public al::LiveActor {
public:
    explicit DoorKey(const char* pName);

private:
    u8 mUnreconstructed[0xb4];
};
static_assert(sizeof(DoorKey) == 0x1f8);

class DoorLock : public al::LiveActor {
public:
    explicit DoorLock(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(DoorLock) == 0x188);

class DokanWorldWarp : public al::LiveActor {
public:
    explicit DokanWorldWarp(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(DokanWorldWarp) == 0x178);

class Donketsu : public al::LiveActor {
public:
    explicit Donketsu(const char* pName);

private:
    u8 mUnreconstructed[0x5c];
};
static_assert(sizeof(Donketsu) == 0x1a0);

class Dossun : public al::LiveActor {
public:
    explicit Dossun(const char* pName);

private:
    u8 mUnreconstructed[0xa4];
};
static_assert(sizeof(Dossun) == 0x1e8);



class EchoBlockMapParts : public al::LiveActor {
public:
    explicit EchoBlockMapParts(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(EchoBlockMapParts) == 0x148);

namespace rc {
class EffectObjGame : public al::LiveActor {
public:
    explicit EffectObjGame(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(EffectObjGame) == 0x188);
}  // namespace rc

class EffectObjBattleArenaBunBun : public al::LiveActor {
public:
    explicit EffectObjBattleArenaBunBun(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(EffectObjBattleArenaBunBun) == 0x188);

namespace rc {
class EffectObjFollowCameraGame : public al::LiveActor {
public:
    explicit EffectObjFollowCameraGame(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(EffectObjFollowCameraGame) == 0x178);
}  // namespace rc

class EnemyGenerator : public al::LiveActor {
public:
    explicit EnemyGenerator(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(EnemyGenerator) == 0x168);

class EnterCatMarioMiddleViewRocket : public al::LiveActor {
public:
    explicit EnterCatMarioMiddleViewRocket(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(EnterCatMarioMiddleViewRocket) == 0x158);


class FairyMii : public al::LiveActor {
public:
    explicit FairyMii(const char* pName);

private:
    u8 mUnreconstructed[0x54];
};
static_assert(sizeof(FairyMii) == 0x198);

class FairyNpc : public al::LiveActor {
public:
    explicit FairyNpc(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(FairyNpc) == 0x158);

class FairyNpcWithGlasses : public al::LiveActor {
public:
    explicit FairyNpcWithGlasses(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(FairyNpcWithGlasses) == 0x170);

class FairyPrincess : public al::LiveActor {
public:
    explicit FairyPrincess(const char* pName);

private:
    u8 mUnreconstructed[0x5c];
};
static_assert(sizeof(FairyPrincess) == 0x1a0);

class FallingPillar : public al::LiveActor {
public:
    explicit FallingPillar(const char* pName);

private:
    u8 mUnreconstructed[0x5c];
};
static_assert(sizeof(FallingPillar) == 0x1a0);

class FieryRotateParts : public al::LiveActor {
public:
    explicit FieryRotateParts(const char* pName);

private:
    u8 mUnreconstructed[0x4c];
};
static_assert(sizeof(FieryRotateParts) == 0x190);

class FireBarRoot : public al::LiveActor {
public:
    explicit FireBarRoot(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(FireBarRoot) == 0x168);

class FireBros : public al::LiveActor {
public:
    explicit FireBros(const char* pName);

private:
    u8 mUnreconstructed[0x84];
};
static_assert(sizeof(FireBros) == 0x1c8);

class FireFlower : public al::LiveActor {
public:
    explicit FireFlower(const char* pName, ItemBubble* = nullptr);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(FireFlower) == 0x188);

class Fireworks : public al::LiveActor {
public:
    explicit Fireworks(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(Fireworks) == 0x158);

class FireworksController : public al::LiveActor {
public:
    explicit FireworksController(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(FireworksController) == 0x168);

class FireworksEffectObj : public al::LiveActor {
public:
    explicit FireworksEffectObj(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(FireworksEffectObj) == 0x188);

class FishColony : public al::LiveActor {
public:
    explicit FishColony(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(FishColony) == 0x180);

class FlipCircusDoorA : public al::LiveActor {
public:
    explicit FlipCircusDoorA(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(FlipCircusDoorA) == 0x148);

class FlingPole : public al::LiveActor {
public:
    explicit FlingPole(const char* pName, bool = false);

private:
    u8 mUnreconstructed[0x104];
};
static_assert(sizeof(FlingPole) == 0x248);

class FloatingTerrain : public al::LiveActor {
public:
    explicit FloatingTerrain(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(FloatingTerrain) == 0x170);

class FloatingIslandRailPart : public al::LiveActor {
public:
    explicit FloatingIslandRailPart(const char* pName);

private:
    u8 mUnreconstructed[0x1e4];
};
static_assert(sizeof(FloatingIslandRailPart) == 0x328);



class FlowerCactus : public al::LiveActor {
public:
    explicit FlowerCactus(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(FlowerCactus) == 0x160);

class FlowerRhythm : public al::LiveActor {
public:
    explicit FlowerRhythm(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(FlowerRhythm) == 0x178);

class FlyOverCamera : public al::LiveActor {
public:
    explicit FlyOverCamera(const char* pName);

private:
    u8 mUnreconstructed[0x8c];
};
static_assert(sizeof(FlyOverCamera) == 0x1d0);

class FortressGoal : public al::LiveActor {
public:
    explicit FortressGoal(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(FortressGoal) == 0x148);

class FrameOutChecker : public al::LiveActor {
public:
    explicit FrameOutChecker(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(FrameOutChecker) == 0x180);

class Fugumannen : public al::LiveActor {
public:
    explicit Fugumannen(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(Fugumannen) == 0x170);

class Gabon : public al::LiveActor {
public:
    explicit Gabon(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(Gabon) == 0x188);

class Gamane : public al::LiveActor {
public:
    explicit Gamane(const char* pName);

private:
    u8 mUnreconstructed[0x6c];
};
static_assert(sizeof(Gamane) == 0x1b0);

class GateKeeperChecker : public al::LiveActor {
public:
    explicit GateKeeperChecker(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(GateKeeperChecker) == 0x168);

class Gesso : public al::LiveActor {
public:
    explicit Gesso(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(Gesso) == 0x178);

class GeneratorBox : public al::LiveActor {
public:
    explicit GeneratorBox(const char* pName);

private:
    u8 mUnreconstructed[0xfc];
};
static_assert(sizeof(GeneratorBox) == 0x240);

class GhostPlayerPlayer : public al::LiveActor {
public:
    explicit GhostPlayerPlayer(const char* pName);

private:
    u8 mUnreconstructed[0x5c];
};
static_assert(sizeof(GhostPlayerPlayer) == 0x1a0);

class GiantTouchBreakMapParts : public al::LiveActor {
public:
    explicit GiantTouchBreakMapParts(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(GiantTouchBreakMapParts) == 0x148);

class GigaBellItem : public al::LiveActor {
public:
    explicit GigaBellItem(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(GigaBellItem) == 0x148);

class GoalBonusGameBlockSlot : public al::LiveActor {
public:
    explicit GoalBonusGameBlockSlot(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(GoalBonusGameBlockSlot) == 0x180);

class GoalDoor : public al::LiveActor {
public:
    explicit GoalDoor(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(GoalDoor) == 0x160);


static_assert(sizeof(GoalItem) == 0x3d0);

class GoalItemWatcher : public al::LiveActor {
public:
    explicit GoalItemWatcher(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(GoalItemWatcher) == 0x158);

class GoalPedestal : public al::LiveActor {
public:
    explicit GoalPedestal(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(GoalPedestal) == 0x150);

class GoalPole : public al::LiveActor {
public:
    explicit GoalPole(const char* pName);

private:
    u8 mUnreconstructed[0xfc];
};
static_assert(sizeof(GoalPole) == 0x240);

namespace al {
class GodRayRequester : public al::LiveActor {
public:
    explicit GodRayRequester(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(GodRayRequester) == 0x158);
}  // namespace al

class Gondola : public al::LiveActor {
public:
    explicit Gondola(const char* pName);

private:
    u8 mUnreconstructed[0x4c];
};
static_assert(sizeof(Gondola) == 0x190);

class Gong : public al::LiveActor {
public:
    explicit Gong(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(Gong) == 0x148);

class Gorobon : public al::LiveActor {
public:
    explicit Gorobon(const char* pName);

private:
    u8 mUnreconstructed[0xe4];
};
static_assert(sizeof(Gorobon) == 0x228);

class Gotogoton : public al::LiveActor {
public:
    explicit Gotogoton(const char* pName);

private:
    u8 mUnreconstructed[0xbc];
};
static_assert(sizeof(Gotogoton) == 0x200);



namespace al {
class GraphicsObjShadowMaskCube : public ShadowMaskBase, public LiveActor {
public:
    explicit GraphicsObjShadowMaskCube(const char* pName);
    void declare(ShadowMaskDrawCategory category) override;
    void update() override;
    void updateMulti() override;
    void addMulti() override;
    ShadowMaskType getShadowMaskType() const override;

private:
    u8 mUnreconstructed[0x1cc];
};
static_assert(sizeof(GraphicsObjShadowMaskCube) == 0x400);
}  // namespace al

namespace al {
class GraphicsObjShadowMaskSphere : public ShadowMaskBase, public LiveActor {
public:
    explicit GraphicsObjShadowMaskSphere(const char* pName);
    void declare(ShadowMaskDrawCategory category) override;
    void update() override;
    void updateMulti() override;
    void addMulti() override;
    ShadowMaskType getShadowMaskType() const override;

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(GraphicsObjShadowMaskSphere) == 0x240);
}  // namespace al



class GreenCoin : public al::LiveActor {
public:
    explicit GreenCoin(const char* pName);

private:
    u8 mUnreconstructed[0x4c];
};
static_assert(sizeof(GreenCoin) == 0x190);

class GreenRing : public al::LiveActor {
public:
    explicit GreenRing(const char* pName);

private:
    u8 mUnreconstructed[0x4c];
};
static_assert(sizeof(GreenRing) == 0x190);

static_assert(sizeof(GreenStar) == 0x1d8);

class GreenStarStand : public al::LiveActor {
public:
    explicit GreenStarStand(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(GreenStarStand) == 0x148);

class GroupClippingDummyTarget : public al::LiveActor {
public:
    explicit GroupClippingDummyTarget(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(GroupClippingDummyTarget) == 0x148);

class GustWind : public al::LiveActor {
public:
    explicit GustWind(const char* pName);

private:
    u8 mUnreconstructed[0x5c];
};
static_assert(sizeof(GustWind) == 0x1a0);

class GuideObj : public al::LiveActor {
public:
    explicit GuideObj(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(GuideObj) == 0x158);

class GuideMessageAppear : public al::LiveActor {
public:
    explicit GuideMessageAppear(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(GuideMessageAppear) == 0x160);

class Hacchin : public al::LiveActor {
public:
    explicit Hacchin(const char* pName);

private:
    u8 mUnreconstructed[0x54];
};
static_assert(sizeof(Hacchin) == 0x198);

class HammerBros : public al::LiveActor {
public:
    explicit HammerBros(const char* pName);

private:
    u8 mUnreconstructed[0x6c];
};
static_assert(sizeof(HammerBros) == 0x1b0);

class HexScrollAppearParts : public al::LiveActor {
public:
    explicit HexScrollAppearParts(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(HexScrollAppearParts) == 0x148);

class IllustItem : public al::LiveActor {
public:
    explicit IllustItem(const char* pName, bool = false);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(IllustItem) == 0x170);

class Imozo : public al::LiveActor {
public:
    explicit Imozo(const char* pName);

private:
    u8 mUnreconstructed[0x84];
};
static_assert(sizeof(Imozo) == 0x1c8);

class InkBomb : public al::LiveActor {
public:
    explicit InkBomb(const char* pName, const al::LiveActor* = nullptr);

private:
    u8 mUnreconstructed[0x64];
};
static_assert(sizeof(InkBomb) == 0x1a8);

class InkPatch : public al::LiveActor {
public:
    explicit InkPatch(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(InkPatch) == 0x160);

class InkPuddle : public al::LiveActor {
public:
    explicit InkPuddle(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(InkPuddle) == 0x150);

class InteractableCatToy : public al::LiveActor {
public:
    explicit InteractableCatToy(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(InteractableCatToy) == 0x178);

class IntroFlyOverCamera : public al::LiveActor {
public:
    explicit IntroFlyOverCamera(const char* pName);

private:
    u8 mUnreconstructed[0xa4];
};
static_assert(sizeof(IntroFlyOverCamera) == 0x1e8);

class IslandFlag : public al::LiveActor {
public:
    explicit IslandFlag(const char* pName);

private:
    u8 mUnreconstructed[0x194];
};
static_assert(sizeof(IslandFlag) == 0x2d8);

class IslandKeyMoveMapParts : public al::LiveActor {
public:
    explicit IslandKeyMoveMapParts(const char* pName);

private:
    u8 mUnreconstructed[0xdc];
};
static_assert(sizeof(IslandKeyMoveMapParts) == 0x220);


static_assert(sizeof(ItemBubbleSingleMode) == 0x168);

class JumpFlipPanel : public al::LiveActor {
public:
    explicit JumpFlipPanel(const char* pName);

private:
    u8 mUnreconstructed[0x6c];
};
static_assert(sizeof(JumpFlipPanel) == 0x1b0);

class JumpFlipSweetsCream : public al::LiveActor {
public:
    explicit JumpFlipSweetsCream(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(JumpFlipSweetsCream) == 0x150);

class JumpPanel : public al::LiveActor {
public:
    explicit JumpPanel(const char* pName);

private:
    u8 mUnreconstructed[0xc4];
};
static_assert(sizeof(JumpPanel) == 0x208);

class Kameck : public al::LiveActor {
public:
    explicit Kameck(const char* pName);

private:
    u8 mUnreconstructed[0x54];
};
static_assert(sizeof(Kameck) == 0x198);

class KarakuriCastleDoorWatcher : public al::LiveActor {
public:
    explicit KarakuriCastleDoorWatcher(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(KarakuriCastleDoorWatcher) == 0x158);

class KaronWing : public al::LiveActor {
public:
    explicit KaronWing(const char* pName);

private:
    u8 mUnreconstructed[0x74];
};
static_assert(sizeof(KaronWing) == 0x1b8);


static_assert(sizeof(KeyMoveLoopLiftGenerator) == 0x170);

class KillerLauncher : public al::LiveActor {
public:
    explicit KillerLauncher(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(KillerLauncher) == 0x170);

class KillerTankPartsNeedle : public al::LiveActor {
public:
    explicit KillerTankPartsNeedle(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(KillerTankPartsNeedle) == 0x148);

class KinokoGiga : public al::LiveActor {
public:
    explicit KinokoGiga(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(KinokoGiga) == 0x168);

class KinokoOneUp : public al::LiveActor {
public:
    explicit KinokoOneUp(const char* pName, ItemBubble* = nullptr);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(KinokoOneUp) == 0x188);

class KinokoOneUpCourseSelect : public al::LiveActor {
public:
    explicit KinokoOneUpCourseSelect(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(KinokoOneUpCourseSelect) == 0x148);

class KinokoSuper : public al::LiveActor {
public:
    explicit KinokoSuper(const char* pName, ItemBubble* = nullptr);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(KinokoSuper) == 0x188);

class KinokoTreasure : public al::LiveActor {
public:
    explicit KinokoTreasure(const char* pName, ItemBubble* = nullptr);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(KinokoTreasure) == 0x188);

class KinopioBrigadeChecker : public al::LiveActor {
public:
    explicit KinopioBrigadeChecker(const char* pName);

private:
    u8 mUnreconstructed[0x84];
};
static_assert(sizeof(KinopioBrigadeChecker) == 0x1c8);


class KinopioNpc : public al::LiveActor {
public:
    explicit KinopioNpc(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(KinopioNpc) == 0x150);

class Koura : public al::LiveActor {
public:
    explicit Koura(const char* pName);

private:
    u8 mUnreconstructed[0x12c];
};
static_assert(sizeof(Koura) == 0x270);

class KouraGold : public al::LiveActor {
public:
    explicit KouraGold(const char* pName);

private:
    u8 mUnreconstructed[0x14c];
};
static_assert(sizeof(KouraGold) == 0x290);

class KoopaChase : public al::LiveActor {
public:
    explicit KoopaChase(const char* pName);

private:
    u8 mUnreconstructed[0x64];
};
static_assert(sizeof(KoopaChase) == 0x1a8);

class KoopaChaseWarpDummy : public al::LiveActor {
public:
    explicit KoopaChaseWarpDummy(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(KoopaChaseWarpDummy) == 0x158);

class KoopaChaseCar : public al::LiveActor {
public:
    explicit KoopaChaseCar(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(KoopaChaseCar) == 0x160);


static_assert(sizeof(KoopaFireBallGenerator) == 0x160);

class KoopaGraffiti : public al::LiveActor {
public:
    explicit KoopaGraffiti(const char* pName);

private:
    u8 mUnreconstructed[0x54];
};
static_assert(sizeof(KoopaGraffiti) == 0x198);

class KoopaLastAttackFire : public al::LiveActor {
public:
    explicit KoopaLastAttackFire(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(KoopaLastAttackFire) == 0x158);

class KoopaLastBeforeCheckPoint : public al::LiveActor {
public:
    explicit KoopaLastBeforeCheckPoint(const char* pName);

private:
    u8 mUnreconstructed[0x4c];
};
static_assert(sizeof(KoopaLastBeforeCheckPoint) == 0x190);

class KoopaLastBlockPow : public al::LiveActor {
public:
    explicit KoopaLastBlockPow(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(KoopaLastBlockPow) == 0x188);

class KoopaLastBreakWall : public al::LiveActor {
public:
    explicit KoopaLastBreakWall(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(KoopaLastBreakWall) == 0x188);

class KoopaLastDefendeBlockPow : public al::LiveActor {
public:
    explicit KoopaLastDefendeBlockPow(const char* pName);

private:
    u8 mUnreconstructed[0x6c];
};
static_assert(sizeof(KoopaLastDefendeBlockPow) == 0x1b0);

class KoopaLastDemoAppear : public al::LiveActor {
public:
    explicit KoopaLastDemoAppear(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(KoopaLastDemoAppear) == 0x170);

class KoopaLastFloor : public al::LiveActor {
public:
    explicit KoopaLastFloor(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(KoopaLastFloor) == 0x150);

class KoopaLastWall : public al::LiveActor {
public:
    explicit KoopaLastWall(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(KoopaLastWall) == 0x150);

class KoopaLastWallClimb : public al::LiveActor {
public:
    explicit KoopaLastWallClimb(const char* pName);

private:
    u8 mUnreconstructed[0x74];
};
static_assert(sizeof(KoopaLastWallClimb) == 0x1b8);

class KoopaLastWallClimbDown : public al::LiveActor {
public:
    explicit KoopaLastWallClimbDown(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(KoopaLastWallClimbDown) == 0x180);

class KoopaSignBoard : public al::LiveActor {
public:
    explicit KoopaSignBoard(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(KoopaSignBoard) == 0x158);

class Kuribo : public al::LiveActor {
public:
    explicit Kuribo(const char* pName);

private:
    u8 mUnreconstructed[0xc4];
};
static_assert(sizeof(Kuribo) == 0x208);

class KuriboClimbRail : public al::LiveActor {
public:
    explicit KuriboClimbRail(const char* pName);

private:
    u8 mUnreconstructed[0x74];
};
static_assert(sizeof(KuriboClimbRail) == 0x1b8);

class KuriboClimbSyncObj : public al::LiveActor {
public:
    explicit KuriboClimbSyncObj(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(KuriboClimbSyncObj) == 0x158);

class KuriboMini : public al::LiveActor {
public:
    explicit KuriboMini(const char* pName);

private:
    u8 mUnreconstructed[0x6c];
};
static_assert(sizeof(KuriboMini) == 0x1b0);

class KuriboMiniGenerator : public al::LiveActor {
public:
    explicit KuriboMiniGenerator(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(KuriboMiniGenerator) == 0x150);



class KyoroHei : public al::LiveActor {
public:
    explicit KyoroHei(const char* pName);

private:
    u8 mUnreconstructed[0x54];
};
static_assert(sizeof(KyoroHei) == 0x198);

class Kyuppon : public al::LiveActor {
public:
    explicit Kyuppon(const char* pName);

private:
    u8 mUnreconstructed[0x12c];
};
static_assert(sizeof(Kyuppon) == 0x270);

class Lantern : public al::LiveActor {
public:
    explicit Lantern(const char* pName);

private:
    u8 mUnreconstructed[0x64];
};
static_assert(sizeof(Lantern) == 0x1a8);

class LavaGeyser : public al::LiveActor {
public:
    explicit LavaGeyser(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(LavaGeyser) == 0x188);

class LiftMike : public al::LiveActor {
public:
    explicit LiftMike(const char* pName);

private:
    u8 mUnreconstructed[0x4c];
};
static_assert(sizeof(LiftMike) == 0x190);

class LiftMikeBlockRail : public al::LiveActor {
public:
    explicit LiftMikeBlockRail(const char* pName);

private:
    u8 mUnreconstructed[0x6c];
};
static_assert(sizeof(LiftMikeBlockRail) == 0x1b0);

class LiftMikeSlide : public al::LiveActor {
public:
    explicit LiftMikeSlide(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(LiftMikeSlide) == 0x170);

class Lighthouse : public al::LiveActor {
public:
    explicit Lighthouse(const char* pName);

private:
    u8 mUnreconstructed[0x1b4];
};
static_assert(sizeof(Lighthouse) == 0x2f8);

class LightningController : public al::LiveActor {
public:
    explicit LightningController(const char* pName);

private:
    u8 mUnreconstructed[0x54];
};
static_assert(sizeof(LightningController) == 0x198);

class LuckyIsland : public al::LiveActor {
public:
    explicit LuckyIsland(const char* pName);

private:
    u8 mUnreconstructed[0x1ec];
};
static_assert(sizeof(LuckyIsland) == 0x330);

class LuckyIslandController : public al::ISceneObj, public al::LiveActor {
public:
    explicit LuckyIslandController(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(LuckyIslandController) == 0x188);

class MagmaFish : public al::LiveActor {
public:
    explicit MagmaFish(const char* pName);

private:
    u8 mUnreconstructed[0x54];
};
static_assert(sizeof(MagmaFish) == 0x198);



class MeraWanwan : public al::LiveActor {
public:
    explicit MeraWanwan(const char* pName);

private:
    u8 mUnreconstructed[0xac];
};
static_assert(sizeof(MeraWanwan) == 0x1f0);

class Mirror : public al::LiveActor {
public:
    explicit Mirror(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(Mirror) == 0x150);

class MoamoaHolder : public al::LiveActor {
public:
    explicit MoamoaHolder(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(MoamoaHolder) == 0x158);

class MultiLift : public al::LiveActor {
public:
    explicit MultiLift(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(MultiLift) == 0x170);

class MysteryBox : public al::LiveActor {
public:
    explicit MysteryBox(const char* pName);

private:
    u8 mUnreconstructed[0x7c];
};
static_assert(sizeof(MysteryBox) == 0x1c0);

class MysteryHouseChecker : public al::LiveActor {
public:
    explicit MysteryHouseChecker(const char* pName);

private:
    u8 mUnreconstructed[0x504];
};
static_assert(sizeof(MysteryHouseChecker) == 0x648);

class NeedleBarRoot : public al::LiveActor {
public:
    explicit NeedleBarRoot(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(NeedleBarRoot) == 0x158);

class NeedleRoller : public al::LiveActor {
public:
    explicit NeedleRoller(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(NeedleRoller) == 0x188);


class NeedleRollerSwing : public al::LiveActor {
public:
    explicit NeedleRollerSwing(const char* pName);

private:
    u8 mUnreconstructed[0x74];
};
static_assert(sizeof(NeedleRollerSwing) == 0x1b8);

class NeedleSeed : public al::LiveActor {
public:
    explicit NeedleSeed(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(NeedleSeed) == 0x150);

class NeedleTrap : public al::LiveActor {
public:
    explicit NeedleTrap(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(NeedleTrap) == 0x168);

class Neko : public al::LiveActor {
public:
    explicit Neko(const char* pName, Neko* = nullptr);

private:
    u8 mUnreconstructed[0x5c];
};
static_assert(sizeof(Neko) == 0x1a0);

class Nokonoko : public al::LiveActor {
public:
    explicit Nokonoko(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(Nokonoko) == 0x160);

namespace al {
class OccludedEffectRequester : public al::LiveActor {
public:
    explicit OccludedEffectRequester(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(OccludedEffectRequester) == 0x168);
}  // namespace al

class PackunFire : public al::LiveActor {
public:
    explicit PackunFire(const char* pName);

private:
    u8 mUnreconstructed[0x5c];
};
static_assert(sizeof(PackunFire) == 0x1a0);

class PackunFlower : public al::LiveActor {
public:
    explicit PackunFlower(const char* pName);

private:
    u8 mUnreconstructed[0x84];
};
static_assert(sizeof(PackunFlower) == 0x1c8);

class PackunFlowerWithPot : public al::LiveActor {
public:
    explicit PackunFlowerWithPot(const char* pName);

private:
    u8 mUnreconstructed[0xac];
};
static_assert(sizeof(PackunFlowerWithPot) == 0x1f0);

class PanelNotePaint : public al::LiveActor {
public:
    explicit PanelNotePaint(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(PanelNotePaint) == 0x170);

class PanelNotePaintGroup : public al::LiveActor {
public:
    explicit PanelNotePaintGroup(const char* pName);

private:
    u8 mUnreconstructed[0x8c];
};
static_assert(sizeof(PanelNotePaintGroup) == 0x1d0);

class Peto : public al::LiveActor {
public:
    explicit Peto(const char* pName);

private:
    u8 mUnreconstructed[0x84];
};
static_assert(sizeof(Peto) == 0x1c8);

class PipePackun : public al::LiveActor {
public:
    explicit PipePackun(const char* pName);

private:
    u8 mUnreconstructed[0x7c];
};
static_assert(sizeof(PipePackun) == 0x1c0);

class PlayerPointLightingObj : public al::LiveActor {
public:
    explicit PlayerPointLightingObj(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(PlayerPointLightingObj) == 0x170);

class PlayerSpotLightingObj : public al::LiveActor {
public:
    explicit PlayerSpotLightingObj(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(PlayerSpotLightingObj) == 0x180);

class PlessieTerrain : public al::LiveActor {
public:
    explicit PlessieTerrain(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(PlessieTerrain) == 0x178);

class PressureDeathObj : public al::LiveActor {
public:
    explicit PressureDeathObj(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(PressureDeathObj) == 0x178);

class PSwitchTimerCoinWatcher : public al::LiveActor {
public:
    explicit PSwitchTimerCoinWatcher(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(PSwitchTimerCoinWatcher) == 0x168);


class PlayerNpc : public al::LiveActor {
public:
    explicit PlayerNpc(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(PlayerNpc) == 0x168);

class Prominence : public al::LiveActor {
public:
    explicit Prominence(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(Prominence) == 0x178);

class Pukupuku : public al::LiveActor {
public:
    explicit Pukupuku(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(Pukupuku) == 0x180);

class PukupukuFly : public al::LiveActor {
public:
    explicit PukupukuFly(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(PukupukuFly) == 0x168);

class Punpun : public al::LiveActor {
public:
    explicit Punpun(const char* pName);

private:
    u8 mUnreconstructed[0x94];
};
static_assert(sizeof(Punpun) == 0x1d8);

class Rabbit : public al::LiveActor {
public:
    explicit Rabbit(const char* pName);

private:
    u8 mUnreconstructed[0x114];
};
static_assert(sizeof(Rabbit) == 0x258);

class RabbitNpc : public al::LiveActor {
public:
    explicit RabbitNpc(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(RabbitNpc) == 0x150);

class RaftConveyer : public al::LiveActor {
public:
    explicit RaftConveyer(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(RaftConveyer) == 0x180);

class Raidon : public al::LiveActor {
public:
    explicit Raidon(const char* pName);

private:
    u8 mUnreconstructed[0x144];
};
static_assert(sizeof(Raidon) == 0x288);

class RaidonNpc : public al::LiveActor {
public:
    explicit RaidonNpc(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(RaidonNpc) == 0x180);

class Kuribon : public al::LiveActor {
public:
    explicit Kuribon(const char* pName);

private:
    u8 mUnreconstructed[0x6c];
};
static_assert(sizeof(Kuribon) == 0x1b0);

class RingBeamer : public al::LiveActor {
public:
    explicit RingBeamer(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(RingBeamer) == 0x170);

class RouteDokan : public al::LiveActor {
public:
    explicit RouteDokan(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(RouteDokan) == 0x170);

class RouteDokanBazooka : public al::LiveActor {
public:
    explicit RouteDokanBazooka(const char* pName);

private:
    u8 mUnreconstructed[0x12c];
};
static_assert(sizeof(RouteDokanBazooka) == 0x270);

class RouteDokanLauncher : public al::LiveActor {
public:
    explicit RouteDokanLauncher(const char* pName);

private:
    u8 mUnreconstructed[0x5c];
};
static_assert(sizeof(RouteDokanLauncher) == 0x1a0);

class Runner : public al::LiveActor {
public:
    explicit Runner(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(Runner) == 0x188);

class SaveDataChecker : public al::LiveActor {
public:
    explicit SaveDataChecker(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(SaveDataChecker) == 0x148);

class SamboSnowHead : public al::LiveActor {
public:
    explicit SamboSnowHead(const char* pName);

private:
    u8 mUnreconstructed[0xbc];
};
static_assert(sizeof(SamboSnowHead) == 0x200);

class SePlayObj : public al::LiveActor {
public:
    explicit SePlayObj(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(SePlayObj) == 0x168);


class Shards : public al::LiveActor {
public:
    explicit Shards(const char* pName, bool = false);

private:
    u8 mUnreconstructed[0xbc];
};
static_assert(sizeof(Shards) == 0x200);

class ShardsWatcher : public al::LiveActor {
public:
    explicit ShardsWatcher(const char* pName);

private:
    u8 mUnreconstructed[0x6c];
};
static_assert(sizeof(ShardsWatcher) == 0x1b0);

class ShadowMarioPlayer : public al::LiveActor {
public:
    explicit ShadowMarioPlayer(const char* pName);

private:
    u8 mUnreconstructed[0x6c];
};
static_assert(sizeof(ShadowMarioPlayer) == 0x1b0);

class SignBoard : public al::LiveActor {
public:
    explicit SignBoard(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(SignBoard) == 0x178);

class SingleModeCheckpoint : public al::LiveActor {
public:
    explicit SingleModeCheckpoint(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(SingleModeCheckpoint) == 0x160);

class SinkedItem : public al::LiveActor {
public:
    explicit SinkedItem(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(SinkedItem) == 0x180);

class SkateShoes : public al::LiveActor {
public:
    explicit SkateShoes(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(SkateShoes) == 0x170);

class Skatis : public al::LiveActor {
public:
    explicit Skatis(const char* pName);

private:
    u8 mUnreconstructed[0x5c];
};
static_assert(sizeof(Skatis) == 0x1a0);

class Skipper : public al::LiveActor {
public:
    explicit Skipper(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(Skipper) == 0x180);

class SkipperWeak : public al::LiveActor {
public:
    explicit SkipperWeak(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(SkipperWeak) == 0x178);

class GameSkyProjection : public al::LiveActor {
public:
    explicit GameSkyProjection(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(GameSkyProjection) == 0x180);

class SoundKeyMoveParts : public al::LiveActor {
public:
    explicit SoundKeyMoveParts(const char* pName);

private:
    u8 mUnreconstructed[0x2c];
};
static_assert(sizeof(SoundKeyMoveParts) == 0x170);

class Spinner : public al::LiveActor {
public:
    explicit Spinner(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(Spinner) == 0x178);

class SplatterPlotter : public al::LiveActor {
public:
    explicit SplatterPlotter(const char* pName);

private:
    u8 mUnreconstructed[0xe4];
};
static_assert(sizeof(SplatterPlotter) == 0x228);

class SpotLightPatroller : public al::LiveActor {
public:
    explicit SpotLightPatroller(const char* pName);

private:
    u8 mUnreconstructed[0x19c];
};
static_assert(sizeof(SpotLightPatroller) == 0x2e0);

class SpotLightPatrollerObserver : public al::LiveActor {
public:
    explicit SpotLightPatrollerObserver(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(SpotLightPatrollerObserver) == 0x150);

class StageStartBindDemoCasinoRoom : public al::LiveActor {
public:
    explicit StageStartBindDemoCasinoRoom(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(StageStartBindDemoCasinoRoom) == 0x158);

class StageStartBindDemoKinopioBrigade : public al::LiveActor {
public:
    explicit StageStartBindDemoKinopioBrigade(const char* pName);

private:
    u8 mUnreconstructed[0x12c];
};
static_assert(sizeof(StageStartBindDemoKinopioBrigade) == 0x270);

class StageStartBindDemoKinopioHouse : public al::LiveActor {
public:
    explicit StageStartBindDemoKinopioHouse(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(StageStartBindDemoKinopioHouse) == 0x158);

class StageStartBindDemoMysteryHouse : public al::LiveActor {
public:
    explicit StageStartBindDemoMysteryHouse(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(StageStartBindDemoMysteryHouse) == 0x158);

class StageStartEventCamera : public al::LiveActor {
public:
    explicit StageStartEventCamera(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(StageStartEventCamera) == 0x150);

class StageStartEventDemo : public al::LiveActor {
public:
    explicit StageStartEventDemo(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(StageStartEventDemo) == 0x168);

class StageStartEventSound : public al::LiveActor {
public:
    explicit StageStartEventSound(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(StageStartEventSound) == 0x158);

class StepScrollWatcher : public al::LiveActor {
public:
    explicit StepScrollWatcher(const char* pName);

private:
    u8 mUnreconstructed[0x84];
};
static_assert(sizeof(StepScrollWatcher) == 0x1c8);

class StrongHipDropReactPoint : public al::LiveActor {
public:
    explicit StrongHipDropReactPoint(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(StrongHipDropReactPoint) == 0x148);

class SuperBell : public al::LiveActor {
public:
    explicit SuperBell(const char* pName, ItemBubble* = nullptr);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(SuperBell) == 0x180);

class SuperBellSpecial : public al::LiveActor {
public:
    explicit SuperBellSpecial(const char* pName, ItemBubble* = nullptr);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(SuperBellSpecial) == 0x180);


class KuriboGiga : public al::LiveActor {
public:
    explicit KuriboGiga(const char* pName);

private:
    u8 mUnreconstructed[0xd4];
};
static_assert(sizeof(KuriboGiga) == 0x218);



class GigaRock : public al::LiveActor {
public:
    explicit GigaRock(const char* pName);

private:
    u8 mUnreconstructed[0x9c];
};
static_assert(sizeof(GigaRock) == 0x1e0);

class SuperBowserShell : public al::LiveActor {
public:
    explicit SuperBowserShell(const char* pName);

private:
    u8 mUnreconstructed[0xcc];
};
static_assert(sizeof(SuperBowserShell) == 0x210);

class SuperbViewArea : public al::LiveActor {
public:
    explicit SuperbViewArea(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(SuperbViewArea) == 0x180);

class SuperKoura : public al::LiveActor {
public:
    explicit SuperKoura(const char* pName);

private:
    u8 mUnreconstructed[0xf4];
};
static_assert(sizeof(SuperKoura) == 0x238);

class SuperLeaf : public al::LiveActor {
public:
    explicit SuperLeaf(const char* pName, ItemBubble* = nullptr);

private:
    u8 mUnreconstructed[0x4c];
};
static_assert(sizeof(SuperLeaf) == 0x190);





class SuperStar : public al::LiveActor {
public:
    explicit SuperStar(const char* pName, ItemBubble* = nullptr);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(SuperStar) == 0x178);

class Swimmer : public al::LiveActor {
public:
    explicit Swimmer(const char* pName);

private:
    u8 mUnreconstructed[0x84];
};
static_assert(sizeof(Swimmer) == 0x1c8);

class SwitchAnd : public al::LiveActor {
public:
    explicit SwitchAnd(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(SwitchAnd) == 0x150);

class SwitchRotateWatcher : public al::LiveActor {
public:
    explicit SwitchRotateWatcher(const char* pName);

private:
    u8 mUnreconstructed[0x74];
};
static_assert(sizeof(SwitchRotateWatcher) == 0x1b8);

class SwitchBlockWatcher : public al::LiveActor {
public:
    explicit SwitchBlockWatcher(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(SwitchBlockWatcher) == 0x160);

class SyumockConveyerGenerator : public al::LiveActor {
public:
    explicit SyumockConveyerGenerator(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(SyumockConveyerGenerator) == 0x168);

class SyumockRailMove : public al::LiveActor {
public:
    explicit SyumockRailMove(const char* pName);

private:
    u8 mUnreconstructed[0x7c];
};
static_assert(sizeof(SyumockRailMove) == 0x1c0);

class SyumockRotate : public al::LiveActor {
public:
    explicit SyumockRotate(const char* pName);

private:
    u8 mUnreconstructed[0x6c];
};
static_assert(sizeof(SyumockRotate) == 0x1b0);

class Takobo : public al::LiveActor {
public:
    explicit Takobo(const char* pName);

private:
    u8 mUnreconstructed[0x54];
};
static_assert(sizeof(Takobo) == 0x198);

class Tentack : public al::LiveActor {
public:
    explicit Tentack(const char* pName);

private:
    u8 mUnreconstructed[0x7c];
};
static_assert(sizeof(Tentack) == 0x1c0);

class TentackLv3 : public al::LiveActor {
public:
    explicit TentackLv3(const char* pName);

private:
    u8 mUnreconstructed[0xf4];
};
static_assert(sizeof(TentackLv3) == 0x238);

class TentenGenerator : public al::LiveActor {
public:
    explicit TentenGenerator(const char* pName);

private:
    u8 mUnreconstructed[0x8c];
};
static_assert(sizeof(TentenGenerator) == 0x1d0);

class Teren : public al::LiveActor {
public:
    explicit Teren(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(Teren) == 0x168);

class TeresaConveyorBench : public al::LiveActor {
public:
    explicit TeresaConveyorBench(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(TeresaConveyorBench) == 0x168);

class TeresaFakeDokan : public al::LiveActor {
public:
    explicit TeresaFakeDokan(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(TeresaFakeDokan) == 0x148);

class TeresaFakeObject : public al::LiveActor {
public:
    explicit TeresaFakeObject(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(TeresaFakeObject) == 0x148);

class TeresaWall : public al::LiveActor {
public:
    explicit TeresaWall(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(TeresaWall) == 0x160);

class Tico : public al::LiveActor {
public:
    explicit Tico(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(Tico) == 0x160);

class TicoCourseSelect : public al::LiveActor {
public:
    explicit TicoCourseSelect(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(TicoCourseSelect) == 0x148);

class TimerClock : public al::LiveActor {
public:
    explicit TimerClock(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(TimerClock) == 0x158);

class TimerCoinHolder : public al::LiveActor {
public:
    explicit TimerCoinHolder(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(TimerCoinHolder) == 0x160);

class TimerGate : public al::LiveActor {
public:
    explicit TimerGate(const char* pName);

private:
    u8 mUnreconstructed[0x124];
};
static_assert(sizeof(TimerGate) == 0x268);

class TimerStageSwitch : public al::LiveActor {
public:
    explicit TimerStageSwitch(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(TimerStageSwitch) == 0x150);

class Togezo : public al::LiveActor {
public:
    explicit Togezo(const char* pName);

private:
    u8 mUnreconstructed[0x54];
};
static_assert(sizeof(Togezo) == 0x198);

class TouchReactionMapParts : public al::LiveActor {
public:
    explicit TouchReactionMapParts(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(TouchReactionMapParts) == 0x180);

class TrampleSwitchTimer : public al::LiveActor {
public:
    explicit TrampleSwitchTimer(const char* pName);

private:
    u8 mUnreconstructed[0xd4];
};
static_assert(sizeof(TrampleSwitchTimer) == 0x218);

class TrampleSwitch : public al::LiveActor {
public:
    explicit TrampleSwitch(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(TrampleSwitch) == 0x158);

class TrampleSwitchChara : public al::LiveActor {
public:
    explicit TrampleSwitchChara(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(TrampleSwitchChara) == 0x158);

class TrampleSwitchGoalItem : public al::LiveActor {
public:
    explicit TrampleSwitchGoalItem(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(TrampleSwitchGoalItem) == 0x160);

class TrampleSwitchGold : public al::LiveActor {
public:
    explicit TrampleSwitchGold(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(TrampleSwitchGold) == 0x158);

class TrampleSwitchTogetherWatcher : public al::LiveActor {
public:
    explicit TrampleSwitchTogetherWatcher(const char* pName);

private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(TrampleSwitchTogetherWatcher) == 0x150);

class Trampoline : public al::LiveActor {
public:
    explicit Trampoline(const char* pName);

private:
    u8 mUnreconstructed[0xfc];
};
static_assert(sizeof(Trampoline) == 0x240);

class TransparentWall : public al::LiveActor {
public:
    explicit TransparentWall(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(TransparentWall) == 0x148);

class Trapeze : public al::LiveActor {
public:
    explicit Trapeze(const char* pName);

private:
    u8 mUnreconstructed[0x84];
};
static_assert(sizeof(Trapeze) == 0x1c8);

class Tree : public al::LiveActor {
public:
    explicit Tree(const char* pName);

private:
    u8 mUnreconstructed[0x13c];
};
static_assert(sizeof(Tree) == 0x280);

class TreeFarLodWatcher : public al::LiveActor {
public:
    explicit TreeFarLodWatcher(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(TreeFarLodWatcher) == 0x178);

class TreeStump : public al::LiveActor {
public:
    explicit TreeStump(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(TreeStump) == 0x158);

class TreeStumpWatcher : public al::LiveActor {
public:
    explicit TreeStumpWatcher(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(TreeStumpWatcher) == 0x160);

class Tuccondor : public al::LiveActor {
public:
    explicit Tuccondor(const char* pName);

private:
    u8 mUnreconstructed[0x8c];
};
static_assert(sizeof(Tuccondor) == 0x1d0);

class TuccondorAround : public al::LiveActor {
public:
    explicit TuccondorAround(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(TuccondorAround) == 0x188);

class TuccondorTrap : public al::LiveActor {
public:
    explicit TuccondorTrap(const char* pName);

private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(TuccondorTrap) == 0x180);

class Ukibo : public al::LiveActor {
public:
    explicit Ukibo(const char* pName);

private:
    u8 mUnreconstructed[0x8c];
};
static_assert(sizeof(Ukibo) == 0x1d0);

class WarpCube : public al::LiveActor {
public:
    explicit WarpCube(const char* pName);

private:
    u8 mUnreconstructed[0xac];
};
static_assert(sizeof(WarpCube) == 0x1f0);

class WarpDoor : public al::LiveActor {
public:
    explicit WarpDoor(const char* pName);

private:
    u8 mUnreconstructed[0x34];
};
static_assert(sizeof(WarpDoor) == 0x178);

class WaterAreaMoveModel : public al::LiveActor {
public:
    explicit WaterAreaMoveModel(const char* pName);

private:
    u8 mUnreconstructed[0x44];
};
static_assert(sizeof(WaterAreaMoveModel) == 0x188);

class WheelWatcher : public al::LiveActor {
public:
    explicit WheelWatcher(const char* pName);

private:
    u8 mUnreconstructed[0x14];
};
static_assert(sizeof(WheelWatcher) == 0x158);



class WoodLogBridge : public al::LiveActor {
public:
    explicit WoodLogBridge(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(WoodLogBridge) == 0x160);

class ZigzagBuildingCover : public al::LiveActor {
public:
    explicit ZigzagBuildingCover(const char* pName);

private:
    u8 mUnreconstructed[0x4];
};
static_assert(sizeof(ZigzagBuildingCover) == 0x148);

class ThrowMapParts : public al::LiveActor {
public:
    explicit ThrowMapParts(const char* pName);

private:
    u8 mUnreconstructed[0x9c];
};
static_assert(sizeof(ThrowMapParts) == 0x1e0);

class TimeLimitStepSwitch : public al::LiveActor {
public:
    explicit TimeLimitStepSwitch(const char* pName);

private:
    u8 mUnreconstructed[0x24];
};
static_assert(sizeof(TimeLimitStepSwitch) == 0x168);

class WindowMessageAppear : public al::LiveActor {
public:
    explicit WindowMessageAppear(const char* pName);

private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(WindowMessageAppear) == 0x160);

class IslandHolder : public al::LiveActor {
public:
    explicit IslandHolder(const char* pName);

private:
    u8 mUnreconstructed[0x414];
};
static_assert(sizeof(IslandHolder) == 0x558);

class TestAndoGoalPole : public al::LiveActor {
public:
    explicit TestAndoGoalPole(const char* pName);

private:
    u8 mUnreconstructed[0xa4];
};
static_assert(sizeof(TestAndoGoalPole) == 0x1e8);

class BgmPlayObj : public al::LiveActor {
public:
    explicit BgmPlayObj(const char* pName);
private:
    u8 mUnreconstructed[0x1c];
};
static_assert(sizeof(BgmPlayObj) == 0x160);

class BgmRegionChanger : public al::LiveActor {
public:
    explicit BgmRegionChanger(const char* pName);
private:
    u8 mUnreconstructed[0xc];
};
static_assert(sizeof(BgmRegionChanger) == 0x150);

class DemoStartPosition : public al::LiveActor {
public:
    explicit DemoStartPosition(const char* pName);
private:
    u8 mUnreconstructed[0x3c];
};
static_assert(sizeof(DemoStartPosition) == 0x180);
