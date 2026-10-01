#pragma once

#include <prim/seadSafeString.h>

#include <erepo/Data/SendData.h>
#include <erepo/Manager.h>
#include <erepo/Types.h>

namespace sead {
class Heap;
}

namespace erepo {

class SaveData;

class ObserverBase {
public:
    virtual ~ObserverBase() {}
    virtual void initialize(sead::Heap* pHeap) = 0;
    virtual void finalize() {}
    virtual const char* getName() const = 0;
    virtual void load() {}
    virtual void save(SaveData* pData) const {}
    virtual void update(const Manager::UpdateArg& rArg) {}
    virtual bool report(const StringId& rId) = 0;

protected:
    SendData* createSendData_(const EventIdString& rEventId, s32 dataNum, s32 arrayNum,
                              s32 structNum, const StringId& rId, bool isNothrow);
    SendData* createSendData_(const SendDataBase::CreateArg& rArg);
};

}  // namespace erepo
