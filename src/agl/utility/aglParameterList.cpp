#include "utility/aglParameterList.h"
#include <basis/seadRawPrint.h>
#include <prim/seadFormatPrint.h>
#include <prim/seadSafeString.h>
#include "utility/aglParameter.h"
#include "utility/aglParameterObj.h"
#include "utility/aglParameterStringMgr.h"
#include <xml/seadXmlElement.h>
#include <xml/seadXmlUtil.h>
#include "detail/aglPrivateResource.h"

namespace agl::utl
{

IParameterList::IParameterList()
{
    setParameterListName_(sead::SafeString::cEmptyString);
}

void IParameterList::setParameterListName_(const sead::SafeString& rName)
{
#ifdef SEAD_DEBUG
    mName = rName;
#endif

    mNameHash = ParameterBase::calcHash(rName);
}

/**
 * Appends a child list and names it.
 * @param pChild list to append
 * @param rName name of the child list
 */
void IParameterList::addList(IParameterList* pChild, const sead::SafeString& rName)
{
    SEAD_ASSERT(pChild != nullptr);
    pChild->setParameterListName_(rName);

    ((mpChildListTail == nullptr) ? mpChildListHead : mpChildListTail->mNext) = pChild;
    mpChildListTail = pChild;
    pChild->mParent = this;
}

/**
 * Appends a child object and names it.
 * @param pChild object to append
 * @param rName name of the child object
 */
void IParameterList::addObj(IParameterObj* pChild, const sead::SafeString& rName)
{
    SEAD_ASSERT(pChild != nullptr);

#ifdef SEAD_DEBUG
    if (ParameterStringMgr::instance())
    {
        pChild->mName = ParameterStringMgr::instance()->appendString(rName);
    }
#endif
    pChild->mNameHash = ParameterBase::calcHash(rName);

    ((mpChildObjTail == nullptr) ? mpChildObjHead : mpChildObjTail->mNext) = pChild;
    mpChildObjTail = pChild;
}

/**
 * Unlinks every child list.
 */
void IParameterList::clearList()
{
    for (auto* i = mpChildListHead; i != nullptr;)
    {
        auto* next = i->mNext;
        i->mNext = nullptr;
        i = next;
    }

    mpChildListHead = nullptr;
    mpChildListTail = nullptr;
}

/**
 * Unlinks every child object.
 */
void IParameterList::clearObj()
{
    for (auto* i = mpChildObjHead; i != nullptr;)
    {
        auto* next = i->mNext;
        i->mNext = nullptr;
        i = next;
    }

    mpChildObjHead = nullptr;
    mpChildObjTail = nullptr;
}

/**
 * Unlinks a child list if it belongs to this list.
 * @param pChild list to remove
 */
void IParameterList::removeList(IParameterList* pChild)
{
    IParameterList* prev = nullptr;

    for (auto* it = mpChildListHead; it != nullptr; it = it->mNext)
    {
        if (it == pChild)
        {
            ((prev != nullptr) ? prev->mNext : mpChildListHead) = pChild->mNext;

            if (pChild->mNext == nullptr)
            {
                mpChildListTail = prev;
            }

            pChild->mNext = nullptr;
            return;
        }

        prev = it;
    }
}

/**
 * Unlinks a child object if it belongs to this list.
 * @param pChild object to remove
 */
void IParameterList::removeObj(IParameterObj* pChild)
{
    IParameterObj* prev = nullptr;

    for (auto* it = mpChildObjHead; it != nullptr; it = it->mNext)
    {
        if (it == pChild)
        {
            ((prev != nullptr) ? prev->mNext : mpChildObjHead) = pChild->mNext;

            if (pChild->mNext == nullptr)
            {
                mpChildObjTail = prev;
            }

            pChild->mNext = nullptr;
            return;
        }

        prev = it;
    }
}

/**
 * Applies a resource list to this list and its children.
 * @param list resource list to apply
 */
void IParameterList::applyResParameterList(ResParameterList list)
{
    return applyResParameterList_(false, list, {}, 0.0);
}

/**
 * Applies one resource list, or the interpolation of two, to this list.
 * @param list1 resource list used at t = 0
 * @param list2 resource list used at t = 1
 * @param t interpolation factor
 */
void IParameterList::applyResParameterList(ResParameterList list1, ResParameterList list2, f32 t)
{
    if (list1.ptr() != nullptr && t <= 0.0)
    {
        return applyResParameterList_(false, list1, {}, 0.0);
    }

    if (list2.ptr() != nullptr && t >= 1.0)
    {
        return applyResParameterList_(false, list2, {}, 0.0);
    }

    return applyResParameterList_(true, list1, list2, t);
}

/**
 * Checks whether a resource list contains exactly the children of this list.
 * @param res resource list to check
 * @param checkValues unused
 * @return whether the resource list is complete
 */
bool IParameterList::isComplete(ResParameterList res, bool checkValues) const
{
    if (res.ptr() == nullptr)
    {
        return false;
    }

    s32 obj_count = 0;

    for (auto* i = mpChildObjHead; i != nullptr; i = i->mNext)
    {
        if (res.searchObjIndex(i->mNameHash) == -1)
        {
            return false;
        }

        ++obj_count;
    }

    s32 list_count = 0;

    for (auto* i = mpChildListHead; i != nullptr; i = i->mNext)
    {
        if (res.searchListIndex(i->mNameHash) == -1)
        {
            return false;
        }

        ++list_count;
    }

    if (obj_count != res.getResParameterObjNum() || list_count != res.getResParameterListNum())
    {
        return false;
    }

    return true;
}

/**
 * Creates this list's XML element as a child of the given element.
 * @param pElement parent element
 * @param pHeap heap used for the element and its attributes
 * @return the created element
 */
sead::XmlElement* IParameterList::createAttribute(sead::XmlElement* pElement,
                                                  sead::Heap* pHeap) const
{
    sead::XmlElement* element =
        sead::XmlUtil::createBackChildAndSetupElement(pElement, getTagName(), "", pHeap);
    element->expandAttributeList(1, pHeap);
    element->addAttribute(ParameterBase::getAttributeNameString(), getParameterListName(),
                          pHeap);
    return element;
}

/**
 * Returns the list's name (empty in release builds).
 * @return the list name
 */
sead::SafeString IParameterList::getParameterListName() const
{
#ifdef SEAD_DEBUG
    return mName;
#else
    return sead::SafeString::cEmptyString;
#endif
}

/**
 * Returns the XML tag used for parameter lists.
 * @return the tag name
 */
const char* IParameterList::getTagName()
{
    return "param_list";
}

/**
 * Writes this list with all child objects and lists to XML.
 * @param pElement parent element
 * @param pHeap heap used for the created elements
 */
void IParameterList::writeToXML(sead::XmlElement* pElement, sead::Heap* pHeap) const
{
    if (!preWrite_())
    {
        return;
    }

    sead::XmlElement* element = createAttribute(pElement, pHeap);

    for (auto* obj = mpChildObjHead; obj != nullptr; obj = obj->mNext)
    {
        obj->writeToXML(element, pHeap);
    }

    for (auto* list = mpChildListHead; list != nullptr; list = list->mNext)
    {
        list->writeToXML(element, pHeap);
    }

    postWrite_();
}

/**
 * Reads all child objects and lists from XML.
 * @param rElement element holding the children
 * @param x forwarded to the parameters' readFromXML
 * @return number of parameters read, or -1 on a parse error
 */
s32 IParameterList::readFromXML(const sead::XmlElement& rElement, bool x)
{
    if (!preRead_())
    {
        return 0;
    }

    s32 count = 0;

    for (const sead::XmlElement* child = rElement.child(); child != nullptr;
         child = child->next())
    {
        const sead::SafeString name =
            child->findAttributeValue(ParameterBase::getAttributeNameString());

        for (auto* obj = mpChildObjHead; obj != nullptr; obj = obj->mNext)
        {
            if (child->getName() != IParameterObj::getTagName())
            {
                continue;
            }

            if (name != obj->getParameterObjName())
            {
                continue;
            }

            const s32 result = obj->readFromXML(*child, x);

            if (result == -1)
            {
                return -1;
            }

            count += result;
            break;
        }

        for (auto* list = mpChildListHead; list != nullptr; list = list->mNext)
        {
            if (child->getName() != getTagName())
            {
                continue;
            }

            if (name != list->getParameterListName())
            {
                continue;
            }

            const s32 result = list->readFromXML(*child, x);

            if (result == -1)
            {
                return -1;
            }

            count += result;
            break;
        }
    }

    postRead_();
    return count;
}

/**
 * Checks recursively that no two siblings share a name hash.
 * @return whether all hashes are unique
 */
bool IParameterList::verify() const
{
    bool ok = verifyList() & verifyObj();

    for (auto* i = mpChildListHead; i != nullptr; i = i->mNext)
    {
        ok &= i->verify();
    }

    for (auto* i = mpChildObjHead; i != nullptr; i = i->mNext)
    {
        ok &= i->verify();
    }

    return ok;
}

/**
 * Checks that no two child lists share a name hash.
 * @return whether all hashes are unique
 */
bool IParameterList::verifyList() const
{
    bool ret = true;

    for (auto* i = mpChildListHead; i != nullptr; i = i->mNext)
    {
        ret &= verifyList(i, i->mNext);
    }

    return ret;
}

/**
 * Checks that no two child objects share a name hash.
 * @return whether all hashes are unique
 */
bool IParameterList::verifyObj() const
{
    bool ret = true;

    for (auto* i = mpChildObjHead; i != nullptr; i = i->mNext)
    {
        ret &= verifyObj(i, i->mNext);
    }

    return ret;
}

/**
 * Checks that no list from pOther onwards shares pCheck's name hash.
 * @param pCheck list whose hash is checked
 * @param pOther first list to compare against
 * @return whether no collision was found
 */
bool IParameterList::verifyList(IParameterList* pCheck, IParameterList* pOther) const
{
    bool ok = true;

    for (auto* it = pOther; it != nullptr; it = it->mNext)
    {
        if (pCheck->getNameHash() == it->getNameHash())
        {
            ok = false;
        }
    }

    return ok;
}

/**
 * Checks that no object from pOther onwards shares pCheck's name hash.
 * @param pCheck object whose hash is checked
 * @param pOther first object to compare against
 * @return whether no collision was found
 */
bool IParameterList::verifyObj(IParameterObj* pCheck, IParameterObj* pOther) const
{
    bool ok = true;

    for (auto* it = pOther; it != nullptr; it = it->mNext)
    {
        if (pCheck->getNameHash() == it->getNameHash())
        {
            ok = false;
        }
    }

    return ok;
}

/**
 * Finds the resource object that applies to a child object.
 * @param res resource list to search
 * @param rObj child object
 * @return the matching resource object, or an empty one
 */
ResParameterObj IParameterList::searchResParameterObj_(ResParameterList res,
                                                       const IParameterObj& rObj) const
{
    if (res.ptr() == nullptr)
    {
        return {};
    }

    for (auto it = res.objBegin(), end = res.objEnd(); it != end; ++it)
    {
        if (rObj.isApply_(*it))
        {
            return *it;
        }
    }

    return {};
}

/**
 * Finds the child object a resource object applies to, starting after the last match.
 * @param res resource object
 * @param pObj child object to start from, or nullptr to start at the head
 * @return the matching child object, or nullptr
 */
IParameterObj* IParameterList::searchChildParameterObj_(ResParameterObj res,
                                                        IParameterObj* pObj) const
{
    if (res.ptr() == nullptr || mpChildObjHead == nullptr)
    {
        return nullptr;
    }

    auto* start = (pObj != nullptr) ? pObj : mpChildObjHead;
    auto* child = start;

    while (!child->isApply_(res))
    {
        child = child->mNext;

        if (child == nullptr)
        {
            child = mpChildObjHead;
        }

        if (child == start)
        {
            return nullptr;
        }
    }

    return child;
}

/**
 * Finds the resource list that applies to a child list.
 * @param res resource list to search
 * @param rList child list
 * @return the matching resource list, or an empty one
 */
ResParameterList IParameterList::searchResParameterList_(ResParameterList res,
                                                         const IParameterList& rList) const
{
    if (res.ptr() == nullptr)
    {
        return {};
    }

    for (auto it = res.listBegin(), end = res.listEnd(); it != end; ++it)
    {
        if (rList.isApply_(it.getList()))
        {
            return it.getList();
        }
    }

    return {};
}

/**
 * Finds the child list a resource list applies to.
 * @param res resource list
 * @return the matching child list, or nullptr
 */
IParameterList* IParameterList::searchChildParameterList_(ResParameterList res) const
{
    if (res.ptr() == nullptr)
    {
        return nullptr;
    }

    for (auto* child = mpChildListHead; child != nullptr; child = child->mNext)
    {
        if (child->isApply_(res))
        {
            return child;
        }
    }

    return nullptr;
}

/**
 * Applies every object of a resource list to the matching child objects.
 * @param interpolate whether the objects are interpolated
 * @param res resource list
 * @param t interpolation factor
 */
void IParameterList::applyResParameterObjB_(bool interpolate, ResParameterList res, f32 t)
{
    if (res.ptr() == nullptr)
    {
        return;
    }

    IParameterObj* obj = nullptr;

    for (auto it = res.objBegin(), end = res.objEnd(); it != end; ++it)
    {
        auto* result = searchChildParameterObj_(*it, obj);

        if (result != nullptr)
        {
            result->applyResParameterObj_(interpolate, {}, *it, t, this);
            obj = result;
        }
    }
}

/**
 * Applies every list of a resource list to the matching child lists.
 * @param interpolate whether the lists are interpolated
 * @param res resource list
 * @param t interpolation factor
 */
void IParameterList::applyResParameterListB_(bool interpolate, ResParameterList res, f32 t)
{
    if (res.ptr() == nullptr)
    {
        return;
    }

    for (auto it = res.listBegin(), end = res.listEnd(); it != end; ++it)
    {
        auto* list = searchChildParameterList_(*it);

        if (list != nullptr)
        {
            list->applyResParameterList_(interpolate, {}, *it, t);
        }
    }
}

/**
 * Applies resource lists recursively to the child objects and lists.
 * @param interpolate whether l1 and l2 are interpolated
 * @param l1 resource list used at t = 0
 * @param l2 resource list used at t = 1
 * @param t interpolation factor
 */
void IParameterList::applyResParameterList_(bool interpolate, ResParameterList l1,
                                            ResParameterList l2, f32 t)
{
    if (!preRead_())
    {
        return;
    }

    if (l1.ptr() != nullptr)
    {
        IParameterObj* obj = nullptr;

        for (auto it = l1.objBegin(), end = l1.objEnd(); it != end; ++it)
        {
            auto* child = searchChildParameterObj_(*it, obj);

            if (child != nullptr)
            {
                const auto obj2 = searchResParameterObj_(l2, *child);
                child->applyResParameterObj_(interpolate, *it, obj2, t, this);
                obj = child;
            }
            else
            {
                applyResParameterObjB_(interpolate, l2, t);
            }
        }
    }
    else
    {
        applyResParameterObjB_(interpolate, l2, t);
    }

    if (l1.ptr() != nullptr)
    {
        for (auto it = l1.listBegin(), end = l1.listEnd(); it != end; ++it)
        {
            auto* child = searchChildParameterList_(*it);

            if (l2.ptr() != nullptr)
            {
                if (child != nullptr)
                {
                    const auto other_list = searchResParameterList_(l2, *child);
                    child->applyResParameterList_(interpolate, *it, other_list, t);
                }
                else
                {
                    applyResParameterListB_(interpolate, l2, t);
                }
            }
            else
            {
                if (child != nullptr)
                {
                    child->applyResParameterList_(interpolate, *it, {}, t);
                }
                else
                {
                    applyResParameterListB_(interpolate, {}, t);
                }
            }
        }
    }
    else
    {
        applyResParameterListB_(interpolate, l2, t);
    }

    postRead_();
}

void IParameterList::sortByHash()
{
    sead::Heap* heap = detail::PrivateResource::instance()->getWorkHeap();

    s32 list_num = 0;

    for (auto* list = mpChildListHead; list != nullptr; list = list->mNext)
    {
        ++list_num;
    }

    s32 obj_num = 0;

    for (auto* obj = mpChildObjHead; obj != nullptr; obj = obj->mNext)
    {
        ++obj_num;
    }

    if (list_num != 0)
    {
        sead::PtrArray<IParameterList> array;
        array.allocBuffer(list_num, heap);

        for (auto* list = mpChildListHead; list != nullptr; list = list->mNext)
        {
            array.pushBack(list);
        }

        heapSortByHash(array);

        mpChildListHead = nullptr;
        mpChildListTail = nullptr;

        for (auto it = array.begin(); it != array.end(); ++it)
        {
            if (mpChildListHead == nullptr)
            {
                mpChildListHead = &*it;
            }

            it->mNext = nullptr;

            if (mpChildListTail != nullptr)
            {
                mpChildListTail->mNext = &*it;
            }

            mpChildListTail = &*it;
        }

        array.freeBuffer();
    }

    if (obj_num != 0)
    {
        sead::PtrArray<IParameterObj> array;
        array.allocBuffer(obj_num, heap);

        for (auto* obj = mpChildObjHead; obj != nullptr; obj = obj->mNext)
        {
            array.pushBack(obj);
        }

        heapSortByHash(array);

        mpChildObjHead = nullptr;
        mpChildObjTail = nullptr;

        for (auto it = array.begin(); it != array.end(); ++it)
        {
            if (mpChildObjHead == nullptr)
            {
                mpChildObjHead = &*it;
            }

            it->mNext = nullptr;

            if (mpChildObjTail != nullptr)
            {
                mpChildObjTail->mNext = &*it;
            }

            mpChildObjTail = &*it;
        }

        array.freeBuffer();
    }
}

void IParameterList::genMessageParameterList(sead::hostio::Context* pContext)
{
    for (auto* obj = mpChildObjHead; obj != nullptr; obj = obj->mNext)
    {
        sead::FormatFixedSafeString<256> meta("GroupHeader=%s", obj->getParameterObjName().cstr());
        obj->genMessageParameter(pContext);
    }

    for (auto* list = mpChildListHead; list != nullptr; list = list->mNext)
    {
        sead::FormatFixedSafeString<256> meta("GroupHeader=%s",
                                              list->getParameterListName().cstr());
        list->genMessageParameterList(pContext);
    }
}

/**
 * Forwards a host IO property event to every child object and list.
 * @param pReflexible node that received the event
 * @param pEvent property event
 */
void IParameterList::listenPropertyEventParameter(sead::hostio::Reflexible* pReflexible,
                                                  const sead::hostio::PropertyEvent* pEvent)
{
    for (auto* obj = mpChildObjHead; obj != nullptr; obj = obj->mNext)
    {
        obj->listenPropertyEventParameter(pReflexible, pEvent);
    }

    for (auto* list = mpChildListHead; list != nullptr; list = list->mNext)
    {
        list->listenPropertyEventParameter(pReflexible, pEvent);
    }
}

}  // namespace agl::utl
