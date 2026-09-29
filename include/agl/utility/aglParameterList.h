#pragma once

#include <container/seadPtrArray.h>
#include <hostio/seadHostIOReflexible.h>
#include <prim/seadSafeString.h>
#include "utility/aglResParameter.h"

namespace sead {
class XmlElement;
}

namespace agl::utl {

class IParameterObj;
class ParameterBase;

class IParameterList {
public:
    IParameterList();
    virtual ~IParameterList() { ; }

    void addList(IParameterList* pChild, const sead::SafeString& rName);
    void addObj(IParameterObj* pChild, const sead::SafeString& rName);
    void clearList();
    void clearObj();
    void removeList(IParameterList* pChild);
    void removeObj(IParameterObj* pChild);

    IParameterObj* getChildObjHead() const { return mpChildObjHead; }
    IParameterObj* getChildObjTail() const { return mpChildObjTail; }
    IParameterList* getChildListHead() const { return mpChildListHead; }
    IParameterList* getChildListTail() const { return mpChildListTail; }
    IParameterList* getNext() const { return mNext; }
    IParameterList* getParent() const { return mParent; }

    sead::SafeString getParameterListName() const;
    sead::SafeString getName() const { return getParameterListName(); }
    u32 getNameHash() const { return mNameHash; }

    void applyResParameterList(ResParameterList list);
    void applyResParameterList(ResParameterList list1, ResParameterList list2, f32 t);

    bool isComplete(ResParameterList res, bool checkValues) const;

    static const char* getTagName();
    sead::XmlElement* createAttribute(sead::XmlElement* pElement, sead::Heap* pHeap) const;
    void writeToXML(sead::XmlElement* pElement, sead::Heap* pHeap) const;
    s32 readFromXML(const sead::XmlElement& rElement, bool x);

    bool verify() const;
    bool verifyList() const;
    bool verifyObj() const;
    bool verifyList(IParameterList* pCheck, IParameterList* pOther) const;
    bool verifyObj(IParameterObj* pCheck, IParameterObj* pOther) const;

    void sortByHash();

    void genMessageParameterList(sead::hostio::Context* pContext);
    void listenPropertyEventParameter(sead::hostio::Reflexible* pReflexible,
                                      const sead::hostio::PropertyEvent* pEvent);

protected:
    friend class IParameterObj;

    virtual bool preWrite_() const { return true; }
    virtual void postWrite_() const {}
    virtual bool preRead_() { return true; }
    virtual void postRead_() {}
    virtual bool isApply_(ResParameterList list) const {
        return list.getParameterListNameHash() == mNameHash;
    }
    virtual void callbackNotAppliable_(IParameterObj*, ParameterBase*, ResParameterObj) {}
    virtual void callbackNotInterpolatable_(IParameterObj*, ParameterBase*, ResParameterObj,
                                            ResParameterObj, ResParameter, ResParameter, f32) {}

    void setParameterListName_(const sead::SafeString& rName);
    void applyResParameterList_(bool interpolate, ResParameterList l1, ResParameterList l2, f32 t);
    ResParameterObj searchResParameterObj_(ResParameterList res, const IParameterObj& rObj) const;
    IParameterObj* searchChildParameterObj_(ResParameterObj res, IParameterObj* pObj) const;
    void applyResParameterObjB_(bool interpolate, ResParameterList res, f32 t);
    ResParameterList searchResParameterList_(ResParameterList res,
                                             const IParameterList& rList) const;
    IParameterList* searchChildParameterList_(ResParameterList res) const;
    void applyResParameterListB_(bool interpolate, ResParameterList res, f32 t);

    IParameterObj* mpChildObjHead = nullptr;
    IParameterObj* mpChildObjTail = nullptr;
    IParameterList* mpChildListHead = nullptr;
    IParameterList* mpChildListTail = nullptr;
    u32 mNameHash;
    IParameterList* mNext = nullptr;
    IParameterList* mParent = nullptr;
    sead::FixedSafeString<64> mName;
};

/**
 * Sorts an array of parameter nodes by name hash using heapsort.
 * @param rArray array to sort
 */
template <typename T>
inline void heapSortByHash(sead::PtrArray<T>& rArray)
{
    const auto cmp = [](const T* a, const T* b) -> s32 {
        if (a->getNameHash() < b->getNameHash())
        {
            return -1;
        }
        if (a->getNameHash() > b->getNameHash())
        {
            return 1;
        }
        return 0;
    };

    const s32 num = rArray.size();
    T** ptrs = rArray.data();
    for (s32 i = num / 2; i > 0; --i)
    {
        T* value = ptrs[i - 1];
        s32 k = i;
        while (2 * k <= num)
        {
            s32 child = 2 * k;
            if (child < num && cmp(ptrs[child - 1], ptrs[child]) < 0)
            {
                ++child;
            }
            if (cmp(value, ptrs[child - 1]) >= 0)
            {
                break;
            }
            ptrs[k - 1] = ptrs[child - 1];
            k = child;
        }
        ptrs[k - 1] = value;
    }

    for (s32 i = num; i > 1; --i)
    {
        T* value = ptrs[i - 1];
        ptrs[i - 1] = ptrs[0];
        const s32 n = i - 1;
        s32 k = 1;
        while (2 * k <= n)
        {
            s32 child = 2 * k;
            if (child < n && cmp(ptrs[child - 1], ptrs[child]) < 0)
            {
                ++child;
            }
            if (cmp(value, ptrs[child - 1]) >= 0)
            {
                break;
            }
            ptrs[k - 1] = ptrs[child - 1];
            k = child;
        }
        ptrs[k - 1] = value;
    }
}

class ParameterList : public IParameterList {
public:
    using IParameterList::IParameterList;
    ~ParameterList() override { ; }
};

}  // namespace agl::utl
