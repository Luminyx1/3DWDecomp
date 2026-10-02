#include "Library/Layout/LayoutSystem.hpp"

#include <eui/euiArcResourceMgr.h>
#include <eui/euiConstantBuffer.h>
#include <eui/euiFontMgr.h>
#include <eui/euiMessageMgr.h>
#include <eui/euiScalableFontMgr.h>
#include <eui/euiScreenMgr.h>
#include <eui/euiUtility.h>
#include <gfx/nin/seadGraphicsNvn.h>
#include <heap/seadExpHeap.h>
#include <heap/seadHeapMgr.h>
#include <nn/font/font_TextureCache.h>
#include <nn/os.h>
#include <nn/pl.h>
#include <nn/time.h>
#include <nn/ui2d/ui2d_GraphicsResource.h>
#include <resource/seadResource.h>
#include <stream/seadRamStream.h>
#include <stream/seadStreamFormat.h>
#include <cstdio>

#include "Library/File/FileUtil.hpp"
#include "Library/Layout/EuiScreenFactory.hpp"
#include "Library/Layout/LayoutInitFunction.hpp"
#include "Library/Memory/HeapUtil.hpp"
#include "Library/Message/LanguageUtil.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {
using al::StringTmp;

typedef eui::ScalableFontMgr::FontParameter FontParameter;

class LayoutScalableFontMgr : public eui::ScalableFontMgr {};

const s32 cScalableFontNum = 7;

sead::Color4u8 sExtraGradationColors[3] = {
    {255, 255, 0, 255},
    {255, 255, 0, 255},
    {80, 160, 210, 255},
};

FontParameter sFontParameters[3][cScalableFontNum] = {
    {
        FontParameter("nintendo_udsg-r_std_003_10.fcpx", 10, 0, 0),
        FontParameter("nintendo_udsg-r_std_003_20.fcpx", 20, 0, 0),
        FontParameter("nintendo_udsg-r_std_003_40.fcpx", 40, 0, 0),
        FontParameter("nintendo_udsg-r_std_003_80.fcpx", 80, 0, 0),
        FontParameter("nintendo_udsg-r_zh-cn_003.fcpx", 40, 1, 0),
        FontParameter("nintendo_udjxh-db_zh-tw_003.fcpx", 40, 2, 0),
        FontParameter("nintendo_udjxh-db_ko_003.fcpx", 40, 3, 0),
    },
    {
        FontParameter("nintendo_udsg-r_std_003_10.fcpx", 10, 1, 0),
        FontParameter("nintendo_udsg-r_std_003_20.fcpx", 20, 1, 0),
        FontParameter("nintendo_udsg-r_std_003_40.fcpx", 40, 1, 0),
        FontParameter("nintendo_udsg-r_std_003_80.fcpx", 80, 1, 0),
        FontParameter("nintendo_udsg-r_zh-cn_003.fcpx", 40, 1, 0),
        FontParameter("nintendo_udjxh-db_zh-tw_003.fcpx", 40, 2, 0),
        FontParameter("nintendo_udjxh-db_ko_003.fcpx", 40, 3, 0),
    },
    {
        FontParameter("nintendo_udsg-r_std_003_10.fcpx", 10, 2, 0),
        FontParameter("nintendo_udsg-r_std_003_20.fcpx", 20, 2, 0),
        FontParameter("nintendo_udsg-r_std_003_40.fcpx", 40, 2, 0),
        FontParameter("nintendo_udsg-r_std_003_80.fcpx", 80, 2, 0),
        FontParameter("nintendo_udsg-r_zh-cn_003.fcpx", 40, 1, 0),
        FontParameter("nintendo_udjxh-db_zh-tw_003.fcpx", 40, 2, 0),
        FontParameter("nintendo_udjxh-db_ko_003.fcpx", 40, 3, 0),
    },
};

const nn::pl::SharedFontType cSharedFontTypes[4][6] = {
    {nn::pl::STANDARD, nn::pl::CHINESE_SIMPLIFIED, nn::pl::CHINESE_TRAD, nn::pl::KOREAN,
     nn::pl::NN_EXT, nn::pl::NN_EXT},
    {nn::pl::STANDARD, nn::pl::CHINESE_SIMPLIFIED, nn::pl::CHINESE_TRAD, nn::pl::KOREAN,
     nn::pl::NN_EXT, nn::pl::NN_EXT},
    {nn::pl::CHINESE_TRAD, nn::pl::STANDARD, nn::pl::CHINESE_TRAD, nn::pl::CHINESE_SIMPLIFIED,
     nn::pl::KOREAN, nn::pl::NN_EXT},
    {nn::pl::KOREAN, nn::pl::STANDARD, nn::pl::CHINESE_SIMPLIFIED, nn::pl::CHINESE_TRAD,
     nn::pl::NN_EXT, nn::pl::STANDARD},
};

inline const FontParameter* getFontParameters() {
    if (al::getLanguageCode() == 6) {
        return sFontParameters[1];
    }

    if (al::getLanguageCode() == 11) {
        return sFontParameters[2];
    }

    return sFontParameters[0];
}

inline s32 getFontParameterNum() {
    if (al::getLanguageCode() == 6) {
        return cScalableFontNum;
    }

    if (al::getLanguageCode() == 11) {
        return cScalableFontNum;
    }

    return cScalableFontNum;
}

inline void waitSharedFontLoad(nn::pl::SharedFontType type) {
    while (nn::pl::GetSharedFontLoadState(type) != nn::pl::LOADED) {
        nn::os::SleepThread(nn::TimeSpan::FromMilliSeconds(100));
    }
}

/**
 * Creates the scalable font manager from the system shared fonts.
 * @param pHeap heap to create the manager in
 * @param isSmallFontHeap whether the font heap is the small one
 * @return the scalable font manager
 */
eui::ScalableFontMgr* createScalableFontMgr(sead::Heap* pHeap, bool isSmallFontHeap) {
    sead::ScopedCurrentHeapSetter setter(pHeap);
    eui::ScalableFontMgr* fontMgr = new LayoutScalableFontMgr();

    nn::font::TextureCache::InitializeArg arg;
    arg.SetDefault();
    arg.noPlotWorkMemorySize = 0x20000;
    arg.glyphNodeCountMax = 0x800;
    arg.fontFaceCount = 4;

    for (s32 i = 0; i < 4; i++) {
        for (s32 j = 0; j < 6; j++) {
            nn::pl::SharedFontType type = cSharedFontTypes[i][j];
            waitSharedFontLoad(type);
            arg.pFontDatas[i][j] = nn::pl::GetSharedFontAddress(type);
            arg.fontDataSizes[i][j] = nn::pl::GetSharedFontSize(type);
            arg.fontDataTypes[i][j] = nn::font::TextureCache::FontDataType_Ttf;

            if (j == 0 && i == 2 && !isSmallFontHeap) {
                arg.charCodeRangeCounts[i][j] = 2;
                arg.charCodeRangeFirsts[i][j][0] = 0x3001;
                arg.charCodeRangeLasts[i][j][0] = 0x303f;
                arg.charCodeRangeFirsts[i][j][1] = 0xff0c;
                arg.charCodeRangeLasts[i][j][1] = 0xff0c;
            }
        }

        arg.innerFontCounts[i] = 6;
    }

    arg.textureCacheWidth = 0x800;
    arg.textureCacheHeight = 0x1100;

    eui::ScalableFontMgr::InitializeArg mgrArg;
    mgrArg.textureCacheArg = &arg;
    mgrArg.heap = pHeap;
    mgrArg.fontParameters = getFontParameters();
    mgrArg.fontParameterNum = getFontParameterNum();
    fontMgr->initialize(mgrArg);
    return fontMgr;
}

inline sead::Buffer<const char*> makeFontNameBuffer(s32 num, const char** pNames) {
    if (num > 0) {
        return sead::Buffer<const char*>(num, pNames);
    }

    return sead::Buffer<const char*>();
}

inline eui::MessageMgr* createMessageMgr(sead::Heap* pHeap) {
    eui::MessageMgr* messageMgr = new eui::MessageMgr();
    messageMgr->initialize(pHeap, 11);

    StringTmp<128> messageArchivePath;
    al::makeLocalizedArchivePath(&messageArchivePath, "MessageData/LayoutMessage");
    al::findOrCreateResource(messageArchivePath, nullptr);
    messageMgr->setGradationColor(0, sead::Color4u8::cWhite, sead::Color4u8::cWhite);
    messageMgr->setGradationColor(1, {255, 12, 12, 255}, {255, 12, 12, 255});
    messageMgr->setGradationColor(2, {0, 149, 0, 255}, {0, 149, 0, 255});
    messageMgr->setGradationColor(3, {0, 40, 255, 255}, {0, 40, 255, 255});
    messageMgr->setGradationColor(4, {200, 150, 0, 255}, {200, 150, 0, 255});
    messageMgr->setGradationColor(5, {255, 50, 80, 255}, {255, 50, 80, 255});
    messageMgr->setGradationColor(6, {0, 140, 255, 255}, {0, 140, 255, 255});
    messageMgr->setGradationColor(7, {120, 120, 120, 255}, {120, 120, 120, 255});
    messageMgr->setGradationColor(8, {255, 53, 0, 255}, {255, 53, 0, 255});
    messageMgr->setGradationColor(9, {0, 0, 0, 255}, {0, 0, 0, 255});
    messageMgr->setGradationColor(10, sExtraGradationColors[2], sExtraGradationColors[2]);
    return messageMgr;
}

inline void makeFontDataArchivePath(StringTmp<256>* pPath) {
    al::makeLocalizedArchivePath(pPath, "LayoutData/FontData");

    if (!al::isExistArchive(*pPath)) {
        *pPath = StringTmp<256>("FontData/FontData");
        al::isExistArchive(*pPath);
    }
}
}  // namespace

namespace al {
/**
 * Creates an empty layout system.
 */
LayoutSystem::LayoutSystem() {}

/**
 * Creates the font heap, the graphics resource, the fonts and the eui managers.
 * @param isSmallFontHeap whether to use a small font heap
 */
void LayoutSystem::init(bool isSmallFontHeap) {
    mIsSmallFontHeap = isSmallFontHeap;
    mFontHeap = sead::ExpHeap::create(isSmallFontHeap ? 0x100000 : 0xf00000, "FontHeap",
                                      getStationedHeap(), 8, sead::Heap::cHeapDirection_Forward,
                                      false);
    addNamedHeap(mFontHeap, "FontHeap");
    initGraphicsResource();

    {
        sead::ScopedCurrentHeapSetter setter(mFontHeap);
        initFont();
    }

    initEui();
}

/**
 * Creates and sets up the ui2d graphics resource.
 */
void LayoutSystem::initGraphicsResource() {
    LayoutAllocatorInScope allocatorScope;
    mGraphicsResource = new nn::ui2d::GraphicsResource();
    mGraphicsResource->Setup(
        static_cast<nn::gfx::Device*>(sead::GraphicsNvn::instance()->getGfxDevice()), 0x400,
        nullptr, 0, 0, nullptr, 0.0f);
    mGraphicsResource->RegisterCommonSamplerSlot(eui::RegisterSlotForSampler, nullptr);
}

/**
 * Loads the font archive and creates the font name pairs.
 */
void LayoutSystem::initFont() {
    initFontList();

    StringTmp<256> archivePath;
    makeFontDataArchivePath(&archivePath);
    mFontResource = findOrCreateResource(archivePath, nullptr);
    s32 entryNum = mFontResource->getEntryNum("/");
    mFontNamePairNum = mFontFileNameNum + cScalableFontNum;
    mFontNamePairs = new FontNamePair[mFontNamePairNum];

    for (s32 i = 0; i < mFontFileNameNum; i++) {
        mFontNamePairs[i].name = mFontFileNames[i].cstr();
    }

    for (s32 i = 0; i < entryNum; i++) {
        StringTmp<128> entryName;
        mFontResource->getEntryName(&entryName, "/", i);

        for (s32 j = 0; j < mFontFileNameNum; j++) {
            FontNamePair& pair = mFontNamePairs[j];

            if (isEqualString(pair.name, entryName)) {
                pair.isLoaded = true;
                break;
            }
        }
    }
}

/**
 * Creates the eui message, font and screen managers.
 */
void LayoutSystem::initEui() {
    sead::Heap* heap = sead::HeapMgr::instance()->getCurrentHeap();
    eui::MessageMgr* messageMgr = createMessageMgr(heap);
    eui::FontMgr* fontMgr = new eui::FontMgr();

    {
        StringTmp<256> fontArchivePath;
        makeFontDataArchivePath(&fontArchivePath);

        const char* fontNames[40];

        for (s32 i = 0; i < mFontNamePairNum; i++) {
            fontNames[i] = mFontNamePairs[i].name.cstr();
        }

        mScalableFontMgr = createScalableFontMgr(mFontHeap, mIsSmallFontHeap);
        sead::ArchiveRes* archive = mFontResource->getFileArchive();
        fontMgr->initialize(mFontHeap, archive->getRawData(), archive->getRawSize(),
                            makeFontNameBuffer(mFontNamePairNum, fontNames), mScalableFontMgr);
        fontMgr->setRubyFont("nintendo_NTLG_DB_002_Switch_50px.bffnt");

        for (s32 i = 0; i < cScalableFontNum; i++) {
            mFontNamePairs[mFontNamePairNum - cScalableFontNum + i].font =
                mScalableFontMgr->getFont(sFontParameters[0][i].name);
            mFontNamePairs[mFontNamePairNum - cScalableFontNum + i].name =
                sFontParameters[0][i].name;
        }
    }

    for (s32 i = 0; i < mFontNamePairNum - cScalableFontNum; i++) {
        FontNamePair& pair = mFontNamePairs[i];
        pair.font = fontMgr->tryGetFont(pair.name);

        if (pair.font != nullptr) {
            mFontNamePairs[i].font->SetAlternateChar('?');
            pair.isLoaded = true;
        } else {
            pair.isLoaded = false;
        }
    }

    eui::ArcResourceMgr* arcResourceMgr = new eui::ArcResourceMgr();
    EuiScreenFactory* screenFactory = new EuiScreenFactory();
    screenFactory->initialize(heap, 10);

    eui::ScreenMgr::InitializeArg arg;
    arg.arcResourceMgr = arcResourceMgr;
    arg.heap = heap;
    arg.screenFactory = screenFactory;
    arg.messageMgr = messageMgr;
    arg.fontMgr = fontMgr;
    Resource* multiFilterResource = findOrCreateResource("SystemData/MultiFilter", nullptr);
    arg.multiFilterArchiveData = multiFilterResource->getFileArchive()->getRawData();
    arg.multiFilterArchiveSize = multiFilterResource->getFileArchive()->getRawSize();
    mScreenMgr = new eui::ScreenMgr();
    mScreenMgr->initialize(arg);
}

/**
 * Finds a loaded font by name.
 * @param pFontName font name
 * @return the font, or nullptr if no font has this name
 */
nn::font::Font* LayoutSystem::tryFindFont(const char* pFontName) const {
    for (s32 i = 0; i < mFontNamePairNum; i++) {
        const FontNamePair& pair = mFontNamePairs[i];

        if (isEqualString(pair.name.cstr(), pFontName)) {
            return pair.font;
        }
    }

    return nullptr;
}

/**
 * Gets a font name pair.
 * @param index pair index
 * @return the font name pair
 */
FontNamePair* LayoutSystem::getFontNamePair(s32 index) const {
    return &mFontNamePairs[index];
}

/**
 * Releases the font manager data, the scalable font manager and the font heap contents.
 */
void LayoutSystem::finalizeFontData() {
    mScreenMgr->getFontMgr()->finalize();

    if (mScalableFontMgr != nullptr) {
        delete mScalableFontMgr;
    }

    mFontResource = nullptr;
    mFontHeap->freeAll();
}

/**
 * Reloads the fonts after a language change.
 */
void LayoutSystem::initFontForChangeLanguage() {
    reinitFont(mFontHeap);
}

/**
 * Reloads the fonts and reinitializes the font manager.
 * @param pHeap heap to create the fonts in
 */
void LayoutSystem::reinitFont(sead::Heap* pHeap) {
    eui::FontMgr* fontMgr = mScreenMgr->getFontMgr();

    {
        sead::ScopedCurrentHeapSetter setter(pHeap);
        initFont();

        StringTmp<256> fontArchivePath;
        makeFontDataArchivePath(&fontArchivePath);

        const char* fontNames[40];

        for (s32 i = 0; i < mFontNamePairNum; i++) {
            fontNames[i] = mFontNamePairs[i].name.cstr();
        }

        mScalableFontMgr = createScalableFontMgr(mFontHeap, mIsSmallFontHeap);
        sead::ArchiveRes* archive = mFontResource->getFileArchive();
        fontMgr->initialize(pHeap, archive->getRawData(), archive->getRawSize(),
                            makeFontNameBuffer(mFontNamePairNum, fontNames), mScalableFontMgr);
        fontMgr->setRubyFont("nintendo_NTLG_DB_002_Switch_50px.bffnt");

        for (s32 i = 0; i < cScalableFontNum; i++) {
            mFontNamePairs[mFontNamePairNum - cScalableFontNum + i].font =
                mScalableFontMgr->getFont(sFontParameters[0][i].name);
            mFontNamePairs[mFontNamePairNum - cScalableFontNum + i].name =
                sFontParameters[0][i].name;
        }
    }

    for (s32 i = 0; i < mFontNamePairNum - cScalableFontNum; i++) {
        FontNamePair& pair = mFontNamePairs[i];
        pair.font = fontMgr->tryGetFont(pair.name);

        if (pair.font != nullptr) {
            pair.font->SetAlternateChar('?');
            pair.isLoaded = true;
        } else {
            pair.isLoaded = false;
        }
    }
}

/**
 * Maps the eui constant buffer for drawing.
 */
void LayoutSystem::beginDraw() const {
    mScreenMgr->getConstantBuffer()->map();
}

/**
 * Switches the eui constant buffer after drawing.
 */
void LayoutSystem::endDraw() const {
    mScreenMgr->getConstantBuffer()->flipBufferIndex();
}

/**
 * Reads the font file names from the font list.
 */
void LayoutSystem::initFontList() {
    Resource* resource = findOrCreateResource("LocalizedData/Common/FontList", nullptr);
    u32 size;
    void* data = resource->getOtherFile("FontList.bin", &size);
    sead::RamStreamSrc src(data, size);
    sead::TextStreamFormat format;
    StringTmp<256> line;
    StringTmp<128> fontName;
    mFontFileNames = new FontFileName[30];
    mFontFileNameNum = 0;

    while (!src.isEOF()) {
        format.readString(&src, &line, line.getBufferSize());

        if (sscanf(line.cstr(), "FontName=\"%s\"", const_cast<char*>(fontName.cstr())) == 1) {
            fontName.copyAt(-2, ".bffnt");
            mFontFileNames[mFontFileNameNum] = fontName;
            mFontFileNameNum++;
        }
    }
}

/**
 * Creates an empty font name pair.
 */
FontNamePair::FontNamePair() : font(nullptr), name(""), isLoaded(false) {}
}  // namespace al
