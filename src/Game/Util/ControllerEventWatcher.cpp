#include "Util/ControllerEventWatcher.hpp"

#include <controller/seadControllerMgr.h>

#include "Library/Controller/NpadController.hpp"
#include "Library/Scene/SceneObjUtil.hpp"
#include "Scene/SceneObjID.hpp"

namespace {
/**
 * Gets the controller event watcher of a scene.
 * @param pHolder scene object holder user
 * @return the watcher
 */
ControllerEventWatcher* getControllerEventWatcher(const al::IUseSceneObjHolder* pHolder) {
    return al::getSceneObj<ControllerEventWatcher>(pHolder, SceneObjID_ControllerEventWatcher);
}
}  // namespace

/**
 * Constructs the watcher with an empty listener list.
 */
ControllerEventWatcher::ControllerEventWatcher() = default;

/**
 * Destroys the watcher, releasing every listener.
 */
ControllerEventWatcher::~ControllerEventWatcher() {
    mListeners.clear();
}

/**
 * Notifies every listener when one of the first two Npad controllers is waiting on its
 * accelerometer.
 */
void ControllerEventWatcher::update() {
    sead::ControllerMgr* pMgr = sead::ControllerMgr::instance();

    if (pMgr->getControllerNum() == 0) {
        return;
    }

    al::NpadController* pFirst = pMgr->getControllerByOrderAs<al::NpadController*>(0);
    al::NpadController* pSecond = pMgr->getControllerByOrderAs<al::NpadController*>(1);

    bool isWaiting = false;

    if (pFirst != nullptr && pFirst->getAccelerometerWaitCount() > 0) {
        isWaiting = true;
    }

    if (pSecond != nullptr && pSecond->getAccelerometerWaitCount() > 0) {
        isWaiting = true;
    }

    if (!isWaiting) {
        return;
    }

    for (s32 i = 0; i < mListeners.size(); i++) {
        Listener* pListener = mListeners(i);
        pListener->mFunc(0, pListener->mUserData);
    }
}

/**
 * Registers a listener; does nothing when the listener list is full.
 * @param pFunc callback to notify
 * @param pUserData user data passed to the callback
 */
void ControllerEventWatcher::addListener(ControllerEventListenerFunc pFunc, void* pUserData) {
    s32 index = mListeners.size();

    if (mListeners.isFull()) {
        return;
    }

    mListeners.emplaceBack();
    mListeners(index)->mFunc = pFunc;
    mListeners(index)->mUserData = pUserData;
}

/**
 * Removes every registered listener.
 */
void ControllerEventWatcher::clearListeners() {
    mListeners.clear();
}

namespace rc {
/**
 * Registers a listener on the scene's controller event watcher.
 * @param pHolder scene object holder user
 * @param pFunc callback to notify
 * @param pUserData user data passed to the callback
 */
void addControllerEventListener(const al::IUseSceneObjHolder* pHolder,
                                ControllerEventListenerFunc pFunc, void* pUserData) {
    getControllerEventWatcher(pHolder)->addListener(pFunc, pUserData);
}

/**
 * Removes every listener of the scene's controller event watcher.
 * @param pHolder scene object holder user
 */
void clearControllerEventListeners(const al::IUseSceneObjHolder* pHolder) {
    getControllerEventWatcher(pHolder)->clearListeners();
}

/**
 * Updates the scene's controller event watcher.
 * @param pHolder scene object holder user
 */
void updateControllerEventWatcher(const al::IUseSceneObjHolder* pHolder) {
    getControllerEventWatcher(pHolder)->update();
}
}  // namespace rc
