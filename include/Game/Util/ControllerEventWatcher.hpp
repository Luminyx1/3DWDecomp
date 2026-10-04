#pragma once

#include <basis/seadTypes.h>
#include <container/seadObjArray.h>

#include "Library/Scene/ISceneObj.hpp"

namespace al {
class IUseSceneObjHolder;
}  // namespace al

/// Callback notified of a controller event (event id, user data).
using ControllerEventListenerFunc = void (*)(s32, void*);

/**
 * @brief Scene object that notifies registered listeners about controller events.
 */
class ControllerEventWatcher : public al::ISceneObj {
public:
    /// A registered listener callback and its user data.
    struct Listener {
        ControllerEventListenerFunc mFunc;
        void* mUserData;
    };

    typedef sead::FixedObjArray<Listener, 64> ListenerArray;

    ControllerEventWatcher();
    ~ControllerEventWatcher();

    void update();
    void addListener(ControllerEventListenerFunc pFunc, void* pUserData);
    void clearListeners();

private:
    ListenerArray mListeners;
};

static_assert(sizeof(ControllerEventWatcher) == 0x628);

namespace rc {
void addControllerEventListener(const al::IUseSceneObjHolder* pHolder,
                                ControllerEventListenerFunc pFunc, void* pUserData);
void clearControllerEventListeners(const al::IUseSceneObjHolder* pHolder);
void updateControllerEventWatcher(const al::IUseSceneObjHolder* pHolder);
}  // namespace rc
