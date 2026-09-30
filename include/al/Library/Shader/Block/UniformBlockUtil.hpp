#pragma once

#include "common/aglUniformBlock.h"

namespace sead {
class Heap;
}

namespace al {
class UniformBlock;

struct UniformBlockLayout {
    s32 mIndex;
    agl::UniformBlock::Type mType;
    s32 mNum;
};

UniformBlock* createUniformBlock(const UniformBlockLayout* pLayout, s32 layoutNum, sead::Heap* pHeap,
                                 s32 bufferNum);
void declareUniformBlock(agl::UniformBlock* pBlock, const UniformBlockLayout* pLayout, s32 layoutNum,
                         sead::Heap* pHeap);
}  // namespace al
