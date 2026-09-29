#include <basis/seadNew.h>
#include <container/seadObjArray.h>
#include <prim/seadMemUtil.h>
#include <xml/seadXmlElement.h>

namespace sead
{
namespace
{
struct AttributeCopy
{
    FixedSafeString<1024> mName;
    FixedSafeString<1024> mValue;
};
}  // namespace

/**
 * Creates an unnamed element without content or attributes.
 */
XmlElement::XmlElement() : mName("") {}

/**
 * Frees the owned content and the attribute buffer.
 */
XmlElement::~XmlElement()
{
    if (mContent && mIsContentOwner)
    {
        delete[] mContent;
    }
    mAttributes.freeBuffer();
}

/**
 * Replaces the content, freeing the previous content if the element owned it.
 * @param pContent new content (may be nullptr)
 * @param size size of the new content in bytes
 * @param isOwner whether the element frees the content with delete[]
 */
void XmlElement::setContent(u8* pContent, u32 size, bool isOwner)
{
    if (mContent == pContent)
    {
        return;
    }

    if (mContent && mIsContentOwner)
    {
        delete[] mContent;
    }
    mIsContentOwner = isOwner;
    mContent = pContent;
    mContentSize = size;
}

/**
 * Looks up the value of an attribute by name.
 * @param rName attribute name
 * @return the attribute value, or an empty string if there is no such attribute
 */
SafeString XmlElement::findAttributeValue(const SafeString& rName) const
{
    for (auto& attribute : mAttributes)
    {
        if (attribute.mName == rName)
        {
            return attribute.mValue;
        }
    }
    return SafeString::cEmptyString;
}

/**
 * Grows the attribute buffer to hold at least num attributes, keeping existing attributes.
 * @param num minimum attribute capacity
 * @param pHeap heap for the buffer, or nullptr for the element's heap
 * @return the attribute list
 */
XmlElement::AttributeList* XmlElement::expandAttributeList(s32 num, Heap* pHeap)
{
    if (!pHeap)
    {
        pHeap = mHeap;
    }

    if (!mAttributes.isBufferReady())
    {
        mAttributes.allocBuffer(num, pHeap);
        return &mAttributes;
    }

    if (mAttributes.size() >= num)
    {
        return &mAttributes;
    }

    ObjArray<AttributeCopy> copies;
    copies.allocBuffer(mAttributes.size(), pHeap, -static_cast<s32>(sizeof(void*)));
    for (auto& attribute : mAttributes)
    {
        AttributeCopy* copy = copies.emplaceBack();
        copy->mName = attribute.mName;
        copy->mValue = attribute.mValue;
    }

    mAttributes.freeBuffer();
    mAttributes.allocBuffer(num, pHeap);
    for (auto& copy : copies)
    {
        mAttributes.emplaceBack(pHeap, copy);
    }
    copies.freeBuffer();
    return &mAttributes;
}

/**
 * Appends an attribute if the name is non-empty, unused and the buffer has room.
 * @param rName attribute name
 * @param rValue attribute value
 * @param pHeap heap for the copied name and value strings
 * @return whether the attribute was added
 */
bool XmlElement::addAttribute(const SafeString& rName, const SafeString& rValue, Heap* pHeap)
{
    if (rName.isEmpty())
    {
        return false;
    }

    if (!mAttributes.isBufferReady() || mAttributes.isFull())
    {
        return false;
    }

    for (auto& attribute : mAttributes)
    {
        if (rName == attribute.mName)
        {
            return false;
        }
    }

    mAttributes.emplaceBack(pHeap, rName, rValue);
    return true;
}

/**
 * Replaces the value of an existing attribute, reallocating it if the new value does not fit.
 * @param rName attribute name
 * @param rValue new attribute value
 * @param pHeap heap for a reallocated attribute
 * @return whether an attribute with that name exists
 */
bool XmlElement::updateAttribute(const SafeString& rName, const SafeString& rValue, Heap* pHeap)
{
    for (auto& attribute : mAttributes)
    {
        if (attribute.mName == rName)
        {
            s32 length = rValue.calcLength();
            if (attribute.mValue.calcLength() > length)
            {
                attribute.mValue.copy(rValue, length);
            }
            else
            {
                mAttributes.erase(&attribute);
                addAttribute(rName, rValue, pHeap);
            }
            return true;
        }
    }
    return false;
}

/**
 * Removes an attribute by name.
 * @param rName attribute name
 * @return whether an attribute was removed
 */
bool XmlElement::eraseAttribute(const SafeString& rName)
{
    for (auto& attribute : mAttributes)
    {
        if (attribute.mName == rName)
        {
            mAttributes.erase(&attribute);
            return true;
        }
    }
    return false;
}

/**
 * Resolves an absolute ("/a/b") or relative ("a/b", "./a", "../a") element path.
 * @param rPath element path
 * @return the element, or nullptr if the path does not resolve
 */
const XmlElement* XmlElement::findElementImpl_(const SafeString& rPath) const
{
    if (rPath.comparen("/", 1) == 0)
    {
        return findElementByAbsolutePath_(rPath);
    }

    if (rPath.comparen("./", 1) == 0)
    {
        return findElementByRelativePath_(rPath.getPart(2));
    }

    return findElementByRelativePath_(rPath);
}

/**
 * Resolves a path starting with '/' from the children of the tree root.
 * @param rPath absolute element path
 * @return the element, or nullptr if the path does not resolve
 */
const XmlElement* XmlElement::findElementByAbsolutePath_(const SafeString& rPath) const
{
    const XmlElement* element = findRoot();
    FixedSafeString<256> path(rPath);
    FixedSafeString<256> name;
    do
    {
        name = path.getPart(1);
        if (name.isEmpty())
        {
            return element;
        }

        element = element->child();
        if (!findSiblingElement_(&element, name))
        {
            return nullptr;
        }

        path = name.getPart(element->mName.calcLength());
    } while (!path.isEmpty());

    return element;
}

/**
 * Resolves a path relative to this element, where ".." selects the parent.
 * @param rPath relative element path
 * @return the element, or nullptr if the path does not resolve
 */
const XmlElement* XmlElement::findElementByRelativePath_(const SafeString& rPath) const
{
    FixedSafeString<256> name;
    FixedSafeString<256> path(rPath);
    const XmlElement* element = this;
    while (true)
    {
        if (path.comparen("..", 2) == 0)
        {
            element = element->parent();
            if (!element)
            {
                return nullptr;
            }
            name = path.getPart(2);
            path = name.getPart(1);
        }
        else
        {
            if (path.isEmpty())
            {
                return element;
            }

            element = element->child();
            if (!findSiblingElement_(&element, path))
            {
                return nullptr;
            }

            name = path.getPart(element->mName.calcLength());
            if (name.isEmpty())
            {
                return element;
            }
            path = name.getPart(1);
        }
    }
}

/**
 * Resolves an element path.
 * @param rPath absolute or relative element path
 * @return the element, or nullptr if the path does not resolve
 */
XmlElement* XmlElement::findElement(const SafeString& rPath)
{
    return const_cast<XmlElement*>(findElementImpl_(rPath));
}

/**
 * Resolves an element path.
 * @param rPath absolute or relative element path
 * @return the element, or nullptr if the path does not resolve
 */
const XmlElement* XmlElement::findElement(const SafeString& rPath) const
{
    return findElementImpl_(rPath);
}

/**
 * Gets the content as a string.
 * @return the content, or an empty string if there is none
 */
SafeString XmlElement::getContentString() const
{
    return mContent ? SafeString(reinterpret_cast<const char*>(mContent)) :
                      SafeString::cEmptyString;
}

/**
 * Walks up the parents to the root of the tree.
 * @return the root element
 */
const XmlElement* XmlElement::findRoot() const
{
    const XmlElement* element = this;
    while (element->parent())
    {
        element = element->parent();
    }
    return element;
}

/**
 * Walks up the parents to the root of the tree.
 * @return the root element
 */
XmlElement* XmlElement::findRoot()
{
    XmlElement* element = this;
    while (element->parent())
    {
        element = element->parent();
    }
    return element;
}

/**
 * Finds, starting at *ppElement, the first sibling whose name is the first component of name.
 * @param ppElement first candidate on input, the found element on success
 * @param name path whose first component (up to '/' or the end) is matched
 * @return whether a sibling was found
 */
bool XmlElement::findSiblingElement_(const XmlElement** ppElement, SafeString name) const
{
    for (const XmlElement* element = *ppElement; element; element = element->next())
    {
        s32 length = element->mName.calcLength();
        if (element->mName.comparen(name, length) == 0)
        {
            if (name.at(length) == '/')
            {
                *ppElement = element;
                return true;
            }
            if (name.at(length) == '\0')
            {
                *ppElement = element;
                return true;
            }
        }
    }
    return false;
}

/**
 * Sets the content to an owned, null-terminated copy of a string.
 * @param rContent content string
 * @param pHeap heap for the copy
 */
void XmlElement::setContentString(const SafeString& rContent, Heap* pHeap)
{
    s32 length = rContent.calcLength();
    u8* content = new (pHeap, sizeof(void*)) u8[length + 1];
    MemUtil::copy(content, rContent.cstr(), length);
    content[length] = '\0';
    setContent(content, length, true);
}

}  // namespace sead
