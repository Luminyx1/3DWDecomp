#include <nn/ui2d/ui2d_FontContainer.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/font/font_ResFontBase.h>
#include <nn/util/util_StringUtil.h>
#include <new>
namespace nn::ui2d {
FontRefLink::FontRefLink() : mFont(nullptr), mOwned(false) {}
FontRefLink::~FontRefLink() = default;
// device owns the font's GPU resources; only an owned font is destroyed.
void FontRefLink::Finalize(nn::gfx::Device* device) {
    if (mOwned && mFont != nullptr) {
        mFont->Finalize(device);
        nn::font::Font* font = mFont;

        if (font != nullptr) { font->~Font(); Layout::FreeMemory(font); }
        mFont = nullptr;
    }
}

// name is copied into the link, font is the referenced font, and owned transfers
// responsibility for font finalization and deallocation to this link.
void FontRefLink::Set(const char* name, nn::font::Font* font, bool owned) {
    nn::util::Strlcpy(mName, name, sizeof(mName));
    mFont = font;
    mOwned = owned;
}

FontContainer::~FontContainer() = default;
// device finalizes every owned font before the registration links are freed.
void FontContainer::Finalize(nn::gfx::Device* device) {
    while (!mFonts.empty()) {
        auto it = mFonts.begin();
        FontRefLink* link = &*it;
        link->Finalize(device);
        mFonts.erase(it);
        link->~FontRefLink();
        Layout::FreeMemory(link);
    }
}

// name is compared through the stored name's 128-byte limit.
nn::font::Font* FontContainer::FindFontByName(const char* name) const {
    for (auto& link : mFonts) {
        bool same = true;

        for (size_t i = 0; i < sizeof(link.mName); ++i) {
            if (name[i] != link.mName[i]) { same = false; break; }
            if (!name[i]) break;
        }

        if (same) return link.mFont;
    }

    return nullptr;
}

// name identifies font; owned makes the container responsible for its lifetime.
// The returned handle identifies this registration for UnregisterFont.
const void* FontContainer::RegisterFont(const char* name, nn::font::Font* font, bool owned) {
    void* memory = Layout::AllocateMemory(sizeof(FontRefLink));

    if (memory == nullptr) return nullptr;
    auto* link = new (memory) FontRefLink;
    link->Set(name, font, owned);
    mFonts.push_back(*link);
    return link;
}

// handle identifies a registration to remove; the referenced font is retained.
void FontContainer::UnregisterFont(const void* handle) {
    auto* link = const_cast<FontRefLink*>(static_cast<const FontRefLink*>(handle));
    mFonts.erase(List::iterator(reinterpret_cast<nn::util::IntrusiveListNode*>(link)));

    if (link != nullptr) { link->~FontRefLink(); Layout::FreeMemory(link); }
}

// callback allocates descriptor slots for owned resource fonts; argument is its context.
void FontContainer::RegisterTextureViewToDescriptorPool(nn::font::RegisterTextureViewSlot callback, void* argument) {
    for (auto& link : mFonts)
        if (link.mOwned) static_cast<nn::font::ResFontBase*>(link.mFont)->RegisterTextureViewToDescriptorPool(callback, argument);
}

// callback releases descriptor slots for owned resource fonts; argument is its context.
void FontContainer::UnregisterTextureViewFromDescriptorPool(nn::font::UnregisterTextureViewSlot callback, void* argument) {
    for (auto& link : mFonts)
        if (link.mOwned) static_cast<nn::font::ResFontBase*>(link.mFont)->UnregisterTextureViewFromDescriptorPool(callback, argument);
}
}
