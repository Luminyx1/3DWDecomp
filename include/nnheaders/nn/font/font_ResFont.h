#pragma once

#include <nn/font/font_ResFontBase.h>

namespace nn {
namespace font {

class ResFont : public ResFontBase {
public:
    NN_RUNTIME_TYPEINFO(ResFontBase);

    ResFont();
    ~ResFont() override;

    void Finalize(nn::gfx::Device* pDevice) override;

    bool SetResource(nn::gfx::Device* pDevice, void* pBfnt, nn::gfx::MemoryPool* pMemoryPool,
                     ptrdiff_t memoryPoolOffset, size_t memoryPoolSize);
    void* RemoveResource(nn::gfx::Device* pDevice);

    static void RevertResource(void* pBfnt);
    static void Unrelocate(void* pBfnt);

private:
    static FontInformation* Rebuild(detail::BinaryFileHeader* pHeader);
};

}  // namespace font
}  // namespace nn
