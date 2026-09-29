#include "Library/Scene/SceneObjHolder.hpp"

#include "Library/Scene/ISceneObj.hpp"
#include "Library/Scene/SceneObjUtil.hpp"

namespace al {
/**
 * @brief Constructs a holder with an empty slot for every scene object id.
 * @param func The factory used to create scene objects by id.
 * @param numObjs The number of scene object slots.
 */
SceneObjHolder::SceneObjHolder(CreateFunc func, int numObjs) {
    mFunc = func;
    mNumObjs = numObjs;
    mObjs = new ISceneObj*[numObjs];

    for (int i = 0; i < mNumObjs; i++) {
        mObjs[i] = nullptr;
    }
}

/**
 * @brief Creates and initializes the scene object with a given id if it does not exist yet.
 * @param objID The id of the scene object.
 * @return The existing or newly created scene object.
 */
__attribute__((noinline)) ISceneObj* SceneObjHolder::create(int objID) {
    if (mObjs[objID] != nullptr) {
        return mObjs[objID];
    }

    mObjs[objID] = mFunc(objID);
    mObjs[objID]->initSceneObj();
    return mObjs[objID];
}

/**
 * @brief Gets the scene object with a given id, if it exists.
 * @param objID The id of the scene object.
 * @return The scene object, or nullptr if it was not created.
 */
__attribute__((noinline)) ISceneObj* SceneObjHolder::tryGetObj(int objID) const {
    return mObjs[objID];
}

/**
 * @brief Gets the scene object with a given id.
 * @param objID The id of the scene object.
 * @return The scene object.
 */
__attribute__((noinline)) ISceneObj* SceneObjHolder::getObj(int objID) const {
    return mObjs[objID];
}

/**
 * @brief Checks whether the scene object with a given id exists.
 * @param objID The id of the scene object.
 * @return True if the scene object was created or set.
 */
__attribute__((noinline)) bool SceneObjHolder::isExist(int objID) const {
    return mObjs[objID] != nullptr;
}

/**
 * @brief Stores a scene object in the slot for a given id.
 * @param pObj The scene object to store.
 * @param objID The id of the slot.
 */
__attribute__((noinline)) void SceneObjHolder::setSceneObj(ISceneObj* pObj, int objID) {
    mObjs[objID] = pObj;
}

/**
 * @brief Runs the after-placement initialization of every existing scene object.
 * @param rInfo The actor init info used for placement.
 */
void SceneObjHolder::initAfterPlacementSceneObj(const ActorInitInfo& rInfo) {
    for (int i = 0; i < mNumObjs; i++) {
        ISceneObj* pObj = mObjs[i];
        if (pObj != nullptr) {
            pObj->initAfterPlacementSceneObj(rInfo);
        }
    }
}

/**
 * @brief Creates the scene object with a given id through the user's scene object holder.
 * @param pUser The scene object holder user.
 * @param objID The id of the scene object.
 * @return The existing or newly created scene object.
 */
ISceneObj* createSceneObj(const IUseSceneObjHolder* pUser, int objID) {
    return pUser->getSceneObjHolder()->create(objID);
}

/**
 * @brief Stores a scene object in the user's scene object holder.
 * @param pUser The scene object holder user.
 * @param pObj The scene object to store.
 * @param objID The id of the slot.
 */
void setSceneObj(const IUseSceneObjHolder* pUser, ISceneObj* pObj, int objID) {
    pUser->getSceneObjHolder()->setSceneObj(pObj, objID);
}

/**
 * @brief Gets the scene object with a given id from the user's scene object holder.
 * @param pUser The scene object holder user.
 * @param objID The id of the scene object.
 * @return The scene object.
 */
ISceneObj* getSceneObj(const IUseSceneObjHolder* pUser, int objID) {
    return pUser->getSceneObjHolder()->getObj(objID);
}

/**
 * @brief Gets the scene object with a given id from the user's scene object holder, if it exists.
 * @param pUser The scene object holder user.
 * @param objID The id of the scene object.
 * @return The scene object, or nullptr if it was not created.
 */
ISceneObj* tryGetSceneObj(const IUseSceneObjHolder* pUser, int objID) {
    return pUser->getSceneObjHolder()->tryGetObj(objID);
}

/**
 * @brief Checks whether the scene object with a given id exists in the user's holder.
 * @param pUser The scene object holder user.
 * @param objID The id of the scene object.
 * @return True if the scene object exists.
 */
bool isExistSceneObj(const IUseSceneObjHolder* pUser, int objID) {
    return pUser->getSceneObjHolder()->isExist(objID);
}
}  // namespace al
