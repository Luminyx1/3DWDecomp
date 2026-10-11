#pragma once

#include <basis/seadTypes.h>
#include <container/seadObjList.h>
#include "MapObj/DrcAssistDirector.hpp"

#include "Library/Scene/ISceneObj.hpp"

namespace rc {
class StampDirector;
}  // namespace rc

class DrcAssistDirector;
class PlayerAliveWatcher;

namespace al {
class IUseSceneObjHolder;
class PlayerHolder;
}  // namespace al

/**
 * @brief Scene object holding the touch-screen (DRC) assist director of every pad port.
 */
class DrcAssistDirectorList : public al::ISceneObj {
public:
    explicit DrcAssistDirectorList(s32 maxNum);

    const char* getSceneObjName() const override;
    void invalidate(bool);
    sead::ObjList<DrcAssistDirector>& getDirectors() { return mDirectors; }
    void addTouchAssist(s32 port, bool isEnable);
    void setEnable(bool isEnable);
    DrcAssistDirector* getDrcAssist(s32 port);
    void setStampDirector(rc::StampDirector* pStampDirector);
    void setKinopioBrigadeFlag(bool isKinopioBrigade);
    void setPlayerHolder(al::PlayerHolder* pPlayerHolder);
    void setPlayerAliveWatcher(PlayerAliveWatcher* pPlayerAliveWatcher);
    void disappearTouchPointer();
    void disappearTouchPointerEffect();
    void disappearTouchPointerImmediately();

private:
    sead::ObjList<DrcAssistDirector> mDirectors;
    u8 _38[8];
};

static_assert(sizeof(DrcAssistDirectorList) == 0x40);

namespace rc {
void updateDrcAssistDirector(const al::IUseSceneObjHolder* pHolder);
}  // namespace rc
