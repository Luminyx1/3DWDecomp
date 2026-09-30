#pragma once

#include <prim/seadEnum.h>
#include <prim/seadSafeString.h>
#include <thread/seadCriticalSection.h>

// Same as SEAD_ENUM, but the enumerator texts are given as a separate string literal (the original
// texts are not valid identifiers).
#define AL_TEXT_ENUM(NAME, TEXT, ...)                                                              \
    class NAME {                                                                                   \
    public:                                                                                        \
        enum ValueType { __VA_ARGS__ };                                                            \
                                                                                                   \
        NAME() : mIdx(0) {}                                                                        \
        /* NOLINTNEXTLINE(google-explicit-constructor) */                                          \
        NAME(ValueType value) { setRelativeIndex(value); }                                         \
        /* NOLINTNEXTLINE(google-explicit-constructor) */                                          \
        NAME(int idx) { setRelativeIndex(idx); }                                                   \
        NAME(const NAME& other) = default;                                                         \
                                                                                                   \
        NAME& operator=(const NAME& other) = default;                                              \
        NAME& operator=(ValueType value) {                                                         \
            setRelativeIndex(value);                                                               \
            return *this;                                                                          \
        }                                                                                          \
                                                                                                   \
        bool operator==(ValueType value) const { return ValueType(mIdx) == value; }                \
        bool operator!=(ValueType value) const { return ValueType(mIdx) != value; }                \
                                                                                                   \
        ValueType value() const { return static_cast<ValueType>(mIdx); }                           \
        ValueType value() const volatile { return static_cast<ValueType>(mIdx); }                  \
        /* NOLINTNEXTLINE(google-explicit-constructor) */                                          \
        operator int() const volatile { return value(); }                                          \
                                                                                                   \
        const char* text() const { return text(mIdx); }                                            \
        static const char* text(int idx) { return text_(idx); }                                    \
                                                                                                   \
        int getRelativeIndex() const { return mIdx; }                                              \
        void setRelativeIndex(int idx) { mIdx = idx; }                                             \
                                                                                                   \
        constexpr static int size() { return cCount; }                                             \
                                                                                                   \
    private:                                                                                       \
        static const char* text_(int idx) {                                                        \
            if (u32(idx) >= cCount)                                                                \
                return nullptr;                                                                    \
                                                                                                   \
            static char** spTextPtr = nullptr;                                                     \
            if (spTextPtr)                                                                         \
                return spTextPtr[idx];                                                             \
            {                                                                                      \
                sead::ScopedLock<sead::CriticalSection> lock(sead::EnumUtil::getParseTextCS_());   \
                if (!spTextPtr) {                                                                  \
                    static char* sTextPtr[cCount];                                                 \
                    static sead::FixedSafeString<cTextAllLen> sTextAll =                           \
                        sead::SafeString(cTextAll);                                                \
                    sead::EnumUtil::parseText_(sTextPtr, sTextAll.getBuffer(), cCount);            \
                    spTextPtr = sTextPtr;                                                          \
                }                                                                                  \
            }                                                                                      \
            return spTextPtr[idx];                                                                 \
        }                                                                                          \
                                                                                                   \
        static constexpr const char* cTextAll = TEXT;                                              \
        static constexpr size_t cTextAllLen = sizeof(TEXT);                                        \
        static constexpr int cCount = sead::EnumUtil::countValues(cTextAll, cTextAllLen);          \
                                                                                                   \
        int mIdx;                                                                                  \
    };
