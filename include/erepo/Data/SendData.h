#pragma once

#include <erepo/Data/SendDataBase.h>

namespace nn::prepo {
class PlayReport;
}

namespace erepo {

class SendData : public SendDataBase {
public:
    SendData(sead::Heap* pHeap, const sead::FixedSafeString<31>& rEventId, s32 dataNum,
             s32 arrayNum, s32 structNum, const StringId& rReporterId, bool isNothrow);
    ~SendData() override;

    static SendData* CreateSendData(const sead::FixedSafeString<31>& rEventId, s32 dataNum,
                                    s32 arrayNum, s32 structNum, const StringId& rReporterId,
                                    bool isNothrow);
    static SendData* CreateSendData(const CreateArg& rArg);

    bool addData(const sead::FixedSafeString<63>& rKey, bool value) override;
    bool addData(const sead::FixedSafeString<63>& rKey, u64 value) override;
    bool addData(const sead::FixedSafeString<63>& rKey, s64 value) override;
    bool addData(const sead::FixedSafeString<63>& rKey, u32 value) override;
    bool addData(const sead::FixedSafeString<63>& rKey, s32 value) override;
    bool addData(const sead::FixedSafeString<63>& rKey, f32 value) override;
    bool addData(const sead::FixedSafeString<63>& rKey, const char* value) override;
    bool addData(const sead::FixedSafeString<63>& rKey, const sead::SafeString& rValue) override;
    bool addData(const sead::FixedSafeString<63>& rKey, const void* pValue, size_t size) override;
    bool addData(const sead::FixedSafeString<63>& rKey, const Array& rValue) override;
    bool addData(const sead::FixedSafeString<63>& rKey, const Struct& rValue) override;
    bool clear() override;

    bool addInternetConnectionStatus();
    size_t GetSize() const;
    s32 GetCount() const;
    nn::Result addSessionId();

protected:
    bool isValid_() const override;
    bool initializeInnerData_(sead::Heap* pHeap) override;
    void finalizeInnerData_() override;
    bool trySetInnerDataEventId_(const sead::FixedSafeString<31>& rEventId) override;
    ESendResult saveInnerData_() override;

private:
    nn::prepo::PlayReport* mReport = nullptr;
    sead::Buffer<u8> mReportBuffer;
};

}  // namespace erepo
