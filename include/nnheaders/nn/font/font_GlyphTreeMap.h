#pragma once

#include <nn/types.h>
#include <nn/util/util_IntrusiveList.h>

namespace nn {
namespace font {

class GlyphNode {
public:
    enum FlagBit {
        FlagBit_Requested = 1 << 0,
        FlagBit_NotPlotted = 1 << 1,
        FlagBit_Used = 1 << 2,
        FlagBit_Erase = 1 << 3,
        FlagBit_UsedInLastFrame = 1 << 4,
        FlagBit_System = 1 << 5,
        FlagBit_NotInFont = 1 << 6,
    };

    union KeyType {
        u64 raw;
        struct {
            u32 code;
            u16 fontFace;
            u16 fontSize;
        } detail;
    };

    GlyphNode(u32 code, u16 fontSize, u16 fontFace)
        : m_pLeftNode(nullptr), m_pRightNode(nullptr), m_Flag(0), m_IsRed(0), m_LineKind(0),
          m_LineNo(0), m_LockGroup(0), m_CachePosX(0), m_CachePosY(0), m_CacheWidth(0),
          m_CacheHeight(0), m_GlyphWidth(0), m_GlyphHeight(0), m_AdvanceX(0), m_LeftOffset(0),
          m_BaselineOffset(0) {
        m_Key.detail.code = code;
        m_Key.detail.fontSize = fontSize;
        m_Key.detail.fontFace = fontFace;
        m_IsRed = 1;
    }

    static u8 CalculateLineKind(u16 height);
    static u32 CalculateLineHeight(u8 kind);

    u32 GetCode() const { return m_Key.detail.code; }
    u16 GetFontSize() const { return m_Key.detail.fontSize; }
    u16 GetFontFace() const { return m_Key.detail.fontFace; }

    bool IsFlagOn(u8 mask) const { return (m_Flag & mask) != 0; }
    void SetFlag(u8 mask) { m_Flag |= mask; }
    void ResetFlag(u8 mask) { m_Flag &= ~mask; }

    util::IntrusiveListNode m_Link;
    util::IntrusiveListNode m_LineLink;
    GlyphNode* m_pLeftNode;
    GlyphNode* m_pRightNode;
    KeyType m_Key;
    u8 m_Flag;
    u8 m_IsRed;
    u8 m_LineKind;
    u8 m_LineNo;
    u32 m_LockGroup;
    u16 m_CachePosX;
    u16 m_CachePosY;
    u16 m_CacheWidth;
    u16 m_CacheHeight;
    u16 m_GlyphWidth;
    u16 m_GlyphHeight;
    u16 m_AdvanceX;
    s16 m_LeftOffset;
    u16 m_BaselineOffset;
};
static_assert(sizeof(GlyphNode) == 0x58);

class GlyphTreeMap {
public:
    typedef void* (*AllocateFunction)(size_t size, size_t alignment, void* pUserData);
    typedef void (*FreeFunction)(void* ptr, void* pUserData);
    typedef void (*DumpFunction)(const GlyphNode* pNode, void* pUserData);

    GlyphTreeMap();

    void Initialize(AllocateFunction pAllocateFunction, void* pUserData, u32 nodeCountMax);
    void Finalize(FreeFunction pFreeFunction, void* pUserData);

    GlyphNode* Find(u32 code, u16 fontSize, u16 fontFace) const;
    GlyphNode* Insert(u32 code, u16 fontSize, u16 fontFace);
    void Erase(u32 code, u16 fontSize, u16 fontFace);

    void UpdateFlagsForCompleteTextureCache() {
        if (m_pRootNode != nullptr) {
            UpdateFlagsForCompleteTextureCacheRecursive(m_pRootNode);
        }
    }

    void ClearLockGroup(u32 group) {
        if (m_pRootNode != nullptr) {
            ClearLockGroupRecursive(m_pRootNode, group);
        }
    }

    void Dump() const;
    void DumpLeftFirst() const;
    void DumpToFunction(DumpFunction pFunction, void* pUserData) const;
    void DumpToFunctionLeftFirst(DumpFunction pFunction, void* pUserData) const;
    void Reset();

private:
    static const int NodePointerStride = sizeof(GlyphNode) / sizeof(GlyphNode*);

    static bool IsRed(const GlyphNode* pNode) { return pNode != nullptr && pNode->m_IsRed == 1; }

    static GlyphNode* RotateLeft(GlyphNode* pNode) {
        GlyphNode* pRight = pNode->m_pRightNode;
        pNode->m_pRightNode = pRight->m_pLeftNode;
        pRight->m_pLeftNode = pNode;
        pRight->m_IsRed = pNode->m_IsRed;
        pNode->m_IsRed = 1;
        return pRight;
    }

    static GlyphNode* RotateRight(GlyphNode* pNode) {
        GlyphNode* pLeft = pNode->m_pLeftNode;
        pNode->m_pLeftNode = pLeft->m_pRightNode;
        pLeft->m_pRightNode = pNode;
        pLeft->m_IsRed = pNode->m_IsRed;
        pNode->m_IsRed = 1;
        return pLeft;
    }

    static void FlipColors(GlyphNode* pNode) {
        pNode->m_IsRed = !pNode->m_IsRed;
        pNode->m_pLeftNode->m_IsRed = !pNode->m_pLeftNode->m_IsRed;
        pNode->m_pRightNode->m_IsRed = !pNode->m_pRightNode->m_IsRed;
    }

    static GlyphNode* FixUp(GlyphNode* pNode) {
        if (IsRed(pNode->m_pRightNode)) {
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

    static GlyphNode* MoveRedLeft(GlyphNode* pNode) {
        FlipColors(pNode);
        if (IsRed(pNode->m_pRightNode->m_pLeftNode)) {
            pNode->m_pRightNode = RotateRight(pNode->m_pRightNode);
            pNode = RotateLeft(pNode);
            FlipColors(pNode);
        }
        return pNode;
    }

    static GlyphNode* MoveRedRight(GlyphNode* pNode) {
        FlipColors(pNode);
        if (IsRed(pNode->m_pLeftNode->m_pLeftNode)) {
            pNode = RotateRight(pNode);
            FlipColors(pNode);
        }
        return pNode;
    }

    static GlyphNode*& NextFreeNode(GlyphNode* pNode) {
        return *reinterpret_cast<GlyphNode**>(pNode);
    }

    void FreeNode(GlyphNode* pNode) {
        NextFreeNode(pNode) = m_pFreeList;
        m_pFreeList = pNode;
    }

    GlyphNode* Find(GlyphNode* pNode, u64 key) const;
    GlyphNode* Insert(GlyphNode* pNode, GlyphNode* pNewNode);
    GlyphNode* Erase(GlyphNode* pNode, u64 key);

    /**
     * Erases the node with the smallest key from a subtree.
     * @param pNode subtree root
     * @return new subtree root
     */
    static GlyphNode* EraseMin(GlyphNode* pNode) {
        if (pNode->m_pLeftNode == nullptr) {
            return nullptr;
        }
        if (!IsRed(pNode->m_pLeftNode) && !IsRed(pNode->m_pLeftNode->m_pLeftNode)) {
            pNode = MoveRedLeft(pNode);
        }
        pNode->m_pLeftNode = EraseMin(pNode->m_pLeftNode);
        return FixUp(pNode);
    }

    static void UpdateFlagsForCompleteTextureCacheRecursive(GlyphNode* pNode);
    static void ClearLockGroupRecursive(GlyphNode* pNode, u32 group);
    static int DumpRecursive(GlyphNode* pNode, u32 level);
    static int DumpRecursiveLeftFirst(GlyphNode* pNode, u32 level);
    static void DumpToFunctionRecursive(GlyphNode* pNode, DumpFunction pFunction, void* pUserData);
    static void DumpToFunctionRecursiveLeftFirst(GlyphNode* pNode, DumpFunction pFunction,
                                                 void* pUserData);

    GlyphNode* m_pRootNode;
    GlyphNode* m_pFreeList;
    void* m_pNodeBuffer;
    u32 m_NodeCountMax;
};
static_assert(sizeof(GlyphTreeMap) == 0x20);

}  // namespace font
}  // namespace nn
