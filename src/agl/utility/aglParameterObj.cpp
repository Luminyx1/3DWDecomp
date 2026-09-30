#include "utility/aglParameterObj.h"
#include <basis/seadRawPrint.h>
#include <prim/seadFormatPrint.h>
#include <xml/seadXmlElement.h>
#include <xml/seadXmlUtil.h>
#include "detail/aglPrivateResource.h"
#include "utility/aglParameter.h"
#include "utility/aglParameterList.h"

namespace agl::utl
{

IParameterObj::IParameterObj() = default;

/**
 * Appends a parameter to the end of this object's parameter list.
 * @param pNode parameter to append
 */
void IParameterObj::pushBackListNode(ParameterBase* pNode)
{
    SEAD_ASSERT(pNode != nullptr);

    ParameterBase** ptr;

    if (mParamListTail)
    {
        ptr = &mParamListTail->mNext;
    }
    else
    {
        ptr = &mParamListHead;
        mParamListTail = pNode;
    }

    *ptr = pNode;
    mParamListTail = pNode;
    ++mParamListSize;
}

/**
 * Creates this object's XML element as a child of the given element.
 * @param pElement parent element
 * @param pHeap heap used for the element and its attributes
 * @return the created element
 */
sead::XmlElement* IParameterObj::createAttribute(sead::XmlElement* pElement,
                                                 sead::Heap* pHeap) const
{
    sead::XmlElement* element =
        sead::XmlUtil::createBackChildAndSetupElement(pElement, getTagName(), "", pHeap);
    element->expandAttributeList(1, pHeap);
    element->addAttribute(ParameterBase::getAttributeNameString(), getParameterObjName(), pHeap);
    return element;
}

/**
 * Returns the XML tag used for parameter objects.
 * @return the tag name
 */
const char* IParameterObj::getTagName()
{
    return "param_array";
}

/**
 * Returns the object's name (empty in release builds).
 * @return the object name
 */
sead::SafeString IParameterObj::getParameterObjName() const
{
#ifdef SEAD_DEBUG
    return mName;
#else
    return sead::SafeString::cEmptyString;
#endif
}

/**
 * Writes this object and all of its parameters to XML.
 * @param pElement parent element
 * @param pHeap heap used for the created elements
 */
void IParameterObj::writeToXML(sead::XmlElement* pElement, sead::Heap* pHeap) const
{
    if (!preWrite_())
    {
        return;
    }

    sead::XmlElement* element = createAttribute(pElement, pHeap);

    for (auto* param = mParamListHead; param; param = param->mNext)
    {
        param->writeToXML(element, pHeap);
    }

    postWrite_();
}

/**
 * Reads all parameters of this object from XML.
 * @param rElement element holding the parameters
 * @param x forwarded to ParameterBase::readFromXML
 * @return number of parameters read, or -1 on a parse error
 */
s32 IParameterObj::readFromXML(const sead::XmlElement& rElement, bool x)
{
    if (!preRead_())
    {
        return 0;
    }

    s32 count = 0;

    for (auto* param = mParamListHead; param; param = param->mNext)
    {
        const s32 result = param->readFromXML(rElement, x);

        if (result == 0)
        {
            ++count;
        }
        else if (result == 1)
        {
            return -1;
        }
    }

    postRead_();
    return count;
}

/**
 * Applies one resource object, or the interpolation of two, to this object.
 * @param obj1 resource object used at t = 0
 * @param obj2 resource object used at t = 1
 * @param t interpolation factor
 * @param pList owning list notified about parameters that cannot be applied
 */
void IParameterObj::applyResParameterObj(ResParameterObj obj1, ResParameterObj obj2, f32 t,
                                         IParameterList* pList)
{
    if (obj1.ptr() && t <= 0.0f)
    {
        applyResParameterObj_(false, obj1, {}, 0.0f, pList);
        return;
    }

    if (obj2.ptr() && t >= 1.0f)
    {
        applyResParameterObj_(false, obj2, {}, 0.0f, pList);
        return;
    }

    applyResParameterObj_(true, obj1, obj2, t, pList);
}

/**
 * Applies resource parameters to every parameter of this object.
 * @param interpolate whether interpolatable parameters blend obj1 and obj2
 * @param obj1 resource object used at t = 0
 * @param obj2 resource object used at t = 1
 * @param t interpolation factor
 * @param pList owning list notified about parameters that cannot be applied
 */
void IParameterObj::applyResParameterObj_(bool interpolate, ResParameterObj obj1,
                                          ResParameterObj obj2, f32 t, IParameterList* pList)
{
    if (!preRead_())
    {
        return;
    }

    for (auto* param = mParamListHead; param; param = param->mNext)
    {
        const u32 hash = param->getNameHash();

        ResParameter res1{};

        if (obj1.ptr())
        {
            const s32 idx = obj1.searchIndex(hash);

            if (idx != -1)
            {
                res1 = obj1.getResParameter(idx);
            }
        }

        if (interpolate && param->isInterpolatable())
        {
            ResParameter res2{};

            if (obj2.ptr())
            {
                const s32 idx = obj2.searchIndex(hash);

                if (idx != -1)
                {
                    res2 = obj2.getResParameter(idx);
                }
            }

            if (res1.ptr() && res2.ptr())
            {
                param->makeZero();
                param->applyResource(res1, 1.0f - t);
                param->applyResource(res2, t);
            }
            else if (pList)
            {
                pList->callbackNotInterpolatable_(this, param, obj1, obj2, res1, res2, t);

                if (pList->mParent)
                {
                    pList->mParent->callbackNotInterpolatable_(this, param, obj1, obj2, res1, res2,
                                                               t);
                }
            }
        }
        else if (res1.ptr())
        {
            param->applyResource(res1);
        }
        else if (pList)
        {
            pList->callbackNotAppliable_(this, param, obj1);

            if (pList->mParent)
            {
                pList->mParent->callbackNotAppliable_(this, param, obj1);
            }
        }
    }

    postRead_();
}

/**
 * Checks whether a resource object provides every parameter of this object.
 * @param obj resource object to check
 * @param checkValues whether the resource values must also equal the current values
 * @return whether the resource object is complete
 */
bool IParameterObj::isComplete(ResParameterObj obj, bool checkValues) const
{
    if (!obj.ptr() || obj.getNum() != mParamListSize)
    {
        return false;
    }

    for (auto* param = mParamListHead; param; param = param->mNext)
    {
        const auto idx = obj.searchIndex(param->getNameHash());

        if (idx == -1)
        {
            return false;
        }

        if (checkValues)
        {
            const auto res = obj.getResParameter(idx);

            if (ParameterType(res.ptr()->getType()) == ParameterType::Bool)
            {
                const bool value = *res.getData<u32>() != 0;

                if (value != *param->ptrT<bool>())
                {
                    return false;
                }
            }
            else
            {
                if (sead::MemUtil::compare(param->ptr(), res.getData<void>(), res.getDataSize()))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

/**
 * Checks that no two parameters of this object share a name hash.
 * @return whether all hashes are unique
 */
bool IParameterObj::verify() const
{
    bool ret = true;

    for (auto* param = mParamListHead; param; param = param->mNext)
    {
        ret &= verify(param, param->mNext);
    }

    return ret;
}

/**
 * Checks that no parameter from pOther onwards shares pCheck's name hash.
 * @param pCheck parameter whose hash is checked
 * @param pOther first parameter to compare against
 * @return whether no collision was found
 */
bool IParameterObj::verify(ParameterBase* pCheck, ParameterBase* pOther) const
{
    bool ok = true;

    for (auto* param = pOther; param; param = param->mNext)
    {
        if (pCheck->getNameHash() == param->getNameHash())
        {
            ok = false;
        }
    }

    return ok;
}

/**
 * Finds a parameter by name hash.
 * @param hash name hash to look for
 * @return the parameter, or nullptr if not found
 */
ParameterBase* IParameterObj::searchParameter_(u32 hash)
{
    for (auto* param = mParamListHead; param; param = param->mNext)
    {
        if (param->getNameHash() == hash)
        {
            return param;
        }
    }

    return nullptr;
}

/**
 * Finds a parameter by name hash.
 * @param hash name hash to look for
 * @return the parameter, or nullptr if not found
 */
ParameterBase* IParameterObj::searchParameter_(u32 hash) const
{
    for (auto* param = mParamListHead; param; param = param->mNext)
    {
        if (param->getNameHash() == hash)
        {
            return param;
        }
    }

    return nullptr;
}

/**
 * Copies a range of parameters, surrounded by the copy hooks.
 * @param pFirst first destination parameter
 * @param pLast destination parameter to stop at
 * @param pSrcFirst first source parameter
 * @param pSrcLast source parameter to stop at
 */
void IParameterObj::copy(ParameterBase* pFirst, ParameterBase* pLast,
                         const ParameterBase* pSrcFirst, const ParameterBase* pSrcLast)
{
    if (!preCopy_())
    {
        return;
    }

    copy_(pFirst, pLast, pSrcFirst, pSrcLast);

    postCopy_();
}

/**
 * Copies a range of parameters pairwise.
 * @param pFirst first destination parameter
 * @param pLast destination parameter to stop at
 * @param pSrcFirst first source parameter
 * @param pSrcLast source parameter to stop at
 */
void IParameterObj::copy_(ParameterBase* pFirst, ParameterBase* pLast,
                          const ParameterBase* pSrcFirst, const ParameterBase* pSrcLast)
{
    auto target = pFirst;
    auto source = pSrcFirst;

    while (target != pLast && source != pSrcLast)
    {
        const bool result = target->copy(*source);
        SEAD_ASSERT(result);
        target = target->mNext;
        source = source->mNext;
    }
}

/**
 * Copies the values of another object, starting at this object's first parameter.
 * @param rObj object to copy from
 */
void IParameterObj::copy(const IParameterObj& rObj)
{
    if (!preCopy_())
    {
        return;
    }

    auto* head = mParamListHead;
    SEAD_ASSERT(head != nullptr);

    auto* src = rObj.mParamListHead;

    while (src && src->getNameHash() != head->getNameHash())
    {
        src = src->mNext;
    }

    while (src && head)
    {
        const bool result = head->copy(*src);
        SEAD_ASSERT(result);
        head = head->mNext;

        if (!head)
        {
            break;
        }

        src = src->mNext;
    }

    postCopy_();
}

/**
 * Interpolates a range of parameters between two source ranges.
 * @param pFirst first destination parameter
 * @param pLast destination parameter to stop at
 * @param pSrc1First first parameter of the range used at t = 0
 * @param pSrc1Last parameter to stop at in the first range
 * @param pSrc2First first parameter of the range used at t = 1
 * @param pSrc2Last parameter to stop at in the second range
 * @param t interpolation factor
 */
void IParameterObj::copyLerp(ParameterBase* pFirst, ParameterBase* pLast,
                             const ParameterBase* pSrc1First, const ParameterBase* pSrc1Last,
                             const ParameterBase* pSrc2First, const ParameterBase* pSrc2Last,
                             f32 t)
{
    if (!preCopy_())
    {
        return;
    }

    if (t <= 0.0)
    {
        copy_(pFirst, pLast, pSrc1First, pSrc1Last);
    }
    else if (t >= 1.0)
    {
        copy_(pFirst, pLast, pSrc2First, pSrc2Last);
    }
    else
    {
        copyLerp_(pFirst, pLast, pSrc1First, pSrc1Last, pSrc2First, pSrc2Last, t);
    }

    postCopy_();
}

void IParameterObj::copyLerp(const IParameterObj& rObj1, const IParameterObj& rObj2, f32 t)
{
    if (!preCopy_())
    {
        return;
    }

    auto* head = mParamListHead;
    SEAD_ASSERT(head);

    const u32 hash = head->getNameHash();

    auto* it1 = rObj1.mParamListHead;

    while (it1 && it1->getNameHash() != hash)
    {
        it1 = it1->mNext;
    }

    auto* it2 = rObj2.mParamListHead;

    while (it2 && it2->getNameHash() != hash)
    {
        it2 = it2->mNext;
    }

    while (head && it2 && it1)
    {
        const bool result = head->copyLerp(*it1, *it2, t);
        SEAD_ASSERT(result);
        it2 = it2->mNext;

        if (!it2)
        {
            break;
        }

        head = head->mNext;

        if (!head)
        {
            break;
        }

        it1 = it1->mNext;
    }

    postCopy_();
}

/**
 * Interpolates a range of parameters pairwise between two source ranges.
 * @param pFirst first destination parameter
 * @param pLast destination parameter to stop at
 * @param pSrc1First first parameter of the range used at t = 0
 * @param pSrc1Last parameter to stop at in the first range
 * @param pSrc2First first parameter of the range used at t = 1
 * @param pSrc2Last parameter to stop at in the second range
 * @param t interpolation factor
 */
void IParameterObj::copyLerp_(ParameterBase* pFirst, ParameterBase* pLast,
                              const ParameterBase* pSrc1First, const ParameterBase* pSrc1Last,
                              const ParameterBase* pSrc2First, const ParameterBase* pSrc2Last,
                              f32 t)
{
    auto* it = pFirst;
    auto* src1 = pSrc1First;
    auto* src2 = pSrc2First;

    while (it != pLast && src1 != pSrc1Last && src2 != pSrc2Last)
    {
        const bool result = it->copyLerp(*src1, *src2, t);
        SEAD_ASSERT(result);
        it = it->mNext;
        src1 = src1->mNext;
        src2 = src2->mNext;
    }
}

void IParameterObj::sortByHash()
{
    if (mParamListSize == 0)
    {
        return;
    }

    sead::PtrArray<ParameterBase> array;
    array.allocBuffer(mParamListSize, detail::PrivateResource::instance()->getWorkHeap());

    for (auto* param = mParamListHead; param; param = param->mNext)
    {
        array.pushBack(param);
    }

    heapSortByHash(array);

    mParamListHead = nullptr;
    mParamListTail = nullptr;

    for (auto it = array.begin(); it != array.end(); ++it)
    {
        if (!mParamListHead)
        {
            mParamListHead = &*it;
        }

        it->mNext = nullptr;

        if (mParamListTail)
        {
            mParamListTail->mNext = &*it;
        }

        mParamListTail = &*it;
    }

    array.freeBuffer();
}

/**
 * Generates host IO messages for the parameters (empty in release builds).
 * @param pContext host IO context
 */
void IParameterObj::genMessageParameter(sead::hostio::Context* pContext) {}

/**
 * Handles host IO property events for the parameters (empty in release builds).
 * @param pReflexible node that received the event
 * @param pEvent property event
 */
void IParameterObj::listenPropertyEventParameter(sead::hostio::Reflexible* pReflexible,
                                                 const sead::hostio::PropertyEvent* pEvent)
{
}

}  // namespace agl::utl
