#include <nn/ui2d/ui2d_Parts.h>
#include <nn/ui2d/ui2d_Material.h>

namespace nn::ui2d {
namespace {
// pStored and pName are the names to compare; length caps the fixed-width resource name.
bool SameName(const char* pStored, const char* pName, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        if (pStored[i] != pName[i]) return false;
        if (pStored[i] == '\0') return true;
    }

    return true;
}
}

Parts::Parts() : m_pLayout(nullptr) {}

// pResource supplies pane properties; pOverride is unused here;
// rArgs supplies the owning layout and build context.
Parts::Parts(const ResParts* pResource, const ResParts* pOverride, const BuildArgSet& rArgs)
    : Pane(nullptr, nullptr, reinterpret_cast<const ResPane*>(pResource), rArgs),
      m_pLayout(nullptr) {}

// rOther supplies pane properties; the nested layout and its parts list start empty.
Parts::Parts(const Parts& rOther) : Pane(rOther), m_pLayout(nullptr) {}

Parts::~Parts() = default;

// pName identifies this parts pane; lookup does not descend into the nested layout.
Pane* Parts::FindPaneByNameRecursive(const char* pName) {
    return SameName(mPanelName, pName, 24) ? this : nullptr;
}

// pName is forwarded to the mutable virtual lookup without modifying the parts pane.
const Pane* Parts::FindPaneByNameRecursive(const char* pName) const {
    return const_cast<Parts*>(this)->FindPaneByNameRecursive(pName);
}

// pName identifies a material attached directly to this parts pane.
Material* Parts::FindMaterialByNameRecursive(const char* pName) {
    const u8 count = GetMaterialCount();
    for (u32 i = 0; i < count; ++i) {
        auto* material = GetMaterial(i);
        if (material && SameName(material->mName, pName, 28)) return material;
    }

    return nullptr;
}

// pName is forwarded to the mutable virtual material lookup.
const Material* Parts::FindMaterialByNameRecursive(const char* pName) const {
    return const_cast<Parts*>(this)->FindMaterialByNameRecursive(pName);
}

// rOther is unused because this check does not compare the nested layout.
bool Parts::CompareCopiedInstanceTest(const Parts& rOther) const { return true; }

static_assert(sizeof(Parts) == 0xf0, "Parts size");
}  // namespace nn::ui2d
