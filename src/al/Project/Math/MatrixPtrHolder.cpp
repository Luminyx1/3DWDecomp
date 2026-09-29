#include "Project/Math/MatrixPtrHolder.hpp"
#include "Project/Base/StringUtil.hpp"

namespace al {
    /** @brief Constructs an empty holder. */
    MtxPtrHolder::MtxPtrHolder() : mNames(nullptr), mMtxPtrs(nullptr), mNum(0) {}

    /**
     * @brief Allocates the slots of the holder.
     * @param num The number of matrices to hold.
     */
    void MtxPtrHolder::init(s32 num) {
        mNum = num;
        mNames = new const char*[num];
        mMtxPtrs = new const sead::Matrix34f*[num];

        for (s32 i = 0; i < mNum; i++) {
            mNames[i] = nullptr;
            mMtxPtrs[i] = nullptr;
        }
    }

    /**
     * @brief Sets the name and matrix of a slot.
     * @param index The index of the slot.
     * @param pName The name of the matrix.
     * @param pMtx The matrix.
     */
    void MtxPtrHolder::setMtxPtrAndName(s32 index, const char* pName, const sead::Matrix34f* pMtx) {
        mNames[index] = pName;
        mMtxPtrs[index] = pMtx;
    }

    /**
     * @brief Sets the matrix of the slot with the given name.
     * @param pName The name of the matrix.
     * @param pMtx The matrix.
     */
    void MtxPtrHolder::setMtxPtr(const char* pName, const sead::Matrix34f* pMtx) {
        s32 index = findIndex(pName);
        mMtxPtrs[index] = pMtx;
    }

    /**
     * @brief Finds the slot with the given name.
     * @param pName The name of the matrix.
     * @return The index of the slot, or -1 if there is none.
     */
    s32 MtxPtrHolder::findIndex(const char* pName) const {
        for (s32 i = 0; i < mNum; i++) {
            if (isEqualString(mNames[i], pName)) {
                return i;
            }
        }

        return -1;
    }

    /**
     * @brief Finds the matrix with the given name.
     * @param pName The name of the matrix.
     * @return The matrix.
     */
    const sead::Matrix34f* MtxPtrHolder::findMtxPtr(const char* pName) const {
        return mMtxPtrs[findIndex(pName)];
    }

    /**
     * @brief Searches for the matrix with the given name.
     * @param pName The name of the matrix.
     * @return The matrix, or nullptr if there is none.
     */
    const sead::Matrix34f* MtxPtrHolder::tryFindMtxPtr(const char* pName) const {
        s32 index = tryFindIndex(pName);
        if (index < 0) {
            return nullptr;
        }

        return mMtxPtrs[index];
    }

    /**
     * @brief Searches for the slot with the given name.
     * @param pName The name of the matrix.
     * @return The index of the slot, or -1 if there is none.
     */
    s32 MtxPtrHolder::tryFindIndex(const char* pName) const {
        for (s32 i = 0; i < mNum; i++) {
            if (isEqualString(mNames[i], pName)) {
                return i;
            }
        }

        return -1;
    }
};
