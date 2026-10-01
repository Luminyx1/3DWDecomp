#include <basis/seadRawPrint.h>
#include <container/seadTreeNode.h>

namespace sead
{
/**
 * Creates a node with no parent, children or siblings.
 */
TreeNode::TreeNode()
{
    clearLinks();
}

/**
 * Unlinks every node below this one, depth first.
 */
void TreeNode::clearChildLinksRecursively_()
{
    TreeNode* node = this->mChild;

    while (node != nullptr)
    {
        TreeNode* next = node->mNext;
        node->clearChildLinksRecursively_();
        node->clearLinks();
        node = next;
    }
}

/**
 * Forgets the parent, first child and siblings (without updating them).
 */
void TreeNode::clearLinks()
{
    mPrev = nullptr;
    mParent = nullptr;
    mNext = nullptr;
    mChild = nullptr;
}

/**
 * @return the number of direct children
 */
s32 TreeNode::countChildren() const
{
    s32 count = 0;
    TreeNode* node = mChild;

    while (node != nullptr)
    {
        ++count;
        node = node->mNext;
    }

    return count;
}

/**
 * Detaches the node from its tree and breaks up its whole subtree.
 */
void TreeNode::detachAll()
{
    detachSubTree();
    clearChildLinksRecursively_();
    clearLinks();
}

/**
 * Detaches the node, keeping its children, from its parent and siblings.
 * The first child's previous link points to the last child, so that is fixed up too.
 */
void TreeNode::detachSubTree()
{
    if (mParent != nullptr && mParent->mChild == this)
    {
        mParent->mChild = mNext;

        if (mNext != nullptr)
        {
            mNext->mPrev = mPrev;
            mNext = nullptr;
        }
    }
    else
    {
        if (mPrev != nullptr)
        {
            mPrev->mNext = mNext;
        }

        if (mNext != nullptr)
        {
            mNext->mPrev = mPrev;
            mNext = nullptr;
        }
        else if (mParent != nullptr)
        {
            mParent->mChild->mPrev = mPrev;
        }
    }

    mPrev = nullptr;
    mParent = nullptr;
}

/**
 * @return the root of the tree the node is in
 */
TreeNode* TreeNode::findRoot()
{
    if (mParent != nullptr)
    {
        return mParent->findRoot();
    }

    return this;
}

/**
 * @return the root of the tree the node is in
 */
const TreeNode* TreeNode::findRoot() const
{
    if (mParent != nullptr)
    {
        return static_cast<const TreeNode*>(mParent)->findRoot();
    }

    return this;
}

/**
 * Moves a node (with its subtree) right after this one, under the same parent.
 * @param pNode node to insert
 */
void TreeNode::insertAfterSelf(TreeNode* pNode)
{
    pNode->detachSubTree();

    TreeNode* next = mNext;
    mNext = pNode;
    pNode->mPrev = this;
    pNode->mNext = next;

    if (next != nullptr)
    {
        next->mPrev = pNode;
    }
    else if (mParent != nullptr)
    {
        mParent->mChild->mPrev = pNode;
    }

    pNode->mParent = mParent;
}

/**
 * Moves a node (with its subtree) right before this one, under the same parent.
 * @param pNode node to insert
 */
void TreeNode::insertBeforeSelf(TreeNode* pNode)
{
    pNode->detachSubTree();

    TreeNode* prev = mPrev;
    mPrev = pNode;
    pNode->mPrev = prev;
    pNode->mNext = this;

    if (mParent != nullptr && mParent->mChild == this)
    {
        mParent->mChild = pNode;
    }
    else if (prev != nullptr)
    {
        prev->mNext = pNode;
    }

    pNode->mParent = mParent;
}

/**
 * Moves a node (with its subtree) to the end of this node's children.
 * @param pNode node to add
 */
void TreeNode::pushBackChild(TreeNode* pNode)
{
    pNode->detachSubTree();

    if (mChild != nullptr)
    {
        TreeNode* n = mChild->mPrev;
        SEAD_ASSERT(n);
        n->mNext = pNode;
        pNode->mPrev = n;
        pNode->mParent = n->mParent;
        mChild->mPrev = pNode;
    }
    else
    {
        mChild = pNode;
        pNode->mParent = this;
        pNode->mPrev = pNode;
    }
}

/**
 * Moves a node (with its subtree) to the end of this node's siblings.
 * @param pNode node to add
 */
void TreeNode::pushBackSibling(TreeNode* pNode)
{
    pNode->detachSubTree();

    TreeNode* m;

    if (mParent != nullptr && mParent->mChild != nullptr)
    {
        m = mParent->mChild->mPrev;
        mParent->mChild->mPrev = pNode;
    }
    else
    {
        m = this;

        while (m->mNext != nullptr)
        {
            m = m->mNext;
        }
    }

    m->mNext = pNode;
    pNode->mPrev = m;
    pNode->mParent = m->mParent;
}

/**
 * Moves a node (with its subtree) to the front of this node's children.
 * @param pNode node to add
 */
void TreeNode::pushFrontChild(TreeNode* pNode)
{
    pNode->detachSubTree();

    if (mChild != nullptr)
    {
        pNode->mNext = mChild;
        pNode->mPrev = mChild->mPrev;
        mChild->mPrev = pNode;
        mChild = pNode;
        pNode->mParent = this;
    }
    else
    {
        mChild = pNode;
        pNode->mParent = this;
        pNode->mPrev = pNode;
    }
}
}  // namespace sead
