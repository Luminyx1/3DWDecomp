#pragma once

#include <prim/seadSafeString.h>

namespace al { class NexDataStoreDownloadInfo; }

class GhostPlayerDisplayInfo {
public:
    explicit GhostPlayerDisplayInfo(const char* pFileName);
    GhostPlayerDisplayInfo();
    void init(const al::NexDataStoreDownloadInfo& rInfo, bool isFlagged);

private:
    sead::WFixedSafeString<32> mName;
    bool mIsFlagged;
    bool mIsInitialized;
};
