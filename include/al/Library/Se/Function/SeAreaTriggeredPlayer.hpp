#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

#include "Project/AreaObj/IUseAreaObj.hpp"
#include "Project/Audio/IUseAudioKeeper.hpp"

namespace al {
class AreaObj;
class AudioDirector;
class PlayerHolder;

class SeAreaTriggeredPlayer : public IUseAudioKeeper, public IUseAreaObj {
public:
    SeAreaTriggeredPlayer(const AudioDirector* pDirector, AreaObjDirector* pAreaObjDirector,
                          const PlayerHolder* pPlayerHolder);

    void reset();
    void update();

    AudioKeeper* getAudioKeeper() const override { return mAudioKeeper; }
    AreaObjDirector* getAreaObjDirector() const override { return mAreaObjDirector; }

private:
    AudioKeeper* mAudioKeeper = nullptr;
    AreaObjDirector* mAreaObjDirector;
    const PlayerHolder* mPlayerHolder;
    sead::PtrArray<AreaObj>** mAreaLists = nullptr;
    s32 mCurListIndex = 0;
};
static_assert(sizeof(SeAreaTriggeredPlayer) == 0x38);
}  // namespace al
