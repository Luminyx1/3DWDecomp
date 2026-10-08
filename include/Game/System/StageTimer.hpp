#pragma once
#include "Library/Layout/LayoutActor.hpp"
#include "Library/Scene/ISceneObj.hpp"

/** @brief Stage countdown interface used by the stage-entry timer event. */
class StageTimer : public al::LayoutActor, public al::ISceneObj {
public:
    bool tryStartHurryUp();
    bool isCountDown() const;
    bool isStop() const;
    void requestStop(const void*);
    void requestRestart(const void*);
    s32 calcDisplayCount() const;
};
