#include <framework/seadMethodTree.h>
#include <thread/seadCriticalSection.h>

namespace sead
{
void MethodTreeNode::pushBackChild(MethodTreeNode* pNode)
{
    lock_();
    pNode->detachSubTree();
    pNode->mCriticalSection = mCriticalSection;

    if (pNode->child() != nullptr)
    {
        auto* parent = pNode->child()->value();

        if (parent != nullptr)
        {
            parent->attachMutexRec_(mCriticalSection);
        }
    }

    TreeNode::pushBackChild(pNode);
    unlock_();
}

void MethodTreeNode::pushFrontChild(MethodTreeNode* pNode)
{
    lock_();
    pNode->detachSubTree();
    pNode->mCriticalSection = mCriticalSection;

    if (pNode->child() != nullptr)
    {
        auto* parent = pNode->child()->value();

        if (parent != nullptr)
        {
            parent->attachMutexRec_(mCriticalSection);
        }
    }

    TreeNode::pushFrontChild(pNode);
    unlock_();
}

void MethodTreeNode::attachMutexRec_(CriticalSection* pM) const
{
    const MethodTreeNode* node = this;

    do
    {
        auto* child = node->child();
        node->mCriticalSection = pM;

        if (child != nullptr && child->value() != nullptr)
        {
            child->value()->attachMutexRec_(pM);
        }
    } while (node->next() != nullptr && ((node = node->next()->value()) != nullptr));
}

void MethodTreeNode::detachAll()
{
    CriticalSection* cs = mCriticalSection;
    attachMutexRec_(NULL);
    mCriticalSection = cs;

    lock_();
    TreeNode::detachAll();
    unlock_();

    mCriticalSection = NULL;
}

void MethodTreeNode::lock_()
{
    if (mCriticalSection == NULL)
    {
        return;
    }

    mCriticalSection->lock();
}

void MethodTreeNode::unlock_()
{
    if (mCriticalSection == NULL)
    {
        return;
    }

    mCriticalSection->unlock();
}

void MethodTreeNode::call()
{
    lock_();
    callRec_();
    unlock_();
}

void MethodTreeNode::callRec_()
{
    if (!mPauseFlag.isOn(cPause_Self))
    {
        (*mDelegateHolder.data())();
    }

    auto* node = child();

    if (node != nullptr && !mPauseFlag.isOn(cPause_Child))
    {
        while (node != nullptr)
        {
            node->value()->callRec_();
            node = node->value()->next();
        }
    }
}

/**
 * Searches the tree depth first, starting at this node, then its children, then its later siblings.
 * @param rCondition test the node has to pass
 * @return the first node that passes, or nullptr
 */
MethodTreeNode* MethodTreeNode::find(Condition& rCondition)
{
    if (rCondition.isMatch(this))
    {
        return this;
    }

    if (child() != nullptr)
    {
        MethodTreeNode* pChild = child()->value();

        if (pChild != nullptr)
        {
            MethodTreeNode* pFound = pChild->find(rCondition);

            if (pFound != nullptr)
            {
                return pFound;
            }
        }
    }

    if (next() != nullptr)
    {
        MethodTreeNode* pNext = next()->value();

        if (pNext != nullptr)
        {
            MethodTreeNode* pFound = pNext->find(rCondition);

            if (pFound != nullptr)
            {
                return pFound;
            }
        }
    }

    return nullptr;
}

}  // namespace sead
