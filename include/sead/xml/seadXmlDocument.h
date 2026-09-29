#pragma once

#include <basis/seadTypes.h>
#include <container/seadObjList.h>
#include <prim/seadSafeString.h>
#include <xml/seadXmlElement.h>

namespace sead
{
class Heap;
class ReadStream;
class WriteStream;

class XmlDocument
{
public:
    struct Entity
    {
        FixedSafeString<8> mName;
        FixedSafeString<128> mValue;
    };

    using EntityList = ObjList<Entity>;

    XmlDocument();
    virtual ~XmlDocument();

    static XmlDocument* create(ReadStream* pStream, Heap* pHeap, bool isBinary, u32 workSize);
    static void setDefaultEntity(const char** pNames, const char** pValues, u32 num);

    void freeXmlElementAll();
    bool addEntity(const SafeString& rName, const SafeString& rValue);
    bool eraseEntity(const SafeString& rName);
    EntityList* expandEntityList(s32 num, Heap* pHeap);
    bool save(WriteStream* pStream, Heap* pHeap, bool isBinary, XmlElement* pElement) const;
    XmlElement* findElement(const SafeString& rPath);

    XmlElement* getRoot() const { return mRoot; }
    const EntityList& getEntities() const { return mEntityList; }

private:
    void resetEntity_(Heap* pHeap);
    void parseXml_(ReadStream* pStream, Heap* pHeap, bool isBinary);
    XmlElement* makeXmlInstance_(ReadStream* pStream, Heap* pHeap);
    s32 parseXmlDeclare_(ReadStream* pStream, Heap* pHeap);
    XmlElement* parseXmlInstance_(ReadStream* pStream, Heap* pHeap);
    bool replaceXmlNumericCharacterReference_(char* pText, u32 bufferSize, u32 startIndex);
    bool replaceXmlCharacterEntityReference_(char* pText, u32 bufferSize, u32 startIndex);

    static void writeXmlInstanceAsText_(WriteStream* pStream, XmlElement* pElement, s32 depth,
                                        u32 workSize, Heap* pHeap);
    static void writeXmlContent_(WriteStream* pStream, const XmlElement* pElement,
                                 XmlElement::ElementType type, u32 workSize, Heap* pHeap);

    static const char** sDefaultEntityNames;
    static const char** sDefaultEntityValues;
    static u32 sDefaultEntityNum;

    XmlElement* mRoot = nullptr;
    char* mWorkBuffer0 = nullptr;
    char* mWorkBuffer1 = nullptr;
    char* mWorkBuffer2 = nullptr;
    u32 mWorkSize = 0x4000;
    EntityList mEntityList;
    Heap* mHeap = nullptr;
};
static_assert(sizeof(XmlDocument) == 0x68);

}  // namespace sead
