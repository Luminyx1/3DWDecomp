#pragma once

#include "Library/Scene/ISceneObj.hpp"
#include "Library/Scene/IUseSceneObjHolder.hpp"

/** @brief Tracks whether a stage contains a stamp and whether it has been collected. */
class IllustItemKeeper : public al::ISceneObj, public al::IUseSceneObjHolder {
public:
    IllustItemKeeper();
    const char* getSceneObjName() const override;
    void initAfterPlacementSceneObj(const al::ActorInitInfo& rInfo) override;
    al::SceneObjHolder* getSceneObjHolder() const override;
    void declare();
    void acquire();

    /** @brief Checks whether a stamp has been declared in the stage.
     * @return Whether the stage contains a declared stamp. */
    bool isDeclared() const { return mIsDeclared; }

private:
    al::SceneObjHolder* mSceneObjHolder = nullptr;
    bool mIsDeclared = false;
    bool mIsAcquired = false;
    bool mIsAcquiredThisVisit = false;
};
static_assert(sizeof(IllustItemKeeper) == 0x20);
