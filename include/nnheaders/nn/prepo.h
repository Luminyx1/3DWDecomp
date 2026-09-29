#pragma once

#include <nn/types.h>

namespace nn::account {
class Uid;
}

namespace nn::prepo {

namespace detail {
class PlayReportGenerator {
public:
    PlayReportGenerator() = default;
    void Initialize();

private:
    void* m_Data = nullptr;
};
}  // namespace detail

enum class TransmissionStatus {};

struct Any64BitId {
    u64 id;
};

class Struct {
public:
    Struct();

    void SetBuffer(void* buffer, size_t size);
    const void* GetBuffer() const;

    Result Add(const char* key, bool value);
    Result Add(const char* key, s64 value);
    Result Add(const char* key, const Any64BitId& value);
    Result Add(const char* key, float value);
    Result Add(const char* key, const char* value);

private:
    void* m_Buffer;
    size_t m_BufferSize;
    size_t m_Position;
    s32 m_Count;
};

class Array {
public:
    Array();

    void SetBuffer(void* buffer, size_t size);
    const void* GetBuffer() const;

    Result Add(bool value);
    Result Add(s64 value);
    Result Add(const Any64BitId& value);
    Result Add(float value);
    Result Add(const char* value);
    Result Add(const Struct& value);

private:
    void* m_Buffer;
    size_t m_BufferSize;
    size_t m_Position;
    s32 m_Count;
};

class PlayReport {
public:
    PlayReport();
    explicit PlayReport(const char* event_id);

    Result SetEventId(const char* event_id);
    void SetBuffer(void* buffer, size_t size);
    void Clear();

    Result Add(const char* key, s64 value);
    Result Add(const char* key, f64 value);
    Result Add(const char* key, const char* value);
    Result Add(const char* key, bool value);
    Result Add(const char* key, const Any64BitId& value);
    Result Add(const char* key, float value);
    Result Add(const char* key, const void* value, size_t size);
    Result Add(const char* key, const Array& value);
    Result Add(const char* key, const Struct& value);

    Result Save();
    Result Save(const account::Uid& uid);

    s32 GetCount() const;
    size_t GetSize() const;

    static size_t CalcBufferSize(s32 num_entries) { return size_t(0x82) * num_entries + 3; }

private:
    char m_EventId[32];
    void* m_Buffer;
    size_t m_BufferSize;
    detail::PlayReportGenerator m_Generator;
};

Result AddInternetConnectionStatus(PlayReport* report, const char* interface_key,
                                   const char* link_level_key, const char* frequency_band_key);
Result AddSessionId(PlayReport* report, const char* key);

void Initialize();

Result RequestImmediateTransmission();
Result GetTransmissionStatus(TransmissionStatus* status);

Result ClearStorage();
Result SetOperationMode(s64 mode);
Result IsUserAgreementCheckEnabled(bool* enabled);
Result SetUserAgreementCheckEnabled(bool enabled);
Result GetStorageUsage(s64*, s64*);

}  // namespace nn::prepo
