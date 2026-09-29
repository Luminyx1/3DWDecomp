#pragma once

#include <prim/seadSafeString.h>

namespace sead
{
class Heap;
class XmlElement;

class XmlUtil
{
public:
    static XmlElement* createBackSiblingElement(XmlElement* pElement, Heap* pHeap);
    static XmlElement* createBackChildElement(XmlElement* pElement, Heap* pHeap);
    static XmlElement* createFrontChildElement(XmlElement* pElement, Heap* pHeap);
    static XmlElement* createBackSiblingAndSetupElement(XmlElement* pElement,
                                                        const SafeString& rName,
                                                        const SafeString& rContent, Heap* pHeap);
    static XmlElement* createBackChildAndSetupElement(XmlElement* pElement,
                                                      const SafeString& rName,
                                                      const SafeString& rContent, Heap* pHeap);
    static XmlElement* createFrontChildAndSetupElement(XmlElement* pElement,
                                                       const SafeString& rName,
                                                       const SafeString& rContent, Heap* pHeap);
};

}  // namespace sead
