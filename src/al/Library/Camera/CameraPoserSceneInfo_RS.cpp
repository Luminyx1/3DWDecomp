#include "Library/Camera/CameraPoserSceneInfo_RS.hpp"

namespace al {

/**
 * Creates the scene info with room for the camera rail holders.
 */
CameraPoserSceneInfo_RS::CameraPoserSceneInfo_RS() {
    railHolders = new CameraRailHolder_RS*[16];
}

/**
 * Sets the scene directors used by camera posers.
 * @param pAreaObjDirector Area object director.
 * @param pCollisionDirector Collision director.
 * @param pAudioDirector Audio director.
 */
void CameraPoserSceneInfo_RS::init(AreaObjDirector* pAreaObjDirector,
                                   CollisionDirector* pCollisionDirector,
                                   const AudioDirector* pAudioDirector) {
    areaObjDirector = pAreaObjDirector;
    collisionDirector = pCollisionDirector;
    audioDirector = pAudioDirector;
}

/**
 * Registers a camera rail holder.
 * @param pRailHolder Camera rail holder.
 */
void CameraPoserSceneInfo_RS::registerCameraRailHolder(CameraRailHolder_RS* pRailHolder) {
    railHolders[railHolderNum] = pRailHolder;
    railHolderNum++;
}

}  // namespace al
