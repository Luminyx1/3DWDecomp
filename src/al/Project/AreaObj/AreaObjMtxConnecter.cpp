#include "Project/AreaObj/AreaObjMtxConnecter.hpp"
#include "Library/Play/Placement/PlacementFunction.hpp"

namespace al {
    /**
     * @brief Constructs a connecter for an area that follows the object with the given placement.
     * @param pAreaObj The area to move.
     * @param rParentInfo The placement of the object the area follows.
     */
    AreaObjMtxConnecter::AreaObjMtxConnecter(AreaObj* pAreaObj, const PlacementInfo& rParentInfo)
        : mAreaObj(pAreaObj), _38(nullptr), _40(rParentInfo) {
        _8 = mAreaObj->_28;
    }

    /**
     * @brief Connects the area to a matrix if it belongs to the followed object.
     * @param pParentMtx The matrix of the object.
     * @param rParentInfo The placement of the object.
     * @return Whether the object is the one the area follows.
     */
    bool AreaObjMtxConnecter::trySetParentMtx(const sead::Matrix34f* pParentMtx, const PlacementInfo& rParentInfo) {
        if (!isEqualPlacementID(_40, rParentInfo)) {
            return false;
        }

        _38 = new MtxConnector();
        _38->init(pParentMtx);
        return true;
    }

    /**
     * @brief Connects the area to a matrix if it belongs to the followed object, taking the matrix as the base.
     * @param pParentMtx The matrix of the object.
     * @param rParentInfo The placement of the object.
     * @return Whether the object is the one the area follows.
     */
    bool AreaObjMtxConnecter::trySyncParentMtx(const sead::Matrix34f* pParentMtx, const PlacementInfo& rParentInfo) {
        if (!isEqualPlacementID(_40, rParentInfo)) {
            return false;
        }

        _8 = *pParentMtx;
        _38 = new MtxConnector();
        _38->init(pParentMtx);
        return true;
    }

    /** @brief Moves the area with the connected matrix. */
    void AreaObjMtxConnecter::update() {
        if (_38 != nullptr) {
            _38->multMtx(&mAreaObj->_28, _8);
        }
    }

    /**
     * @brief Constructs a holder for a number of connecters.
     * @param maxNum The maximum number of connecters.
     */
    AreaObjMtxConnecterHolder::AreaObjMtxConnecterHolder(s32 maxNum) : mNumConnecters(0), mMaxNumConnecters(maxNum) {
        mConnecters = new AreaObjMtxConnecter*[maxNum];
        for (s32 i = 0; i < mMaxNumConnecters; i++) {
            mConnecters[i] = nullptr;
        }
    }

    /**
     * @brief Adds a connecter for an area if it is linked to an object to follow.
     * @param pAreaObj The area.
     * @param rInfo The placement of the area.
     * @return Whether a connecter was added.
     */
    bool AreaObjMtxConnecterHolder::tryAddArea(AreaObj* pAreaObj, const PlacementInfo& rInfo) {
        if (calcLinkChildNum(rInfo, "NoDelete_FollowMtxTarget") == 0) {
            return false;
        }

        PlacementInfo parentInfo;
        getLinksInfo(&parentInfo, rInfo, "NoDelete_FollowMtxTarget");
        mConnecters[mNumConnecters] = new AreaObjMtxConnecter(pAreaObj, parentInfo);
        mNumConnecters++;
        return true;
    }

    /**
     * @brief Connects the areas that follow an object to its matrix.
     * @param pParentMtx The matrix of the object.
     * @param rParentInfo The placement of the object.
     */
    void AreaObjMtxConnecterHolder::registerParentMtx(const sead::Matrix34f* pParentMtx, const PlacementInfo& rParentInfo) {
        for (s32 i = 0; i < mNumConnecters; i++) {
            mConnecters[i]->trySetParentMtx(pParentMtx, rParentInfo);
        }
    }

    /**
     * @brief Connects the areas that follow an object to its matrix, taking the matrix as the base.
     * @param pParentMtx The matrix of the object.
     * @param rParentInfo The placement of the object.
     */
    void AreaObjMtxConnecterHolder::registerSyncParentMtx(const sead::Matrix34f* pParentMtx, const PlacementInfo& rParentInfo) {
        for (s32 i = 0; i < mNumConnecters; i++) {
            mConnecters[i]->trySyncParentMtx(pParentMtx, rParentInfo);
        }
    }

    /** @brief Moves all connected areas. */
    void AreaObjMtxConnecterHolder::update() {
        for (s32 i = 0; i < mNumConnecters; i++) {
            mConnecters[i]->update();
        }
    }
};
