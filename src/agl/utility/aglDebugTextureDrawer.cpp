#include "utility/aglDebugTextureDrawer.h"

#include <hostio/seadHostIOPropertyEvent.h>

#include "utility/aglPrimitiveTexture.h"

namespace agl::utl {

SEAD_SINGLETON_DISPOSER_IMPL(DebugTextureDrawer)

/**
 * Constructs the drawer with empty page and texture lists.
 */
DebugTextureDrawer::DebugTextureDrawer() = default;

/**
 * Destroys the drawer and its texture pool.
 */
DebugTextureDrawer::~DebugTextureDrawer()
{
    destroy();
}

/**
 * Allocates the pool of debug textures and puts all of them in the free list.
 * @param pHeap heap to allocate the pool from
 */
void DebugTextureDrawer::initialize(sead::Heap* pHeap)
{
    destroy();

    if (pHeap != nullptr)
    {
        mTextures.tryAllocBuffer(cTextureNum, pHeap);

        for (s32 i = 0; i < cTextureNum; i++)
        {
            mFreeTextures.pushBack(&mTextures[i]);
        }
    }
}

/**
 * Frees the pool of debug textures.
 */
void DebugTextureDrawer::destroy()
{
    mTextures.freeBuffer();
}

/**
 * Deactivates every active page.
 */
void DebugTextureDrawer::inactivateAll()
{
    while (mActivePages.size() > 0)
    {
        DebugTexturePage* pPage = mActivePages.popBack();

        if (pPage == nullptr)
        {
            break;
        }

        pPage->setActive(false);
    }
}

/**
 * Registers a page with the drawer.
 * @param pPage page to register
 */
void DebugTextureDrawer::entryPage_(DebugTexturePage* pPage)
{
    mPageCS.lock();
    mPages.pushBack(pPage);
    mPageCS.unlock();
}

/**
 * Unregisters a page from the drawer.
 * @param pPage page to unregister
 */
void DebugTextureDrawer::erasePage_(DebugTexturePage* pPage)
{
    mPageCS.lock();
    mPages.erase(pPage);
    mPageCS.unlock();
}

/**
 * Removes a page from the list of active pages.
 * @param pPage page to remove
 */
void DebugTextureDrawer::eraseActivePage_(DebugTexturePage* pPage)
{
    mActivePageCS.lock();
    s32 index = mActivePages.indexOf(pPage);

    if (index >= 0)
    {
        mActivePages.erase(index);
        pPage->invalidateHostIoNode();
    }

    mActivePageCS.unlock();
}

/**
 * Returns a debug texture to the free list.
 * @param pTexture texture to return
 */
void DebugTextureDrawer::pushBack_(DebugTexture* pTexture)
{
    mTextureCS.lock();
    mFreeTextures.pushBack(pTexture);
    mTextureCS.unlock();
}

/**
 * Takes a debug texture from the free list.
 * @return a free texture, or nullptr if none is left
 */
DebugTexture* DebugTextureDrawer::popBack_()
{
    mTextureCS.lock();
    DebugTexture* pTexture;

    if (mFreeTextures.size() > 0)
    {
        sead::TListNode<DebugTexture*>* pNode = mFreeTextures.popBack();

        if (pNode != nullptr)
        {
            pNode->mList = nullptr;
        }

        pTexture = pNode->mData;
    }
    else
    {
        pTexture = nullptr;
    }

    mTextureCS.unlock();
    return pTexture;
}

/**
 * Draws the primitive texture debug page when drawing is enabled.
 * @param pDrawContext unused
 * @param rFrameBuffer unused
 * @param rViewport unused
 */
void DebugTextureDrawer::draw(DrawContext* pDrawContext,
                              const sead::LogicalFrameBuffer& rFrameBuffer,
                              const sead::Viewport& rViewport) const
{
    if (mIsEnableDraw)
    {
        PrimitiveTexture::instance()->entryDebugPage();
    }
}

/**
 * Does nothing.
 */
void DebugTextureDrawer::clearEntryTextures() {}

/**
 * Searches the registered pages for one with the given name.
 * @param rName name of the page
 * @return the page, or nullptr if it was not found
 */
DebugTexturePage* DebugTextureDrawer::searchPage(const sead::SafeString& rName)
{
    mPageCS.lock();
    DebugTexturePage* pResult = nullptr;

    for (auto it = mPages.begin(); it != mPages.end(); ++it)
    {
        if ((*it)->mName == rName)
        {
            pResult = *it;
            break;
        }
    }

    mPageCS.unlock();
    return pResult;
}

/**
 * Generates the host IO nodes of every registered page.
 * @param pContext host IO context
 */
void DebugTextureDrawer::genMessage(sead::hostio::Context* pContext)
{
    for (DebugTexturePage* pPage : mPages)
    {
        pPage->genNodePage_(pContext);
    }
}

/**
 * Deactivates every active page when the corresponding property is changed.
 * @param pEvent property event
 */
void DebugTextureDrawer::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    if (pEvent->getId() == reinterpret_cast<const void*>(40000))
    {
        inactivateAll();
    }
}

/**
 * Does nothing.
 * @param pEvent unused
 */
void DebugTextureDrawer::listenNodeEvent(const sead::hostio::NodeEvent* pEvent) {}

/**
 * Invalidates the host IO node of a page and of the previously active page.
 * @param pPage page whose node is invalidated
 */
void DebugTextureDrawer::invalidateHostIoNode_(DebugTexturePage* pPage)
{
    if (pPage != nullptr)
    {
        pPage->invalidateHostIoNode();
    }

    mActivePageCS.lock();

    if (mActivePages.size() >= 2)
    {
        DebugTexturePage* pPrevPage = mActivePages[mActivePages.size() - 2];

        if (pPrevPage != pPage)
        {
            pPrevPage->invalidateHostIoNode();
        }
    }

    mActivePageCS.unlock();
}

/**
 * Copies a texture name into the label table.
 * @param index index of the label
 * @param rLabel name to copy
 * @return whether the index was in range
 */
bool DebugTextureDrawer::copyTextureLabel_(s32 index, const sead::SafeString& rLabel)
{
    if (index >= cTextureLabelNum)
    {
        return false;
    }

    mTextureLabels[index].copy(rLabel);
    return true;
}

}  // namespace agl::utl
