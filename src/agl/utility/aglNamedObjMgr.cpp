#include "utility/aglNamedObjMgr.h"

#include <cstddef>
#include <hostio/seadHostIOPropertyEvent.h>

#include "detail/aglRootNode.h"

namespace agl::utl {

namespace {

s32 compareNamedObj(const INamedObj* pLhs, const INamedObj* pRhs)
{
    if (pLhs->getObjType() < pRhs->getObjType())
    {
        return -1;
    }
    if (pRhs->getObjType() < pLhs->getObjType())
    {
        return 1;
    }
    return pLhs->getObjName().compare(pRhs->getObjName()) < 0 ? -1 : 1;
}

}  // namespace

/**
 * Constructs an empty named object manager.
 */
INamedObjMgr::INamedObjMgr()
{
    mIndexList.initOffset(offsetof(INamedObjIndex, mListNode));
}

/**
 * Frees the object list and the groups.
 */
INamedObjMgr::~INamedObjMgr()
{
    mNamedObj.freeBuffer();
    mGroup.freeBuffer();
    mGroupPtr.freeBuffer();
    mIndexList.clear();
}

/**
 * Allocates the object list and the groups.
 * @param objNum maximum number of named objects
 * @param groupNum maximum number of groups
 * @param pHeap heap to allocate from
 */
void INamedObjMgr::initialize(u32 objNum, u32 groupNum, sead::Heap* pHeap)
{
    mNamedObj.allocBuffer(objNum, pHeap);
    mGroup.tryAllocBuffer(groupNum, pHeap);
    mGroupPtr.allocBuffer(groupNum, pHeap);
    for (auto it = mGroup.begin(), itEnd = mGroup.end(); it != itEnd; ++it)
    {
        it->initialize(it.getIndex(), this, pHeap);
    }
}

/**
 * Initializes a group.
 * @param index index of the group
 * @param pMgr manager owning the group
 * @param pHeap heap to allocate from
 */
void INamedObjMgr::Group::initialize(s32 index, INamedObjMgr* pMgr, sead::Heap* pHeap)
{
    mComment = "";
    mIndex = index;
    mMgr = pMgr;
}

/**
 * Registers a named object.
 * @param pObj object to register
 */
void INamedObjMgr::pushBackNamedObj(INamedObj* pObj)
{
    mNamedObj.pushBack(pObj);
}

/**
 * Unregisters a named object.
 * @param pObj object to unregister
 */
void INamedObjMgr::eraseNamedObj(INamedObj* pObj)
{
    s32 index = 0;
    for (auto& rObj : mNamedObj)
    {
        if (&rObj == pObj)
        {
            mNamedObj.erase(index);
        }
        index++;
    }
}

/**
 * Rebuilds the object list and the bound indices if the list was changed.
 */
void INamedObjMgr::updateList()
{
    if (!mFlag.isOn(1))
    {
        return;
    }

    constructList();
    for (auto& rIndex : mIndexList)
    {
        rIndex.syncNameToIndex();
    }
    mFlag.reset(1);
}

/**
 * Updates the indices bound to the manager from their names.
 */
void INamedObjMgr::syncNameToIndex()
{
    for (auto& rIndex : mIndexList)
    {
        rIndex.syncNameToIndex();
    }
}

/**
 * Sorts the named objects and builds the sorted group list.
 */
void INamedObjMgr::constructList()
{
    mNamedObj.heapSort_<INamedObj>(compareNamedObj);
    mGroupPtr.clear();

    for (auto it = mNamedObj.begin(), itEnd = mNamedObj.end(); it != itEnd; ++it)
    {
        bool isFound = false;
        for (auto itGroup = mGroupPtr.begin(); itGroup != mGroupPtr.end(); ++itGroup)
        {
            if (itGroup->mName == it->getGroupName())
            {
                isFound = true;
                break;
            }
        }
        if (isFound)
        {
            continue;
        }

        mGroup[mGroupPtr.size()].reset(it->getGroupName());
        mGroupPtr.pushBack(&mGroup[mGroupPtr.size()]);
    }

    mGroupPtr.heapSort_<Group>(Group::compare);
}

/**
 * Sets the name of a group.
 * @param rName group name
 */
void INamedObjMgr::Group::reset(const sead::SafeString& rName)
{
    mName = rName;
    if (rName == INamedObj::getDefaultGroupName())
    {
        detail::RootNode::setNodeMeta(this, "Icon=FOLDER_RED");
    }
    else
    {
        detail::RootNode::setNodeMeta(this, "Icon=FOLDER_GREEN");
    }
}

/**
 * Compares two groups by name.
 * @param pLhs first group
 * @param pRhs second group
 * @return -1 if the first group is sorted before the second one, 1 otherwise
 */
s32 INamedObjMgr::Group::compare(const Group* pLhs, const Group* pRhs)
{
    return pLhs->mName.compare(pRhs->mName) < 0 ? -1 : 1;
}

/**
 * Notifies the objects of the current group, or of all objects, that the list was built.
 * @param isAll whether every object is notified
 */
void INamedObjMgr::constructListByName(bool isAll)
{
    for (auto it = mNamedObj.begin(), itEnd = mNamedObj.end(); it != itEnd; ++it)
    {
        if (mCurrentGroup == -1 || mGroup[mCurrentGroup].mName == it->getGroupName())
        {
            it->isHostIOEnabled();
        }
    }
}

/**
 * Notifies the objects of each group that the list was built.
 * @param isAll whether every group is handled
 */
void INamedObjMgr::constructListByGroup(bool isAll)
{
    s32 groupIndex = 0;
    for (auto itGroup = mGroupPtr.begin(); itGroup != mGroupPtr.end(); ++itGroup, ++groupIndex)
    {
        if (mCurrentGroup != -1 && groupIndex != mCurrentGroup)
        {
            continue;
        }
        for (auto it = mNamedObj.begin(), itEnd = mNamedObj.end(); it != itEnd; ++it)
        {
            if (isAll)
            {
                if (itGroup->mName == it->getGroupName())
                {
                    it->isHostIOEnabled();
                }
            }
            else
            {
                if (itGroup->mName == it->getGroupName())
                {
                    it->isHostIOEnabled();
                }
            }
        }
    }
}

/**
 * Generates the host I/O combo box used to select the current group (no-op in release builds).
 * @param pContext host I/O context
 */
void INamedObjMgr::genGroupComboBox(sead::hostio::Context* pContext) {}

/**
 * Generates the host I/O messages of the manager (no-op in release builds).
 * @param pContext host I/O context
 */
void INamedObjMgr::genMessage(sead::hostio::Context* pContext) {}

/**
 * Handles a host I/O property event.
 * @param pEvent property event
 */
void INamedObjMgr::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    const void* id = pEvent->getId();
    if ((pEvent->getType() & 2) == 0 && id < &mCurrentGroup + 1 && id >= &mCurrentGroup)
    {
        mFlag.set(1);
    }
}

/**
 * Constructs an unused group.
 */
INamedObjMgr::Group::Group() = default;

/**
 * Destroys the group.
 */
INamedObjMgr::Group::~Group()
{
    ;
}

/**
 * Generates the host I/O messages of the group.
 * @param pContext host I/O context
 */
void INamedObjMgr::Group::genMessage(sead::hostio::Context* pContext)
{
    {
        sead::FormatFixedSafeString<1024> str("セーブ（グループ\"%s\"）", mName.cstr());
    }
    {
        sead::FormatFixedSafeString<1024> str("ロード（グループ名が\"%s\"のデータ）", mName.cstr());
    }
    if (mMgr->getSaveFilePath() != "")
    {
        sead::FormatFixedSafeString<1024> str("%s", mMgr->getSaveFilePath().cstr());
    }
}

/**
 * Handles a host I/O property event.
 * @param pEvent property event
 */
void INamedObjMgr::Group::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    switch (reinterpret_cast<uintptr_t>(pEvent->getId()))
    {
    case 100491:
        mMgr->listenPropertyEventFromGroup(cGroupEventType_0, this);
        break;
    case 100492:
        mMgr->listenPropertyEventFromGroup(cGroupEventType_1, this);
        break;
    case 100493:
        mMgr->listenPropertyEventFromGroup(cGroupEventType_2, this);
        break;
    case 100502:
        mMgr->listenPropertyEventFromGroup(cGroupEventType_3, this);
        break;
    default:
        break;
    }
}

/**
 * Gets the name of a named object.
 * @param index index of the object
 * @param type object type (unused)
 * @return object name
 */
const sead::SafeString& INamedObjMgr::getNamedObjName(s32 index, s32 type) const
{
    return mNamedObj.unsafeAt(index)->getObjName();
}

/**
 * Gets the number of named objects.
 * @param type object type (unused)
 * @return number of objects
 */
s32 INamedObjMgr::getNamedObjNum(s32 type) const
{
    return mNamedObj.size();
}

/**
 * Gets the path of the file the groups are saved to.
 * @return empty string
 */
const sead::SafeString& INamedObjMgr::getSaveFilePath() const
{
    return sead::SafeString::cEmptyString;
}

/**
 * Handles a host I/O event sent from a group (no-op by default).
 * @param type event type
 * @param pGroup group sending the event
 */
void INamedObjMgr::listenPropertyEventFromGroup(GroupEventType type, Group* pGroup) {}

}  // namespace agl::utl
