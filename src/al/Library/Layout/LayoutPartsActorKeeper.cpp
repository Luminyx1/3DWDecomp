#include "Library/Layout/LayoutPartsActorKeeper.hpp"

#include "Library/Layout/LayoutActor.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Creates a keeper for parts actors.
 * @param maxActors expected number of parts actors
 */
LayoutPartsActorKeeper::LayoutPartsActorKeeper(s32 maxActors) : mMaxActors(maxActors * 2) {
    mPartsActors = new LayoutActor*[maxActors * 4];
}

/**
 * Destroys the keeper.
 */
LayoutPartsActorKeeper::~LayoutPartsActorKeeper() {
    delete[] mPartsActors;
}

/**
 * Registers a parts actor.
 * @param pActor parts actor to add
 */
void LayoutPartsActorKeeper::resisterPartsActor(LayoutActor* pActor) {
    mPartsActors[mNumActors] = pActor;
    mNumActors++;
}

/**
 * Makes every parts actor appear.
 */
void LayoutPartsActorKeeper::appear() {
    for (s32 i = 0; i < mNumActors; i++) {
        mPartsActors[i]->appear();
    }
}

/**
 * Kills every parts actor.
 */
void LayoutPartsActorKeeper::kill() {
    for (s32 i = 0; i < mNumActors; i++) {
        mPartsActors[i]->kill();
    }
}

/**
 * Runs the movement of every parts actor.
 */
void LayoutPartsActorKeeper::update() {
    for (s32 i = 0; i < mNumActors; i++) {
        mPartsActors[i]->movement();
    }
}

/**
 * Calculates the animation of every parts actor.
 * @param isRecursive whether to calculate recursively
 */
void LayoutPartsActorKeeper::calcAnim(bool isRecursive) {
    for (s32 i = 0; i < mNumActors; i++) {
        mPartsActors[i]->calcAnim(isRecursive);
    }
}

/**
 * Finds a parts actor by name.
 * @param pName name of the parts actor
 * @return the parts actor, or nullptr
 */
LayoutActor* LayoutPartsActorKeeper::getPartsActor(const char* pName) const {
    for (s32 i = 0; i < mNumActors; i++) {
        if (isEqualString(pName, mPartsActors[i]->getName())) {
            return mPartsActors[i];
        }
    }
    return nullptr;
}
}  // namespace al
