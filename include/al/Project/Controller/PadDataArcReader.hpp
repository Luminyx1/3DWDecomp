#pragma once

#include <basis/seadTypes.h>

#include "Library/Controller/IUsePadDataReader.hpp"

namespace al {
class PadDataArcReader : public IUsePadDataReader {
public:
    PadDataArcReader(const char* pPath);
    PadDataArcReader(const char* pPath, const char* pResourceName);

    void readResource(const char* pResourceName);
    void checkEnd();
    void read(PadDataPack* pFrameData) override;

    bool isEnd() const override { return mIsEnd; }
    u32 getCursorFrame() const override { return mCursorFrame; }
    s32 getRemainFrame() const override { return mTotalFrame - mCursorFrame; }

private:
    PadDataPack* mDataFrames = nullptr;
    u32 mCursorFrame = 0;
    u32 mTotalFrame = 0;
    bool mIsEnd = false;
    const char* mPath = nullptr;
};
}  // namespace al
