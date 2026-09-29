#pragma once

#include <basis/seadTypes.h>
#include <container/seadObjList.h>
#include <container/seadTreeNode.h>
#include <prim/seadSafeString.h>

namespace sead
{
class Heap;

class XmlElement : public TreeNode
{
public:
    enum ElementType
    {
        cElementType_Text = 0,
        cElementType_CData = 1,
        cElementType_Base64 = 2,
    };

    struct Attribute
    {
        Attribute(Heap* pHeap, const SafeString& rName, const SafeString& rValue)
            : mName(pHeap, rName), mValue(pHeap, rValue)
        {
        }

        template <typename T>
        Attribute(Heap* pHeap, const T& rOther)
            : mName(pHeap, rOther.mName), mValue(pHeap, rOther.mValue)
        {
        }

        HeapSafeString mName;
        HeapSafeString mValue;
    };

    using AttributeList = ObjList<Attribute>;

    XmlElement();
    virtual ~XmlElement();

    void setContent(u8* pContent, u32 size, bool isOwner);
    void setContentString(const SafeString& rContent, Heap* pHeap);
    SafeString getContentString() const;
    u8* getContent() const { return mContent; }
    u32 getContentSize() const { return mContentSize; }

    ElementType getElementType() const { return mElementType; }
    void setElementType(ElementType type) { mElementType = type; }

    const FixedSafeString<64>& getName() const { return mName; }
    FixedSafeString<64>& getName() { return mName; }
    void setName(const SafeString& rName) { mName = rName; }

    SafeString findAttributeValue(const SafeString& rName) const;
    AttributeList* expandAttributeList(s32 num, Heap* pHeap);
    bool addAttribute(const SafeString& rName, const SafeString& rValue, Heap* pHeap);
    bool updateAttribute(const SafeString& rName, const SafeString& rValue, Heap* pHeap);
    bool eraseAttribute(const SafeString& rName);
    const AttributeList& getAttributes() const { return mAttributes; }

    XmlElement* findElement(const SafeString& rPath);
    const XmlElement* findElement(const SafeString& rPath) const;
    XmlElement* findRoot();
    const XmlElement* findRoot() const;

    XmlElement* parent() const { return static_cast<XmlElement*>(mParent); }
    XmlElement* child() const { return static_cast<XmlElement*>(mChild); }
    XmlElement* next() const { return static_cast<XmlElement*>(mNext); }
    XmlElement* prev() const { return static_cast<XmlElement*>(mPrev); }

    Heap* getHeap() const { return mHeap; }
    void setHeap(Heap* pHeap) { mHeap = pHeap; }

private:
    friend class XmlDocument;

    const XmlElement* findElementImpl_(const SafeString& rPath) const;
    const XmlElement* findElementByAbsolutePath_(const SafeString& rPath) const;
    const XmlElement* findElementByRelativePath_(const SafeString& rPath) const;
    bool findSiblingElement_(const XmlElement** ppElement, SafeString name) const;

    FixedSafeString<64> mName;
    u8* mContent = nullptr;
    u32 mContentSize = 0;
    bool mIsContentOwner = false;
    ElementType mElementType = cElementType_Text;
    AttributeList mAttributes;
    Heap* mHeap = nullptr;
};
static_assert(sizeof(XmlElement) == 0xd0);

}  // namespace sead
