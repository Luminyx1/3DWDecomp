#pragma once
namespace al {
class HitSensor;
class LiveActor;
} // namespace al
class NpcTargetFinder;
class NpcTargetFinderParam;
namespace npc {
enum class NpcFindTargetType : unsigned int {
    None = 0,
    PlayerRegular = 1,
    PlayerClimb = 2,
    KoopaJr = 4,
    Cursor = 8,
    Ball = 16,
    Koura = 32,
    Bird = 64,
    Neko = 128,
    Player = PlayerRegular | PlayerClimb,
    ChaseObj = Ball | Koura | Bird,
};
bool isSensorBall(const al::HitSensor* pSensor);
bool isSensorBird(const al::HitSensor* pSensor);
bool isSensorNeko(const al::HitSensor* pSensor);
bool isSensorNekoDisaster(const al::HitSensor* pSensor);
const char* NpcFindTargetTypeToString(const NpcFindTargetType& rType);
bool calcIsTargetInSight(const NpcTargetFinder* pFinder, const al::LiveActor* pTarget,
                         const NpcTargetFinderParam* pParam);
bool calcIsTargetInChaseRange(const NpcTargetFinder* pFinder, const al::LiveActor* pTarget,
                              const NpcTargetFinderParam* pParam);
} // namespace npc
