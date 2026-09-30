#pragma once

#include "common/aglUniformBlock.h"

namespace al {
class UniformBlock : public agl::UniformBlock {
public:
    UniformBlock();

    void swap();
    void flushOnly() const;

    s32 getSwapIndex() const { return mSwapIndex; }

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
