#include "utility/aglNamedObjIndex.h"

#include <hostio/seadHostIOPropertyEvent.h>

#include "utility/aglNamedObjMgr.h"

namespace agl::utl {

/**
 * Constructs an unbound index with an empty name.
 */
INamedObjIndex::INamedObjIndex()
{
    mIndex = cIndexNone;
    mValue = sead::SafeString::cEmptyString;
}

/**
 * Constructs an unbound index as a named parameter.
 * @param rValue initial object name
 * @param rName parameter name
 * @param rLabel parameter label
 * @param pObj parameter object the parameter belongs to
 */
INamedObjIndex::INamedObjIndex(const sead::FixedSafeString<32>& rValue,
                               const sead::SafeString& rName, const sead::SafeString& rLabel,
                               IParameterObj* pObj)
    : Parameter(rValue, rName, rLabel, pObj)
{
    mIndex = cIndexNone;
    mValue = sead::SafeString::cEmptyString;
}

/**
 * Unbinds the index from its manager.
 */
INamedObjIndex::~INamedObjIndex()
{
    if (mListNode.isLinked() && mMgr)
    {
        mMgr->mIndexList.erase(this);
        mMgr = nullptr;
    }
}

/**
 * Binds the index to a named object manager.
 * @param pMgr manager to bind to, or nullptr to unbind
 */
void INamedObjIndex::bind(INamedObjMgr* pMgr)
{
    if (mMgr == pMgr)
    {
        return;
    }

    if (mMgr)
    {
        mMgr->mIndexList.erase(this);
    }

    mMgr = pMgr;
    if (pMgr)
    {
        pMgr->mIndexList.pushBack(this);
    }
}

/**
 * Updates the object name from the index.
 */
void INamedObjIndex::syncIndexToName()
{
    if (!mMgr)
    {
        return;
    }

    if (mIndex >= 0)
    {
        mValue = getNamedObjName(mIndex);
    }
    else if (mIndex != cIndexNotFound)
    {
        mIndex = cIndexNone;
        mValue = sead::SafeString::cEmptyString;
    }

    if (mCallback)
    {
        mCallback->callbackSyncIndexToName(this);
    }
}

/**
 * Updates the index from the object name.
 */
void INamedObjIndex::syncNameToIndex()
{
    if (!mMgr)
    {
        return;
    }

    if (mValue == sead::SafeString::cEmptyString)
    {
        mIndex = cIndexNone;
        mValue = sead::SafeString::cEmptyString;
    }
    else
    {
        mIndex = cIndexNotFound;
        s32 num = getNamedObjNum();
        for (s32 i = 0; i < num; i++)
        {
            if (mValue == getNamedObjName(i))
            {
                mIndex = i;
                break;
            }
        }
    }

    if (mCallback)
    {
        mCallback->callbackSyncNameToIndex(this);
    }
}

/**
 * Generates the host I/O combo box used to select the object.
 * @param pContext host I/O context
 * @param isEnable whether the combo box can be edited
 */
void INamedObjIndex::genComboBoxSelect(sead::hostio::Context* pContext, bool isEnable)
{
    if (!mMgr)
    {
        return;
    }

    sead::FormatFixedSafeString<1024> meta(
        "%s IsEnable = %s", getLabel() == sead::SafeString::cEmptyString ? "Mode=Simple," : "",
        isEnable ? "True" : "False");
    {
        sead::SafeString label = getLabel();
    }

    if (mIndex == cIndexNotFound)
    {
        sead::FormatFixedSafeString<1024> str("%s <not found>", mValue.cstr());
    }

    s32 num = getNamedObjNum();
    for (s32 i = 0; i < num; i++)
    {
        getNamedObjName(i);
    }
}

/**
 * Handles a host I/O property event.
 * @param pEvent property event
 * @return true if the event changed the index
 */
bool INamedObjIndex::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    const void* id = pEvent->getId();
    if ((pEvent->getType() & 2) == 0 && id < &mIndex + 1 && id >= &mIndex)
    {
        syncIndexToName();
        return true;
    }

    return false;
}

}  // namespace agl::utl
