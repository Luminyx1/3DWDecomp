#include <basis/seadNew.h>
#include <xml/seadXmlElement.h>
#include <xml/seadXmlUtil.h>

namespace sead
{
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

    auto* element = new (pHeap, sizeof(void*)) XmlElement();
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

    auto* element = new (pHeap, sizeof(void*)) XmlElement();
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

    auto* element = new (pHeap, sizeof(void*)) XmlElement();
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

    auto* element = new (pHeap, sizeof(void*)) XmlElement();
    pElement->pushBackSibling(element);
    element->setName(rName);

    if (rContent.isEmpty())
    {
        element->setContent(nullptr, 0, false);
    }
    else
    {
        element->setContentString(rContent, pHeap);
    }

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

    auto* element = new (pHeap, sizeof(void*)) XmlElement();
    pElement->pushBackChild(element);
    element->setName(rName);

    if (rContent.isEmpty())
    {
        element->setContent(nullptr, 0, false);
    }
    else
    {
        element->setContentString(rContent, pHeap);
    }

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

    auto* element = new (pHeap, sizeof(void*)) XmlElement();
    pElement->pushFrontChild(element);
    element->setName(rName);

    if (rContent.isEmpty())
    {
        element->setContent(nullptr, 0, false);
    }
    else
    {
        element->setContentString(rContent, pHeap);
    }

    return element;
}

}  // namespace sead
