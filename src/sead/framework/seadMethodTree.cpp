#include <framework/seadMethodTree.h>
#include <thread/seadCriticalSection.h>

namespace sead
{
void MethodTreeNode::pushBackChild(MethodTreeNode* pNode)
{
    lock_();
    pNode->detachSubTree();
    pNode->mCriticalSection = mCriticalSection;

    if (pNode->child())
    {
        auto* parent = pNode->child()->value();

        if (parent)
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

    if (pNode->child())
    {
        auto* parent = pNode->child()->value();

        if (parent)
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

        if (child && child->value())
        {
            child->value()->attachMutexRec_(pM);
        }
    } while (node->next() && (node = node->next()->value()));
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

    if (node && !mPauseFlag.isOn(cPause_Child))
    {
        while (node)
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

    if (child())
    {
        MethodTreeNode* pChild = child()->value();

        if (pChild)
        {
            MethodTreeNode* pFound = pChild->find(rCondition);

            if (pFound)
            {
                return pFound;
            }
        }
    }

    if (next())
    {
        MethodTreeNode* pNext = next()->value();

        if (pNext)
        {
            MethodTreeNode* pFound = pNext->find(rCondition);

            if (pFound)
            {
                return pFound;
            }
        }
    }

    return nullptr;
}

}  // namespace sead
