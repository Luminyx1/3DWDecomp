#pragma once

#include <basis/seadTypes.h>
#include <gfx/seadColor.h>
#include <math/seadVector.h>

namespace sead {
    class WriteStream;
};

namespace al {
    /// Writes YAML text into a sead write stream.
    class YamlFormatter {
    public:
        __attribute__((used)) YamlFormatter() : mStream(nullptr), mIndent(0) {}

        __attribute__((used)) virtual ~YamlFormatter() {}

        void startDocument();
        void writeString(const char* pFormat, ...);
        void startHash(const char* pKey);
        void writeIndent();
        void endHash();
        void startArray();
        void endArray();
        void writeHashBool(const char* pKey, bool value);
        void writeHashInt(const char* pKey, s32 value);
        void writeHashUInt(const char* pKey, u32 value);
        void writeHashInt64(const char* pKey, s64 value);
        void writeHashUInt64(const char* pKey, u64 value);
        void writeHashFloat(const char* pKey, f32 value);
        void writeHashString(const char* pKey, const char* pValue);
        void writeBlockString(const char* pKey, const char* pValue);
        void writeHashV2f(const char* pKey, const sead::Vector2f& rValue);
        void writeHashV3f(const char* pKey, const sead::Vector3f& rValue);
        void writeHashV4f(const char* pKey, const sead::Vector4f& rValue);
        void writeHashColor(const char* pKey, const sead::Color4f& rValue);
        void writeHashNull(const char* pKey);
        void writeArrayBool(bool value);
        void writeArrayInt(s32 value);
        void writeArrayFloat(f32 value);
        void writeArrayString(const char* pValue);
        void writeArrayInt64(s64 value);
        void writeArrayUInt64(u64 value);
        void writeArrayDouble(f64 value);
        void setStream(sead::WriteStream* pStream);

        sead::WriteStream* mStream;     // _8
        s32 mIndent;                    // _10
    };
};
