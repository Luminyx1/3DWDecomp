#pragma once

#include <basis/seadTypes.h>
#include "Library/Scene/Scene.hpp"

/**
 * @brief Common base of the in-game scenes.
 * @note Adds no data members (sizeof == sizeof(al::Scene)); only the virtual interface.
 */
class InGameSceneBase : public al::Scene {
public:
    explicit InGameSceneBase(const char* pName);

    virtual void prepareDestroy() = 0;
    virtual bool isRestartCheck() const;
    virtual bool isGoal() const = 0;
    virtual bool isWorldWarp() const;
    virtual bool isTriggerPause(s32* pPort) const;
    virtual void recordClearData();
};
static_assert(sizeof(InGameSceneBase) == 0xe8);
