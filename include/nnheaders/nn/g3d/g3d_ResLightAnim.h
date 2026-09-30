#pragma once
#include <nn/g3d/g3d_ResAnimCurve.h>
#include <nn/g3d/g3d_Resources.h>
#include <nn/g3d/g3d_BindFuncTable.h>

namespace nn::g3d {
struct LightAnimResult {
    int enabled;
    float position[3];
    float direction[3];
    float distanceAttenuation[2];
    float angleAttenuation[2];
    float color0[3];
    float color1[3];
};
class ResLightAnim {
public:
    // result receives the resource's constant animation values.
    void Initialize(LightAnimResult* result) const;
    // table maps stored function names to application-provided function indices.
    BindResult Bind(const BindFuncTable& table);
    void Release();
    // result receives samples at frame; cache, when supplied, has one entry per curve.
    void Evaluate(LightAnimResult* result, float frame) const;
    void Evaluate(LightAnimResult* result, float frame, AnimFrameCache* cache) const;
    // buffer supplies size writable bytes for replacing curves with baked samples.
    bool BakeCurve(void* buffer, size_t size);
    void* ResetCurve();
    void Reset();
    void ResetCurves() {
        if (!(flags & 1)) return;
        int count = curveCount;
        for (int i = 0; i < count; ++i) curves[i].Reset();
        flags ^= 1;
    }

    u32 signature;
    u16 flags;
    u16 _6;
    const char* name;
    ResAnimCurve* curves; // 0x10
    void* baseValues;
    void* userData;
    void* userDataDictionary;
    const char* lightFuncName; // 0x30, binary string including its length prefix
    const char* distanceFuncName;
    const char* angleFuncName;
    u32 frameCount; // 0x48
    u32 bakedSize;
    u16 userDataCount;
    u8 curveCount;
    u8 lightFuncIndex;
    u8 distanceFuncIndex;
    u8 angleFuncIndex;
    u8 _56[2];
};
}
