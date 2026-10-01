#include "environment/aglEnvObjMgr.h"

#include <gfx/seadGraphicsContext.h>
#include <gfx/seadProjection.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <xml/seadXmlDocument.h>
#include <xml/seadXmlUtil.h>

#include "common/aglDrawContext.h"
#include "detail/aglPrivateResource.h"
#include "detail/aglRootNode.h"
#include "utility/aglDevTools.h"

namespace agl::env {

EnvObjMgr::InitArg::InitArg() = default;

/**
 * Constructs an empty manager.
 */
EnvObjMgr::EnvObjMgr() : IParameterIO("aglenv", 0) {}

/**
 * Frees the objects, the views and the type nodes.
 */
EnvObjMgr::~EnvObjMgr()
{
    for (auto* pObj : mObj)
    {
        if (pObj != nullptr)
        {
            delete pObj;
        }
    }

    mUpdateObj.freeBuffer();
    mTypeNode.freeBuffer();
    mView.freeBuffer();
    mEnvObjSet.freeBuffer();
}

/**
 * Allocates the objects, the views and the type nodes.
 * @param rArg initialization arguments
 * @param pHeap heap used for allocations
 */
void EnvObjMgr::initialize(const InitArg& rArg, sead::Heap* pHeap)
{
    allocBuffer(rArg, pHeap);
    mEnvObjSet.allocBuffer(rArg, pHeap);
    mEnvObjSet.bind(this);
    mUpdateObj.allocBuffer(mObj.size(), pHeap);
    mTypeNode.tryAllocBuffer(EnvObj::sTypeNum, pHeap);
    mView.tryAllocBuffer(rArg.getViewNum(), pHeap);

    for (auto it = mTypeNode.begin(), itEnd = mTypeNode.end(); it != itEnd; ++it)
    {
        s32 type = it.getIndex();
        it->initialize(type, this, pHeap);
        addList(&*it, EnvObj::getTypeData(type).mName);
        s32 num = mTypeRange[type].mNum;

        for (s32 i = 0; i < num; i++)
        {
            EnvObj* pObj = EnvObj::getTypeData(type).mCreateFunc(pHeap);
            pObj->initialize_(i, rArg.getViewNum(), this, pHeap);
            mObj[mTypeRange[type].mStart + i] = pObj;
            mEnvObjSet.pushBack(pObj);
            it->addObj(pObj, sead::FormatFixedSafeString<1024>(
                                 "%s%d", EnvObj::getTypeData(pObj->getTypeID()).mName, i));
        }
    }

    for (auto& rView : mView)
    {
        rView.mViewMtx.makeIdentity();
        rView.mInvViewMtx.makeIdentity();
        rView.mProjMtx = sead::Matrix44f(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
        rView.mDirectionalLightNum = 0;
        rView._ac = 0;
        rView.mNear = 0.01f;
        rView.mFar = 10000.0f;
        rView.mRenderBuffer = nullptr;
        rView.mDebugScale = 0.7853982f;
    }

    for (s32 i = 0; i < EnvObj::cMetaInfo_Num; i++)
    {
        mEnvObjSet.setSelectedObj(static_cast<EnvObj::MetaInfo>(i), nullptr);
    }

    INamedObjMgr::initialize(mObj.size(), rArg.getGroupNum(), pHeap);

    for (auto* pObj : mObj)
    {
        pushBackNamedObj(pObj);
    }

    detail::RootNode::setNodeMeta(static_cast<INamedObjMgr*>(this), "Icon = LIGHT");
    reconstruct();
}

/**
 * Binds the node to one object type.
 * @param type object type
 * @param pMgr owning manager
 * @param pHeap heap used for allocations
 */
void EnvObjMgr::TypeNode::initialize(s32 type, EnvObjMgr* pMgr, sead::Heap* pHeap)
{
    mType = type;
    mMgr = pMgr;
}

/**
 * Removes an object from the manager.
 * @param pObj object to remove
 */
void EnvObjMgr::removeObj(EnvObj* pObj)
{
    mTypeNode[pObj->getObjType()].removeObj(pObj);
}

/**
 * Resets the objects of one group.
 * @param group group index, or -1 for every group
 */
void EnvObjMgr::clear(s32 group)
{
    for (auto& rpObj : mObj)
    {
        if (group == -1 ||
            mGroup[group].getName() == static_cast<utl::INamedObj&>(*rpObj).getGroupName())
        {
            rpObj->clear_();
        }
    }

    reconstruct();
}

/**
 * Rebuilds the object list.
 */
void EnvObjMgr::reconstruct()
{
    mFlag.set(1);
    setListDirty();
}

/**
 * Updates every enabled object.
 */
void EnvObjMgr::update()
{
    if (mFlag.isOn(1))
    {
        mUpdateObj.clear();

        for (auto& rpObj : mObj)
        {
            if (rpObj->isEnable())
            {
                rpObj->update_();
                mUpdateObj.pushBack(rpObj);
            }
        }

        mFlag.reset(1);
    }
    else
    {
        for (auto& rObj : mUpdateObj)
        {
            rObj.update_();
        }
    }

    updateList();
}

void EnvObjMgr::constructList()
{
    INamedObjMgr::constructList();

    switch (mListMode)
    {
    case 0:
        constructListByName(mFlag.isOn(0x100));
        return;
    case 2:
        constructListByGroup(mFlag.isOn(0x100));
        return;
    case 1:
    {
        for (s32 type = 0; type < mTypeNode.size(); type++)
        {
            if (mTypeRange[type].mNum == 0)
            {
                continue;
            }

            mEnvObjSet.sort(type);

            for (auto it = mEnvObjSet.begin(type), itEnd = mEnvObjSet.end(type); it != itEnd;
                 ++it)
            {
                if (*it != nullptr && mCurrentGroup != -1)
                {
                    if (mGroup[mCurrentGroup].getName() ==
                        static_cast<utl::INamedObj&>(**it).getGroupName())
                    {
                    }
                }
            }
        }

        break;
    }
    default:
        break;
    }
}

/**
 * Updates the view data and every object for one view.
 * @param rViewMtx view matrix
 * @param rProjMtx projection matrix
 * @param viewIndex view index
 */
void EnvObjMgr::updateView(const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                           s32 viewIndex)
{
    View& rView = mView[viewIndex];
    rView.mViewMtx = rViewMtx;
    rView.mInvViewMtx.setInverse(rViewMtx);
    rView.mProjMtx = rProjMtx;
    rView.mDirectionalLightNum = 0;

    f32 a = (1.0f - rProjMtx(2, 2)) / (rProjMtx(2, 2) + 1.0f);
    f32 b = rProjMtx(2, 3) * 0.5f;
    rView.mFar = b * (a + 1.0f);
    rView.mNear = -(b * (a + 1.0f)) / a;

    EnvObj::ViewData data(rViewMtx, rView.mInvViewMtx, rProjMtx);

    for (auto& rObj : mUpdateObj)
    {
        rObj.updateView(data, viewIndex);
    }
}

/**
 * Draws the debug display of every object.
 * @param pDrawContext draw context that receives the commands
 * @param viewIndex view index
 * @param pRenderBuffer render buffer to draw into
 * @param scale debug draw scale
 */
void EnvObjMgr::drawDebug(DrawContext* pDrawContext, s32 viewIndex,
                          const RenderBuffer* pRenderBuffer, f32 scale) const
{
    View& rView = mView[viewIndex];
    rView.mRenderBuffer = pRenderBuffer;
    rView.mDebugScale = scale;

    for (auto& rObj : mUpdateObj)
    {
        rObj.drawDebug_(pDrawContext, rView.mViewMtx, rView.mProjMtx, viewIndex);
    }
}

void EnvObjMgr::drawDirectionalLight_(DrawContext* pDrawContext, s32 viewIndex,
                                      const EnvObj& rObj, const sead::Vector3f& rDir,
                                      const sead::Color4f& rColor0, const sead::Color4f& rColor1,
                                      const sead::Color4f& rColor2) const
{
    View& rView = mView[viewIndex];
    f32 aspect = rView.mProjMtx(1, 1) / rView.mProjMtx(0, 0);
    sead::OrthoProjection projection(-0.25f, 0.25f, 0.5f, -0.5f, aspect * -0.5f, aspect * 0.5f);

    sead::Matrix34f viewMtx = rView.mViewMtx;
    viewMtx.setTranslation(aspect * 0.375f, 0.375f, 0.0f);

    if (rView.mDirectionalLightNum == 0)
    {
        sead::GraphicsContext context;
        context.setDepthEnable(true, true);
        context.setDepthFunc(8);
        context.setBlendEnable(false);
        context.setColorMask(false, false, false, false);
        context.apply(pDrawContext);

        sead::Matrix34f mtx(0.25f, 0.0f, 0.0f, aspect * 0.375f, 0.0f, 0.25f, 0.0f, 0.375f, 0.0f,
                            0.0f, 0.25f, -0.25f);
        utl::DevTools::drawColorQuad(pDrawContext, sead::Color4f::cWhite, mtx,
                                     projection.getProjectionMatrix());

        context.setDepthFunc(2);
        context.setBlendEnable(true);
        context.setColorMask(true, true, true, false);
        context.apply(pDrawContext);

        utl::DevTools::beginDrawImm(pDrawContext, viewMtx, projection.getProjectionMatrix());

        mtx.makeSRT({0.25f, 0.25f, 0.25f}, {sead::Mathf::piHalf(), 0.0f, 0.0f},
                    {0.0f, 0.0f, 0.0f});
        utl::DevTools::drawWireCircleImm(pDrawContext, mtx, sead::Color4f::cGreen, 1.0f, 32);

        mtx = sead::Matrix34f(0.125f, 0.0f, 0.0f, 0.0f, 0.0f, 0.125f, 0.0f, 0.0f, 0.0f, 0.0f,
                              0.125f, 0.0f);
        utl::DevTools::drawAxisImm(pDrawContext, mtx, 1.0f, 1.0f, 1.0f);
    }

    utl::DevTools::drawDirectionalLight(pDrawContext, rDir, rColor0, rColor1, rColor2, viewMtx,
                                        projection.getProjectionMatrix(), 0.25f,
                                        mSelectedDirectionalLight ==
                                            rView.mDirectionalLightNum);
    rView.mDirectionalLightNum++;
}

/**
 * Draws a point light gizmo.
 * @param pDrawContext draw context that receives the commands
 * @param viewIndex view index
 * @param rObj object that owns the light
 * @param rPos light position
 * @param radius light radius
 * @param rColor0 primary color
 * @param rColor1 secondary color
 */
void EnvObjMgr::drawPointLight_(DrawContext* pDrawContext, s32 viewIndex, const EnvObj& rObj,
                                const sead::Vector3f& rPos, f32 radius,
                                const sead::Color4f& rColor0, const sead::Color4f& rColor1) const
{
    const View& rView = mView[viewIndex];
    utl::DevTools::drawPointLight(pDrawContext, rPos, radius, rColor0, rView.mViewMtx,
                                  rView.mProjMtx);
}

/**
 * Draws a fog gizmo.
 * @param pDrawContext draw context that receives the commands
 * @param viewIndex view index
 * @param rObj object that owns the fog
 * @param start fog start
 * @param end fog end
 * @param rDir fog direction
 * @param rColor fog color
 */
void EnvObjMgr::drawFog_(DrawContext* pDrawContext, s32 viewIndex, const EnvObj& rObj,
                         f32 start, f32 end, const sead::Vector3f& rDir,
                         const sead::Color4f& rColor) const
{
    const View& rView = mView[viewIndex];

    if (rView.mRenderBuffer == nullptr)
    {
        return;
    }

    f32 depth[2] = {start, end};
    sead::Color4f color[2] = {sead::Color4f::cBlack, sead::Color4f::cWhite};
    utl::DevTools::drawDepthGradation(pDrawContext, *rView.mRenderBuffer, 2, depth, color,
                                      rView.mNear, rView.mFar);
}

/**
 * Handles a group event from the named object manager.
 * @param type group event type
 * @param pGroup group that changed
 */
void EnvObjMgr::listenPropertyEventFromGroup(GroupEventType type, Group* pGroup)
{
    switch (type)
    {
    case cGroupEventType_0:
        saveImpl_(sead::SafeString::cEmptyString, 1, pGroup->getIndex());
        break;
    case cGroupEventType_1:
        saveImpl_(mPath, 1, pGroup->getIndex());
        break;
    case cGroupEventType_2:
        load(sead::SafeString::cEmptyString, false);
        break;
    case cGroupEventType_3:
        clear(pGroup->getIndex());
        break;
    }
}

bool EnvObjMgr::saveImpl_(const sead::SafeString& rPath, u32 flag, s32 group) const
{
    sead::Heap* pHeap = detail::PrivateResource::instance()->getDebugHeap();
    sead::XmlDocument* pDocument = sead::XmlDocument::create(nullptr, pHeap, false, 0x4000);
    sead::XmlElement* pRoot = pDocument->getRoot();
    writeHeader_(pRoot, pHeap);
    sead::XmlElement* pData =
        sead::XmlUtil::createBackChildAndSetupElement(pRoot, "data", "", pHeap);
    sead::XmlElement* pParamRoot = createAttribute(pData, pHeap);

    for (s32 type = 0; type < EnvObj::sTypeNum; type++)
    {
        if (mTypeRange[type].mNum == 0)
        {
            continue;
        }

        sead::XmlElement* pTypeElement = mTypeNode[type].createAttribute(pParamRoot, pHeap);

        for (auto it = begin(type), itEnd = end(type); it != itEnd; ++it)
        {
            EnvObj* pObj = *it;

            if (group != -1 && !(mGroup[group].getName() == static_cast<utl::INamedObj&>(*pObj).getGroupName()))
            {
                continue;
            }

            if ((flag & 1) && !pObj->mFlag.isOn(1))
            {
                continue;
            }

            if ((flag & 2) && !pObj->isEnable())
            {
                continue;
            }

            pObj->writeToXML(pTypeElement, pHeap);
        }
    }

    bool result = save_(rPath, pDocument);
    delete pDocument;
    return result;
}

void EnvObjMgr::applyResource_(utl::ResParameterArchive arc0, utl::ResParameterArchive arc1,
                               f32 t, s32 group)
{
    mCS.lock();

    utl::ResParameterArchive arc = arc0;
    utl::ResParameterArchive arcB = arc0;

    if (t > 0.0f)
    {
        arc = t < 1.0f ? arc0 : arc1;
        arcB = arc1;
    }

    static const u32 cNameHash = utl::ParameterBase::calcHash("name");
    static const u32 cGroupHash = utl::ParameterBase::calcHash("group");

    utl::ResParameterList root = arc.getRootList();

    for (auto itList = root.listBegin(), itListEnd = root.listEnd(); itList != itListEnd;
         ++itList)
    {
        utl::ResParameterList list = *itList;

        s32 type = 0;
        IParameterList* pChild = getChildListHead();

        for (; pChild != nullptr; pChild = pChild->getNext(), type++)
        {
            if (list.getParameterListNameHash() == pChild->getNameHash())
            {
                break;
            }
        }

        if (pChild == nullptr)
        {
            continue;
        }

        s32 lastIndex = -1;

        for (auto itObj = list.objBegin(), itObjEnd = list.objEnd(); itObj != itObjEnd; ++itObj)
        {
            utl::ResParameterObj obj = *itObj;
            s32 groupIndex = obj.searchIndex(cGroupHash);
            utl::ResParameter groupParam =
                groupIndex != -1 ? obj.getResParameter(groupIndex) : utl::ResParameter{};
            s32 nameIndex = obj.searchIndex(cNameHash);

            if (nameIndex == -1 || groupParam.ptr() == nullptr)
            {
                continue;
            }

            utl::ResParameter nameParam = obj.getResParameter(nameIndex);

            if (nameParam.ptr() == nullptr)
            {
                continue;
            }

            if (group != -1 &&
                !(mGroup[group].getName() == sead::SafeString(groupParam.getData<char>())))
            {
                continue;
            }

            sead::SafeString name(nameParam.getData<char>());
            EnvObj* pFound = nullptr;

            for (auto it = begin(type), itEnd = end(type); it != itEnd; ++it)
            {
                EnvObj* pObj = *it;

                if (!pObj->mFlag.isOn(1))
                {
                    continue;
                }

                if (pFound == nullptr && it.getIndex() > lastIndex &&
                    pObj->getGroupName() == utl::INamedObj::getDefaultGroupName())
                {
                    pFound = pObj;
                    lastIndex = it.getIndex();
                }

                if (pObj->getEnvObjName() == name)
                {
                    pFound = pObj;
                    break;
                }
            }

            if (pFound == nullptr)
            {
                continue;
            }

            if (arc.ptr() != arcB.ptr())
            {
                utl::ResParameterList rootB = arcB.getRootList();
                s32 listIndex = rootB.searchListIndex(list.getParameterListNameHash());

                if (listIndex != -1)
                {
                    utl::ResParameterList listB = rootB.getResParameterList(listIndex);
                    bool applied = false;

                    for (auto itObjB = listB.objBegin(), itObjBEnd = listB.objEnd();
                         itObjB != itObjBEnd; ++itObjB)
                    {
                        utl::ResParameterObj objB = *itObjB;
                        sead::SafeString nameB(
                            utl::getResParameter(objB, "name").getData<char>());
                        if (name == nameB)
                        {
                            pFound->applyResParameterObj(obj, objB, t, nullptr);
                            applied = true;
                            break;
                        }
                    }

                    if (applied)
                    {
                        continue;
                    }
                }
            }

            pFound->applyResParameterObj(obj);
        }
    }

    reconstruct();
    mCS.unlock();
}

bool EnvObjMgr::saveToGroupFilePath(const sead::SafeString& rPath) const
{
    s32 i = 0;

    for (auto it = mGroupPtr.begin(), itEnd = mGroupPtr.end(); it != itEnd; ++it, ++i)
    {
        if (mGroup[i].getComment() == rPath)
        {
            saveImpl_(rPath, 1, i);
        }
    }

    return true;
}

/**
 * Generates the host IO messages.
 * @param pContext host IO context
 */
void EnvObjMgr::genMessage(sead::hostio::Context* pContext)
{
    genMessageIO(pContext, 6);
    genGroupComboBox(pContext);

    for (auto* pObj : mObj)
    {
        if (pObj != nullptr && pObj->isEnable() && pObj->mFlag.isOn(0x20))
        {
            pObj->getEnvObjName();
        }
    }

    s32 total = 0;
    u32 n = mTypeNode.size();

    for (s32 type = 0; type < n; type++)
    {
        const EnvObj::TypeData& rData = EnvObj::getTypeData(type);
        s32 size = rData.mPriority;
        s32 num = mTypeRange[type].mNum;
        sead::FormatFixedSafeString<1024> str("%2d:%-64s(%4d[byte]) x %3d = %8d[byte]", type,
                                              rData.mLabel, size, num, size * num);
        total += size * num;
    }

    {
        sead::FormatFixedSafeString<1024> str("Total:%8d[byte]", total);
    }

    INamedObjMgr::genMessage(pContext);
}

/**
 * Handles a host IO property event.
 * @param pEvent property event to handle
 */
void EnvObjMgr::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    INamedObjMgr::listenPropertyEvent(pEvent);

    if (listenPropertyEventIO(this, pEvent) == 2)
    {
        setListDirty();
        return;
    }

    if ((pEvent->getType() & 2) == 0 && pEvent->getId() < &mListMode + 1 &&
        pEvent->getId() >= &mListMode)
    {
        setListDirty();
        return;
    }

    switch (reinterpret_cast<uintptr_t>(pEvent->getId()))
    {
    case 101004:
        for (auto* pObj : mObj)
        {
            pObj->mFlag.set(8);
        }

        break;
    case 101005:
        for (auto* pObj : mObj)
        {
            pObj->mFlag.reset(8);
        }

        break;
    case 101006:
        for (auto it = begin(mSelectedType), itEnd = end(mSelectedType); it != itEnd; ++it)
        {
            (*it)->mFlag.set(8);
        }

        break;
    case 101007:
        for (auto it = begin(mSelectedType), itEnd = end(mSelectedType); it != itEnd; ++it)
        {
            (*it)->mFlag.reset(8);
        }

        break;
    case 101008:
        mFlag.toggle(0x100);
        break;
    case 101009:
        clear(-1);
        return;
    default:
        return;
    }

    setListDirty();
}

/**
 * Constructs an unbound type node.
 */
EnvObjMgr::TypeNode::TypeNode()
{
    detail::RootNode::setNodeMeta(this, "Icon=FOLDER_BLUE");
}

/**
 * Generates the host IO messages of the type.
 * @param pContext host IO context
 */
void EnvObjMgr::TypeNode::genMessage(sead::hostio::Context* pContext)
{
    sead::FormatFixedSafeString<1024>("GroupHeader = %s", EnvObj::getTypeData(mType).mLabel);
}

/**
 * Handles a host IO property event of the type.
 * @param pEvent property event to handle
 */
void EnvObjMgr::TypeNode::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    switch (reinterpret_cast<uintptr_t>(pEvent->getId()))
    {
    case 101002:
        mMgr->setEnable(mType, true);
        break;
    case 101003:
        mMgr->setEnable(mType, false);
        break;
    default:
        break;
    }
}

}  // namespace agl::env
