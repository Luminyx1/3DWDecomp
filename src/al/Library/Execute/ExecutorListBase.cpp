#include "Library/Execute/ExecutorListBase.hpp"

namespace al {
    /**
     * @brief Creates a named executor list.
     * @param pName The name of the list.
     * @param pPauseName The name used while the list is paused, or nullptr to use pName.
     */
    ExecutorListBase::ExecutorListBase(const char* pName, const char* pPauseName)
        : mName(pName), mPauseName(pPauseName) {
        if (pPauseName == nullptr) {
            mPauseName = mName;
        }
    }
};
