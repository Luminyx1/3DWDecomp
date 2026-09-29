#include "Library/Execute/ExecutorListBase.hpp"

namespace al {
    /** @brief Creates a named list; the pause name falls back to the list name when none is given. */
    ExecutorListBase::ExecutorListBase(const char* pName, const char* pPauseName)
        : mName(pName), mPauseName(pPauseName) {
        if (pPauseName == nullptr) {
            mPauseName = mName;
        }
    }
};
