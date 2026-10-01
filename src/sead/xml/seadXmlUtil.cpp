#include <basis/seadNew.h>
#include <xml/seadXmlElement.h>
#include <xml/seadXmlUtil.h>

namespace sead
{
namespace
{
inline XmlElement* createElement(Heap* pHeap)
{
    return new (pHeap, sizeof(void*)) XmlElement();
}

inline void setupElement(XmlElement* pElement, const SafeString& rName, const SafeString& rContent,
                         Heap* pHeap)
{
    pElement->setName(rName);

    if (rContent.isEmpty())
    {
        pElement->setContent(nullptr, 0, false);
    }
    else
    {
        pElement->setContentString(rContent, pHeap);
    }
}
}  // namespace

/**
 * Creates an empty element and links it as the last sibling of an element.
 * @param pElement element whose sibling list receives the new element
 * @param pHeap heap for the new element, or nullptr for the element's heap
 * @return the new element
 */
XmlElement* XmlUtil::createBackSiblingElement(XmlElement* pElement, Heap* pHeap)
{
    if (pHeap == nullptr)
    {
        pHeap = pElement->getHeap();
    }

    XmlElement* element = createElement(pHeap);
    pElement->pushBackSibling(element);
    return element;
}

/**
 * Creates an empty element and links it as the last child of an element.
 * @param pElement parent of the new element
 * @param pHeap heap for the new element, or nullptr for the parent's heap
 * @return the new element
 */
XmlElement* XmlUtil::createBackChildElement(XmlElement* pElement, Heap* pHeap)
{
    if (pHeap == nullptr)
    {
        pHeap = pElement->getHeap();
    }

    XmlElement* element = createElement(pHeap);
    pElement->pushBackChild(element);
    return element;
}

/**
 * Creates an empty element and links it as the first child of an element.
 * @param pElement parent of the new element
 * @param pHeap heap for the new element, or nullptr for the parent's heap
 * @return the new element
 */
XmlElement* XmlUtil::createFrontChildElement(XmlElement* pElement, Heap* pHeap)
{
    if (pHeap == nullptr)
    {
        pHeap = pElement->getHeap();
    }

    XmlElement* element = createElement(pHeap);
    pElement->pushFrontChild(element);
    return element;
}

/**
 * Creates a named element with text content as the last sibling of an element.
 * @param pElement element whose sibling list receives the new element
 * @param rName name of the new element
 * @param rContent text content of the new element, may be empty
 * @param pHeap heap for the new element and its content, or nullptr for the element's heap
 * @return the new element
 */
XmlElement* XmlUtil::createBackSiblingAndSetupElement(XmlElement* pElement, const SafeString& rName,
                                                      const SafeString& rContent, Heap* pHeap)
{
    if (pHeap == nullptr)
    {
        pHeap = pElement->getHeap();
    }

    XmlElement* element = createElement(pHeap);
    pElement->pushBackSibling(element);
    setupElement(element, rName, rContent, pHeap);

    return element;
}

/**
 * Creates a named element with text content as the last child of an element.
 * @param pElement parent of the new element
 * @param rName name of the new element
 * @param rContent text content of the new element, may be empty
 * @param pHeap heap for the new element and its content, or nullptr for the parent's heap
 * @return the new element
 */
XmlElement* XmlUtil::createBackChildAndSetupElement(XmlElement* pElement, const SafeString& rName,
                                                    const SafeString& rContent, Heap* pHeap)
{
    if (pHeap == nullptr)
    {
        pHeap = pElement->getHeap();
    }

    XmlElement* element = createElement(pHeap);
    pElement->pushBackChild(element);
    setupElement(element, rName, rContent, pHeap);

    return element;
}

/**
 * Creates a named element with text content as the first child of an element.
 * @param pElement parent of the new element
 * @param rName name of the new element
 * @param rContent text content of the new element, may be empty
 * @param pHeap heap for the new element and its content, or nullptr for the parent's heap
 * @return the new element
 */
XmlElement* XmlUtil::createFrontChildAndSetupElement(XmlElement* pElement, const SafeString& rName,
                                                     const SafeString& rContent, Heap* pHeap)
{
    if (pHeap == nullptr)
    {
        pHeap = pElement->getHeap();
    }

    XmlElement* element = createElement(pHeap);
    pElement->pushFrontChild(element);
    setupElement(element, rName, rContent, pHeap);

    return element;
}

}  // namespace sead
