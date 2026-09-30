#include <basis/seadRawPrint.h>
#include <container/seadListImpl.h>
#include <random/seadRandom.h>

namespace sead
{
/**
 * Links a node right after this one.
 * @param pNode unlinked node to insert
 */
void ListNode::insertBack_(ListNode* pNode)
{
    SEAD_ASSERT_MSG(!pNode->isLinked(), "pNode is already linked.");
    ListNode* next = mNext;
    mNext = pNode;
    pNode->mPrev = this;
    pNode->mNext = next;

    if (next)
    {
        next->mPrev = pNode;
    }
}

/**
 * Links a node right before this one.
 * @param pNode unlinked node to insert
 */
void ListNode::insertFront_(ListNode* pNode)
{
    SEAD_ASSERT_MSG(!pNode->isLinked(), "pNode is already linked.");
    ListNode* prev = mPrev;
    this->mPrev = pNode;
    pNode->mPrev = prev;
    pNode->mNext = this;
    if (prev == NULL)
    {
        return;
    }

    prev->mNext = pNode;
}

/**
 * Unlinks the node from its neighbours.
 */
void ListNode::erase_()
{
    SEAD_ASSERT_MSG(isLinked(), "node is not linked.");
    if (mPrev != nullptr)
    {
        mPrev->mNext = mNext;
    }

    if (mNext != nullptr)
    {
        mNext->mPrev = mPrev;
    }

    mPrev = mNext = NULL;
}

/**
 * Removes the last node.
 * @return the removed node, or nullptr if the list is empty
 */
ListNode* ListImpl::popBack()
{
    if (mCount < 1)
    {
        return nullptr;
    }

    ListNode* back = mStartEnd.mPrev;
    back->erase_();
    --mCount;
    return back;
}

/**
 * Removes the first node.
 * @return the removed node, or nullptr if the list is empty
 */
ListNode* ListImpl::popFront()
{
    if (mCount < 1)
    {
        return nullptr;
    }

    ListNode* front = mStartEnd.mNext;
    front->erase_();
    --mCount;
    return front;
}

/**
 * @param index position to look up
 * @return the node at that position, or nullptr if it is out of range
 */
ListNode* ListImpl::nth(s32 index) const
{
    if (u32(mCount) <= u32(index))
    {
        SEAD_ASSERT_MSG(false, "index exceeded[%d/%d]", index, mCount);
        return nullptr;
    }

    ListNode* node = mStartEnd.mNext;
    for (s32 i = 0; i < index; ++i)
    {
        node = node->mNext;
    }

    return node;
}

/**
 * @param pN node to look for
 * @return its position, or -1 if it is not in the list
 */
s32 ListImpl::indexOf(const ListNode* pN) const
{
    ListNode* node = mStartEnd.mNext;
    s32 index = 0;
    while (node != &mStartEnd)
    {
        if (node == pN)
        {
            return index;
        }

        ++index;
        node = node->mNext;
    }

    return -1;
}

/**
 * Unlinks every node and empties the list.
 */
void ListImpl::clear()
{
    ListNode* node = mStartEnd.mNext;
    while (node != &mStartEnd)
    {
        ListNode* next = node->mNext;
        node->init_();
        node = next;
    }

    mCount = 0;
    mStartEnd.mPrev = &mStartEnd;
    mStartEnd.mNext = &mStartEnd;
}

/**
 * Swaps the positions of two nodes of the list.
 * @param pN1 first node
 * @param pN2 second node
 */
void ListImpl::swap(ListNode* pN1, ListNode* pN2)
{
    SEAD_ASSERT(pN1->mPrev && pN1->mNext && pN2->mPrev && pN2->mNext);
    if (pN1 == pN2)
    {
        return;
    }

    ListNode* n1_prev = pN1->mPrev;
    ListNode* n2_prev = pN2->mPrev;

    if (n2_prev != pN1)
    {
        pN1->erase_();
        n2_prev->insertBack_(pN1);
    }

    if (n1_prev != pN2)
    {
        pN2->erase_();
        n1_prev->insertBack_(pN2);
    }
}

/**
 * Moves a node of the list right after another.
 * @param pBasis node to move it after
 * @param pNode node to move
 */
void ListImpl::moveAfter(ListNode* pBasis, ListNode* pNode)
{
    if (pBasis == pNode)
    {
        return;
    }

    pNode->erase_();
    pBasis->insertBack_(pNode);
}

/**
 * Moves a node of the list right before another.
 * @param pBasis node to move it before
 * @param pNode node to move
 */
void ListImpl::moveBefore(ListNode* pBasis, ListNode* pNode)
{
    if (pBasis == pNode)
    {
        return;
    }

    pNode->erase_();
    pBasis->insertFront_(pNode);
}

/**
 * Reverses the order of the nodes.
 */
void ListImpl::reverse()
{
    if (mCount < 2)
    {
        return;
    }

    ListNode* pFront = mStartEnd.mNext;
    ListNode* pNode = mStartEnd.mPrev;
    ListNode* pPrev;
    do
    {
        pPrev = pNode->mPrev;
        pNode->erase_();
        pFront->insertFront_(pNode);
        pNode = pPrev;
    } while (pPrev != pFront);
}

/**
 * Shuffles the nodes (Fisher-Yates).
 * @param pRandom random number generator to use
 */
void ListImpl::shuffle(Random* pRandom)
{
    SEAD_ASSERT(pRandom);
    for (s32 i = mCount; i > 1; --i)
    {
        const u32 j = pRandom->getU32(i);
        swap(nth(i - 1), nth(j));
    }
}

/**
 * Consistency check of the links (only implemented in debug builds).
 * @return true
 */
bool ListImpl::checkLinks() const
{
    return true;
}

}  // namespace sead
