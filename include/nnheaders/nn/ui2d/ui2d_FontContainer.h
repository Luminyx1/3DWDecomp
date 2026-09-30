#pragma once
#include <nn/font/font_Font.h>
#include <nn/util/util_IntrusiveList.h>
namespace nn::ui2d {
class FontRefLink {
public:
    FontRefLink();
    ~FontRefLink();
    void Finalize(nn::gfx::Device* device);
    void Set(const char* name, nn::font::Font* font, bool owned);
    nn::util::IntrusiveListNode m_Link;
    char mName[128];
    nn::font::Font* mFont;
    bool mOwned;
};
static_assert(sizeof(FontRefLink) == 0xa0, "FontRefLink size");
class FontContainer {
public:
    using List = nn::util::IntrusiveList<FontRefLink, nn::util::IntrusiveListMemberNodeTraits<FontRefLink, &FontRefLink::m_Link>>;
    ~FontContainer();
    void Finalize(nn::gfx::Device* device);
    nn::font::Font* FindFontByName(const char* name) const;
    const void* RegisterFont(const char* name, nn::font::Font* font, bool owned);
    void UnregisterFont(const void* handle);
    void RegisterTextureViewToDescriptorPool(nn::font::RegisterTextureViewSlot callback, void* argument);
    void UnregisterTextureViewFromDescriptorPool(nn::font::UnregisterTextureViewSlot callback, void* argument);
    List mFonts;
};
}
