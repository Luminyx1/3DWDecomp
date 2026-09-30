#include <nn/g3d/g3d_ResSceneAnim.h>
#include <nn/g3d/g3d_ResCameraAnim.h>
#include <nn/g3d/g3d_ResLightAnim.h>
#include <nn/g3d/g3d_ResFogAnim.h>
#include <cstring>

namespace nn::g3d {
static_assert(sizeof(ResAnimCurve) == 0x30, "Curve resource size");
static_assert(sizeof(ResCameraAnim) == 0x40, "Camera animation size");
static_assert(sizeof(ResLightAnim) == 0x58, "Light animation size");
static_assert(sizeof(ResFogAnim) == 0x48, "Fog animation size");

// result receives the constant values stored by this camera animation.
void ResCameraAnim::Initialize(CameraAnimResult* result) const {
    *result = *static_cast<const CameraAnimResult*>(baseValues);
}

// result receives values at frame; a temporary cache is used for each sample.
void ResCameraAnim::Evaluate(CameraAnimResult* result, float frame) const {
    int count = curveCount;

    for (int i = 0; i < count; ++i) {
        const ResAnimCurve* curve = &curves[i];
        u32 offset = curve->targetOffset;
        AnimFrameCache temporary;
        *reinterpret_cast<float*>(reinterpret_cast<u8*>(result) + offset) = curve->EvaluateFloat(frame, &temporary);
    }
}

// result receives values at frame; cache stores the current interval for each curve.
void ResCameraAnim::Evaluate(CameraAnimResult* result, float frame, AnimFrameCache* cache) const {
    int count = curveCount;

    for (int i = 0; i < count; ++i) {
        const ResAnimCurve* curve = &curves[i];
        u32 offset = curve->targetOffset;
        *reinterpret_cast<float*>(reinterpret_cast<u8*>(result) + offset) = curve->EvaluateFloat(frame, &cache[i]);
    }
}

// buffer is the beginning of size writable bytes; an empty request already succeeds.
bool ResCameraAnim::BakeCurve(void* buffer, size_t size) {
    if (!size) return true;

    if ((buffer == nullptr) || bakedSize > size) return false;
    u8* output = static_cast<u8*>(buffer);
    int count = curveCount;

    for (int i = 0; i < count; ++i) {
        ResAnimCurve* curve = &curves[i];
        size_t bytes = curve->CalculateBakedFloatSize();
        curve->BakeFloat(output, bytes);
        output += bytes;
    }

    flags |= 1;
    return true;
}

void* ResCameraAnim::ResetCurve() {
    if (!(flags & 1)) return nullptr;
    void* buffer = nullptr;
    bool found = false;
    int count = curveCount;

    for (int i = 0; i < count; ++i) {
        ResAnimCurve* curve = &curves[i];

        if (((curve->flags & 0x70) == 0x20) & !found) {
            buffer = curve->keys;
            found = true;
        }

        curve->Reset();
    }

    flags ^= 1;
    return buffer;
}

void ResCameraAnim::Reset() { ResetCurves(); }
// result receives the constant values stored by this light animation.
void ResLightAnim::Initialize(LightAnimResult* result) const {
    const u8* data = static_cast<const u8*>(baseValues);

    if (flags & (1u << 9)) { std::memcpy(&result->enabled, data, 4); data += 4; }
    if (flags & (1u << 10)) { std::memcpy(&result->position, data, 12); data += 12; }
    if (flags & (1u << 11)) { std::memcpy(&result->direction, data, 12); data += 12; }
    if (flags & (1u << 12)) { std::memcpy(&result->distanceAttenuation, data, 8); data += 8; }
    if (flags & (1u << 13)) { std::memcpy(&result->angleAttenuation, data, 8); data += 8; }
    if (flags & (1u << 14)) { std::memcpy(&result->color0, data, 12); data += 12; }
    if (static_cast<s16>(flags) < 0) { std::memcpy(&result->color1, data, 12); data += 12; }
}

// table supplies name/length pairs for each supported application function category.
BindResult ResLightAnim::Bind(const BindFuncTable& table) {
    BindResult result;

    if (lightFuncIndex == 0xff) {
        int count = table.lengths[BindFuncTable::Light];
        const BindFuncTable::StringLength* names = table.strings[BindFuncTable::Light];

        for (int i = 0; i < count; ++i) {
            if (std::strncmp(lightFuncName + 2, names[i].content, names[i].length) == 0) {
                lightFuncIndex = i;
                break;
            }
        }

        if (lightFuncIndex == 0xff) result.Merge(BindResult(BindResult::Flag_Failure));
        else result.Merge(BindResult(BindResult::Flag_Success));
    }

    if (distanceFuncIndex == 0xff) {
        int count = table.lengths[BindFuncTable::DistanceAttenuation];
        const BindFuncTable::StringLength* names = table.strings[BindFuncTable::DistanceAttenuation];

        for (int i = 0; i < count; ++i) {
            if (std::strncmp(distanceFuncName + 2, names[i].content, names[i].length) == 0) {
                distanceFuncIndex = i;
                break;
            }
        }

        if (distanceFuncIndex == 0xff) result.Merge(BindResult(BindResult::Flag_Failure));
        else result.Merge(BindResult(BindResult::Flag_Success));
    }

    if (angleFuncIndex == 0xff) {
        int count = table.lengths[BindFuncTable::AngleAttenuation];
        const BindFuncTable::StringLength* names = table.strings[BindFuncTable::AngleAttenuation];

        for (int i = 0; i < count; ++i) {
            if (std::strncmp(angleFuncName + 2, names[i].content, names[i].length) == 0) {
                angleFuncIndex = i;
                break;
            }
        }

        if (angleFuncIndex == 0xff) result.Merge(BindResult(BindResult::Flag_Failure));
        else result.Merge(BindResult(BindResult::Flag_Success));
    }

    return result;
}

void ResLightAnim::Release() { lightFuncIndex = 0xff; distanceFuncIndex = 0xff; angleFuncIndex = 0xff; }
// result receives values at frame; a temporary cache is used for each sample.
void ResLightAnim::Evaluate(LightAnimResult* result, float frame) const {
    unsigned int first = 0;

    if (flags & 0x100) {
        const ResAnimCurve* curve = &curves[0];
        AnimFrameCache temporary;
        result->enabled = curve->EvaluateInt(frame, &temporary);
        first = 1;
    }

    unsigned int count = curveCount;

    for (unsigned int i = first; i < count; ++i) {
        const ResAnimCurve* curve = &curves[i];
        u32 offset = curve->targetOffset;
        AnimFrameCache temporary;
        *reinterpret_cast<float*>(reinterpret_cast<u8*>(result) + offset) = curve->EvaluateFloat(frame, &temporary);
    }
}

// result receives values at frame; cache stores the current interval for each curve.
void ResLightAnim::Evaluate(LightAnimResult* result, float frame, AnimFrameCache* cache) const {
    unsigned int first = 0;

    if (flags & 0x100) {
        result->enabled = curves[0].EvaluateInt(frame, cache);
        first = 1;
    }

    cache += first;
    unsigned int count = curveCount;

    for (unsigned int i = first; i < count; ++i, ++cache) {
        const ResAnimCurve* curve = &curves[i];
        u32 offset = curve->targetOffset;
        *reinterpret_cast<float*>(reinterpret_cast<u8*>(result) + offset) = curve->EvaluateFloat(frame, cache);
    }
}

// buffer is the beginning of size writable bytes; an empty request already succeeds.
bool ResLightAnim::BakeCurve(void* buffer, size_t size) {
    if (!size) return true;

    if ((buffer == nullptr) || bakedSize > size) return false;
    u8* output = static_cast<u8*>(buffer);
    int count = curveCount;

    for (int i = 0; i < count; ++i) {
        ResAnimCurve* curve = &curves[i];
        size_t bytes = curve->CalculateBakedFloatSize();
        curve->BakeFloat(output, bytes);
        output += bytes;
    }

    flags |= 1;
    return true;
}

void* ResLightAnim::ResetCurve() {
    if (!(flags & 1)) return nullptr;
    void* buffer = nullptr;
    bool found = false;
    int count = curveCount;

    for (int i = 0; i < count; ++i) {
        ResAnimCurve* curve = &curves[i];

        if (!found && ((curve->flags & 0x70) == 0x20 || (curve->flags & 0x70) == 0x70)) {
            buffer = curve->keys;
            found = true;
        }

        curve->Reset();
    }

    flags ^= 1;
    return buffer;
}

void ResLightAnim::Reset() { ResetCurves(); }
// result receives the constant values stored by this fog animation.
void ResFogAnim::Initialize(FogAnimResult* result) const {
    *result = *static_cast<const FogAnimResult*>(baseValues);
}

// table supplies name/length pairs for each supported application function category.
BindResult ResFogAnim::Bind(const BindFuncTable& table) {
    BindResult result;

    if (fogFuncIndex == 0xff) {
        int count = table.lengths[BindFuncTable::Fog];
        const BindFuncTable::StringLength* names = table.strings[BindFuncTable::Fog];

        for (int i = 0; i < count; ++i) {
            if (std::strncmp(fogFuncName + 2, names[i].content, names[i].length) == 0) {
                fogFuncIndex = i;
                break;
            }
        }

        result.Merge(BindResult(fogFuncIndex == 0xff ? BindResult::Flag_Failure : BindResult::Flag_Success));
    }

    return result;
}

void ResFogAnim::Release() { fogFuncIndex = 0xff; }
// result receives values at frame; a temporary cache is used for each sample.
void ResFogAnim::Evaluate(FogAnimResult* result, float frame) const {
    int count = curveCount;

    for (int i = 0; i < count; ++i) {
        const ResAnimCurve* curve = &curves[i];
        u32 offset = curve->targetOffset;
        AnimFrameCache temporary;
        *reinterpret_cast<float*>(reinterpret_cast<u8*>(result) + offset) = curve->EvaluateFloat(frame, &temporary);
    }
}

// result receives values at frame; cache stores the current interval for each curve.
void ResFogAnim::Evaluate(FogAnimResult* result, float frame, AnimFrameCache* cache) const {
    int count = curveCount;

    for (int i = 0; i < count; ++i) {
        const ResAnimCurve* curve = &curves[i];
        u32 offset = curve->targetOffset;
        *reinterpret_cast<float*>(reinterpret_cast<u8*>(result) + offset) = curve->EvaluateFloat(frame, &cache[i]);
    }
}

// buffer is the beginning of size writable bytes; an empty request already succeeds.
bool ResFogAnim::BakeCurve(void* buffer, size_t size) {
    if (!size) return true;

    if ((buffer == nullptr) || bakedSize > size) return false;
    u8* output = static_cast<u8*>(buffer);
    int count = curveCount;

    for (int i = 0; i < count; ++i) {
        ResAnimCurve* curve = &curves[i];
        size_t bytes = curve->CalculateBakedFloatSize();
        curve->BakeFloat(output, bytes);
        output += bytes;
    }

    flags |= 1;
    return true;
}

void* ResFogAnim::ResetCurve() {
    if (!(flags & 1)) return nullptr;
    void* buffer = nullptr;
    bool found = false;
    int count = curveCount;

    for (int i = 0; i < count; ++i) {
        ResAnimCurve* curve = &curves[i];

        if (((curve->flags & 0x70) == 0x20) & !found) {
            buffer = curve->keys;
            found = true;
        }

        curve->Reset();
    }

    flags ^= 1;
    return buffer;
}

void ResFogAnim::Reset() { ResetCurves(); }
// table resolves the light and fog function names in this scene's child resources.
BindResult ResSceneAnim::Bind(const BindFuncTable& table) {
    BindResult result;
    int count = mLightAnimCount;

    for (int i = 0; i < count; ++i) result.Merge(mLightAnims[i].Bind(table));
    count = mFogAnimCount;

    for (int i = 0; i < count; ++i) result.Merge(mFogAnims[i].Bind(table));
    return result;
}
}
