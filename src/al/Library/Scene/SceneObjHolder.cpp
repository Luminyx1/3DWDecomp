#include "Library/Scene/SceneObjHolder.hpp"

#include "Library/Scene/ISceneObj.hpp"
#include "Library/Scene/SceneObjUtil.hpp"

namespace al {
/**
 * Creates an empty scene object table.
 * @param pCreator Factory function for scene objects.
 * @param numObjs Number of scene object slots.
 */
SceneObjHolder::SceneObjHolder(CreateFunc pCreator, int numObjs)
    : mFunc(pCreator), mNumObjs(numObjs) {
    mObjs = new ISceneObj*[numObjs];

    for (int i = 0; i < mNumObjs; i++) {
        mObjs[i] = nullptr;
    }
}

/**
 * Creates and initializes a scene object if it doesn't exist yet.
 * @param objID Scene object id.
 * @return The scene object.
 */
__attribute__((noinline)) ISceneObj* SceneObjHolder::create(int objID) {
    if (mObjs[objID]) {
        return mObjs[objID];
    }

    mObjs[objID] = mFunc(objID);
    mObjs[objID]->initSceneObj();
    return mObjs[objID];
}

/**
 * Gets a scene object if it exists.
 * @param objID Scene object id.
 * @return The scene object or nullptr.
 */
__attribute__((noinline)) ISceneObj* SceneObjHolder::tryGetObj(int objID) const {
    return mObjs[objID];
}

/**
 * Gets a scene object.
 * @param objID Scene object id.
 * @return The scene object.
 */
__attribute__((noinline)) ISceneObj* SceneObjHolder::getObj(int objID) const {
    return mObjs[objID];
}

/**
 * Checks whether a scene object exists.
 * @param objID Scene object id.
 * @return True if it exists.
 */
__attribute__((noinline)) bool SceneObjHolder::isExist(int objID) const {
    return mObjs[objID] != nullptr;
}

/**
 * Sets a scene object.
 * @param pObj The scene object.
 * @param objID Scene object id.
 */
__attribute__((noinline)) void SceneObjHolder::setSceneObj(ISceneObj* pObj, int objID) {
    mObjs[objID] = pObj;
}

/**
 * Calls initAfterPlacementSceneObj on every existing scene object.
 * @param rInfo Actor init info.
 */
void SceneObjHolder::initAfterPlacementSceneObj(const ActorInitInfo& rInfo) {
    for (int i = 0; i < mNumObjs; i++) {
        if (mObjs[i]) {
            mObjs[i]->initAfterPlacementSceneObj(rInfo);
        }
    }
}

/**
 * Creates a scene object through the holder's scene object holder.
 * @param pHolder Scene object holder user.
 * @param objID Scene object id.
 * @return The scene object.
 */
ISceneObj* createSceneObj(const IUseSceneObjHolder* pHolder, int objID) {
    return pHolder->getSceneObjHolder()->create(objID);
}

/**
 * Sets a scene object in the holder's scene object holder.
 * @param pHolder Scene object holder user.
 * @param pObj The scene object.
 * @param objID Scene object id.
 */
void setSceneObj(const IUseSceneObjHolder* pHolder, ISceneObj* pObj, int objID) {
    pHolder->getSceneObjHolder()->setSceneObj(pObj, objID);
}

/**
 * Gets a scene object from the holder's scene object holder.
 * @param pHolder Scene object holder user.
 * @param objID Scene object id.
 * @return The scene object.
 */
ISceneObj* getSceneObj(const IUseSceneObjHolder* pHolder, int objID) {
    return pHolder->getSceneObjHolder()->getObj(objID);
}

/**
 * Gets a scene object from the holder's scene object holder if it exists.
 * @param pHolder Scene object holder user.
 * @param objID Scene object id.
 * @return The scene object or nullptr.
 */
ISceneObj* tryGetSceneObj(const IUseSceneObjHolder* pHolder, int objID) {
    return pHolder->getSceneObjHolder()->tryGetObj(objID);
}

/**
 * Checks whether a scene object exists in the holder's scene object holder.
 * @param pHolder Scene object holder user.
 * @param objID Scene object id.
 * @return True if it exists.
 */
bool isExistSceneObj(const IUseSceneObjHolder* pHolder, int objID) {
    return pHolder->getSceneObjHolder()->isExist(objID);
}
}  // namespace al
