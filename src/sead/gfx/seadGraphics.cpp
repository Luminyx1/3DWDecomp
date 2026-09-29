#include "gfx/seadGraphics.h"

#include "basis/seadNew.h"

namespace sead
{
/**
 * Constructs the graphics object without a draw lock context.
 */
Graphics::Graphics() : _20(nullptr), mDrawLockContext(nullptr) {}

Graphics::~Graphics() = default;

/**
 * Creates the draw lock context and runs the platform initialization under the lock.
 * @param pHeap Heap to allocate from.
 */
void Graphics::initialize(Heap* pHeap)
{
    mDrawLockContext = new (pHeap) DrawLockContext();
    initializeDrawLockContext(pHeap);
    mDrawLockContext->lock();
    initializeImpl(pHeap);
    mDrawLockContext->unlock();
}

/**
 * Locks the draw context through the callback if set, otherwise the draw lock context.
 */
void Graphics::lockDrawContext()
{
    if (_20)
    {
        _20(1);
    }
    else
    {
        mDrawLockContext->lock();
    }
}

/**
 * Unlocks the draw context through the callback if set, otherwise the draw lock context.
 */
void Graphics::unlockDrawContext()
{
    if (_20)
    {
        _20(0);
    }
    else
    {
        mDrawLockContext->unlock();
    }
}

/**
 * Initializes host IO; does nothing in release builds.
 */
void Graphics::initHostIO() {}

/**
 * Initializes the draw lock context.
 * @param pHeap Heap to allocate from.
 */
void Graphics::initializeDrawLockContext(Heap* pHeap)
{
    mDrawLockContext->initialize(pHeap);
}

}  // namespace sead
