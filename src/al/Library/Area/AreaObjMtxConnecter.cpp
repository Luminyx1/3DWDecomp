#include "Project/AreaObj/AreaObjMtxConnecter.hpp"

#include "Library/Play/Placement/PlacementFunction.hpp"

namespace al {
/**
 * Constructs a connecter for an area.
 * @param pAreaObj area to move
 * @param rInfo placement info of the parent the area follows
 */
AreaObjMtxConnecter::AreaObjMtxConnecter(AreaObj* pAreaObj, const PlacementInfo& rInfo)
    : mAreaObj(pAreaObj), _40(rInfo) {
    _8 = mAreaObj->_28;
}

/**
 * Connects the area to a parent matrix if the placement matches.
 * @param pParentMtx parent matrix
 * @param rInfo placement info of the parent
 * @return true if the placement matched
 */
bool AreaObjMtxConnecter::trySetParentMtx(const sead::Matrix34f* pParentMtx,
                                          const PlacementInfo& rInfo) {
    if (!isEqualPlacementID(_40, rInfo)) {
        return false;
    }

    _38 = new MtxConnector();
    _38->init(pParentMtx);
    return true;
}

/**
 * Connects the area to a parent matrix and uses it as base matrix if the placement matches.
 * @param pParentMtx parent matrix
 * @param rInfo placement info of the parent
 * @return true if the placement matched
 */
bool AreaObjMtxConnecter::trySyncParentMtx(const sead::Matrix34f* pParentMtx,
                                           const PlacementInfo& rInfo) {
    if (!isEqualPlacementID(_40, rInfo)) {
        return false;
    }

    _8 = *pParentMtx;
    _38 = new MtxConnector();
    _38->init(pParentMtx);
    return true;
}

/**
 * Updates the area matrix from the parent matrix.
 */
void AreaObjMtxConnecter::update() {
    if (_38) {
        _38->multMtx(&mAreaObj->_28, _8);
    }
}

/**
 * Constructs a holder for area connecters.
 * @param maxConnecters capacity of the holder
 */
AreaObjMtxConnecterHolder::AreaObjMtxConnecterHolder(s32 maxConnecters)
    : mNumConnecters(0), mMaxNumConnecters(maxConnecters) {
    mConnecters = new AreaObjMtxConnecter*[maxConnecters];

    for (s32 i = 0; i < mMaxNumConnecters; i++) {
        mConnecters[i] = nullptr;
    }
}

/**
 * Adds a connecter for an area if it is linked to a parent.
 * @param pAreaObj area to add
 * @param rInfo placement info of the area
 * @return true if the area was added
 */
bool AreaObjMtxConnecterHolder::tryAddArea(AreaObj* pAreaObj, const PlacementInfo& rInfo) {
    if (calcLinkChildNum(rInfo, "NoDelete_FollowMtxTarget") == 0) {
        return false;
    }

    PlacementInfo linkInfo;
    getLinksInfo(&linkInfo, rInfo, "NoDelete_FollowMtxTarget");
    mConnecters[mNumConnecters] = new AreaObjMtxConnecter(pAreaObj, linkInfo);
    mNumConnecters++;
    return true;
}

/**
 * Connects all matching areas to a parent matrix.
 * @param pParentMtx parent matrix
 * @param rInfo placement info of the parent
 */
void AreaObjMtxConnecterHolder::registerParentMtx(const sead::Matrix34f* pParentMtx,
                                                  const PlacementInfo& rInfo) {
    for (s32 i = 0; i < mNumConnecters; i++) {
        mConnecters[i]->trySetParentMtx(pParentMtx, rInfo);
    }
}

/**
 * Connects all matching areas to a parent matrix, using it as base matrix.
 * @param pParentMtx parent matrix
 * @param rInfo placement info of the parent
 */
void AreaObjMtxConnecterHolder::registerSyncParentMtx(const sead::Matrix34f* pParentMtx,
                                                      const PlacementInfo& rInfo) {
    for (s32 i = 0; i < mNumConnecters; i++) {
        mConnecters[i]->trySyncParentMtx(pParentMtx, rInfo);
    }
}

/**
 * Updates all connected areas.
 */
void AreaObjMtxConnecterHolder::update() {
    for (s32 i = 0; i < mNumConnecters; i++) {
        mConnecters[i]->update();
    }
}
}  // namespace al
