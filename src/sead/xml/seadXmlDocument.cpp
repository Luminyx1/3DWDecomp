#include <basis/seadNew.h>
#include <codec/seadBase64.h>
#include <container/seadObjArray.h>
#include <container/seadRingBuffer.h>
#include <heap/seadHeapMgr.h>
#include <prim/seadEndian.h>
#include <prim/seadMemUtil.h>
#include <stream/seadStream.h>
#include <xml/seadXmlDocument.h>
#include <xml/seadXmlUtil.h>

namespace sead
{
namespace
{
const char* sEntityNames[] = {"lt", "gt", "amp", "apos", "quot"};
const char* sEntityValues[] = {"<", ">", "&", "'", "\""};
}  // namespace

const char** XmlDocument::sDefaultEntityNames = sEntityNames;
const char** XmlDocument::sDefaultEntityValues = sEntityValues;
u32 XmlDocument::sDefaultEntityNum = 5;

/**
 * Deletes an element together with its children and following siblings.
 * @param pElement first element to delete (may be nullptr)
 */
static void freeXmlElement_(XmlElement* pElement)
{
    if (!pElement)
    {
        return;
    }

    if (pElement->child())
    {
        freeXmlElement_(pElement->child());
    }
    if (pElement->next())
    {
        freeXmlElement_(pElement->next());
    }
    pElement->clearLinks();
    delete pElement;
}

/**
 * Creates an empty document without a root element.
 */
XmlDocument::XmlDocument() = default;

/**
 * Deletes the element tree and the entity buffer.
 */
XmlDocument::~XmlDocument()
{
    if (mRoot)
    {
        freeXmlElementAll();
    }
    mEntityList.freeBuffer();
}

/**
 * Deletes the whole element tree.
 */
void XmlDocument::freeXmlElementAll()
{
    freeXmlElement_(mRoot);
    mRoot = nullptr;
}

/**
 * Replaces the entity list with the default entities.
 * @param pHeap heap for the entity buffer
 */
void XmlDocument::resetEntity_(Heap* pHeap)
{
    mEntityList.freeBuffer();
    mEntityList.allocBuffer(sDefaultEntityNum, pHeap);
    mEntityList.clear();
    for (u32 i = 0; i < sDefaultEntityNum; i++)
    {
        Entity* entity = mEntityList.emplaceBack();
        entity->mName = sDefaultEntityNames[i];
        entity->mValue = sDefaultEntityValues[i];
    }
}

/**
 * Adds an entity, or replaces the value of an entity with the same name.
 * @param rName entity name
 * @param rValue entity value
 * @return false if the entity buffer is full, true otherwise
 */
bool XmlDocument::addEntity(const SafeString& rName, const SafeString& rValue)
{
    if (mEntityList.isFull())
    {
        return false;
    }

    for (auto& entity : mEntityList)
    {
        if (entity.mName == rName)
        {
            entity.mValue = rValue;
            return true;
        }
    }

    Entity* entity = mEntityList.emplaceBack();
    entity->mName = rName;
    entity->mValue = rValue;
    return true;
}

/**
 * Removes an entity by name.
 * @param rName entity name
 * @return whether an entity was removed
 */
bool XmlDocument::eraseEntity(const SafeString& rName)
{
    for (auto& entity : mEntityList)
    {
        if (entity.mName == rName)
        {
            mEntityList.erase(&entity);
            return true;
        }
    }
    return false;
}

/**
 * Grows the entity buffer to hold at least num entities, keeping the current entities.
 * @param num minimum number of entities
 * @param pHeap heap for the buffers, or nullptr for the document's heap
 * @return the entity list.
 */
XmlDocument::EntityList* XmlDocument::expandEntityList(s32 num, Heap* pHeap)
{
    if (!pHeap)
    {
        pHeap = mHeap;
    }

    if (!mEntityList.isBufferReady())
    {
        mEntityList.allocBuffer(num, pHeap);
        return &mEntityList;
    }

    if (mEntityList.size() >= num)
    {
        return &mEntityList;
    }

    EntityList copies;
    copies.allocBuffer(mEntityList.size(), pHeap, -static_cast<s32>(sizeof(void*)));
    for (auto& entity : mEntityList)
    {
        Entity* copy = copies.emplaceBack();
        copy->mName = entity.mName;
        copy->mValue = entity.mValue;
    }

    mEntityList.freeBuffer();
    mEntityList.allocBuffer(num, pHeap);
    for (auto& copy : copies)
    {
        Entity* entity = mEntityList.birthBack();
        entity->mName = copy.mName;
        entity->mValue = copy.mValue;
    }
    copies.freeBuffer();
    return &mEntityList;
}

// NON_MATCHING: the writeU16(0) of the child and no-content paths is hoisted
static void writeXmlInstanceAsBinary_(WriteStream* pStream, XmlElement* pElement)
{
    while (true)
    {
        u8 nameLength = pElement->getName().calcLength();
        pStream->writeU8(nameLength);
        pStream->writeString(pElement->getName(), nameLength);

        const XmlElement::AttributeList& attributes = pElement->getAttributes();
        if (attributes.size() > 0)
        {
            pStream->writeU8(attributes.size());
            for (auto& attribute : attributes)
            {
                u8 length = attribute.mName.calcLength();
                pStream->writeU8(length);
                pStream->writeString(attribute.mName, length);
                length = attribute.mValue.calcLength();
                pStream->writeU8(length);
                pStream->writeString(attribute.mValue, length);
            }
        }
        else
        {
            pStream->writeU8(0);
        }

        if (pElement->getContent() && !pElement->child())
        {
            u16 size = Endian::fromHostU16(Endian::cLittle, pElement->getContentSize());
            pStream->writeU16(size);
            if (size != 0)
            {
                pStream->writeMemBlock(pElement->getContent(), size);
            }

            if (!pElement->next())
            {
                pStream->writeU8(3);
                return;
            }
            pStream->writeU8(0);
            pElement = pElement->next();
        }
        else if (pElement->child())
        {
            XmlElement* next = pElement->next();
            pStream->writeU16(0);
            if (!next)
            {
                pStream->writeU8(1);
                pElement = pElement->child();
            }
            else
            {
                pStream->writeU8(2);
                writeXmlInstanceAsBinary_(pStream, pElement->child());
                pElement = next;
            }
        }
        else
        {
            pStream->writeU16(0);
            if (!pElement->next())
            {
                pStream->writeU8(5);
                return;
            }
            pStream->writeU8(4);
            pElement = pElement->next();
        }
    }
}

/**
 * Writes the user entities and an element tree as XML text or in the binary format.
 * @param pStream destination stream
 * @param pHeap heap for temporary buffers
 * @param isBinary whether to write the binary format
 * @param pElement element to write, or nullptr for the root element
 * @return false if there is nothing to write, true otherwise
 */
bool XmlDocument::save(WriteStream* pStream, Heap* pHeap, bool isBinary,
                       XmlElement* pElement) const
{
    pStream->setMode(Stream::Modes::Binary);
    if (!pElement)
    {
        pElement = mRoot;
        if (!pElement)
        {
            return false;
        }
    }

    s32 userEntityNum = mEntityList.size() - sDefaultEntityNum;
    if (isBinary)
    {
        u8 num = userEntityNum;
        pStream->writeMemBlock(&num, 1);
        if (userEntityNum > 0)
        {
            auto it = mEntityList.begin();
            auto end = mEntityList.end();
            for (u32 i = 0; i < sDefaultEntityNum; i++)
            {
                ++it;
            }

            for (; it != end; ++it)
            {
                u8 length = 0;
                length = it->mName.calcLength();
                pStream->writeMemBlock(&length, 1);
                pStream->writeMemBlock(it->mName.cstr(), length);
                length = it->mValue.calcLength();
                pStream->writeMemBlock(&length, 1);
                pStream->writeMemBlock(it->mValue.cstr(), length);
            }
        }
        writeXmlInstanceAsBinary_(pStream, pElement);
    }
    else
    {
        SafeString header = "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
        pStream->writeString(header, header.calcLength());
        if (userEntityNum > 0)
        {
            SafeString doctype = "<!DOCTYPE doc[\n";
            pStream->writeString(doctype, doctype.calcLength());

            auto it = mEntityList.begin();
            auto end = mEntityList.end();
            for (u32 i = 0; i < sDefaultEntityNum; i++)
            {
                ++it;
            }

            for (; it != end; ++it)
            {
                FixedSafeString<150> line;
                line.format("<!ENTITY %s \"%s\">\n", it->mName.cstr(), it->mValue.cstr());
                pStream->writeString(line, line.calcLength());
            }

            SafeString footer = "]>\n";
            pStream->writeString(footer, footer.calcLength());
        }
        writeXmlInstanceAsText_(pStream, pElement, 0, mWorkSize, pHeap);
    }
    pStream->flush();
    return true;
}

/**
 * Writes the attributes of an element as ` name="value"` pairs.
 * @param pStream destination stream
 * @param pElement element whose attributes are written
 */
static void writeXmlAttributes_(WriteStream* pStream, const XmlElement* pElement)
{
    if (pElement->getAttributes().size() < 1)
    {
        return;
    }

    for (auto& attribute : pElement->getAttributes())
    {
        pStream->writeU8(' ');
        pStream->writeString(attribute.mName, attribute.mName.calcLength());
        pStream->writeU8('=');
        pStream->writeU8('"');
        pStream->writeString(attribute.mValue, attribute.mValue.calcLength());
        pStream->writeU8('"');
    }
}

// NON_MATCHING: instruction scheduling in the tab loop of the inlined sibling path
void XmlDocument::writeXmlInstanceAsText_(WriteStream* pStream, XmlElement* pElement, s32 depth,
                                          u32 workSize, Heap* pHeap)
{
    if (!pElement)
    {
        return;
    }

    const bool hasName = !pElement->getName().isEmpty();
    if (hasName)
    {
        for (s32 i = 0; i < depth; i++)
        {
            pStream->writeU8('\t');
        }
        pStream->writeU8('<');
        pStream->writeString(pElement->getName(), pElement->getName().calcLength());
        writeXmlAttributes_(pStream, pElement);
    }

    XmlElement* child = pElement->child();
    if (child)
    {
        pStream->writeString(">\n", 2);
        writeXmlInstanceAsText_(pStream, child, depth + 1, workSize, pHeap);
        for (XmlElement* sibling = child->next(); sibling; sibling = sibling->next())
        {
            if (sibling->child())
            {
                writeXmlInstanceAsText_(pStream, sibling, depth + 1, workSize, pHeap);
                continue;
            }

            const bool siblingHasName = !sibling->getName().isEmpty();
            if (siblingHasName)
            {
                for (s32 i = 0; i < depth + 1; i++)
                {
                    pStream->writeU8('\t');
                }
                pStream->writeU8('<');
                pStream->writeString(sibling->getName(), sibling->getName().calcLength());
                writeXmlAttributes_(pStream, sibling);
            }

            if (sibling->getContent())
            {
                if (!siblingHasName)
                {
                    for (s32 i = 0; i < depth + 1; i++)
                    {
                        pStream->writeU8('\t');
                    }
                    writeXmlContent_(pStream, sibling, sibling->getElementType(), workSize, pHeap);
                }
                else
                {
                    pStream->writeU8('>');
                    writeXmlContent_(pStream, sibling, sibling->getElementType(), workSize, pHeap);
                    pStream->writeString("</", 2);
                    pStream->writeString(sibling->getName(), sibling->getName().calcLength());
                    pStream->writeU8('>');
                }
            }
            else
            {
                if (!siblingHasName)
                {
                    continue;
                }
                pStream->writeString("/>", 2);
            }
            pStream->writeU8('\n');
        }

        if (!hasName)
        {
            return;
        }

        for (s32 i = 0; i < depth; i++)
        {
            pStream->writeU8('\t');
        }
        pStream->writeString("</", 2);
        pStream->writeString(pElement->getName(), pElement->getName().calcLength());
        pStream->writeU8('>');
    }
    else if (pElement->getContent())
    {
        if (!hasName)
        {
            for (s32 i = 0; i < depth; i++)
            {
                pStream->writeU8('\t');
            }
            writeXmlContent_(pStream, pElement, pElement->getElementType(), workSize, pHeap);
            pStream->writeU8('\n');
            return;
        }
        else
        {
            pStream->writeU8('>');
            writeXmlContent_(pStream, pElement, pElement->getElementType(), workSize, pHeap);
            pStream->writeString("</", 2);
            pStream->writeString(pElement->getName(), pElement->getName().calcLength());
            pStream->writeU8('>');
        }
    }
    else
    {
        if (!hasName)
        {
            return;
        }
        pStream->writeString("/>", 2);
    }
    pStream->writeU8('\n');
}

/**
 * Reads an element tree in the binary format into pElement and newly created elements.
 * @param pElement element that receives the first record
 * @param pStream source stream
 * @param pHeap heap for elements, attributes and contents
 * @param pName scratch buffer for attribute names
 * @param pValue scratch buffer for attribute values
 */
static void readXmlInstanceAsBinary_(XmlElement* pElement, ReadStream* pStream, Heap* pHeap,
                                     BufferedSafeString* pName, BufferedSafeString* pValue)
{
    while (true)
    {
        u8 nameLength = pStream->readU8();
        if (nameLength != 0)
        {
            pStream->readString(&pElement->getName(), nameLength);
        }
        pElement->getName().trim(nameLength);

        u8 attributeNum = pStream->readU8();
        if (attributeNum != 0)
        {
            pElement->expandAttributeList(attributeNum, pHeap);
            for (u8 i = 0; i < attributeNum; i++)
            {
                u8 length = pStream->readU8();
                pStream->readString(pName, length);
                pName->trim(length);
                length = pStream->readU8();
                pStream->readString(pValue, length);
                pValue->trim(length);
                pElement->addAttribute(*pName, *pValue, pHeap);
            }
        }

        u16 size = Endian::toHostU16(Endian::cLittle, pStream->readU16());
        if (size != 0)
        {
            u8* content = new (pHeap, sizeof(void*)) u8[size + 1];
            pStream->readMemBlock(content, size);
            content[size] = SafeString::cNullChar;
            pElement->setContent(content, size, true);
        }

        switch (pStream->readU8())
        {
        case 1:
            pElement = XmlUtil::createBackChildElement(pElement, pHeap);
            break;
        case 2:
            readXmlInstanceAsBinary_(XmlUtil::createBackChildElement(pElement, pHeap), pStream,
                                     pHeap, pName, pValue);
            // fallthrough
        case 0:
        case 4:
            pElement = XmlUtil::createBackSiblingElement(pElement, pHeap);
            if (!pElement)
            {
                return;
            }
            break;
        default:
            return;
        }
    }
}

/**
 * Replaces the document contents with a document read from a stream.
 * @param pStream source stream
 * @param pHeap heap for elements, entities and work buffers
 * @param isBinary whether the stream holds the binary format
 */
void XmlDocument::parseXml_(ReadStream* pStream, Heap* pHeap, bool isBinary)
{
    if (mRoot)
    {
        freeXmlElementAll();
    }

    pStream->setMode(Stream::Modes::Binary);
    pStream->rewind();
    if (isBinary)
    {
        FixedSafeString<8> name;
        FixedSafeString<128> value;
        u8 length = 0;
        u8 entityNum = 0;
        if (!pStream->readMemBlock(&entityNum, 1))
        {
            return;
        }

        resetEntity_(pHeap);
        expandEntityList(entityNum, pHeap);
        for (u8 i = 0; i < entityNum; i++)
        {
            if (!pStream->readMemBlock(&length, 1))
            {
                return;
            }
            if (pStream->readMemBlock(name.getBuffer(), length) < length)
            {
                return;
            }
            name.trim(length);

            if (!pStream->readMemBlock(&length, 1))
            {
                return;
            }
            if (pStream->readMemBlock(value.getBuffer(), length) < length)
            {
                return;
            }
            value.trim(length);

            addEntity(name, value);
        }

        mRoot = makeXmlInstance_(pStream, pHeap);
    }
    else
    {
        mWorkBuffer0 = new (pHeap, -static_cast<s32>(sizeof(void*))) char[mWorkSize];
        mWorkBuffer1 = new (pHeap, -static_cast<s32>(sizeof(void*))) char[mWorkSize];
        mWorkBuffer2 = new (pHeap, -static_cast<s32>(sizeof(void*))) char[mWorkSize];
        resetEntity_(pHeap);
        s32 offset = parseXmlDeclare_(pStream, pHeap);
        if (offset >= 0)
        {
            pStream->rewind();
            pStream->skip(offset);
            mRoot = parseXmlInstance_(pStream, pHeap);
        }

        delete mWorkBuffer0;
        mWorkBuffer0 = nullptr;
        delete mWorkBuffer1;
        mWorkBuffer1 = nullptr;
        delete mWorkBuffer2;
        mWorkBuffer2 = nullptr;
    }
}

/**
 * Reads an element tree in the binary format.
 * @param pStream source stream
 * @param pHeap heap for the elements
 * @return the new root element
 */
XmlElement* XmlDocument::makeXmlInstance_(ReadStream* pStream, Heap* pHeap)
{
    auto* root = new (pHeap, sizeof(void*)) XmlElement();
    FixedSafeString<1024> attributeName;
    FixedSafeString<1024> attributeValue;
    readXmlInstanceAsBinary_(root, pStream, pHeap, &attributeName, &attributeValue);
    return root;
}

static s32 skipXmlUntil_(ReadStream* pStream, char terminator)
{
    s32 count = 0;
    char c = 0;
    do
    {
        if (!pStream->readMemBlock(&c, 1))
        {
            return 0;
        }
        count++;
    } while (c != terminator);
    return count;
}

// NON_MATCHING: 83%, block layout (the target keeps the skipped-length check of the <? and <!DOCTYPE paths)
s32 XmlDocument::parseXmlDeclare_(ReadStream* pStream, Heap* pHeap)
{
    u8 c = 0;
    if (!pStream->readMemBlock(&c, 1))
    {
        return -1;
    }

    s32 state = 0;
    s32 pos = 0;
    do
    {
        s32 next = pos + 1;
        switch (state)
        {
        case 0:
            state = c == '<';
            break;
        case 1:
            if (c == '!')
            {
                state = 2;
                break;
            }
            if (c != '?')
            {
                return pos - 1;
            }
            {
                s32 count = skipXmlUntil_(pStream, '?');
                if (count == 0)
                {
                    return -1;
                }
                if (pStream->readU8() != '>')
                {
                    return -1;
                }
                state = 0;
                next = pos + 2 + count;
            }
            break;
        case 2:
            if (c == '-')
            {
                if (pStream->readU8() != '-')
                {
                    return -1;
                }
                next = pos + 2;
                state = 3;
            }
            else if (c == 'D')
            {
                const char* keyword = "OCTYPE";
                s32 keywordLength = SafeString(keyword).calcLength();
                for (s32 i = 0; i < keywordLength; i++)
                {
                    if (pStream->readU8() != static_cast<u8>(keyword[i]))
                    {
                        return -1;
                    }
                }
                s32 count = skipXmlUntil_(pStream, '[');
                if (count == 0)
                {
                    return -1;
                }
                state = 0;
                next = pos + 7 + count;
            }
            else if (c == 'E')
            {
                const char* keyword = "NTITY ";
                s32 keywordLength = SafeString(keyword).calcLength();
                for (s32 i = 0; i < keywordLength; i++)
                {
                    if (pStream->readU8() != static_cast<u8>(keyword[i]))
                    {
                        return -1;
                    }
                }

                FixedSafeString<8> name;
                char* nameBuffer = name.getBuffer();
                s32 nameBufferSize = name.getBufferSize();
                s32 i = 0;
                for (; i < nameBufferSize; i++)
                {
                    nameBuffer[i] = pStream->readU8();
                    if (nameBuffer[i] == ' ')
                    {
                        break;
                    }
                }
                if (i >= nameBufferSize)
                {
                    return -1;
                }
                nameBuffer[i] = SafeString::cNullChar;
                s32 nameLength = name.calcLength();

                if (!pStream->readMemBlock(&c, 1))
                {
                    return -1;
                }
                const u8 quote = c;
                if (quote != '\'' && quote != '"')
                {
                    return -1;
                }

                FixedSafeString<128> value;
                char* valueBuffer = value.getBuffer();
                s32 valueBufferSize = value.getBufferSize();
                for (i = 0; i < valueBufferSize; i++)
                {
                    valueBuffer[i] = pStream->readU8();
                    if (valueBuffer[i] == quote)
                    {
                        break;
                    }
                }
                if (i >= valueBufferSize)
                {
                    return -1;
                }
                valueBuffer[i] = SafeString::cNullChar;
                s32 valueLength = value.calcLength();

                if (!replaceXmlNumericCharacterReference_(valueBuffer, value.getBufferSize(), 0))
                {
                    return -1;
                }
                if (!replaceXmlCharacterEntityReference_(valueBuffer, value.getBufferSize(), 0))
                {
                    return -1;
                }

                expandEntityList(mEntityList.size() + 1, pHeap);
                Entity* entity = mEntityList.emplaceBack();
                entity->mName = name;
                entity->mValue = value;
                state = 0;
                next = pos + nameLength + valueLength + 9;
            }
            else
            {
                state = 2;
            }
            break;
        case 3:
            state = c == '-' ? 4 : 3;
            break;
        case 4:
            if (c == '-')
            {
                if (!pStream->readMemBlock(&c, 1))
                {
                    return -1;
                }
                if (c != '>')
                {
                    return -1;
                }
                state = 0;
                next = pos + 2;
            }
            else
            {
                state = 3;
            }
            break;
        }
        pos = next;
    } while (pStream->readMemBlock(&c, 1));
    return -1;
}

namespace
{
struct XmlAttributeWork
{
    FixedSafeString<1024> mName;
    FixedSafeString<1024> mValue;
};

bool isXmlSpace_(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

void rstripXmlName_(BufferedSafeString* pString)
{
    char* buffer = const_cast<char*>(pString->cstr());
    for (s32 i = SafeString(buffer).calcLength(); i > 0; i--)
    {
        if (buffer[i - 1] > ' ' && buffer[i - 1] != 0x7f)
        {
            break;
        }
        buffer[i - 1] = SafeString::cNullChar;
    }
}
}  // namespace

// NON_MATCHING: ~42%, same logic; block layout, loop unswitching of the closing-tag loop and register allocation differ
XmlElement* XmlDocument::parseXmlInstance_(ReadStream* pStream, Heap* pHeap)
{
    u8 c = 0;
    RingBuffer<XmlElement*> stack;
    stack.allocBuffer(32, pHeap, -static_cast<s32>(sizeof(void*)));
    ObjArray<XmlAttributeWork> attributes;
    attributes.allocBuffer(24, pHeap, -static_cast<s32>(sizeof(void*)));
    FixedSafeString<64> tagName;
    char* textBuffer = mWorkBuffer2;
    char* nameBuffer = tagName.getBuffer();

    XmlElement* root = nullptr;
    XmlElement* element = nullptr;
    XmlElement* base64Element = nullptr;
    XmlElement* closeElement = nullptr;
    XmlElement* textElement = nullptr;

    {
        s32 state = 0;
        u32 textLength = 0;
        u32 nameLength = 0;
        u32 replaceStart = 0;
        bool isInText = false;
        bool isTopLevelComment = false;
        bool isBase64InCData = false;

        while (pStream->readMemBlock(&c, 1))
        {
            if (state == 1)
            {
                if (isXmlSpace_(c))
                {
                    state = 1;
                }
                else if (c == '!')
                {
                    const char* keyword = "--";
                    s32 keywordLength = SafeString(keyword).calcLength();
                    for (s32 i = 0; i < keywordLength; i++)
                    {
                        if (pStream->readU8() != static_cast<u8>(keyword[i]))
                        {
                            goto fail;
                        }
                    }
                    state = 3;
                    isTopLevelComment = true;
                }
                else if (c == '/')
                {
                    tagName.clear();
                    bool isSkippingSpace = true;
                    nameLength = 0;
                    while (true)
                    {
                        if (!pStream->readMemBlock(&c, 1))
                        {
                            goto fail;
                        }
                        if (c == '>')
                        {
                            tagName.trim(nameLength);
                            rstripXmlName_(&tagName);
                            if (tagName == stack.back()->getName())
                            {
                                XmlElement* closed;
                                stack.popBack(&closed);
                                if (stack.size() == 0)
                                {
                                    goto success;
                                }
                                isInText = false;
                                textLength = 0;
                                state = 0;
                                break;
                            }
                        }
                        if (isSkippingSpace && isXmlSpace_(c))
                        {
                            continue;
                        }
                        nameBuffer[nameLength] = c;
                        isSkippingSpace = false;
                        if (nameLength++ >= 64)
                        {
                            goto fail;
                        }
                    }
                }
                else
                {
                    nameBuffer[0] = c;
                    nameLength = 1;
                    state = 10;
                }
            }
            else
            {
                switch (state)
                {
                case 0:
                    if (c == '<')
                    {
                        if (textLength != 0)
                        {
                            if (stack.size() == 0)
                            {
                                stack.clear();
                            }
                            else
                            {
                                u32 end = textLength;
                                while (end != 0 && isXmlSpace_(mWorkBuffer2[end - 1]))
                                {
                                    end--;
                                }
                                mWorkBuffer2[end] = SafeString::cNullChar;
                                replaceXmlNumericCharacterReference_(textBuffer, mWorkSize, 0);
                                replaceXmlCharacterEntityReference_(textBuffer, mWorkSize, 0);
                                u32 length = 0;
                                while (mWorkBuffer2[length] != SafeString::cNullChar)
                                {
                                    length++;
                                }
                                if (length != 0)
                                {
                                    auto* text = new (pHeap, sizeof(void*)) XmlElement();
                                    u8* content = new (pHeap, sizeof(void*)) u8[length + 1];
                                    MemUtil::copy(content, mWorkBuffer2, length);
                                    content[length] = SafeString::cNullChar;
                                    text->setContent(content, length, true);
                                    stack.back()->pushBackChild(text);
                                }
                                textLength = 0;
                            }
                        }
                        state = 1;
                    }
                    else
                    {
                        if (isInText || !isXmlSpace_(c))
                        {
                            textBuffer[textLength++] = c;
                            isInText = true;
                        }
                        if (textLength + 1 > mWorkSize)
                        {
                            goto fail;
                        }
                        state = 0;
                    }
                    break;
                case 3:
                    state = c == '-' ? 4 : 3;
                    break;
                case 4:
                    if (c == '-')
                    {
                        if (!pStream->readMemBlock(&c, 1))
                        {
                            goto fail;
                        }
                        if (c != '>')
                        {
                            goto fail;
                        }
                        if (isTopLevelComment)
                        {
                            isInText = false;
                            textLength = 0;
                            state = 0;
                        }
                        else
                        {
                            state = 6;
                        }
                    }
                    else
                    {
                        state = 3;
                    }
                    break;
                case 5:
                {
                    FixedSafeString<1024> name;
                    FixedSafeString<1024> value;
                    char* attrNameBuffer = name.getBuffer();
                    char* attrValueBuffer = value.getBuffer();
                    while (true)
                    {
                        while (isXmlSpace_(c))
                        {
                            if (!pStream->readMemBlock(&c, 1))
                            {
                                break;
                            }
                        }

                        if (c == '/' || c == '>')
                        {
                            break;
                        }

                        u32 i = 0;
                        if (c != '=')
                        {
                            while (true)
                            {
                                attrNameBuffer[i] = c;
                                if (i >= 0x3ff)
                                {
                                    goto fail;
                                }
                                if (!pStream->readMemBlock(&c, 1))
                                {
                                    goto readValue;
                                }
                                i++;
                                if (c == '=')
                                {
                                    break;
                                }
                            }
                        }
                        name.trim(i);
                        rstripXmlName_(&name);
                        if (i == 0)
                        {
                            goto fail;
                        }

                    readValue:
                        u8 quote;
                        while (true)
                        {
                            if (!pStream->readMemBlock(&c, 1))
                            {
                                quote = c;
                                if (quote != '"' && quote != '\'')
                                {
                                    goto fail;
                                }
                                break;
                            }
                            quote = c;
                            if (quote > '\'')
                            {
                                goto fail;
                            }
                            if (isXmlSpace_(quote))
                            {
                                continue;
                            }
                            if (quote != '"' && quote != '\'')
                            {
                                goto fail;
                            }
                            break;
                        }

                        for (i = 0;; i++)
                        {
                            if (!pStream->readMemBlock(&c, 1))
                            {
                                goto addAttribute;
                            }
                            if (c == quote)
                            {
                                break;
                            }
                            attrValueBuffer[i] = c;
                            if (i > 0x3fe)
                            {
                                goto fail;
                            }
                        }
                        value.trim(i);
                        replaceXmlCharacterEntityReference_(attrValueBuffer, value.getBufferSize(), 0);
                        replaceXmlNumericCharacterReference_(attrValueBuffer, value.getBufferSize(), 0);

                    addAttribute:
                        XmlAttributeWork* attribute = attributes.emplaceBack();
                        if (!attribute)
                        {
                            goto fail;
                        }
                        attribute->mName = name;
                        attribute->mValue = value;

                        if (!pStream->readMemBlock(&c, 1))
                        {
                            goto fail;
                        }
                    }

                    XmlElement** ppElement;
                    if (c == '/')
                    {
                        u32 count = skipXmlUntil_(pStream, '>');
                        if (stack.size() == 0 || count == 0)
                        {
                            goto fail;
                        }
                        ppElement = stack.get(stack.size() - 1);
                        stack.popBack(&element);
                        isInText = false;
                        state = 0;
                    }
                    else
                    {
                        if (stack.size() == 0)
                        {
                            goto fail;
                        }
                        ppElement = stack.get(stack.size() - 1);
                        replaceStart = 0;
                        state = 6;
                    }
                    if (!ppElement)
                    {
                        goto fail;
                    }

                    if (attributes.size() != 0)
                    {
                        XmlElement* target = *ppElement;
                        target->expandAttributeList(attributes.size(), pHeap);
                        target->mAttributes.clear();
                        for (auto& attribute : attributes)
                        {
                            target->mAttributes.emplaceBack(pHeap, attribute);
                        }
                        attributes.clear();
                    }
                    textLength = 0;
                    break;
                }
                case 6:
                {
                    bool isAllSpace = true;
                    while (true)
                    {
                        if (c == '<')
                        {
                            if (!pStream->readMemBlock(&c, 1))
                            {
                                goto fail;
                            }
                            if (c == '!')
                            {
                                if (!pStream->readMemBlock(&c, 1))
                                {
                                    goto fail;
                                }
                                if (c == '-')
                                {
                                    if (pStream->readU8() != '-')
                                    {
                                        goto fail;
                                    }
                                    isTopLevelComment = false;
                                    state = 3;
                                    break;
                                }
                                if (c == '[')
                                {
                                    const char* keyword = "CDATA[";
                                    s32 keywordLength = SafeString(keyword).calcLength();
                                    for (s32 i = 0; i < keywordLength; i++)
                                    {
                                        if (pStream->readU8() != static_cast<u8>(keyword[i]))
                                        {
                                            goto fail;
                                        }
                                    }
                                    u32 end = isAllSpace ? 0 : textLength;
                                    mWorkBuffer2[end] = SafeString::cNullChar;
                                    replaceXmlNumericCharacterReference_(textBuffer, mWorkSize, replaceStart);
                                    replaceXmlCharacterEntityReference_(textBuffer, mWorkSize, replaceStart);
                                    u32 length = 0;
                                    while (mWorkBuffer2[length] != SafeString::cNullChar)
                                    {
                                        length++;
                                    }
                                    state = 7;
                                    replaceStart = end < length ? length : end;
                                    textLength = replaceStart;
                                    break;
                                }
                                if (c == 'B')
                                {
                                    const char* keyword = "ASE64[";
                                    s32 keywordLength = SafeString(keyword).calcLength();
                                    for (s32 i = 0; i < keywordLength; i++)
                                    {
                                        if (pStream->readU8() != static_cast<u8>(keyword[i]))
                                        {
                                            goto fail;
                                        }
                                    }
                                    u32 end = isAllSpace ? 0 : textLength;
                                    mWorkBuffer2[end] = SafeString::cNullChar;
                                    replaceXmlNumericCharacterReference_(textBuffer, mWorkSize, replaceStart);
                                    replaceXmlCharacterEntityReference_(textBuffer, mWorkSize, replaceStart);
                                    u32 length = 0;
                                    while (mWorkBuffer2[length] != SafeString::cNullChar)
                                    {
                                        length++;
                                    }
                                    isBase64InCData = false;
                                    state = 8;
                                    replaceStart = end < length ? length : end;
                                    textLength = replaceStart;
                                    break;
                                }
                                textBuffer[textLength] = '<';
                                textBuffer[textLength + 1] = '!';
                                textLength += 2;
                            }
                            else if (c == '/')
                            {
                                mWorkBuffer2[textLength] = SafeString::cNullChar;
                                replaceXmlNumericCharacterReference_(textBuffer, mWorkSize, replaceStart);
                                replaceXmlCharacterEntityReference_(textBuffer, mWorkSize, replaceStart);
                                stack.popBack(&textElement);
                                if (textElement->getElementType() != XmlElement::cElementType_Base64)
                                {
                                    textLength = 0;
                                    while (mWorkBuffer2[textLength] != SafeString::cNullChar)
                                    {
                                        textLength++;
                                    }
                                }
                                stack.pushBack(textElement);
                                state = 9;
                                break;
                            }
                            else
                            {
                                if (textLength != 0)
                                {
                                    u32 end = textLength;
                                    while (end != 0 && isXmlSpace_(mWorkBuffer2[end - 1]))
                                    {
                                        end--;
                                    }
                                    mWorkBuffer2[end] = SafeString::cNullChar;
                                    u32 start = 0;
                                    while (start < end && isXmlSpace_(mWorkBuffer2[start]))
                                    {
                                        start++;
                                    }
                                    u32 length = end - start;
                                    for (u32 i = 0; i < length; i++)
                                    {
                                        mWorkBuffer2[i] = mWorkBuffer2[start + i];
                                    }
                                    mWorkBuffer2[length] = SafeString::cNullChar;
                                    replaceXmlNumericCharacterReference_(textBuffer, mWorkSize, 0);
                                    replaceXmlCharacterEntityReference_(textBuffer, mWorkSize, 0);
                                    length = 0;
                                    while (mWorkBuffer2[length] != SafeString::cNullChar)
                                    {
                                        length++;
                                    }
                                    if (length != 0)
                                    {
                                        auto* text = new (pHeap, sizeof(void*)) XmlElement();
                                        u8* content = new (pHeap, sizeof(void*)) u8[length + 1];
                                        MemUtil::copy(content, mWorkBuffer2, length);
                                        content[length] = SafeString::cNullChar;
                                        text->setContent(content, length, true);
                                        stack.back()->pushBackChild(text);
                                    }
                                }
                                nameBuffer[0] = c;
                                nameLength = 1;
                                textLength = 0;
                                state = 10;
                                break;
                            }
                        }

                        textBuffer[textLength] = c;
                        isAllSpace = isAllSpace && isXmlSpace_(c);
                        if (textLength + 2 > mWorkSize)
                        {
                            goto fail;
                        }
                        textLength++;
                        if (!pStream->readMemBlock(&c, 1))
                        {
                            goto fail;
                        }
                    }
                    break;
                }
                case 7:
                    if (c == ']')
                    {
                        if (!pStream->readMemBlock(&c, 1))
                        {
                            goto fail;
                        }
                        if (c == ']')
                        {
                            if (!pStream->readMemBlock(&c, 1))
                            {
                                goto fail;
                            }
                            if (c == '>')
                            {
                                replaceStart = textLength;
                                state = 6;
                                break;
                            }
                            textBuffer[textLength] = ']';
                            textBuffer[textLength + 1] = ']';
                            textLength += 2;
                        }
                        else
                        {
                            textBuffer[textLength] = ']';
                            textLength += 1;
                        }
                    }
                    else if (c == '<')
                    {
                        if (!pStream->readMemBlock(&c, 1))
                        {
                            goto fail;
                        }
                        if (c == '!')
                        {
                            const char* keyword = "BASE64[";
                            s32 keywordLength = SafeString(keyword).calcLength();
                            for (s32 i = 0; i < keywordLength; i++)
                            {
                                if (pStream->readU8() != static_cast<u8>(keyword[i]))
                                {
                                    goto fail;
                                }
                            }
                            state = 8;
                            isBase64InCData = true;
                            break;
                        }
                        textBuffer[textLength] = '<';
                        textLength += 1;
                    }
                    textBuffer[textLength] = c;
                    textLength++;
                    state = 7;
                    break;
                case 8:
                {
                    stack.popBack(&base64Element);
                    base64Element->setElementType(XmlElement::cElementType_Base64);
                    stack.pushBack(base64Element);
                    char* encoded = mWorkBuffer0;
                    char* decoded = mWorkBuffer1;
                    s32 encodedLength = 0;
                    while (true)
                    {
                        if (c == ']')
                        {
                            if (!pStream->readMemBlock(&c, 1))
                            {
                                goto fail;
                            }
                            if (c == '>')
                            {
                                size_t decodedSize = 0;
                                if (!Base64::decode(decoded, mWorkSize, encoded, encodedLength, &decodedSize))
                                {
                                    goto fail;
                                }
                                u32 i = 0;
                                for (; i < decodedSize; i++)
                                {
                                    textBuffer[i] = decoded[i];
                                }
                                state = isBase64InCData ? 7 : 6;
                                replaceStart = i;
                                textLength = i;
                                break;
                            }
                        }
                        if (!isXmlSpace_(c))
                        {
                            encoded[encodedLength++] = c;
                        }
                        if (!pStream->readMemBlock(&c, 1))
                        {
                            textLength = 0;
                            state = 8;
                            break;
                        }
                    }
                    break;
                }
                case 9:
                    if (c == '>')
                    {
                        tagName.trim(nameLength);
                        rstripXmlName_(&tagName);
                        if (tagName == stack.back()->getName())
                        {
                            stack.popBack(&closeElement);
                            if (textLength != 0)
                            {
                                u32 length = closeElement->getElementType() == XmlElement::cElementType_Base64 ?
                                                 textLength & ~3u :
                                                 textLength;
                                u8* content = new (pHeap, sizeof(void*)) u8[length + 1];
                                MemUtil::copy(content, mWorkBuffer2, length);
                                content[length] = SafeString::cNullChar;
                                closeElement->setContent(content, length, true);
                                replaceStart = 0;
                                textLength = 0;
                            }
                            isInText = false;
                            state = 0;
                            break;
                        }
                        nameBuffer[nameLength] = c;
                        if (++nameLength > 64)
                        {
                            goto fail;
                        }
                    }
                    break;
                case 10:
                {
                    if (stack.size() >= stack.capacity())
                    {
                        goto fail;
                    }
                    auto* newElement = new (pHeap, sizeof(void*)) XmlElement();
                    while (true)
                    {
                        if (isXmlSpace_(c) || c == '>')
                        {
                            tagName.trim(nameLength);
                            if (tagName.at(0) == SafeString::cNullChar)
                            {
                                delete newElement;
                                goto fail;
                            }
                            newElement->setName(tagName);
                            if (stack.size() == 0)
                            {
                                root = newElement;
                            }
                            else
                            {
                                stack.back()->pushBackChild(newElement);
                            }
                            stack.pushBack(newElement);
                            if (c == '>')
                            {
                                textLength = 0;
                                replaceStart = 0;
                                state = 6;
                            }
                            else
                            {
                                state = 5;
                            }
                            break;
                        }
                        if (c == '/')
                        {
                            if (pStream->readU8() != '>')
                            {
                                delete newElement;
                                goto fail;
                            }
                            tagName.trim(nameLength);
                            rstripXmlName_(&tagName);
                            newElement->setName(tagName);
                            if (stack.size() == 0)
                            {
                                if (root)
                                {
                                    delete newElement;
                                    goto fail;
                                }
                                stack.freeBuffer();
                                attributes.freeBuffer();
                                return newElement;
                            }
                            stack.back()->pushBackChild(newElement);
                            isInText = false;
                            textLength = 0;
                            state = 0;
                            break;
                        }
                        nameBuffer[nameLength] = c;
                        if (++nameLength >= 0x41)
                        {
                            delete newElement;
                            goto fail;
                        }
                        if (!pStream->readMemBlock(&c, 1))
                        {
                            delete newElement;
                            goto fail;
                        }
                    }
                    break;
                }
                default:
                    break;
                }
            }
        }

        if (!root)
        {
            goto fail;
        }
    }

success:
    stack.freeBuffer();
    attributes.freeBuffer();
    return root;

fail:
    stack.freeBuffer();
    attributes.freeBuffer();
    freeXmlElement_(root);
    return nullptr;
}

/**
 * Creates a document, reading it from a stream or creating an empty root element.
 * @param pStream source stream, or nullptr for an empty document
 * @param pHeap heap for the document, or nullptr for the current heap
 * @param isBinary whether the stream holds the binary format
 * @param workSize size of the text parser's work buffers
 * @return the document, or nullptr if the stream could not be parsed
 */
XmlDocument* XmlDocument::create(ReadStream* pStream, Heap* pHeap, bool isBinary, u32 workSize)
{
    if (!pHeap)
    {
        pHeap = HeapMgr::instance()->getCurrentHeap();
    }

    auto* document = new (pHeap, sizeof(void*)) XmlDocument();
    document->mWorkSize = workSize;
    if (pStream)
    {
        document->parseXml_(pStream, pHeap, isBinary);
        if (!document->mRoot)
        {
            delete document;
            return nullptr;
        }
    }
    else
    {
        document->mRoot = new (pHeap, sizeof(void*)) XmlElement();
        document->resetEntity_(pHeap);
    }
    document->mHeap = pHeap;
    return document;
}

static u32 convertHexCharToInt_(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    return 0;
}

static u32 convertCodeToUtf8_(char* pDst, u32 code)
{
    if (code < 0x80)
    {
        pDst[0] = code;
        return 1;
    }
    if (code < 0x800)
    {
        pDst[0] = 0xc0 | (code >> 6);
        pDst[1] = 0x80 | (code & 0x3f);
        return 2;
    }
    if (code < 0x10000)
    {
        pDst[0] = 0xe0 | (code >> 12);
        pDst[1] = 0x80 | ((code >> 6) & 0x3f);
        pDst[2] = 0x80 | (code & 0x3f);
        return 3;
    }
    pDst[0] = '?';
    return 1;
}

// NON_MATCHING: loop structure
bool XmlDocument::replaceXmlNumericCharacterReference_(char* pText, u32 bufferSize, u32 startIndex)
{
    u32 length = 0;
    for (; pText[length] != SafeString::cNullChar; length++)
    {
        mWorkBuffer0[length] = pText[length];
    }
    mWorkBuffer0[length] = SafeString::cNullChar;

    if (length > bufferSize)
    {
        return false;
    }

    char* dst = mWorkBuffer1;
    u32 pos = 0;
    for (u32 i = 0; pos < bufferSize && i < length; i++)
    {
        char c = pText[i];
        if (i < startIndex || i > length - 4 || c != '&')
        {
            dst[pos++] = c;
            continue;
        }

        if (i + 1 >= bufferSize)
        {
            break;
        }

        if (pText[i + 1] == '#')
        {
            u32 code;
            if (i + 2 >= bufferSize)
            {
                break;
            }

            if (pText[i + 2] == 'x')
            {
                if (i + 5 >= bufferSize)
                {
                    break;
                }
                if (pText[i + 5] != ';')
                {
                    continue;
                }
                code = convertHexCharToInt_(pText[i + 3]) * 16 +
                       convertHexCharToInt_(pText[i + 4]);
                i += 5;
            }
            else
            {
                if (i + 4 >= bufferSize)
                {
                    break;
                }
                if (pText[i + 4] != ';')
                {
                    continue;
                }
                code = convertHexCharToInt_(pText[i + 2]) * 10 +
                       convertHexCharToInt_(pText[i + 3]);
                i += 4;
            }
            pos += convertCodeToUtf8_(&dst[pos], code);
            continue;
        }
        dst[pos++] = c;
    }

    if (pos >= bufferSize)
    {
        pos = bufferSize - 1;
    }
    dst[pos] = SafeString::cNullChar;
    MemUtil::copy(pText, dst, pos + 1);
    return true;
}

// NON_MATCHING: register allocation
bool XmlDocument::replaceXmlCharacterEntityReference_(char* pText, u32 bufferSize, u32 startIndex)
{
    u32 length = 0;
    for (; pText[length] != SafeString::cNullChar; length++)
    {
        mWorkBuffer0[length] = pText[length];
    }
    mWorkBuffer0[length] = SafeString::cNullChar;

    char* dst = mWorkBuffer1;
    u32 pos = 0;
    for (u32 i = 0; i < length && pos < bufferSize; i++)
    {
        char c = pText[i];
        if (i > length - 2 || i < startIndex || c != '&')
        {
            dst[pos++] = c;
            continue;
        }

        u32 nameLength = 0;
        for (auto& entity : mEntityList)
        {
            nameLength = entity.mName.calcLength();
            if (bufferSize - i - 1 < nameLength)
            {
                return false;
            }
            if (pText[i + 1 + nameLength] != ';')
            {
                continue;
            }

            const char* name = entity.mName.cstr();
            bool isMatch = true;
            for (u32 k = 0; k < nameLength; k++)
            {
                if (name[k] != mWorkBuffer0[i + 1 + k])
                {
                    isMatch = false;
                    break;
                }
            }
            if (!isMatch)
            {
                nameLength = 0;
                continue;
            }

            u32 valueLength = entity.mValue.calcLength();
            if (bufferSize - i - 1 < valueLength)
            {
                return false;
            }
            const char* value = entity.mValue.cstr();
            for (u32 k = 0; k < valueLength; k++)
            {
                dst[pos++] = value[k];
            }
            break;
        }
        i += 1 + nameLength;
    }

    if (pos >= bufferSize)
    {
        pos = bufferSize - 1;
    }
    dst[pos] = SafeString::cNullChar;
    MemUtil::copy(pText, dst, pos + 1);
    return true;
}

/**
 * Resolves an element path from the root element.
 * @param rPath element path
 * @return the element, or nullptr if there is no root or the path does not resolve
 */
XmlElement* XmlDocument::findElement(const SafeString& rPath)
{
    if (!mRoot)
    {
        return nullptr;
    }
    return mRoot->findElement(rPath);
}

/**
 * Replaces the table of entities every document starts with.
 * @param pNames entity names
 * @param pValues entity values
 * @param num number of entities
 */
void XmlDocument::setDefaultEntity(const char** pNames, const char** pValues, u32 num)
{
    sDefaultEntityNames = pNames;
    sDefaultEntityValues = pValues;
    sDefaultEntityNum = num;
}

/**
 * Writes the content of an element as base64 CDATA, raw CDATA or escaped text.
 * @param pStream destination stream
 * @param pElement element whose content is written
 * @param type how the content is written
 * @param workSize maximum size of the base64 encoded content
 * @param pHeap heap for temporary buffers
 */
void XmlDocument::writeXmlContent_(WriteStream* pStream, const XmlElement* pElement,
                                   XmlElement::ElementType type, u32 workSize, Heap* pHeap)
{
    if (!pElement->getContent())
    {
        return;
    }

    switch (type)
    {
    case XmlElement::cElementType_CData:
        pStream->writeString("<![CDATA[", 9);
        pStream->writeMemBlock(pElement->getContent(), pElement->getContentSize());
        pStream->writeString("]]>", 3);
        break;
    case XmlElement::cElementType_Base64:
    {
        u32 size = pElement->getContentSize();
        u32 encodedSize = (size / 3 + (size % 3 != 0 ? 1 : 0)) * 4;
        if (encodedSize > workSize)
        {
            return;
        }

        char* encoded = new (pHeap, -static_cast<s32>(sizeof(void*))) char[encodedSize];
        Base64::encode(encoded, pElement->getContent(), pElement->getContentSize(), false);
        pStream->writeString("<![CDATA[<!BASE64[", 18);
        pStream->writeMemBlock(encoded, encodedSize);
        pStream->writeString("]>]]>", 5);
        delete[] encoded;
        break;
    }
    default:
    {
        const char* text = pElement->getContentString().cstr();
        s32 length = pElement->getContentString().calcLength();

        s32 entityCharNum = 0;
        for (s32 i = 0; i < length; i++)
        {
            for (u32 j = 0; j < sDefaultEntityNum; j++)
            {
                if (sDefaultEntityValues[j][0] == text[i])
                {
                    entityCharNum++;
                    break;
                }
            }
        }

        if (entityCharNum == 0)
        {
            pStream->writeMemBlock(pElement->getContent(), pElement->getContentSize());
            return;
        }

        char* escaped =
            new (pHeap, -static_cast<s32>(sizeof(void*))) char[entityCharNum * 5 + length];
        s32 pos = 0;
        for (s32 i = 0; i < length; i++)
        {
            char c = text[i];
            if (c == '&' && text[i + 1] == '&')
            {
                escaped[pos++] = '&';
                i++;
                continue;
            }

            bool isEntity = false;
            for (u32 j = 0; j < sDefaultEntityNum; j++)
            {
                if (sDefaultEntityValues[j][0] == c)
                {
                    escaped[pos++] = '&';
                    SafeString name = sDefaultEntityNames[j];
                    s32 nameLength = name.calcLength();
                    for (s32 k = 0; k < nameLength; k++)
                    {
                        escaped[pos++] = sDefaultEntityNames[j][k];
                    }
                    escaped[pos++] = ';';
                    isEntity = true;
                    break;
                }
            }

            if (!isEntity)
            {
                escaped[pos++] = c;
            }
        }
        escaped[pos] = SafeString::cNullChar;
        pStream->writeMemBlock(escaped, pos);
        delete[] escaped;
        break;
    }
    }
}

}  // namespace sead
