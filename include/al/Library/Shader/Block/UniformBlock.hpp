#pragma once

#include "common/aglUniformBlock.h"

namespace al {
class UniformBlock : public agl::UniformBlock {
public:
    UniformBlock();

    void swap();
    void flushOnly() const;

    s32 getSwapIndex() const { return mSwapIndex; }

    template <typename T>
    void setValue(s32 memberIndex, T value) const {
        void* buffer = getCurrentBuffer();
        T data = value;
        setData_(buffer, memberIndex, &data, 0, 1);
    }

private:
    s32 mSwapIndex;
};

static_assert(sizeof(UniformBlock) == 0x80);

class UniformBlockSetter {
public:
    UniformBlockSetter(UniformBlock* pBlock, s32 blockIndex);
    ~UniformBlockSetter();

private:
    UniformBlock* mBlock;
    s32 mBlockIndex;
};
}  // namespace al
