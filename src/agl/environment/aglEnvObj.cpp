#include "environment/aglEnvObj.h"

#include <hostio/seadHostIOPropertyEvent.h>

#include "detail/aglRootNode.h"
#include "environment/aglEnvObjMgr.h"

namespace agl::env {

namespace {

const char* const cMetaInfoIconName[EnvObj::cMetaInfo_Num] = {
    "CIRCLE_RED",
    "CIRCLE_ORENGE",
    "CIRCLE_YELLOW",
    "CIRCLE_PURPLE",
};

const char* const cMetaInfoName[EnvObj::cMetaInfo_Num] = {
    "ライト",
    "フォグ",
    "投影点",
    "その他",
};

s16 sRegistNum;
TypeInfo sTypeInfo[EnvObj::cTypeMax];

}  // namespace

s32 EnvObj::sTypeNum;
EnvObj::TypeData EnvObj::sTypeInfoTable[EnvObj::cTypeMax];

/**
 * Constructs an environment object with its common parameters.
 */
EnvObj::EnvObj()
    : mEnable(false, "enable", "有効", this),
      mEnvObjName(sead::FixedSafeString<32>("env"), "name", "オブジェクト名",
                  "AcceptReturn = False", this),
      mGroupName(sead::FixedSafeString<32>(utl::INamedObj::getDefaultGroupName()), "group",
                 "グループ名", "AcceptReturn = False", this)
{
}

/**
 * Destroys the environment object.
 */
EnvObj::~EnvObj() = default;

/**
 * Binds the object to its manager and initializes it.
 * @param index index of the object within its type
 * @param viewNum number of views
 * @param pMgr manager owning the object
 * @param pHeap heap to allocate from
 */
void EnvObj::initialize_(s32 index, s32 viewNum, EnvObjMgr* pMgr, sead::Heap* pHeap)
{
    mMgr = pMgr;
    mIndex = index;
    getEnvObjName();
    detail::RootNode::setNodeMeta(
        this, sead::FormatFixedSafeString<1024>(
                  "Icon = %s", cMetaInfoIconName[sTypeInfoTable[getTypeID()].mMetaInfo]));
    initialize(viewNum, pHeap);
    clear_();
}

/**
 * Resets the name, the group and the enable flag of the object.
 */
void EnvObj::clear_()
{
    becomeDefaultName_();
    setGroupNameCopy(utl::INamedObj::getDefaultGroupName());
    setEnable(false);
}

/**
 * Names the object after its type and index.
 */
void EnvObj::becomeDefaultName_()
{
    mEnvObjName->format("%s%d", sTypeInfoTable[getTypeID()].mName, mIndex);
}

/**
 * Sets the group of the object.
 * @param rName group name
 */
void EnvObj::setGroupNameCopy(const sead::SafeString& rName)
{
    mGroupName->copy(rName);
    mMgr->setDirty();
}

/**
 * Enables or disables the object.
 * @param enable whether the object is enabled
 */
void EnvObj::setEnable(bool enable)
{
    if (isEnable() ^ enable)
    {
        *mEnable = enable;
        mMgr->setDirty();
    }
}

/**
 * Sets the name of the object.
 * @param rName object name
 */
void EnvObj::setEnvObjNameCopy(const sead::SafeString& rName)
{
    mEnvObjName->copy(rName);
    mMgr->setDirty();
}

/**
 * Sets whether the object can be edited from host I/O.
 * @param editable whether the object can be edited
 */
void EnvObj::setEditable(bool editable)
{
    if (mFlag.isOn(1) != editable)
    {
        mFlag.change(1, editable);
        mMgr->setListDirty();
    }
}

/**
 * Copies the parameters of another object of the same type.
 * @param rOther object to copy from
 */
void EnvObj::copyFrom(const EnvObj& rOther)
{
    copyFromImpl_(rOther);
}

/**
 * Copies the type specific parameters of another object of the same type.
 * @param rOther object to copy from
 */
void EnvObj::copyFromImpl_(const EnvObj& rOther)
{
    utl::ParameterBase* pFirst = mGroupName.getNext();

    if (pFirst != nullptr)
    {
        copy(pFirst, mParamListTail, rOther.mGroupName.getNext(), rOther.mParamListTail);
    }
}

/**
 * Updates the object.
 */
void EnvObj::update_()
{
    update();
    mFlag.reset(1 << 5);
}

/**
 * Draws the debug view of the object if it is enabled.
 * @param pDrawContext draw context
 * @param rViewMtx view matrix
 * @param rProjMtx projection matrix
 * @param viewIndex index of the view
 */
void EnvObj::drawDebug_(DrawContext* pDrawContext, const sead::Matrix34f& rViewMtx,
                        const sead::Matrix44f& rProjMtx, s32 viewIndex) const
{
    if (mFlag.isOn(1 << 3))
    {
        drawDebug(pDrawContext, rViewMtx, rProjMtx, viewIndex);
    }
}

/**
 * Draws the debug view of a directional light.
 * @param pDrawContext draw context
 * @param viewIndex index of the view
 * @param rDir light direction
 * @param rColor0 first color
 * @param rColor1 second color
 * @param rColor2 third color
 */
void EnvObj::drawDirectionalLight(DrawContext* pDrawContext, s32 viewIndex,
                                  const sead::Vector3f& rDir, const sead::Color4f& rColor0,
                                  const sead::Color4f& rColor1,
                                  const sead::Color4f& rColor2) const
{
    mFlag.set(1 << 5);
    mMgr->drawDirectionalLight_(pDrawContext, viewIndex, *this, rDir, rColor0, rColor1, rColor2);
}

/**
 * Draws the debug view of a point light.
 * @param pDrawContext draw context
 * @param viewIndex index of the view
 * @param rPos light position
 * @param radius light radius
 * @param rColor0 first color
 * @param rColor1 second color
 */
void EnvObj::drawPointLight(DrawContext* pDrawContext, s32 viewIndex, const sead::Vector3f& rPos,
                            f32 radius, const sead::Color4f& rColor0,
                            const sead::Color4f& rColor1) const
{
    mMgr->drawPointLight_(pDrawContext, viewIndex, *this, rPos, radius, rColor0, rColor1);
}

/**
 * Draws the debug view of a fog.
 * @param pDrawContext draw context
 * @param viewIndex index of the view
 * @param start fog start distance
 * @param end fog end distance
 * @param rDir fog direction
 * @param rColor fog color
 */
void EnvObj::drawFog(DrawContext* pDrawContext, s32 viewIndex, f32 start, f32 end,
                     const sead::Vector3f& rDir, const sead::Color4f& rColor) const
{
    mMgr->drawFog_(pDrawContext, viewIndex, *this, start, end, rDir, rColor);
}

/**
 * Generates the host I/O messages of the object.
 * @param pContext host I/O context
 */
void EnvObj::genMessage(sead::hostio::Context* pContext)
{
    mEnable.genMessageParameter(pContext, mEnable.getMeta());
    mEnvObjName.genMessageParameter(pContext, mEnvObjName.getMeta());
    mGroupName.genMessageParameter(pContext, mGroupName.getMeta());
    getTypeID();

    for (auto it = mMgr->begin(getObjType()), itEnd = mMgr->end(getObjType()); it != itEnd;
         ++it)
    {
        if (*it != this)
        {
            (*it)->getEnvObjName();
        }
    }

    sead::FormatFixedSafeString<1024> str("GroupHeader = %s settings, IsEnable = %s",
                                          sTypeInfoTable[getTypeID()].mLabel,
                                          *mEnable ? "true" : "false");
}

/**
 * Handles a host I/O property event.
 * @param pEvent property event
 */
void EnvObj::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    switch (reinterpret_cast<uintptr_t>(pEvent->getId()))
    {
    case 1000:
        mMgr->setListDirty();
        break;
    case 1001:
        becomeDefaultName_();
        mMgr->setListDirty();
        break;
    case 1002:
        setGroupNameCopy(utl::INamedObj::getDefaultGroupName());
        mMgr->setListDirty();
        break;
    case 1004:
    {
        mMgr->getEnvObjSet().setSelectedObj(sTypeInfoTable[getTypeID()].mMetaInfo, this);

        for (auto& rpObj : mMgr->mObj)
        {
            if (sTypeInfoTable[rpObj->getTypeID()].mMetaInfo ==
                sTypeInfoTable[getTypeID()].mMetaInfo)
            {
                rpObj->mFlag.change(1 << 1, rpObj != this);
            }
        }

        break;
    }
    case 1005:
    {
        mMgr->getEnvObjSet().setSelectedObj(sTypeInfoTable[getTypeID()].mMetaInfo, nullptr);

        for (auto& rpObj : mMgr->mObj)
        {
            if (sTypeInfoTable[rpObj->getTypeID()].mMetaInfo ==
                sTypeInfoTable[getTypeID()].mMetaInfo)
            {
                rpObj->mFlag.reset(1 << 1);
            }
        }

        break;
    }
    case 1007:
        if (mCopySrcIndex != -1)
        {
            EnvObjMgr* pMgr = mMgr;
            s32 type = getObjType();

            if (mCopySrcIndex >= 0 && mCopySrcIndex < pMgr->getObjNum(type))
            {
                const EnvObj* pSrc = pMgr->getObj(type, mCopySrcIndex);

                if (pSrc != nullptr)
                {
                    copyFrom(*pSrc);
                }
            }
        }

        break;
    default:
        break;
    }

    const void* id = pEvent->getId();

    if ((pEvent->getType() & 2) == 0 && id < &*mEnable + 1 && id >= &*mEnable)
    {
        mMgr->setDirty();
    }

    listenPropertyEventParameter(this, pEvent);
}

/**
 * Handles a host I/O node event (no-op in release builds).
 * @param pEvent node event
 */
void EnvObj::listenNodeEvent(const sead::hostio::NodeEvent* pEvent) {}

/**
 * Registers a type of environment object.
 * @param rName type name
 * @param rLabel type label
 * @param createFunc function creating an object of the type
 * @param metaInfo category of the type
 * @param priority sort priority of the type
 * @return type information of the registered type
 */
const TypeInfo* EnvObj::registClass(const sead::SafeString& rName, const sead::SafeString& rLabel,
                                    CreateFunc createFunc, MetaInfo metaInfo, u32 priority)
{
    s32 index = sRegistNum++;
    sTypeNum = sRegistNum;

    TypeData& rData = sTypeInfoTable[index];
    rData.mName = rName.cstr();
    rData.mLabel = rLabel.cstr();
    rData.mCreateFunc = createFunc;
    rData.mMetaInfo = metaInfo;
    rData.mIndex = index;
    rData.mPriority = priority;
    TypeInfo* pInfo = &sTypeInfo[index];
    pInfo->id = index;

    for (s32 i = index; i >= 1; i--)
    {
        TypeData& rCur = sTypeInfoTable[i];
        TypeData& rPrev = sTypeInfoTable[i - 1];

        if (rCur.mMetaInfo >= rPrev.mMetaInfo)
        {
            if (rCur.mMetaInfo != rPrev.mMetaInfo)
            {
                break;
            }

            if (sead::SafeString(rCur.mName).compare(sead::SafeString(rPrev.mName)) >= 0)
            {
                break;
            }
        }

        sTypeInfo[rCur.mIndex].id--;
        sTypeInfo[rPrev.mIndex].id++;
        TypeData tmp = rPrev;
        rPrev = rCur;
        rCur = tmp;
    }

    return pInfo;
}

/**
 * Searches a registered type by name.
 * @param rName type name
 * @return type index, or -1 if the type is not registered
 */
s32 EnvObj::searchTypeIndex(const sead::SafeString& rName)
{
    for (s32 i = 0; i < sTypeNum; i++)
    {
        if (rName == sead::SafeString(sTypeInfoTable[i].mName))
        {
            return i;
        }
    }

    return -1;
}

/**
 * Gets the display name of a category of types.
 * @param metaInfo category
 * @return category name
 */
sead::SafeString EnvObj::getMetaInfoName(MetaInfo metaInfo)
{
    return cMetaInfoName[metaInfo];
}

/**
 * Gets the name of an object of the indexed type.
 * @param index index of the object
 * @return object name
 */
const sead::SafeString& EnvObj::Index::getNamedObjName(s32 index) const
{
    return mMgr->getNamedObjName(index, mType);
}

/**
 * Gets the number of objects of the indexed type.
 * @return number of objects
 */
s32 EnvObj::Index::getNamedObjNum() const
{
    return mMgr->getNamedObjNum(mType);
}

/**
 * Initializes the type specific resources of the object (no-op by default).
 * @param viewNum number of views
 * @param pHeap heap to allocate from
 */
void EnvObj::initialize(s32 viewNum, sead::Heap* pHeap) {}

}  // namespace agl::env
