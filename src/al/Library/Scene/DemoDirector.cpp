#include "Library/Sequence/DemoDirector.hpp"

#include "Library/Audio/System/AudioKeeperFunction.hpp"
#include "Library/Effect/EffectSystem.hpp"
#include "Library/LiveActor/LiveActor.hpp"

namespace al {
/**
 * Constructs a demo director.
 * @param maxActors maximum demo actor count
 */
DemoDirector::DemoDirector(s32 maxActors) : mDemoActorMax(maxActors) {
    mDemoActors = new LiveActor*[maxActors];

    for (s32 i = 0; i < mDemoActorMax; i++) {
        mDemoActors[i] = nullptr;
    }

    mAddDemoActors = new LiveActor*[mDemoActorMax];

    for (s32 i = 0; i < mDemoActorMax; i++) {
        mAddDemoActors[i] = nullptr;
    }

    for (s32 i = 0; i < 20; i++) {
        mOtherDemoActors[i] = nullptr;
    }
}

/**
 * Checks if a demo is active.
 * @return whether a demo is active
 */
bool DemoDirector::isActiveDemo() const {
    return mActiveDemoName != nullptr;
}

/**
 * Checks if the demo of an actor is active.
 * @param pActor actor
 * @return whether the actor's demo is active
 */
bool DemoDirector::isActiveDemo(const LiveActor* pActor) const {
    return mActiveDemoActor == pActor;
}

/**
 * Checks if any demo, including other running demos, is active.
 * @return whether any demo is active
 */
bool DemoDirector::isAnyActiveDemo() const {
    if (mActiveDemoName != nullptr) {
        return true;
    }

    for (s32 i = 0; i < 20; i++) {
        if (mOtherDemoActors[i] != nullptr) {
            return true;
        }
    }

    return false;
}

/**
 * Starts the demo of an actor.
 * @param pActor actor
 * @param pName demo name
 * @return whether the demo started
 */
bool DemoDirector::startDemo(const LiveActor* pActor, const char* pName) {
    mActiveDemoActor = pActor;
    return true;
}

/**
 * Ends the demo of an actor.
 * @param pActor actor
 * @param pName demo name
 */
void DemoDirector::endDemo(const LiveActor* pActor, const char* pName) {
    mActiveDemoActor = nullptr;
}

/**
 * Checks if another demo is running.
 * @return whether another demo is running
 */
bool DemoDirector::isOtherDemoRunning() {
    for (s32 i = 0; i < 20; i++) {
        if (mOtherDemoActors[i] != nullptr) {
            return true;
        }
    }

    return false;
}

/**
 * Changes the audio demo type.
 * @param type new demo type
 */
void DemoDirector::changeActiveAudioDemoType(alSeFunction::DemoType type) {
    alAudioSystemFunction::changeDemo(mAudioDirector,
                                      static_cast<alSeFunction::DemoType>(mAudioDemoType), type);
    mAudioDemoType = type;
    mIsChangedAudioDemoType = true;
}

/**
 * Registers or unregisters an actor running another demo.
 * @param pActor actor
 * @param isRunning whether the actor's demo is running
 */
void DemoDirector::setIsOtherDemoRunning(LiveActor* pActor, bool isRunning) {
    if (isRunning) {
        for (s32 i = 0; i < 20; i++) {
            if (mOtherDemoActors[i] == pActor) {
                return;
            }
        }

        for (s32 i = 0; i < 20; i++) {
            if (mOtherDemoActors[i] == nullptr) {
                mOtherDemoActors[i] = pActor;
                return;
            }
        }
    } else {
        for (s32 i = 0; i < 20; i++) {
            if (mOtherDemoActors[i] == pActor) {
                mOtherDemoActors[i] = nullptr;
                return;
            }
        }
    }
}

/**
 * Gets the name of the active demo.
 * @return demo name, or an empty string
 */
const char* DemoDirector::getActiveDemoName() const {
    return (mActiveDemoName != nullptr) ? mActiveDemoName : "";
}

/**
 * Requests starting a demo.
 * @param pActor actor
 * @param pName demo name
 * @return whether the demo started
 */
bool DemoDirector::requestStartDemo(const LiveActor* pActor, const char* pName) {
    if (isOtherDemoRunning()) {
        return false;
    }

    if (!startDemo(pActor, pName)) {
        return false;
    }

    mActiveDemoName = pName;
    return true;
}

/**
 * Requests starting a demo if no demo is active.
 * @param pActor actor
 * @param pName demo name
 * @return whether the demo started
 */
bool DemoDirector::tryRequestStartDemo(const LiveActor* pActor, const char* pName) {
    if (isOtherDemoRunning()) {
        return false;
    }

    if (mActiveDemoName != nullptr) {
        return false;
    }

    if (!startDemo(pActor, pName)) {
        return false;
    }

    mActiveDemoName = pName;
    return true;
}

/**
 * Ends the active demo and clears the demo actors.
 * @param pActor actor
 * @param pName demo name
 */
void DemoDirector::requestEndDemo(const LiveActor* pActor, const char* pName) {
    endDemo(pActor, pName);
    mActiveDemoName = nullptr;

    for (s32 i = 0; i < mDemoActorNum; i++) {
        mDemoActors[i] = nullptr;
    }

    mDemoActorNum = 0;

    for (s32 i = 0; i < mAddDemoActorNum; i++) {
        mAddDemoActors[i] = nullptr;
    }

    mAddDemoActorNum = 0;
    mAudioDemoType = 0;
    _d5 = false;
}

/**
 * Adds a demo actor.
 * @param pActor actor
 */
void DemoDirector::addDemoActor(LiveActor* pActor) {
    if (mIsUpdatingDemoActor) {
        mAddDemoActors[mAddDemoActorNum++] = pActor;
    } else {
        mDemoActors[mDemoActorNum++] = pActor;
    }
}

/**
 * Removes a demo actor.
 * @param pActor actor
 */
void DemoDirector::removeDemoActor(LiveActor* pActor) {
    for (s32 i = 0; i < mDemoActorNum; i++) {
        if (mDemoActors[i] == pActor) {
            mDemoActorNum--;

            if (mDemoActorNum >= 1) {
                mDemoActors[i] = mDemoActors[mDemoActorNum];
                mDemoActors[mDemoActorNum] = nullptr;
            }

            return;
        }
    }
}

/**
 * Gets the demo actor list.
 * @return demo actor list
 */
LiveActor** DemoDirector::getDemoActorList() const {
    return mDemoActors;
}

/**
 * Gets the number of demo actors.
 * @return demo actor count
 */
s32 DemoDirector::getDemoActorNum() const {
    return mDemoActorNum;
}

/**
 * Updates the demo actors and adds the actors registered during the update.
 * @param pEffectSystem effect system to calculate the actor effects with, or null
 */
void DemoDirector::updateDemoActor(EffectSystem* pEffectSystem) {
    mIsUpdatingDemoActor = true;

    for (s32 i = 0; i < mDemoActorNum; i++) {
        LiveActor* actor = mDemoActors[i];
        actor->movement();

        if (actor->getModelKeeper() != nullptr) {
            actor->calcAnim();
        }

        if (pEffectSystem != nullptr && actor->getEffectKeeper() != nullptr) {
            pEffectSystem->addCalcEffect(reinterpret_cast<u64>(actor->getEffectKeeper()));
        }
    }

    mIsUpdatingDemoActor = false;

    for (s32 i = 0; i < mAddDemoActorNum; i++) {
        LiveActor* actor = mAddDemoActors[i];
        mAddDemoActors[i] = nullptr;
        addDemoActor(actor);
    }

    mAddDemoActorNum = 0;
}
}  // namespace al
