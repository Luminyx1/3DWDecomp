#pragma once
#include "Library/Nerve/NerveExecutor.hpp"
namespace al {
class Sequence;
struct GameSystemInfo;
class GamePadSystem;
class AudioSystem;
} // namespace al
class GameSystem : public al::NerveExecutor {
  public:
    GameSystem();
    void init();
    void movement();
    void drawMain();
    void drawSub();
    void exePlay();

  private:
    al::Sequence* mpSequence;
    al::GameSystemInfo* mpInfo;
    al::GamePadSystem* mpGamePad;
    al::AudioSystem* mpAudio;
    void* mpUnknown30;
    void* mpUnknown38;
    void* mpUnknown40;
};
namespace GameSystemFunction {
GameSystem* getGameSystem();
}
static_assert(sizeof(GameSystem) == 0x48);
