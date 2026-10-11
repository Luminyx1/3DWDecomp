#include "NPC/GhostPlayerDisplayInfo.hpp"
#include "NPC/GhostPlayerUserInfo.hpp"
#include <prim/seadStringUtil.h>

/**
 * @brief Loads the display name and flag for a built-in ghost recording.
 * @param pFileName Recording filename used to find its user-information entry.
 */
GhostPlayerDisplayInfo::GhostPlayerDisplayInfo(const char* pFileName)
    : mIsFlagged(false), mIsInitialized(false) {
    const auto* pInfo = rc::findGhostPlayerUserInfo(pFileName);
    const char* pName = pInfo->mName;
    sead::StringUtil::convertSjisToUtf16(mName.getBuffer(), mName.getBufferSize(), pName, -1);
    mIsFlagged = pInfo->mIsFlagged;
    mIsInitialized = true;
}

/** @brief Creates an empty, uninitialized ghost display record. */
GhostPlayerDisplayInfo::GhostPlayerDisplayInfo() : mIsFlagged(false), mIsInitialized(false) {}

/**
 * @brief Marks a downloaded ghost's display record as initialized.
 * @param rInfo Download metadata; unused by this implementation.
 * @param isFlagged Flag to associate with the downloaded ghost.
 */
void GhostPlayerDisplayInfo::init(const al::NexDataStoreDownloadInfo& rInfo, bool isFlagged) {
    mIsFlagged = isFlagged;
    mIsInitialized = true;
}
