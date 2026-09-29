#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadOffsetList.h>
#include <container/seadPtrArray.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadBitFlag.h>
#include <prim/seadSafeString.h>

#include "utility/aglNamedObj.h"
#include "utility/aglNamedObjIndex.h"

namespace sead {
class Heap;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace agl::utl {

class INamedObjMgr : public sead::hostio::Node {
public:
    enum GroupEventType {
        cGroupEventType_0 = 0,
        cGroupEventType_1 = 1,
        cGroupEventType_2 = 2,
        cGroupEventType_3 = 3,
    };

    class Group : public sead::hostio::Node {
    public:
        Group();
        virtual ~Group();

        void initialize(s32 index, INamedObjMgr* pMgr, sead::Heap* pHeap);
        void reset(const sead::SafeString& rName);
        static s32 compare(const Group* pLhs, const Group* pRhs);

        void genMessage(sead::hostio::Context* pContext);
        void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

        const sead::SafeString& getName() const { return mName; }
        s32 getIndex() const { return mIndex; }

    private:
        friend class INamedObjMgr;

        INamedObjMgr* mMgr = nullptr;
        s32 mIndex;
        sead::FixedSafeString<32> mName{""};
        sead::FixedSafeString<256> mComment;
    };
    static_assert(sizeof(Group) == 0x168);

    INamedObjMgr();
    virtual ~INamedObjMgr();

    virtual const sead::SafeString& getNamedObjName(s32 index, s32 type) const;
    virtual s32 getNamedObjNum(s32 type) const;
    virtual void constructList();
    virtual const sead::SafeString& getSaveFilePath() const;
    virtual void listenPropertyEventFromGroup(GroupEventType type, Group* pGroup);

    void initialize(u32 objNum, u32 groupNum, sead::Heap* pHeap);
    void pushBackNamedObj(INamedObj* pObj);
    void eraseNamedObj(INamedObj* pObj);
    void updateList();
    void syncNameToIndex();
    void constructListByName(bool isAll);
    void constructListByGroup(bool isAll);

    void genGroupComboBox(sead::hostio::Context* pContext);
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    void setListDirty() { mFlag.set(1); }

protected:
    friend class INamedObjIndex;

    sead::PtrArray<INamedObj> mNamedObj;
    sead::Buffer<Group> mGroup;
    sead::PtrArray<Group> mGroupPtr;
    sead::OffsetList<INamedObjIndex> mIndexList;
    s32 mCurrentGroup = -1;
    sead::BitFlag16 mFlag{1};
};
static_assert(sizeof(INamedObjMgr) == 0x58);

}  // namespace agl::utl
