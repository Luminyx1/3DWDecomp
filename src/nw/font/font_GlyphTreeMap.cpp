#include <nn/font/font_GlyphTreeMap.h>

#include <new>

namespace nn {
namespace font {

/**
 * Calculates the line kind that fits a glyph of the given height.
 * @param height glyph height in pixels
 * @return line kind
 */
u8 GlyphNode::CalculateLineKind(u16 height) {
    if (height > 0x400) {
        return 0;
    }

    if (height > 0x200) {
        return 10;
    }

    if (height > 0x100) {
        return 9;
    }

    if (height > 0x80) {
        return 8;
    }

    if (height > 0x40) {
        return 7;
    }

    if (height > 0x20) {
        return 6;
    }

    if (height > 0x10) {
        return 5;
    }

    return 4;
}

/**
 * Calculates the height of a line of the given kind.
 * @param kind line kind
 * @return line height in pixels
 */
u32 GlyphNode::CalculateLineHeight(u8 kind) {
    return 1 << kind;
}

/**
 * Constructs an empty glyph tree map.
 */
GlyphTreeMap::GlyphTreeMap()
    : m_pRootNode(nullptr), m_pFreeList(nullptr), m_pNodeBuffer(nullptr), m_NodeCountMax(0) {}

/**
 * Allocates the node buffer and builds the free node list.
 * @param pAllocateFunction allocator for the node buffer
 * @param pUserData user data passed to the allocator
 * @param nodeCountMax maximum number of nodes
 */
void GlyphTreeMap::Initialize(AllocateFunction pAllocateFunction, void* pUserData,
                              u32 nodeCountMax) {
    m_pRootNode = nullptr;
    m_NodeCountMax = nodeCountMax;
    GlyphNode* pNodes = static_cast<GlyphNode*>(
        pAllocateFunction(sizeof(GlyphNode) * nodeCountMax, 8, pUserData));
    m_pFreeList = pNodes;
    int lastIndex = static_cast<int>(nodeCountMax) - 1;

    for (int i = 0; i < lastIndex; i++) {
        NextFreeNode(&pNodes[i]) = &pNodes[i + 1];
    }

    reinterpret_cast<GlyphNode**>(pNodes)[lastIndex * NodePointerStride] = nullptr;
    m_pNodeBuffer = pNodes;
}

/**
 * Frees the node buffer.
 * @param pFreeFunction deallocator for the node buffer
 * @param pUserData user data passed to the deallocator
 */
void GlyphTreeMap::Finalize(FreeFunction pFreeFunction, void* pUserData) {
    pFreeFunction(m_pNodeBuffer, pUserData);
    m_pNodeBuffer = nullptr;
    m_pFreeList = nullptr;
}

/**
 * Finds the node of a glyph.
 * @param code character code
 * @param fontSize font size
 * @param fontFace font face
 * @return the node, or nullptr if not registered
 */
GlyphNode* GlyphTreeMap::Find(u32 code, u16 fontSize, u16 fontFace) const {
    GlyphNode::KeyType key;
    key.detail.code = code;
    key.detail.fontSize = fontSize;
    key.detail.fontFace = fontFace;
    return Find(m_pRootNode, key.raw);
}

/**
 * Finds the node with the given key in a subtree.
 * @param pNode subtree root
 * @param key node key
 * @return the node, or nullptr if not found
 */
GlyphNode* GlyphTreeMap::Find(GlyphNode* pNode, u64 key) const {
    while (pNode != nullptr) {
        if (key < pNode->m_Key.raw) {
            pNode = pNode->m_pLeftNode;
        } else if (key > pNode->m_Key.raw) {
            pNode = pNode->m_pRightNode;
        } else {
            return pNode;
        }
    }

    return nullptr;
}

/**
 * Takes a node from the free list and inserts it for a glyph.
 * @param code character code
 * @param fontSize font size
 * @param fontFace font face
 * @return the inserted node, or nullptr if no free node is left
 */
GlyphNode* GlyphTreeMap::Insert(u32 code, u16 fontSize, u16 fontFace) {
    GlyphNode* pNode = m_pFreeList;

    if (pNode == nullptr) {
        return nullptr;
    }

    m_pFreeList = NextFreeNode(pNode);
    new (pNode) GlyphNode(code, fontSize, fontFace);
    m_pRootNode = Insert(m_pRootNode, pNode);
    m_pRootNode->m_IsRed = 0;
    return pNode;
}

/**
 * Inserts a node into a subtree, keeping it a left-leaning red-black tree.
 * @param pNode subtree root
 * @param pNewNode node to insert
 * @return new subtree root
 */
GlyphNode* GlyphTreeMap::Insert(GlyphNode* pNode, GlyphNode* pNewNode) {
    if (pNode == nullptr) {
        pNewNode->m_IsRed = 1;
        pNewNode->m_pLeftNode = nullptr;
        pNewNode->m_pRightNode = nullptr;
        return pNewNode;
    }

    if (pNewNode->m_Key.raw < pNode->m_Key.raw) {
        pNode->m_pLeftNode = Insert(pNode->m_pLeftNode, pNewNode);
    } else if (pNewNode->m_Key.raw > pNode->m_Key.raw) {
        pNode->m_pRightNode = Insert(pNode->m_pRightNode, pNewNode);
    } else if (pNode != pNewNode) {
        pNewNode->m_pRightNode = pNode->m_pRightNode;
        pNewNode->m_pLeftNode = pNode->m_pLeftNode;
        pNewNode->m_IsRed = pNode->m_IsRed;
        FreeNode(pNode);
        pNode = pNewNode;
    }

    if (IsRed(pNode->m_pRightNode) && !IsRed(pNode->m_pLeftNode)) {
        pNode = RotateLeft(pNode);
    }

    if (IsRed(pNode->m_pLeftNode) && IsRed(pNode->m_pLeftNode->m_pLeftNode)) {
        pNode = RotateRight(pNode);
    }

    if (IsRed(pNode->m_pLeftNode) && IsRed(pNode->m_pRightNode)) {
        FlipColors(pNode);
    }

    return pNode;
}

/**
 * Erases the node of a glyph.
 * @param code character code
 * @param fontSize font size
 * @param fontFace font face
 */
void GlyphTreeMap::Erase(u32 code, u16 fontSize, u16 fontFace) {
    GlyphNode::KeyType key;
    key.detail.code = code;
    key.detail.fontSize = fontSize;
    key.detail.fontFace = fontFace;
    m_pRootNode = Erase(m_pRootNode, key.raw);

    if (m_pRootNode != nullptr) {
        m_pRootNode->m_IsRed = 0;
    }
}

/**
 * Erases the node with the given key from a subtree and returns it to the free list.
 * @param pNode subtree root
 * @param key node key
 * @return new subtree root
 */
GlyphNode* GlyphTreeMap::Erase(GlyphNode* pNode, u64 key) {
    if (key < pNode->m_Key.raw) {
        if (!IsRed(pNode->m_pLeftNode) && !IsRed(pNode->m_pLeftNode->m_pLeftNode)) {
            pNode = MoveRedLeft(pNode);
        }

        pNode->m_pLeftNode = Erase(pNode->m_pLeftNode, key);
    } else {
        if (IsRed(pNode->m_pLeftNode)) {
            pNode = RotateRight(pNode);
        }

        if (key == pNode->m_Key.raw && pNode->m_pRightNode == nullptr) {
            FreeNode(pNode);
            return nullptr;
        }

        if (pNode->m_pRightNode->m_IsRed != 1 && !IsRed(pNode->m_pRightNode->m_pLeftNode)) {
            pNode = MoveRedRight(pNode);
        }

        if (key == pNode->m_Key.raw) {
            GlyphNode* pMinNode = pNode->m_pRightNode;

            while (pMinNode->m_pLeftNode != nullptr) {
                pMinNode = pMinNode->m_pLeftNode;
            }

            GlyphNode* pSuccessor = Find(pNode->m_pRightNode, pMinNode->m_Key.raw);
            pSuccessor->m_pRightNode = EraseMin(pNode->m_pRightNode);
            pSuccessor->m_pLeftNode = pNode->m_pLeftNode;
            pSuccessor->m_IsRed = pNode->m_IsRed;
            FreeNode(pNode);
            pNode = pSuccessor;
        } else {
            pNode->m_pRightNode = Erase(pNode->m_pRightNode, key);
        }
    }

    return FixUp(pNode);
}

/**
 * Dumps the tree; does nothing in release builds.
 */
void GlyphTreeMap::Dump() const {}

/**
 * Dumps the tree in order; does nothing in release builds.
 */
void GlyphTreeMap::DumpLeftFirst() const {}

/**
 * Passes every node to a function; does nothing in release builds.
 * @param pFunction function called per node
 * @param pUserData user data passed to the function
 */
void GlyphTreeMap::DumpToFunction(DumpFunction pFunction, void* pUserData) const {}

/**
 * Passes every node to a function in order; does nothing in release builds.
 * @param pFunction function called per node
 * @param pUserData user data passed to the function
 */
void GlyphTreeMap::DumpToFunctionLeftFirst(DumpFunction pFunction, void* pUserData) const {}

/**
 * Removes every node and rebuilds the free node list.
 */
void GlyphTreeMap::Reset() {
    m_pRootNode = nullptr;
    GlyphNode* pNodes = static_cast<GlyphNode*>(m_pNodeBuffer);
    m_pFreeList = pNodes;
    int lastIndex = static_cast<int>(m_NodeCountMax) - 1;

    for (int i = 0; i < lastIndex; i++) {
        NextFreeNode(&pNodes[i]) = &pNodes[i + 1];
    }

    reinterpret_cast<GlyphNode**>(pNodes)[lastIndex * NodePointerStride] = nullptr;
    m_pNodeBuffer = pNodes;
}

/**
 * Moves the per-frame usage flags of a subtree into the last-frame flag.
 * @param pNode subtree root
 */
void GlyphTreeMap::UpdateFlagsForCompleteTextureCacheRecursive(GlyphNode* pNode) {
    bool isUsed = pNode->IsFlagOn(GlyphNode::FlagBit_Requested | GlyphNode::FlagBit_Used);
    pNode->m_Flag = (pNode->m_Flag & (GlyphNode::FlagBit_System | GlyphNode::FlagBit_NotInFont)) |
                    (isUsed ? GlyphNode::FlagBit_UsedInLastFrame : 0);
    if (pNode->m_pLeftNode != nullptr) {
        UpdateFlagsForCompleteTextureCacheRecursive(pNode->m_pLeftNode);
    }

    if (pNode->m_pRightNode != nullptr) {
        UpdateFlagsForCompleteTextureCacheRecursive(pNode->m_pRightNode);
    }
}

/**
 * Clears lock group bits in a subtree.
 * @param pNode subtree root
 * @param group lock group bits to clear
 */
void GlyphTreeMap::ClearLockGroupRecursive(GlyphNode* pNode, u32 group) {
    pNode->m_LockGroup &= ~group;

    if (pNode->m_pLeftNode != nullptr) {
        ClearLockGroupRecursive(pNode->m_pLeftNode, group);
    }

    if (pNode->m_pRightNode != nullptr) {
        ClearLockGroupRecursive(pNode->m_pRightNode, group);
    }
}

/**
 * Counts the nodes of a subtree.
 * @param pNode subtree root
 * @param level depth of the subtree root
 * @return number of nodes
 */
int GlyphTreeMap::DumpRecursive(GlyphNode* pNode, u32 level) {
    level++;
    int count = 1;

    if (pNode->m_pLeftNode != nullptr) {
        count += DumpRecursive(pNode->m_pLeftNode, level);
    }

    if (pNode->m_pRightNode != nullptr) {
        count += DumpRecursive(pNode->m_pRightNode, level);
    }

    return count;
}

/**
 * Counts the nodes of a subtree in order.
 * @param pNode subtree root
 * @param level depth of the subtree root
 * @return number of nodes
 */
int GlyphTreeMap::DumpRecursiveLeftFirst(GlyphNode* pNode, u32 level) {
    level++;
    int count = 1;

    if (pNode->m_pLeftNode != nullptr) {
        count += DumpRecursiveLeftFirst(pNode->m_pLeftNode, level);
    }

    if (pNode->m_pRightNode != nullptr) {
        count += DumpRecursiveLeftFirst(pNode->m_pRightNode, level);
    }

    return count;
}

/**
 * Passes every node of a subtree to a function, parents first.
 * @param pNode subtree root
 * @param pFunction function called per node
 * @param pUserData user data passed to the function
 */
void GlyphTreeMap::DumpToFunctionRecursive(GlyphNode* pNode, DumpFunction pFunction,
                                           void* pUserData) {
    pFunction(pNode, pUserData);

    if (pNode->m_pLeftNode != nullptr) {
        DumpToFunctionRecursive(pNode->m_pLeftNode, pFunction, pUserData);
    }

    if (pNode->m_pRightNode != nullptr) {
        DumpToFunctionRecursive(pNode->m_pRightNode, pFunction, pUserData);
    }
}

/**
 * Passes every node of a subtree to a function in key order.
 * @param pNode subtree root
 * @param pFunction function called per node
 * @param pUserData user data passed to the function
 */
void GlyphTreeMap::DumpToFunctionRecursiveLeftFirst(GlyphNode* pNode, DumpFunction pFunction,
                                                    void* pUserData) {
    if (pNode->m_pLeftNode != nullptr) {
        DumpToFunctionRecursiveLeftFirst(pNode->m_pLeftNode, pFunction, pUserData);
    }

    pFunction(pNode, pUserData);

    if (pNode->m_pRightNode != nullptr) {
        DumpToFunctionRecursiveLeftFirst(pNode->m_pRightNode, pFunction, pUserData);
    }
}

}  // namespace font
}  // namespace nn
