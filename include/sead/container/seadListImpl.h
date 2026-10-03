#ifndef SEAD_LIST_IMPL_H_
#define SEAD_LIST_IMPL_H_

#include <attributes.h>
#include <basis/seadTypes.h>
#include <utility>

namespace sead
{
class Random;

class ListNode
{
public:
    ListNode* next() const { return mNext; }
    ListNode* prev() const { return mPrev; }
    bool isLinked() const { return mNext || mPrev; }

private:
    friend class ListImpl;

    void init_() { *this = {}; }
    void insertBack_(ListNode* node);
    void insertFront_(ListNode* node);
    void erase_();

    ListNode* mPrev = nullptr;
    ListNode* mNext = nullptr;
};

class ListImpl
{
public:
    ALWAYS_INLINE ListImpl() : mStartEnd(), mCount(0)
    {
        mStartEnd.mNext = &mStartEnd;
        mStartEnd.mPrev = &mStartEnd;
    }

    bool isEmpty() const { return mCount == 0; }
    s32 size() const { return mCount; }

    void reverse();
    void shuffle();
    void shuffle(Random* random);
    bool checkLinks() const;

protected:
    using CompareCallbackImpl = int (*)(const void*, const void*);

    template <class T, class ComparePredicate>
    void sort([[maybe_unused]] s32 offset, const ComparePredicate& cmp)
    {
        if (mCount < 2)
        {
            return;
        }

        ListNode* node = nth(1);
        while (node != &mStartEnd)
        {
            ListNode* prev = node->mPrev;
            ListNode* next = node->mNext;
            if (cmp(reinterpret_cast<T*>(prev), reinterpret_cast<T*>(node)) >= 0)
            {
                do
                {
                    prev = prev->mPrev;
                } while (prev != &mStartEnd &&
                         cmp(reinterpret_cast<T*>(prev), reinterpret_cast<T*>(node)) >= 0);
                node->erase_();
                prev->insertBack_(node);
            }
            node = next;
        }
    }

    /**
     * @brief Stably sorts the intrusive list with a caller-supplied comparator.
     * @tparam T Object type containing each node.
     * @tparam ComparePredicate Callable comparator type, retaining function references.
     * @param offset Byte offset of the list node within T.
     * @param rCmp Comparator returning a negative, zero, or positive ordering result.
     */
    template <class T, class ComparePredicate>
    void mergeSort(s32 offset, ComparePredicate&& rCmp)
    {
        this->mergeSortImpl_<T, ComparePredicate>(mStartEnd.mNext, mStartEnd.mPrev, size(), offset,
                                                  std::forward<ComparePredicate>(rCmp));
    }

    void pushBack(ListNode* item)
    {
        mStartEnd.insertFront_(item);
        ++mCount;
    }

    void pushFront(ListNode* item)
    {
        mStartEnd.insertBack_(item);
        ++mCount;
    }

    ListNode* popBack();
    ListNode* popFront();

    void insertBefore(ListNode* node, ListNode* node_to_insert)
    {
        node->insertFront_(node_to_insert);
        ++mCount;
    }

    void insertAfter(ListNode* node, ListNode* node_to_insert)
    {
        node->insertBack_(node_to_insert);
        ++mCount;
    }

    void erase(ListNode* item)
    {
        item->erase_();
        --mCount;
    }

    ListNode* front() const { return mCount > 0 ? mStartEnd.mNext : nullptr; }
    ListNode* back() const { return mCount > 0 ? mStartEnd.mPrev : nullptr; }
    ListNode* nth(int n) const;
    s32 indexOf(const ListNode*) const;

    void swap(ListNode* n1, ListNode* n2);
    void moveAfter(ListNode* basis, ListNode* n);
    void moveBefore(ListNode* basis, ListNode* n);

    ListNode* find(const void* ptr, s32 offset, CompareCallbackImpl cmp) const;
    void uniq(s32 offset, CompareCallbackImpl cmp);

    void clear();

    /**
     * @brief Gets the object containing an intrusive list node.
     * @tparam T Containing object type.
     * @param pNode Embedded node of a valid T object.
     * @param offset Nonnegative byte offset of the node within T.
     * @return Pointer to the containing object.
     */
    template <class T>
    static T* getObjectFromNode_(ListNode* pNode, s32 offset)
    {
        return reinterpret_cast<T*>(reinterpret_cast<u8*>(pNode) + static_cast<s32>(-static_cast<u32>(offset)));
    }

    /**
     * @brief Sorts an inclusive node range, using insertion sort for short ranges.
     * @tparam T Object type containing each pNode.
     * @tparam ComparePredicate Callable comparator type.
     * @param pFront First node in the range.
     * @param pBack Last node in the range.
     * @param num Number of nodes; must be nonnegative.
     * @param offset Byte offset of the list node within T.
     * @param rPredicate Comparator returning the relative ordering of two objects.
     */
    template <class T, class ComparePredicate>
    static void mergeSortImpl_(ListNode* pFront, ListNode* pBack, s32 num, s32 offset,
                               ComparePredicate&& rPredicate)
    {
        if (num >= 9)
        {
            const u32 leftNum = num / 2;
            ListNode* pMiddle = pFront;
            for (u32 i = 1; i < leftNum; ++i)
            {
                pMiddle = pMiddle->mNext;
            }
            ListNode* pBeforeLeft = pFront->mPrev;
            ListNode* pRight = pMiddle->mNext;
            const s32 rightNum = num - leftNum;
            mergeSortImpl_<T, ComparePredicate>(pFront, pMiddle, leftNum, offset,
                                                std::forward<ComparePredicate>(rPredicate));
            ListNode* pLeft = pBeforeLeft->mNext;
            ListNode* pBeforeRight = pRight->mPrev;
            mergeSortImpl_<T, ComparePredicate>(pRight, pBack, rightNum, offset,
                                                std::forward<ComparePredicate>(rPredicate));
            pRight = pBeforeRight->mNext;
            s32 leftRemaining = leftNum;
            s32 rightRemaining = rightNum;
            ListNode* pPrevious = pBeforeLeft;
            while (rightRemaining > 0 || leftRemaining > 0)
            {
                if (leftRemaining != 0 &&
                    (rightRemaining == 0 ||
                     rPredicate(getObjectFromNode_<T>(pLeft, offset), getObjectFromNode_<T>(pRight, offset)) <= 0))
                {
                    ListNode* pNext = pLeft->mNext;
                    pLeft->erase_();
                    pPrevious->insertBack_(pLeft);
                    pPrevious = pLeft;
                    pLeft = pNext;
                    --leftRemaining;
                }
                else
                {
                    ListNode* pNext = pRight->mNext;
                    pRight->erase_();
                    pPrevious->insertBack_(pRight);
                    pPrevious = pRight;
                    pRight = pNext;
                    --rightRemaining;
                }
            }
        }
        else if (num >= 2)
        {
            ListNode* pNode = pFront->mNext;
            ListNode* pEnd = pBack->mNext;
            ListNode* pBegin = pFront->mPrev;
            while (pNode != pEnd)
            {
                ListNode* pPrevious = pNode->mPrev;
                ListNode* pNext = pNode->mNext;
                T* pObject = getObjectFromNode_<T>(pNode, offset);
                if (rPredicate(getObjectFromNode_<T>(pPrevious, offset), pObject) > 0)
                {
                    do
                    {
                        pPrevious = pPrevious->mPrev;
                    } while (pPrevious != pBegin &&
                             rPredicate(getObjectFromNode_<T>(pPrevious, offset), pObject) > 0);
                    pNode->erase_();
                    pPrevious->insertBack_(pNode);
                }
                pNode = pNext;
            }
        }
    }

protected:
    ListNode mStartEnd;
    s32 mCount;
};

}  // namespace sead

#endif  // SEAD_LIST_IMPL_H_
