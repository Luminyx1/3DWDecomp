#include <nn/ui2d/ui2d_ShaderContainer.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/util/util_StringUtil.h>
#include <new>
namespace nn::ui2d {
namespace {
// name and candidate are compared through the fixed eight-byte shader name.
inline bool SameShaderName(const char* name, const char* candidate) {
    for (size_t i = 0; i < 8; ++i) {
        if (name[i] != candidate[i]) return false;

        if (!name[i]) return true;
    }

    return true;
}
}

// shared leaves the underlying shader resource alive during finalization.
ShaderRefLink::ShaderRefLink(bool shared) : mShared(shared) { m_Name[0] = 0; }
ShaderRefLink::~ShaderRefLink() = default;
// device owns the shader resources; shared resources are retained.
void ShaderRefLink::Finalize(nn::gfx::Device* device) { mShader.Finalize(device, !mShared); }
// name is copied and terminated within the eight-byte shader name buffer.
void ShaderRefLink::SetName(const char* name) { nn::util::Strlcpy(m_Name, name, sizeof(m_Name)); }
ShaderContainer::~ShaderContainer() = default;
// device finalizes each shader before its registration link is released.
void ShaderContainer::Finalize(nn::gfx::Device* device) {
    while (!mShaders.empty()) {
        auto it = mShaders.begin();
        ShaderRefLink* link = &*it;
        link->Finalize(device);
        mShaders.erase(it);
        link->~ShaderRefLink();
        Layout::FreeMemory(link);
    }
}

// name identifies the new shader; shared controls ownership of its resource.
ShaderInfo* ShaderContainer::RegisterShader(const char* name, bool shared) {
    void* memory = Layout::AllocateMemory(sizeof(ShaderRefLink));

    if (!memory) return nullptr;
    auto* link = new (memory) ShaderRefLink(shared);
    link->SetName(name);
    mShaders.push_back(*link);
    return &link->mShader;
}

// shader is the registered shader whose link should be removed and freed.
void ShaderContainer::UnregisterShader(ShaderInfo* shader) {
    for (auto it = mShaders.begin(); it != mShaders.end(); ++it) {
        auto* link = &*it;

        if (&link->mShader == shader) {
            mShaders.erase(it);
            link->~ShaderRefLink();
            Layout::FreeMemory(link);
            return;
        }
    }
}

// name is compared through the stored shader name's eight-byte limit.
ShaderInfo* ShaderContainer::FindShaderByName(const char* name) const {
    for (auto& link : mShaders) {
        if (SameShaderName(name, link.m_Name)) return const_cast<ShaderInfo*>(&link.mShader);
    }

    return nullptr;
}
}
