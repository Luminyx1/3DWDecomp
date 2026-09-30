#include "Library/Shader/Block/UniformBlock.hpp"

namespace al {
/**
 * Constructs a double-buffered uniform block.
 */
UniformBlock::UniformBlock() : mSwapIndex(0) {}

/**
 * Switches to the other buffer.
 */
void UniformBlock::swap() {
    mSwapIndex = 1 - mSwapIndex;
    setCurrentBufferIndex(mSwapIndex);
}

/**
 * Flushes the CPU cache of the current buffer.
 */
void UniformBlock::flushOnly() const {
    u32 offset = getCurrentBlockOffset(0);
    agl::GPUMemVoidAddr addr = getBuffer();
    agl::GPUMemVoidAddr(addr, offset).flushCPUCache(getBlockSize());
}

/**
 * Swaps the buffer of a uniform block for writing.
 * @param pBlock Uniform block to write.
 * @param blockIndex Index of the block to flush when done.
 */
UniformBlockSetter::UniformBlockSetter(UniformBlock* pBlock, s32 blockIndex)
    : mBlock(pBlock), mBlockIndex(blockIndex) {
    pBlock->swap();
}

/**
 * Flushes the written block.
 */
UniformBlockSetter::~UniformBlockSetter() {
    u32 offset = mBlock->getCurrentBlockOffset(mBlockIndex);
    agl::GPUMemVoidAddr addr = mBlock->getBuffer();
    agl::GPUMemVoidAddr(addr, offset).flushCPUCache(mBlock->getBlockSize());
}
}  // namespace al
