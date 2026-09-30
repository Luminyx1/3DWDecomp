#include <nn/g3d/g3d_Resources.h>
#include <nn/gfx/gfx_Sampler.h>
#include <nn/gfx/gfx_SamplerInfo.h>
#include <nn/util/util_Arithmetic.h>
#include <cstring>
#include <new>

namespace nn::g3d {
// type identifies scalar/vector, matrix, or transform parameter data.
size_t ResShaderParam::GetSize(Type type) {
    if (type <= 15)
        return ((type & 3) + 1) * 4;
    if (type <= 27)
        return size_t(((type - 16) >> 2) + 2) * 16;
    static const size_t sizes[] asm("lbl_71016C3F40") = {24, 48, 24};
    return sizes[type - 28];
}
// type identifies packed source data before uniform-block conversion.
size_t ResShaderParam::GetSrcSize(Type type) {
    if (type <= 15)
        return ((type & 3) + 1) * 4;
    if (type <= 27) {
        int rows = ((type - 16) >> 2) + 2;
        size_t bytes = ((type & 3) + 1) * 4;
        return bytes * rows;
    }
    static const size_t sizes[] asm("lbl_710140A880") = {20, 36, 24, 32};
    return sizes[type - 28];
}
// destination receives source scalars or matrix rows; swap is unused for these representations.
template <bool swap> void ResShaderParam::Convert(void* destination, const void* source) const {
    Type format = Type(type);
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
// source holds parameter storage; dependency is installed only in a root parameter's extra pointer slot.
bool ResShaderParam::SetDependPointer(void* source, const void* dependency) const {
    if (sourceSize <= GetSrcSize(Type(type)) || index != dependencyIndex)
        return false;
    uintptr_t address = (reinterpret_cast<uintptr_t>(source) + GetSrcSize(Type(type)) + 7) & ~uintptr_t(7);
    *reinterpret_cast<const void**>(address) = dependency;
    return true;
}
// source holds parameter storage; dependency receives its optional aligned pointer.
bool ResShaderParam::GetDependPointer(void** dependency, const void* source) const {
    if (sourceSize <= GetSrcSize(Type(type))) {
        *dependency = nullptr;
        return false;
    }
    uintptr_t address = (reinterpret_cast<uintptr_t>(source) + GetSrcSize(Type(type)) + 7) & ~uintptr_t(7);
    *dependency = *reinterpret_cast<void* const*>(address);
    return true;
}
// destination receives a 2D affine matrix from source scale, rotation, and translation.
// parameter and user are unused by this built-in conversion.
size_t ResShaderParam::ConvertSrt2dCallback(void* destination, const void* source,
                                            const ResShaderParam* parameter, const void* user) {
    const float* input = static_cast<const float*>(source);
    float* output = static_cast<float*>(destination);
    nn::util::AngleIndex angle = nn::util::RadianToAngleIndex(input[2]);
    float sine = nn::util::SinTable(angle);
    float cosine = nn::util::CosTable(angle);
    output[0] = input[0] * cosine;
    output[1] = input[0] * sine;
    output[2] = -input[1] * sine;
    output[3] = cosine * input[1];
    output[4] = input[3];
    output[5] = input[4];
    return 24;
}
// callback resolves missing texture views and descriptors; user is passed to that callback.
BindResult ResMaterial::BindTexture(TextureBindCallback callback, void* user) {
    BindResult result;
    int count = samplerCount;
    for (int i = 0; i < count; ++i) {
        if ((GetTextureView(i) != nullptr) && GetTextureDescriptorSlot(i) != TextureRef::InvalidDescriptorSlot)
            continue;
        TextureRef texture = callback(GetTextureName(i), user);
        ForceBindTexture(i, texture);
        if ((texture.GetTextureView() == nullptr) || texture.GetDescriptorSlot() == TextureRef::InvalidDescriptorSlot)
            result.Merge(BindResult(BindResult::Flag_Failure));
        else
            result.Merge(BindResult(BindResult::Flag_Success));
    }
    return result;
}
// texture replaces every binding whose stored name equals name.
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
void ResMaterial::ReleaseTexture() {
    int count = samplerCount;
    for (int i = 0; i < count; ++i)
        ReleaseTexture(i);
}
// device owns samplers constructed from resource descriptions and named for debugging.
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
    static const ShaderParamConvertCallback callbacks[] asm(
        "lbl_7101AD4FA0") = {ResShaderParam::ConvertSrt2dCallback, ResShaderParam::ConvertSrt3dCallback,
                             ResShaderParam::ConvertTexSrtCallback, ResShaderParam::ConvertTexSrtExCallback};
    count = shaderParamCount;
    for (int i = 0; i < count; ++i) {
        ResShaderParamData* parameter = &pShaderParamArray.Get()[i];
        if (parameter->type >= 28 && (parameter->callback == nullptr))
            parameter->callback = callbacks[parameter->type - 28];
    }
}
// device owns initialized sampler objects being finalized and destroyed.
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
    std::memset(pVolatileParamFlags.Get(), 0, size_t(count) / 32);
}
// guard bit zero preserves the caller-owned user pointer while clearing material state.
void ResMaterial::Reset(u32 guard) {
    int count = shaderParamCount;
    for (int i = 0; i < count; ++i) {
        ResShaderParamData* parameter = &pShaderParamArray.Get()[i];
        parameter->offset = -1;
        parameter->callback = nullptr;
    }
    textureCount = samplerCount;
    materialBlockSize = 0;
    if (!(guard & 1))
        pUserPtr.Clear();
    volatileParamCount = 0;
    std::memset(pVolatileParamFlags.Get(), 0, size_t(count) / 32);
}
} // namespace nn::g3d
