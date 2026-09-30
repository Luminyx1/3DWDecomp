#include "Library/Layout/LayoutActor.hpp"

#include "Library/Layout/LayoutActorUtil.hpp"
#include "Library/Layout/LayoutKeeper.hpp"
#include "Library/Layout/LayoutPartsActorKeeper.hpp"
#include "Library/Layout/LayoutSceneInfo.hpp"
#include "Library/Layout/LayoutTextPaneAnimator.hpp"
#include "Library/Nerve/NerveKeeper.hpp"
#include "Project/Audio/AudioKeeper.hpp"
#include "Project/Effect/Core/EffectKeeper.hpp"
#include "Project/Layout/LayoutActionKeeper.hpp"

namespace al {
/**
 * Creates a layout actor.
 * @param pName actor name
 */
LayoutActor::LayoutActor(const char* pName)
    : mName(pName), mNerveKeeper(nullptr), mLayoutKeeper(nullptr), mLayoutActionKeeper(nullptr),
      mTextPaneAnimator(nullptr), mEffectKeeper(nullptr), mAudioKeeper(nullptr),
      mHitReactionKeeper(nullptr), mLayoutSceneInfo(nullptr), mLayoutPartsActorKeeper(nullptr),
      mIsAlive(false) {}

/**
 * Makes the actor appear.
 */
void LayoutActor::appear() {
    mIsAlive = true;

    if (mAudioKeeper) {
        mAudioKeeper->appear();
    }

    if (mLayoutPartsActorKeeper) {
        mLayoutPartsActorKeeper->appear();
    }

    updateLayoutPaneRecursive(this);
    calcAnim(false);
}

/**
 * Kills the actor.
 */
void LayoutActor::kill() {
    if (mEffectKeeper) {
        mEffectKeeper->deleteAndClearEffectAll();
    }

    if (mAudioKeeper) {
        mAudioKeeper->kill();
    }

    if (mLayoutPartsActorKeeper) {
        mLayoutPartsActorKeeper->kill();
    }

    mIsAlive = false;
}

/**
 * Updates the nerve, control and keepers of the actor.
 */
void LayoutActor::movement() {
    if (!mIsAlive) {
        return;
    }

    if (mNerveKeeper) {
        mNerveKeeper->update();

        if (!mIsAlive) {
            return;
        }
    }

    control();

    if (mLayoutPartsActorKeeper) {
        mLayoutPartsActorKeeper->update();
    }

    if (mLayoutKeeper->getGroupNum() < 1) {
        return;
    }

    if (mEffectKeeper) {
        mEffectKeeper->update();
    }

    if (mAudioKeeper) {
        mAudioKeeper->update();
    }

    if (mLayoutActionKeeper) {
        mLayoutActionKeeper->update();
    }

    if (mTextPaneAnimator) {
        mTextPaneAnimator->update();
    }
}

/**
 * Updates the action related keepers of the actor.
 */
void LayoutActor::syncAction() {
    if (mEffectKeeper) {
        mEffectKeeper->update();
    }

    if (mAudioKeeper) {
        mAudioKeeper->update();
    }

    if (mLayoutActionKeeper) {
        mLayoutActionKeeper->update();
    }

    if (mTextPaneAnimator) {
        mTextPaneAnimator->update();
    }
}

/**
 * Calculates the layout animation.
 * @param isRecursive whether to calculate recursively
 */
void LayoutActor::calcAnim(bool isRecursive) {
    if (!mIsAlive) {
        return;
    }

    mLayoutKeeper->calcAnim(isRecursive);

    if (mLayoutPartsActorKeeper) {
        mLayoutPartsActorKeeper->calcAnim(isRecursive);
    }
}

/**
 * Sets the layout keeper.
 * @param pLayoutKeeper layout keeper
 */
void LayoutActor::initLayoutKeeper(LayoutKeeper* pLayoutKeeper) {
    mLayoutKeeper = pLayoutKeeper;
}

/**
 * Creates the action keeper.
 */
void LayoutActor::initActionKeeper() {
    mLayoutActionKeeper = new LayoutActionKeeper(mLayoutKeeper, mAudioKeeper ? this : nullptr,
                                                 mEffectKeeper ? this : nullptr);
}

/**
 * Sets the text pane animator.
 * @param pAnimator text pane animator
 */
void LayoutActor::initTextPaneAnimator(LayoutTextPaneAnimator* pAnimator) {
    mTextPaneAnimator = pAnimator;
}

/**
 * Sets the hit reaction keeper and passes it to the action keeper.
 * @param pKeeper hit reaction keeper
 */
void LayoutActor::initHitReactionKeeper(HitReactionKeeper* pKeeper) {
    mHitReactionKeeper = pKeeper;

    if (getLayoutActionKeeper()) {
        getLayoutActionKeeper()->setHitReactionKeeper(mHitReactionKeeper);
    }
}

/**
 * Sets the scene info.
 * @param pSceneInfo scene info
 */
void LayoutActor::initSceneInfo(LayoutSceneInfo* pSceneInfo) {
    mLayoutSceneInfo = pSceneInfo;
}

/**
 * Creates the parts actor keeper.
 * @param capacity number of parts actors
 */
void LayoutActor::initLayoutPartsActorKeeper(s32 capacity) {
    mLayoutPartsActorKeeper = new LayoutPartsActorKeeper(capacity);
}

/**
 * Sets the effect keeper and connects it to the layout.
 * @param pEffectKeeper effect keeper
 */
void LayoutActor::initEffectKeeper(EffectKeeper* pEffectKeeper) {
    mEffectKeeper = pEffectKeeper;

    if (mLayoutKeeper) {
        alEffectKeeperInitFunction::setupLayoutToEffectKeeper(pEffectKeeper, this);
    }
}

/**
 * Sets the audio keeper.
 * @param pAudioKeeper audio keeper
 */
void LayoutActor::initAudioKeeper(AudioKeeper* pAudioKeeper) {
    mAudioKeeper = pAudioKeeper;
}

/**
 * Creates the nerve keeper.
 * @param pNerve initial nerve
 * @param maxStates maximum number of nerve states
 */
void LayoutActor::initNerve(const Nerve* pNerve, s32 maxStates) {
    mNerveKeeper = new NerveKeeper(this, pNerve, maxStates);
}

/**
 * Sets the group whose actions are started by default.
 * @param pGroupName group name
 */
void LayoutActor::setMainGroupName(const char* pGroupName) {
    mLayoutKeeper->getGroup(pGroupName);
    mLayoutActionKeeper->setMainGroupName(pGroupName);
}

/**
 * Returns the scene camera info.
 * @return scene camera info
 */
SceneCameraInfo* LayoutActor::getSceneCameraInfo() const {
    return *reinterpret_cast<SceneCameraInfo* const*>(
        reinterpret_cast<const u8*>(mLayoutSceneInfo->getCameraDirector()) + 0x28);
}

/**
 * Returns the scene object holder.
 * @return scene object holder
 */
SceneObjHolder* LayoutActor::getSceneObjHolder() const {
    return mLayoutSceneInfo->getSceneObjHolder();
}

/**
 * Returns the message system.
 * @return message system
 */
const MessageSystem* LayoutActor::getMessageSystem() const {
    return mLayoutSceneInfo->getMessageSystem();
}
}  // namespace al
