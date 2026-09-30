#include "Library/Shader/Block/UniformBlockUtil.hpp"

#include "Library/Shader/Block/UniformBlock.hpp"

namespace al {
/**
 * Creates and declares a uniform block from a layout.
 * @param pLayout Layout entries.
 * @param layoutNum Number of layout entries.
 * @param pHeap Heap to allocate from.
 * @param bufferNum Number of buffers.
 * @return The created uniform block.
 */
UniformBlock* createUniformBlock(const UniformBlockLayout* pLayout, s32 layoutNum, sead::Heap* pHeap,
                                 s32 bufferNum) {
    UniformBlock* block = new UniformBlock();
    block->startDeclare(layoutNum, pHeap);
    for (s32 i = 0; i < layoutNum; i++) {
        block->declare(pLayout[i].mType, pLayout[i].mNum);
    }

    block->create(pHeap, bufferNum, 1);
    return block;
}

/**
 * Declares the members of a uniform block from a layout.
 * @param pBlock Uniform block to declare.
 * @param pLayout Layout entries.
 * @param layoutNum Number of layout entries.
 * @param pHeap Heap to allocate from.
 */
void declareUniformBlock(agl::UniformBlock* pBlock, const UniformBlockLayout* pLayout, s32 layoutNum,
                         sead::Heap* pHeap) {
    pBlock->startDeclare(layoutNum, pHeap);
    for (s32 i = 0; i < layoutNum; i++) {
        pBlock->declare(pLayout[i].mType, pLayout[i].mNum);
    }
}
}  // namespace al
