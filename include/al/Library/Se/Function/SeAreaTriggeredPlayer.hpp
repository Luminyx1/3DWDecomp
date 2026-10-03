#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

#include "Project/AreaObj/IUseAreaObj.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"

namespace al {
class AreaObj;
class AudioDirector;
class PlayerHolder;

using AreaObjArray = sead::PtrArray<AreaObj>;

class SeAreaTriggeredPlayer : public IUseAudioKeeper, public IUseAreaObj {
  public:
    SeAreaTriggeredPlayer(const AudioDirector* pDirector, AreaObjDirector* pAreaObjDirector,
                          const PlayerHolder* pPlayerHolder);

    void reset();
    void update();

    /**
     * @brief Gets the audio keeper used for area sounds.
     * @return Audio keeper created for this player.
     */
    AudioKeeper* getAudioKeeper() const override { return mAudioKeeper; }
    /**
     * @brief Gets the director used to find sound-triggering areas.
     * @return Area director supplied at construction; may be nullptr.
     */
    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

  private:
    AudioKeeper* mAudioKeeper = nullptr;
    AreaObjDirector* mAreaObjDirector;
    const PlayerHolder* mPlayerHolder;
    AreaObjArray** mAreaLists = nullptr;
    s32 mCurListIndex = 0;
};

static_assert(sizeof(SeAreaTriggeredPlayer) == 0x38);
} // namespace al
