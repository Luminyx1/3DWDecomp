#pragma once

#include <erepo/Data/SendDataBase.h>

namespace nn::prepo {
class PlayReport;
}

namespace erepo {

class SendData : public SendDataBase {
public:
    SendData(sead::Heap* pHeap, const EventIdString& rEventId, s32 dataNum,
             s32 arrayNum, s32 structNum, const StringId& rReporterId, bool isNothrow);
    ~SendData() override;

    static SendData* CreateSendData(const EventIdString& rEventId, s32 dataNum,
                                    s32 arrayNum, s32 structNum, const StringId& rReporterId,
                                    bool isNothrow);
    static SendData* CreateSendData(const CreateArg& rArg);

    bool addData(const KeyString& rKey, bool value) override;
    bool addData(const KeyString& rKey, u64 value) override;
    bool addData(const KeyString& rKey, s64 value) override;
    bool addData(const KeyString& rKey, u32 value) override;
    bool addData(const KeyString& rKey, s32 value) override;
    bool addData(const KeyString& rKey, f32 value) override;
    bool addData(const KeyString& rKey, const char* value) override;
    bool addData(const KeyString& rKey, const sead::SafeString& rValue) override;
    bool addData(const KeyString& rKey, const void* pValue, size_t size) override;
    bool addData(const KeyString& rKey, const Array& rValue) override;
    bool addData(const KeyString& rKey, const Struct& rValue) override;
    bool clear() override;

    bool addInternetConnectionStatus();
    size_t GetSize() const;
    s32 GetCount() const;
    nn::Result addSessionId();

protected:
    bool isValid_() const override;
    bool initializeInnerData_(sead::Heap* pHeap) override;
    void finalizeInnerData_() override;
    bool trySetInnerDataEventId_(const EventIdString& rEventId) override;
    ESendResult saveInnerData_() override;

private:
    nn::prepo::PlayReport* mReport = nullptr;
    sead::Buffer<u8> mReportBuffer;
};

}  // namespace erepo
