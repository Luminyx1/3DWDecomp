#include <nn/ui2d/ui2d_ResourceAccessor.h>
#include <nn/ui2d/ui2d_BuildArgSet.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/ui2d/ui2d_Util.h>
#include <nn/font/font_ResFont.h>
#include <cstring>
#include <new>
namespace nn::ui2d {
ResourceAccessor::ResourceAccessor() = default;
ResourceAccessor::~ResourceAccessor() = default;
// device initializes GPU font data, and name selects the font resource.
// Invalid resources release the newly allocated font before returning null.
nn::font::Font* ResourceAccessor::LoadFont(nn::gfx::Device* device, const char* name) {
    size_t size = 0;
    void* data = FindResourceByName(&size, 0x666f6e74, name);
    nn::font::ResFont* font = nullptr;
    if (data && size) {
        void* memory = Layout::AllocateMemory(sizeof(nn::font::ResFont));
        if (memory) {
            font = new (memory) nn::font::ResFont;
            if (!font->SetResource(device, data, nullptr, 0, 0)) {
                font->~ResFont();
                Layout::FreeMemory(font);
                font = nullptr;
            }
        }
    }
    return font;
}
// device is unused by the base accessor, which owns no resources itself.
void ResourceAccessor::Finalize(nn::gfx::Device* device) {}
// output optionally receives the resolved name in a size-byte buffer; device
// acquires the texture. A nonempty prefix is joined to name with a percent sign.
TextureInfo* ResourceAccessor::AcquireDynamicGenerateTexture(char* output, int size, nn::gfx::Device* device, const char* prefix, const char* name) {
    if (prefix) {
        size_t prefixLength = std::strlen(prefix);
        if (prefixLength) {
            size_t nameLength = std::strlen(name);
            char* combined = static_cast<char*>(__builtin_alloca(prefixLength + nameLength + 2));
            std::memcpy(combined, prefix, prefixLength);
            char* suffix = combined + prefixLength;
            *suffix++ = '%';
            std::memcpy(suffix, name, nameLength);
            suffix[nameLength] = 0;
            name = combined;
        }
    }
    TextureInfo* texture = AcquireTexture(device, name);
    if (output) {
        std::strncpy(output, name, size);
        output[size - 1] = 0;
    }
    return texture;
}
// output and size describe an optional resolved-name buffer. args supplies the
// prefix hierarchy, alternate selects its alternate depth, device acquires the
// texture, and name is the texture name before prefix resolution.
TextureInfo* ResourceAccessor::AcquireDynamicGenerateTextureWithResolvePrefix(char* output, int size, const BuildArgSet& args, bool alternate, nn::gfx::Device* device, const char* name) {
    int depth;
    char* prefix = nullptr;
    if (alternate ? (depth = args.mAlternateDynamicTexturePrefixDepth) > 0
                  : (depth = args.mDynamicTexturePrefixDepth) > 0) {
        size_t length = CalcDynamicGenerateTexturePrefixLength(args, depth);
        prefix = static_cast<char*>(__builtin_alloca(length));
        ConcatDynamicGenerateTexturePrefixString(prefix, length, args, depth);
    }
    return AcquireDynamicGenerateTexture(output, size, device, prefix, name);
}
}
