#include <nn/g3d/g3d_Flag.h>

namespace nn::g3d::detail {
// flagCount is the number of bits per buffer; bufferCount is the number of copies.
// buffer supplies storage and bufferSize describes its capacity (unchecked in this build).
void FlagSet::Initialize(int flagCount, int bufferCount, void* buffer, size_t bufferSize) {
    mFlagCount = flagCount;
    mBufferCount = bufferCount;
    mWordCount = (flagCount + 31) >> 5;
    if (flagCount > 0) {
        mPending = static_cast<u32*>(buffer);
        if (bufferCount == 1) mBufferFlags = mPending;
        else mBufferFlags = mPending + mWordCount;
    } else {
        mPending = nullptr;
        mBufferFlags = nullptr;
    }

    mFlags = 0;
    mDirtyBuffers = 0;
}

void FlagSet::Caclulate() {
    if (!(mFlags & 1)) return;
    if (mBufferCount > 1 && mFlagCount > 0) {
        for (int word = 0; word < mWordCount; ++word) {
            u32 pending = mPending[word];
            for (int buffer = 0; buffer < mBufferCount; ++buffer)
                mBufferFlags[buffer * mWordCount + word] |= pending;
            mPending[word] = 0;
        }
    }

    mDirtyBuffers = 0xff;
    mFlags &= ~1;
}
}
