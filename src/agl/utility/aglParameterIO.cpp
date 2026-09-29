#include "utility/aglParameterIO.h"
#include <basis/seadRawPrint.h>
#include <heap/seadExpHeap.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <stream/seadRamStream.h>
#include <time/seadCalendarTime.h>
#include <time/seadDateTime.h>
#include <xml/seadXmlDocument.h>
#include <xml/seadXmlElement.h>
#include <xml/seadXmlUtil.h>
#include "detail/aglPrivateResource.h"
#include "utility/aglParameterObj.h"

namespace agl::utl
{

IParameterIO::IParameterIO(const sead::SafeString& name, u32 version)
{
    mType = name;
    mVersion = version;
    setParameterListName_("param_root");
}

IParameterIO::IParameterIO() : _21c(1)
{
    mType = "xml";
    mVersion = 0;
    setParameterListName_("param_root");
}

/**
 * Writes this IO as an XML document and saves it through the file IO manager.
 * @param rPath default file path
 * @param workSize work buffer size of the XML document
 * @return whether the file was saved
 */
bool IParameterIO::save(const sead::SafeString& rPath, u32 workSize) const
{
    sead::Heap* parent = detail::PrivateResource::instance()->getDebugHeap();
    sead::ExpHeap* heap = sead::ExpHeap::create(workSize + 0x400, "agl::Debug::xml", parent, 8,
                                                sead::Heap::cHeapDirection_Forward, false);
    sead::XmlDocument* document = sead::XmlDocument::create(nullptr, heap, false, workSize);
    sead::XmlElement* root = document->getRoot();
    writeHeader_(root, heap);
    writeToXML(sead::XmlUtil::createBackChildAndSetupElement(root, "data", "", heap), heap);
    const bool result = save_(rPath, document);
    heap->destroy();
    return result;
}

/**
 * Writes the root name and the header element (type, version and time stamp).
 * @param pElement root element
 * @param pHeap heap used for the created elements
 */
void IParameterIO::writeHeader_(sead::XmlElement* pElement, sead::Heap* pHeap) const
{
    pElement->setName("root");
    sead::XmlElement* header =
        sead::XmlUtil::createBackChildAndSetupElement(pElement, "header", "", pHeap);
    header->expandAttributeList(3, pHeap);
    header->addAttribute("type", mType, pHeap);
    header->addAttribute("version", sead::FormatFixedSafeString<1024>("%d", mVersion), pHeap);

    sead::DateTime dateTime(0);
    dateTime.setNow();
    sead::CalendarTime calendarTime;
    dateTime.getCalendarTime(&calendarTime);
    sead::FormatFixedSafeString<64> date(
        "time stamp : %4d/%02d/%02d %02d:%02d:%02d", calendarTime.getYear(),
        calendarTime.getMonth().getValueOneOrigin(), calendarTime.getDay(),
        calendarTime.getHour(), calendarTime.getMinute(), calendarTime.getSecond());
    header->addAttribute("date", date, pHeap);
}

/**
 * Reads this IO from an XML text document.
 * @param pData XML text
 * @param size size of the text in bytes
 * @param x forwarded to the parameters' readFromXML
 * @return number of parameters read, or -1 on a parse error
 */
s32 IParameterIO::loadText(const void* pData, u32 size, bool x)
{
    sead::Heap* heap = detail::PrivateResource::instance()->getDebugHeap();
    sead::RamReadStream stream(pData, size, sead::Stream::Modes::Binary);
    sead::XmlDocument* document = sead::XmlDocument::create(&stream, heap, false, 0x4000);
    sead::XmlElement* root = document->getRoot();
    sead::XmlElement* header = root->findElement("./header");
    sead::XmlElement* data = root->findElement("./data");

    if (header)
    {
        const sead::SafeString version = header->findAttributeValue("version");
        const sead::FormatFixedSafeString<1024> expected("%d", mVersion);
        if (version != expected)
        {
            callbackInvalidVersion_(ResParameterArchive(nullptr));
        }
    }

    s32 result;
    if (sead::XmlElement* list = data->findElement(getTagName()))
    {
        result = readFromXML(*list, x);
    }
    else
    {
        sead::XmlElement* obj = data->findElement(IParameterObj::getTagName());
        if (obj)
        {
            result = mpChildObjHead ? mpChildObjHead->readFromXML(*obj, x) : 0;
        }
        else
        {
            result = mpChildObjHead ? mpChildObjHead->readFromXML(*data, x) : 0;
        }
    }

    delete document;
    return result;
}

/**
 * Applies a parameter archive to this IO, reporting version mismatches.
 * @param arc archive to apply
 */
void IParameterIO::applyResParameterArchive(ResParameterArchive arc)
{
    SEAD_ASSERT(arc.isValid());
    mResFileSize = arc.ptr()->file_size;

    if (mVersion != arc.ptr()->pio_version)
    {
        callbackInvalidVersion_(arc);
    }

    applyResParameterList(arc.getRootList());
}

/**
 * Applies the interpolation of two parameter archives to this IO.
 * @param arc_a archive used at t = 0
 * @param arc_b archive used at t = 1
 * @param t interpolation factor
 */
void IParameterIO::applyResParameterArchiveLerp(ResParameterArchive arc_a,
                                                ResParameterArchive arc_b, f32 t)
{
    SEAD_ASSERT(arc_a.isValid());
    SEAD_ASSERT(arc_b.isValid());

    if (mVersion != arc_a.ptr()->pio_version)
    {
        callbackInvalidVersion_(arc_a);
    }

    if (mVersion != arc_b.ptr()->pio_version)
    {
        callbackInvalidVersion_(arc_b);
    }

    applyResParameterList(arc_a.getRootList(), arc_b.getRootList(), t);
}

void IParameterIO::genMessageIO(sead::hostio::Context* pContext, u32 flags)
{
    if (flags & 1)
    {
        sead::FormatFixedSafeString<1024> meta("Save (*.%s)", mType.cstr());
    }

    if (flags & 2)
    {
        sead::FormatFixedSafeString<1024> meta("Load (*.%s)", mType.cstr());
    }

    const char* is_enable = "false";
    if (mPath != sead::SafeString::cEmptyString)
    {
        sead::FormatFixedSafeString<1024> meta("%s", mPath.cstr());
        is_enable = "true";
    }

    {
        sead::FormatFixedSafeString<1024> meta("Mode = Small, IsEnable=%s", is_enable);
    }
    {
        sead::FormatFixedSafeString<1024> meta("Size of last binary loaded:%d[byte]", mResFileSize);
    }
}

/**
 * Checks whether an archive contains every list and object of this IO.
 * @param archive archive to check
 * @param checkValues forwarded to isComplete
 * @return whether the archive is complete
 */
bool IParameterIO::isCompleteArchive(ResParameterArchive archive, bool checkValues) const
{
    return isComplete(archive.getRootList(), checkValues);
}

/**
 * Handles the save and load host IO events of this IO.
 * @param pReflexible reflexible that received the event
 * @param pEvent property event
 * @return 1 when saved, 2 when loaded, 0 otherwise
 */
s32 IParameterIO::listenPropertyEventIO(sead::hostio::Reflexible* pReflexible,
                                        const sead::hostio::PropertyEvent* pEvent)
{
    const uintptr_t id = reinterpret_cast<uintptr_t>(pEvent->getId());
    const uintptr_t self = reinterpret_cast<uintptr_t>(this);
    if (id == self)
    {
        if (save(sead::SafeString::cEmptyString, 0x2000000))
        {
            return 1;
        }
    }
    if (id == self + 1)
    {
        if (save(mPath, 0x2000000))
        {
            return 1;
        }
    }
    if (id == self + 2)
    {
        load(sead::SafeString::cEmptyString, (_21c >> 1) & 1);
        return 2;
    }
    return 0;
}

/**
 * Destroys the parameter IO.
 */
IParameterIO::~IParameterIO() = default;

}  // namespace agl::utl
