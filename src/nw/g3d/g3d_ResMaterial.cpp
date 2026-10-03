#include <nn/g3d/g3d_Resources.h>
#include <nn/gfx/gfx_Sampler.h>
#include <nn/gfx/gfx_SamplerInfo.h>
#include <nn/util/util_Arithmetic.h>
#include <cstring>
#include <new>

namespace nn::g3d {
namespace {
struct TextureSrt {
    u32 mode;
    util::Float2 scale;
    float rotation;
    util::Float2 translation;
};
struct TextureSrtEx : TextureSrt {
    const float* pDependency;
};
static_assert(sizeof(TextureSrt) == 0x18);
static_assert(sizeof(TextureSrtEx) == 0x20);

struct RotationValues {
    float sine;
    float cosine;
};
/**
 * @brief Evaluate sine and cosine using the resource transform lookup table.
 * @param rotation Rotation angle in radians.
 * @return Interpolated sine and cosine for the angle.
 */
inline RotationValues EvaluateRotation(float rotation) {
    util::AngleIndex angle = util::RadianToAngleIndex(rotation);
    return {util::SinTable(angle), util::CosTable(angle)};
}

static size_t ConvertTextureMode0(void* pDestination, const void* pSource) asm("sub_71005FECC0");
static size_t ConvertTextureMode1(void* pDestination, const void* pSource) asm("sub_71005FEDB0");
static size_t ConvertTextureMode2(void* pDestination, const void* pSource) asm("sub_71005FEE90");

/**
 * @brief Convert texture-transform mode zero to six affine coefficients.
 * @param pDestination Writable storage for six floats.
 * @param pSource Packed texture scale, rotation in radians, and translation.
 * @return Number of bytes written, always 24.
 */
static size_t ConvertTextureMode0(void* pDestination, const void* pSource) {
    const auto* pInput = static_cast<const TextureSrt*>(pSource);
    auto* pOutput = static_cast<float*>(pDestination);
    RotationValues values = EvaluateRotation(pInput->rotation);
    float sine = values.sine;
    float cosine = values.cosine;
    float halfSine = sine * 0.5f - 0.5f;
    float halfCosine = cosine * -0.5f;
    pOutput[0] = pInput->scale.x * cosine;
    pOutput[1] = -pInput->scale.y * sine;
    pOutput[2] = pInput->scale.x * sine;
    pOutput[3] = cosine * pInput->scale.y;
    pOutput[4] = pInput->scale.x * ((halfCosine - halfSine) - pInput->translation.x);
    pOutput[5] = pInput->scale.y * ((halfCosine + halfSine) + pInput->translation.y) + 1.0f;
    return 6 * sizeof(float);
}
/**
 * @brief Convert texture-transform mode one around the texture center.
 * @param pDestination Writable storage for six floats.
 * @param pSource Packed texture scale, rotation in radians, and translation.
 * @return Number of bytes written, always 24.
 */
static size_t ConvertTextureMode1(void* pDestination, const void* pSource) {
    const auto* pInput = static_cast<const TextureSrt*>(pSource);
    auto* pOutput = static_cast<float*>(pDestination);
    RotationValues values = EvaluateRotation(pInput->rotation);
    float sine = values.sine;
    float cosine = values.cosine;
    float x = pInput->scale.x;
    float y = pInput->scale.y;
    float xc = x * cosine;
    float ys = y * sine;
    float xs = x * sine;
    float yc = y * cosine;
    pOutput[0] = xc;
    pOutput[1] = -y * sine;
    pOutput[2] = xs;
    pOutput[3] = yc;
    float offsetX = xc * (pInput->translation.x + 0.5f);
    pOutput[4] = xs * (pInput->translation.y - 0.5f) - offsetX + 0.5f;
    pOutput[5] = ys * (pInput->translation.x + 0.5f) + yc * (pInput->translation.y - 0.5f) + 0.5f;
    return 6 * sizeof(float);
}
/**
 * @brief Convert texture-transform mode two with its inverted vertical-axis convention.
 * @param pDestination Writable storage for six floats.
 * @param pSource Packed texture scale, rotation in radians, and translation.
 * @return Number of bytes written, always 24.
 */
static size_t ConvertTextureMode2(void* pDestination, const void* pSource) {
    const auto* pInput = static_cast<const TextureSrt*>(pSource);
    auto* pOutput = static_cast<float*>(pDestination);
    RotationValues values = EvaluateRotation(pInput->rotation);
    float sine = values.sine;
    float cosine = values.cosine;
    float x = pInput->scale.x;
    float y = pInput->scale.y;
    float xc = x * cosine;
    float xs = x * sine;
    float yc = y * cosine;
    float ys = y * sine;
    pOutput[2] = -x * sine;
    pOutput[3] = yc;
    pOutput[0] = xc;
    pOutput[1] = ys;
    pOutput[4] = (xs - xc * pInput->translation.x) - xs * pInput->translation.y;
    pOutput[5] = (-y * cosine - ys * pInput->translation.x) + yc * pInput->translation.y + 1.0f;
    return 6 * sizeof(float);
}
} // namespace
/**
 * @brief Get the converted storage size of a shader-parameter type.
 * @param type Valid scalar, vector, matrix or transform parameter type.
 * @return Converted size in bytes.
 */
size_t ResShaderParam::GetSize(Type type) {
    if (type <= 15) {
        return ((type & 3) + 1) * 4;
    }
    if (type <= 27) {
        return static_cast<size_t>(((type - 16) >> 2) + 2) * 16;
    }
    static const size_t sizes[] asm("lbl_71016C3F40") = {24, 48, 24};
    return sizes[type - 28];
}

/**
 * @brief Get the packed source size of a shader-parameter type.
 * @param type Valid scalar, vector, matrix or transform parameter type.
 * @return Packed source size in bytes.
 */
size_t ResShaderParam::GetSrcSize(Type type) {
    if (type <= 15) {
        return ((type & 3) + 1) * 4;
    }
    if (type <= 27) {
        int rows = ((type - 16) >> 2) + 2;
        size_t bytes = ((type & 3) + 1) * 4;
        return bytes * rows;
    }

    static const size_t sizes[] asm("lbl_710140A880") = {20, 36, 24, 32};
    return sizes[type - 28];
}

/**
 * @brief Copy scalar values or pad matrix rows into uniform-block storage.
 * @tparam swap Byte-order policy; unused for these supported representations.
 * @param destination Writable storage of at least GetSize(type) bytes.
 * @param source Packed parameter values with at least GetSrcSize(type) bytes.
 */
template <bool swap> void ResShaderParam::Convert(void* destination, const void* source) const {
    Type format = static_cast<Type>(type);

    if (format <= 15) {
        std::memcpy(destination, source, ((format & 3) + 1) * 4);
    } else if (format <= 27) {
        int rows = ((format - 16) >> 2) + 2;
        size_t bytes = ((format & 3) + 1) * 4;
        u8* output = static_cast<u8*>(destination);
        const u8* input = static_cast<const u8*>(source);

        for (int i = 0; i < rows; ++i) {
            std::memcpy(output, input, bytes);
            output += 16;
            input += bytes;
        }
    }
}

template void ResShaderParam::Convert<true>(void*, const void*) const;
template void ResShaderParam::Convert<false>(void*, const void*) const;
/**
 * @brief Store a dependency pointer in a root parameter's extended storage.
 * @param source Writable packed parameter storage including its optional aligned pointer slot.
 * @param dependency Dependency pointer to install; may be nullptr.
 * @return True when the parameter has an extended slot and is a dependency root.
 */
bool ResShaderParam::SetDependPointer(void* source, const void* dependency) const {
    if (sourceSize <= GetSrcSize(static_cast<Type>(type)) || index != dependencyIndex) {
        return false;
    }
    uintptr_t address =
        (reinterpret_cast<uintptr_t>(source) + GetSrcSize(static_cast<Type>(type)) + 7) & ~uintptr_t(7);
    *reinterpret_cast<const void**>(address) = dependency;
    return true;
}

/**
 * @brief Read a parameter's optional dependency pointer.
 * @param dependency Non-null output receiving the pointer, or nullptr when no slot exists.
 * @param source Packed parameter storage including any optional aligned pointer slot.
 * @return True when the source contains a dependency slot.
 */
bool ResShaderParam::GetDependPointer(void** dependency, const void* source) const {
    if (sourceSize <= GetSrcSize(static_cast<Type>(type))) {
        *dependency = nullptr;
        return false;
    }

    uintptr_t address =
        (reinterpret_cast<uintptr_t>(source) + GetSrcSize(static_cast<Type>(type)) + 7) & ~uintptr_t(7);
    *dependency = *reinterpret_cast<void* const*>(address);
    return true;
}

/**
 * @brief Convert scale, rotation and translation to six affine coefficients.
 * @param destination Writable storage for six floats.
 * @param source Five floats containing two scale components, a radian angle and two translations.
 * @param parameter Parameter description, unused by this built-in conversion.
 * @param user User context, unused by this built-in conversion.
 * @return Number of bytes written, always 24.
 */
size_t ResShaderParam::ConvertSrt2dCallback(void* destination, const void* source,
                                            const ResShaderParam* parameter, const void* user) {
    const float* input = static_cast<const float*>(source);
    float* output = static_cast<float*>(destination);
    RotationValues values = EvaluateRotation(input[2]);
    float sine = values.sine;
    float cosine = values.cosine;
    output[0] = input[0] * cosine;
    output[1] = input[0] * sine;
    output[2] = -input[1] * sine;
    output[3] = cosine * input[1];
    output[4] = input[3];
    output[5] = input[4];
    return 24;
}

/**
 * @brief Dispatch a packed texture transform to its selected conversion convention.
 * @param pDestination Writable storage for six output floats.
 * @param pSource Packed texture transform whose mode is in the range [0, 3).
 * @param pParameter Parameter description, unused by the built-in converters.
 * @param pUser User context, unused by the built-in converters.
 * @return Number of bytes written, always 24.
 */
size_t ResShaderParam::ConvertTexSrtCallback(void* pDestination, const void* pSource,
                                             const ResShaderParam* pParameter, const void* pUser) {
    using ConvertFunction = size_t (*)(void*, const void*);
    static const ConvertFunction converters[] asm("lbl_7101AD4F70") = {
        ConvertTextureMode0, ConvertTextureMode1, ConvertTextureMode2};
    return converters[static_cast<const TextureSrt*>(pSource)->mode](pDestination, pSource);
}

/**
 * @brief Convert packed two-dimensional transform coefficients to an extended matrix.
 * @param pDestination Writable storage for twelve output floats; must not be null.
 * @param pSource Packed transform and optional twelve-float dependency matrix; must not be null.
 * @param pParameter Parameter description, unused by this built-in conversion.
 * @param pUser User context, unused by this built-in conversion.
 * @return Number of bytes written, always 48.
 */
size_t ResShaderParam::ConvertSrt2dExCallback(void* pDestination, const void* pSource,
                                              const ResShaderParam* pParameter, const void* pUser) {
    const auto* pInput = static_cast<const TextureSrtEx*>(pSource);
    auto* pOutput = static_cast<float*>(pDestination);
    RotationValues values = EvaluateRotation(pInput->rotation);
    float sine = values.sine;
    float cosine = values.cosine;
    float x = pInput->scale.x;
    float y = pInput->scale.y;
    float xc = x * cosine;
    float ys = y * sine;
    float xs = x * sine;
    float yc = y * cosine;
    const float* pDependency = pInput->pDependency;
    if (pDependency != nullptr) {
        for (int i = 0; i < 4; ++i) {
            pOutput[i] = xc * pDependency[i] - xs * pDependency[4 + i] + x * pDependency[8 + i];
            pOutput[4 + i] = ys * pDependency[i] + yc * pDependency[4 + i] + y * pDependency[8 + i];
        }
        std::memcpy(pOutput + 8, pDependency + 8, 4 * sizeof(float));
    } else {
        pOutput[4] = ys;
        pOutput[5] = yc;
        pOutput[7] = 0.0f;
        pOutput[8] = 0.0f;
        pOutput[0] = xc;
        pOutput[1] = -xs;
        pOutput[2] = x;
        pOutput[6] = y;
        pOutput[11] = 0.0f;
        pOutput[9] = 0.0f;
        pOutput[10] = 1.0f;
        pOutput[3] = 0.0f;
    }
    return 12 * sizeof(float);
}

/**
 * @brief Resolve missing texture bindings using a caller-supplied callback.
 * @param callback Non-null function resolving a texture name to its view and descriptor.
 * @param user Opaque context forwarded unchanged to the callback.
 * @return Combined success and failure flags for attempted bindings.
 */
BindResult ResMaterial::BindTexture(TextureBindCallback callback, void* user) {
    BindResult result;
    int count = samplerCount;

    for (int i = 0; i < count; ++i) {
        if ((GetTextureView(i) != nullptr) &&
            GetTextureDescriptorSlot(i) != TextureRef::InvalidDescriptorSlot) {
            continue;
        }
        TextureRef texture = callback(GetTextureName(i), user);
        ForceBindTexture(i, texture);

        if ((texture.GetTextureView() == nullptr) ||
            texture.GetDescriptorSlot() == TextureRef::InvalidDescriptorSlot) {

            result.Merge(BindResult(BindResult::Flag_Failure));

        } else {
            result.Merge(BindResult(BindResult::Flag_Success));
        }
    }

    return result;
}

/**
 * @brief Replace every texture binding with the specified name.
 * @param texture Replacement view and descriptor slot.
 * @param name Non-null texture name to compare against stored names.
 * @return True if at least one stored texture name matched.
 */
bool ResMaterial::ForceBindTexture(const TextureRef& texture, const char* name) {
    bool found = false;
    int count = samplerCount;

    for (int i = 0; i < count; ++i) {
        if (strcmp(GetTextureName(i), name) == 0) {
            ForceBindTexture(i, texture);
            found = true;
        }
    }

    return found;
}

/** @brief Release all material texture views and invalidate their descriptor slots. */
void ResMaterial::ReleaseTexture() {
    int count = samplerCount;

    for (int i = 0; i < count; ++i) {
        ReleaseTexture(i);
    }
}

/**
 * @brief Initialize samplers and install missing built-in shader-parameter converters.
 * @param device Initialized graphics device owning the sampler objects.
 */
void ResMaterial::Setup(nn::gfx::Device* device) {
    int count = samplerCount;

    for (int i = 0; i < count; ++i) {
        const nn::gfx::SamplerInfo* info = &pSamplerInfoArray.Get()[i];
        nn::gfx::Sampler* sampler = reinterpret_cast<nn::gfx::Sampler*>(&pSamplerArray.Get()[i]);
        new (sampler) nn::gfx::Sampler;
        sampler->Initialize(device, *info);
        nn::util::ResDic* dictionary = pSamplerDic.Get();
        const char* name = (dictionary != nullptr) ? dictionary->GetKey(i).data() : nullptr;
        nn::gfx::util::SetSamplerDebugLabel(sampler, name);
    }

    static const ShaderParamConvertCallback callbacks[] asm("lbl_7101AD4FA0") = {
        ResShaderParam::ConvertSrt2dCallback, ResShaderParam::ConvertSrt3dCallback,
        ResShaderParam::ConvertTexSrtCallback, ResShaderParam::ConvertTexSrtExCallback};
    count = shaderParamCount;

    for (int i = 0; i < count; ++i) {
        ResShaderParamData* parameter = &pShaderParamArray.Get()[i];

        if (parameter->type >= 28 && (parameter->callback == nullptr)) {

            parameter->callback = callbacks[parameter->type - 28];
        }
    }
}

/**
 * @brief Finalize and destroy initialized material samplers.
 * @param device Graphics device used to initialize the samplers.
 */
void ResMaterial::Cleanup(nn::gfx::Device* device) {
    int count = samplerCount;

    for (int i = 0; i < count; ++i) {
        nn::gfx::Sampler* sampler = reinterpret_cast<nn::gfx::Sampler*>(&pSamplerArray.Get()[i]);

        if (sampler->ToData()->state) {
            sampler->Finalize(device);
            sampler->~TSampler();
        }
    }
}

/** @brief Reset parameter conversion, texture counts and the caller-owned user pointer. */
void ResMaterial::Reset() {
    int count = shaderParamCount;

    for (int i = 0; i < count; ++i) {
        ResShaderParamData* parameter = &pShaderParamArray.Get()[i];
        parameter->offset = -1;
        parameter->callback = nullptr;
    }

    materialBlockSize = 0;
    pUserPtr.Clear();
    volatileParamCount = 0;
    textureCount = samplerCount;
    std::memset(pVolatileParamFlags.Get(), 0, static_cast<size_t>(count) / 32);
}

/**
 * @brief Reset conversion and texture state with optional user-pointer preservation.
 * @param guard Bit zero preserves the user pointer; other bits are unused.
 */
void ResMaterial::Reset(u32 guard) {
    int count = shaderParamCount;

    for (int i = 0; i < count; ++i) {
        ResShaderParamData* parameter = &pShaderParamArray.Get()[i];
        parameter->offset = -1;
        parameter->callback = nullptr;
    }

    textureCount = samplerCount;
    materialBlockSize = 0;

    if (!(guard & 1)) {

        pUserPtr.Clear();
    }
    volatileParamCount = 0;
    std::memset(pVolatileParamFlags.Get(), 0, static_cast<size_t>(count) / 32);
}
} // namespace nn::g3d
