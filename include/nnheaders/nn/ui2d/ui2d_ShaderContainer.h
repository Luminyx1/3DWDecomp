#pragma once

#include <nn/gfx/gfx_Types.h>
#include <nn/util/util_IntrusiveList.h>
#include <nn/ui2d/ui2d_ShaderInfo.h>

namespace nn {
namespace ui2d {
class ShaderRefLink {
public:
    ShaderRefLink(bool shared);
    void SetName(const char* name);

    ~ShaderRefLink();

    void Finalize(nn::gfx::Device*);

    nn::util::IntrusiveListNode m_Link;
    char m_Name[8];
    ShaderInfo mShader;
    bool mShared;
};
class ShaderContainer {
public:
    using List = nn::util::IntrusiveList<ShaderRefLink, nn::util::IntrusiveListMemberNodeTraits<ShaderRefLink, &ShaderRefLink::m_Link>>;
    ~ShaderContainer();
    void Finalize(nn::gfx::Device* device);
    ShaderInfo* RegisterShader(const char* name, bool shared);
    void UnregisterShader(ShaderInfo* shader);
    ShaderInfo* FindShaderByName(const char* name) const;
    List mShaders;
};
static_assert(sizeof(ShaderRefLink) == 0x60, "ShaderRefLink size");
};  // namespace ui2d
};  // namespace nn