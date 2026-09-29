#pragma once

#include <hostio/seadHostIOReflexible.h>
#include <prim/seadSafeString.h>
#include "utility/aglResParameter.h"

namespace sead {
class XmlElement;
}

namespace agl::utl {

class IParameterList;
class ParameterBase;

class IParameterObj {
public:
    IParameterObj();
    virtual ~IParameterObj() { ; }

    void pushBackListNode(ParameterBase* pNode);
    void sortByHash();

    sead::SafeString getParameterObjName() const;
    sead::SafeString getName() const { return getParameterObjName(); }
    u32 getNameHash() const { return mNameHash; }

    ParameterBase* getParamListHead() const { return mParamListHead; }
    ParameterBase* getParamListTail() const { return mParamListTail; }
    u32 getParamListSize() const { return mParamListSize; }
    IParameterObj* getNext() const { return mNext; }

    void writeToXML(sead::XmlElement* pElement, sead::Heap* pHeap) const;
    s32 readFromXML(const sead::XmlElement& rElement, bool x);
    sead::XmlElement* createAttribute(sead::XmlElement* pElement, sead::Heap* pHeap) const;
    static const char* getTagName();

    void applyResParameterObj(ResParameterObj obj1, ResParameterObj obj2, f32 t,
                              IParameterList* pList);

    void applyResParameterObj(ResParameterObj obj, IParameterList* pList = nullptr) {
        applyResParameterObj_(false, obj, {}, 0.0, pList);
    }

    bool isComplete(ResParameterObj obj, bool checkValues) const;
    bool verify() const;
    bool verify(ParameterBase* pCheck, ParameterBase* pOther) const;

    void copy(ParameterBase* pFirst, ParameterBase* pLast, const ParameterBase* pSrcFirst,
              const ParameterBase* pSrcLast);
    void copy(const IParameterObj& rObj);
    void copyLerp(ParameterBase* pFirst, ParameterBase* pLast, const ParameterBase* pSrc1First,
                  const ParameterBase* pSrc1Last, const ParameterBase* pSrc2First,
                  const ParameterBase* pSrc2Last, f32 t);
    void copyLerp(const IParameterObj& rObj1, const IParameterObj& rObj2, f32 t);

    void genMessageParameter(sead::hostio::Context* pContext);
    void listenPropertyEventParameter(sead::hostio::Reflexible* pReflexible,
                                      const sead::hostio::PropertyEvent* pEvent);

protected:
    friend class IParameterList;

    virtual bool preWrite_() const { return true; }
    virtual void postWrite_() const {}
    virtual bool preRead_() { return true; }
    virtual void postRead_() {}
    virtual bool preCopy_() { return true; }
    virtual void postCopy_() {}
    virtual bool isApply_(ResParameterObj obj) const {
        return obj.getParameterObjNameHash() == mNameHash;
    }

    void applyResParameterObj_(bool interpolate, ResParameterObj obj1, ResParameterObj obj2, f32 t,
                               IParameterList* pList);
    ParameterBase* searchParameter_(u32 hash);
    ParameterBase* searchParameter_(u32 hash) const;

    void copy_(ParameterBase* pFirst, ParameterBase* pLast, const ParameterBase* pSrcFirst,
               const ParameterBase* pSrcLast);
    void copyLerp_(ParameterBase* pFirst, ParameterBase* pLast, const ParameterBase* pSrc1First,
                   const ParameterBase* pSrc1Last, const ParameterBase* pSrc2First,
                   const ParameterBase* pSrc2Last, f32 t);

    ParameterBase* mParamListHead = nullptr;
    ParameterBase* mParamListTail = nullptr;
    u32 mParamListSize = 0;
    u32 mNameHash = 0;
    IParameterObj* mNext = nullptr;
    const char* mName = nullptr;
};

class ParameterObj : public IParameterObj {
public:
    using IParameterObj::IParameterObj;
    ~ParameterObj() override { ; }
};

}  // namespace agl::utl
