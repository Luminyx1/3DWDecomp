#include "environment/aglEnvObjSet.h"

#include <hostio/seadHostIOPropertyEvent.h>

#include "detail/aglRootNode.h"
#include "environment/aglEnvObjMgr.h"

namespace agl::env {

/**
 * Constructs an empty set.
 */
EnvObjSet::EnvObjSet()
{
    detail::RootNode::setNodeMeta(this, "Icon=CIRCLE_GREEN");
}

/**
 * Frees the references and unbinds the manager.
 */
EnvObjSet::~EnvObjSet()
{
    mRef.freeBuffer();
    bind(nullptr);
}

/**
 * Binds the set to a manager and syncs every reference.
 * @param pMgr manager to bind, or nullptr
 */
void EnvObjSet::bind(EnvObjMgr* pMgr)
{
    mMgr = pMgr;
    for (auto& rRef : mRef)
    {
        rRef.mIndex.bind(pMgr);
    }
}

void EnvObjSet::allocBuffer(const AllocateArg& rArg, sead::Heap* pHeap)
{
    EnvObjBuffer::allocBuffer(rArg, pHeap);
    mRef.allocBufferAssert(rArg.getTotal(), pHeap);

    s32 index = 0;
    for (s32 type = 0; type < EnvObj::sTypeNum; type++)
    {
        s32 i = 0;
        for (; i < mTypeRange[type].mNum; i++)
        {
            Ref& rRef = mRef[index + i];
            rRef.mIndex.setType(type);
            rRef.mIndex.setCallback(this);
        }

        index += i;
    }

    mSetName.init(sead::FixedSafeString<32>("EnvSet"), "name", "名前", "", &mRefObj);

    for (s32 i = 0; i < mRef.size(); i++)
    {
        Ref& rRef = mRef(i);
        s32 type = searchType(i);
        rRef.mTypeName.init(sead::FixedSafeString<32>(EnvObj::getTypeData(type).mName), "type",
                            "タイプ", "", &rRef);
        rRef.mIndex.init(sead::FixedSafeString<32>(sead::SafeString::cEmptyString), "name", "名前",
                         "", &rRef);
        mRefList.addObj(&rRef, sead::FormatFixedSafeString<1024>("%d", i));
    }

    addObj(&mRefObj, "setting");
    addList(&mRefList, "env_obj_ref_array");
}

/**
 * Adds an object to the first free reference.
 * @param pObj object to add
 * @return true if the object was added
 */
bool EnvObjSet::pushBack(EnvObj* pObj)
{
    if (!pObj)
    {
        return false;
    }

    for (s32 i = 0; i < mTypeRange[pObj->getTypeID()].mNum; i++)
    {
        s32 index = mTypeRange[pObj->getTypeID()].mStart + i;
        EnvObj::Index& rIndex = mRef[index].mIndex;
        if (!(rIndex.getIndex() >= 0 || rIndex.getIndex() == utl::INamedObjIndex::cIndexNotFound))
        {
            mObj[index] = pObj;
            rIndex->copy(pObj->getEnvObjName());
            rIndex.syncNameToIndex();
            return true;
        }
    }

    return false;
}

/**
 * Removes the reference to an object.
 * @param pObj object to remove
 * @return true if the object was found
 */
bool EnvObjSet::erase(EnvObj* pObj)
{
    if (!pObj)
    {
        return false;
    }

    for (s32 i = 0; i < mTypeRange[pObj->getTypeID()].mNum; i++)
    {
        if (tryGetObj(pObj->getTypeID(), i) == pObj)
        {
            s32 index = mTypeRange[pObj->getTypeID()].mStart + i;
            mObj[index] = nullptr;
            Ref& rRef = mRef[index];
            rRef.mIndex.setIndex(utl::INamedObjIndex::cIndexNone);
            rRef.mIndex->copy(sead::SafeString::cEmptyString);
            return true;
        }
    }

    return false;
}

/**
 * Clears every reference before reading.
 */
bool EnvObjSet::preRead_()
{
    for (auto& rRef : mRef)
    {
        rRef.mIndex.setIndex(utl::INamedObjIndex::cIndexNone);
        rRef.mIndex->copy(sead::SafeString::cEmptyString);
    }

    return true;
}

/**
 * Resolves every reference after reading.
 */
void EnvObjSet::postRead_()
{
    for (auto& rRef : mRef)
    {
        rRef.mIndex.syncNameToIndex();
    }
}

void EnvObjSet::syncIndex_(utl::INamedObjIndex* pIndex)
{
    for (auto it = mRef.begin(); it != mRef.end(); ++it)
    {
        if (&it->mIndex == pIndex)
        {
            s32 index = pIndex->getIndex();
            if (index < 0)
            {
                mObj[it.getIndex()] = nullptr;
            }
            else
            {
                mObj[it.getIndex()] =
                    mMgr->tryGetObj(static_cast<EnvObj::Index*>(pIndex)->getType(), index);
            }

            return;
        }
    }
}

void EnvObjSet::callbackSyncNameToIndex(utl::INamedObjIndex* pIndex)
{
    syncIndex_(pIndex);
}

void EnvObjSet::callbackSyncIndexToName(utl::INamedObjIndex* pIndex)
{
    syncIndex_(pIndex);
}

void EnvObjSet::genMessage(sead::hostio::Context* pContext)
{
    sead::FixedSafeString<1024> str;
    bool isEnable = true;
    s32 typeNum = EnvObj::getTypeNum();
    for (s32 type = 0; type < typeNum; type++)
    {
        if (mTypeRange[type].mNum == 0)
        {
            continue;
        }

        for (s32 i = 0; i < mTypeRange[type].mNum; i++)
        {
            str.format("GroupHeader = %s (%d/%d), Dir = Y", EnvObj::getTypeData(type).mLabel,
                       i + 1, mTypeRange[type].mNum);
            s32 index = mTypeRange[type].mStart + i;
            Ref& rRef = mRef[index];
            rRef.mIndex.genComboBoxSelect(pContext, isEnable);
            if (isEnable)
            {
                genMessageEachObj(pContext, index, mObj[index]);
            }

            isEnable &= rRef.isUsed();
        }
    }
}

/**
 * Handles a host IO property event.
 * @param pEvent property event to handle
 */
void EnvObjSet::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    if (reinterpret_cast<uintptr_t>(pEvent->getId()) == 1000)
    {
        return;
    }

    for (auto& rRef : mRef)
    {
        rRef.mIndex.listenPropertyEvent(pEvent);
    }
}

bool EnvObjSet::Ref::isApply_(utl::ResParameterObj obj) const
{
    return *mIndex == sead::SafeString::cEmptyString &&
           mTypeName->isEqual(utl::getResParameter(obj, "type").getData<char>());
}

/**
 * Generates the host IO messages of one object.
 * @param pContext host IO context
 * @param type object type
 * @param pObj object to describe
 */
void EnvObjSet::genMessageEachObj(sead::hostio::Context* pContext, s32 type, const EnvObj* pObj)
{
}

EnvObjSet::Ref::~Ref() = default;

}  // namespace agl::env
