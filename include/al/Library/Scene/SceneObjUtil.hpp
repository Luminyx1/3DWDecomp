#pragma once

#include <basis/seadTypes.h>

#include "Library/Scene/SceneObjHolder.hpp"

namespace al {
    ISceneObj* createSceneObj(const IUseSceneObjHolder *, int);
    void setSceneObj(const IUseSceneObjHolder *, ISceneObj *, int);
    ISceneObj* getSceneObj(const IUseSceneObjHolder *, int);
    ISceneObj* tryGetSceneObj(const IUseSceneObjHolder *, int);
    bool isExistSceneObj(const IUseSceneObjHolder *, int);

    /**
     * @brief Get a scene object as its concrete type.
     * @param pHolder Object with access to the scene object holder.
     * @param id Id of the scene object.
     * @return The scene object.
     */
    template <typename T>
    T* getSceneObj(const IUseSceneObjHolder* pHolder, s32 id) {
        return static_cast<T*>(getSceneObj(pHolder, id));
    }

    /**
     * @brief Get a scene object as its concrete type if it exists.
     * @param pHolder Object with access to the scene object holder.
     * @param id Id of the scene object.
     * @return The scene object, or nullptr.
     */
    template <typename T>
    T* tryGetSceneObj(const IUseSceneObjHolder* pHolder, s32 id) {
        return static_cast<T*>(tryGetSceneObj(pHolder, id));
    }
};
