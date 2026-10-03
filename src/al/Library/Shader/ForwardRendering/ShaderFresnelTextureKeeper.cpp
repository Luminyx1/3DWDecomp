#include "Library/Shader/ForwardRendering/ShaderFresnelTextureKeeper.hpp"

#include <attributes.h>
#include <common/aglDisplayList.h>
#include <common/aglShaderLocation.h>
#include <driver/aglGraphicsDriverMgr.h>

#include "Library/File/FileUtil.hpp"
#include "Library/Framework/GameFrameworkNx.hpp"
#include "Library/Memory/Util.hpp"
#include "Library/Shader/DeferredRendering/SamplerLocation.hpp"
#include "Library/Shader/ForwardRendering/ShaderSamplerSetter.hpp"
#include "Project/Base/StringUtil.hpp"

namespace {

/**
 * @brief Description of one look-up curve texture.
 */
struct LutCurveEntry {
    const char* name;
    const char* label;
    const char* category;
};

constexpr s32 cFresnelNum = 48;
constexpr s32 cThicknessNum = 15;
constexpr s32 cSilhouetteNum = 2;

// The curve tables are referenced by their original data labels. USED keeps them as separate
// symbols instead of letting the compiler merge them into one global.
// clang-format off
USED static LutCurveEntry sFresnelCurveEntries[cFresnelNum] asm("lbl_7101AFBED0") = {
    {"No Fresnel", "No Fresnel", "Map"},
    {"Only Edge", "Only Edge", "Map"},
    {"Weak Whole", "Weak Whole", "Map"},
    {"Middle Whole", "Middle Whole", "Map"},
    {"Strong Whole", "Strong Whole", "Map"},
    {"Flat Ground", "Flat Ground", "Map"},
    {"Mat Whole", "Mat Whole", "Obj"},
    {"Weak Obj", "Weak Obj", "Obj"},
    {"Middle Obj", "Middle Obj", "Obj"},
    {"Strong Obj", "Strong Obj", "Obj"},
    {"Mat Flat", "Mat Flat", "Obj"},
    {"Route Dokan Obj", "Route Dokan Obj", "Obj"},
    {"Weak Water", "Weak Water", "Universal"},
    {"Middle Water", "Middle Water", "Universal"},
    {"Strong Water", "Strong Water", "Universal"},
    {"Block Obj", "Block Obj", "Obj"},
    {"Block Map", "Block Map", "Map"},
    {"Stamp Obj", "Stamp Obj", "Obj"},
    {"Player Obj", "Player Obj", "Obj"},
    {"Wide Mat Map", "Wide Mat Map", "Map"},
    {"Middle Mat Map", "Middle Mat Map", "Map"},
    {"Thin Mat Map", "Thin Mat Map", "Map"},
    {"Lawn", "Lawn", "Universal"},
    {"Tree Special Obj", "Tree Special Obj", "Obj"},
    {"Route Dokan 2 Obj", "Route Dokan 2 Obj", "Obj"},
    {"Route Dokan 3 Obj", "Route Dokan 3 Obj", "Obj"},
    {"Route Dokan 4 Obj", "Route Dokan 4 Obj", "Obj"},
    {"Fresnel 1 World", "Fresnel 1 World", "World"},
    {"Fresnel 2 World", "Fresnel 2 World", "World"},
    {"Fresnel 3 World", "Fresnel 3 World", "World"},
    {"Fresnel 4 World", "Fresnel 4 World", "World"},
    {"Fresnel 5 World", "Fresnel 5 World", "World"},
    {"Wet Obj", "Wet Obj", "Obj"},
    {"Wet Map", "Wet Map", "Map"},
    {"Tree Obj", "Tree Obj", "Obj"},
    {"Iron Obj", "Iron Obj", "Obj"},
    {"Edge Strong Whole Weak Map", "Edge Strong Whole Weak Map", "Map"},
    {"Mii Face", "Mii Face", "Obj"},
    {"Liquid Metal", "Liquid Metal", "Obj"},
    {"Magma Map", "Magma Map", "Map"},
    {"Wet Obj Strong", "Wet Obj Strong", "Obj"},
    {"Lake", "Lake", "Universal"},
    {"InkObj", "InkObj", "Obj"},
    {"InkDarkBowser", "InkDarkBowser", "Obj"},
    {"InkOcean", "InkOcean", "Obj"},
    {"InkShadowLuigi", "InkShadowLuigi", "Obj"},
    {"Marble_Gloss", "Marble_Gloss", "Obj"},
    {"Marble_Matte", "Marble_Matte", "Obj"},
};

USED static LutCurveEntry sThicknessCurveEntries[cThicknessNum] asm("lbl_7101AFC350") = {
    {"Very Wide", "Very Wide", "Thickness"},
    {"Wide", "Wide", "Thickness"},
    {"Middle", "Middle", "Thickness"},
    {"Thin", "Thin", "Thickness"},
    {"Ghost", "Ghost", "Thickness"},
    {"Bulb", "Bulb", "Thickness"},
    {"RouteDokanLauncher", "RouteDokanLauncher", "Thickness"},
    {"Grass Soil", "Grass Soil", "Thickness"},
    {"Curve Fur", "Curve Fur", "Thickness"},
    {"Magma", "Magma", "Thickness"},
    {"LampPaper", "LampPaper", "Thickness"},
    {"SoftFur", "SoftFur", "Thickness"},
    {"SoftFurMid", "SoftFurMid", "Thickness"},
    {"SoftFurThick", "SoftFurThick", "Thickness"},
    {"SoftFurFill", "SoftFurFill", "Thickness"},
};

USED static LutCurveEntry sSilhouetteCurveEntries[cSilhouetteNum] asm("lbl_7101AFC4B8") = {
    {"Player", "Player", "Silhouette"},
    {"Ride", "Ride", "Silhouette"},
};
// clang-format on
}  // namespace

namespace al {

/**
 * Finds the index of a fresnel curve.
 * @param pName Label of the fresnel curve.
 * @return The fresnel curve index, or -1 if it does not exist.
 */
s32 ShaderFresnelTextureKeeper::findFresnelId(const char* pName) {
    for (s32 i = 0; i < cFresnelNum; i++) {
        if (isEqualString(pName, sFresnelCurveEntries[i].label)) {
            return i;
        }
    }

    return -1;
}

/**
 * Gets the number of fresnel curves.
 * @return The number of fresnel curves.
 */
s32 ShaderFresnelTextureKeeper::getFresnelNum() {
    return cFresnelNum;
}

/**
 * Gets the number of thickness curves.
 * @return The number of thickness curves.
 */
s32 ShaderFresnelTextureKeeper::getThicknessNum() {
    return cThicknessNum;
}

/**
 * Creates the look-up curve textures of a table.
 * @param pAllocator Allocator for the display lists.
 * @param pCurves Array receiving the created curves.
 * @param pEntries Table describing the curves.
 * @param num Number of curves in the table.
 * @param pParamObjs Parameter objects of the map, obj, universal, world, thickness and
 * silhouette curve files.
 */
static void createLutCurves(GpuMemAllocator* pAllocator, sead::PtrArray<LutCurve>* pCurves,
                            const LutCurveEntry* pEntries, s32 num,
                            agl::utl::IParameterObj* const* pParamObjs) asm("sub_71008B9810");

static void createLutCurves(GpuMemAllocator* pAllocator, sead::PtrArray<LutCurve>* pCurves,
                            const LutCurveEntry* pEntries, s32 num,
                            agl::utl::IParameterObj* const* pParamObjs) {
    pCurves->allocBuffer(num, nullptr);

    for (s32 i = 0; i < num; i++) {
        const LutCurveEntry& entry = pEntries[i];
        const agl::SamplerLocation* location = &getSamplerLocationFresnelCurve();
        agl::utl::IParameterObj* paramObj;

        if (isEqualString(entry.category, "Map")) {
            paramObj = pParamObjs[0];
        } else if (isEqualString(entry.category, "Obj")) {
            paramObj = pParamObjs[1];
        } else if (isEqualString(entry.category, "Universal")) {
            paramObj = pParamObjs[2];
        } else if (isEqualString(entry.category, "World")) {
            paramObj = pParamObjs[3];
        } else if (isEqualString(entry.category, "Thickness")) {
            location = &getSamplerLocationThicknessCurve();
            paramObj = pParamObjs[4];
        } else if (isEqualString(entry.category, "Silhouette")) {
            location = &getSamplerLocationSilhouetteCurve();
            paramObj = pParamObjs[5];
        } else {
            paramObj = nullptr;
        }

        LutCurve* curve = new LutCurve(entry.name, entry.label, paramObj);
        curve->updateTexData();
        curve->createDisplayList(pAllocator, GameFrameworkNx::getAglDrawContext(), *location,
                                 getCurrentHeap());
        pCurves->pushBack(curve);
    }
}

/**
 * Loads the curve files and creates every look-up curve texture.
 * @param pAllocator Allocator for the display lists.
 * @param pShaderHolder Shader holder. Unused.
 * @param pLodSettingName Suffix of the silhouette curve archive, or nullptr.
 */
ShaderFresnelTextureKeeper::ShaderFresnelTextureKeeper(GpuMemAllocator* pAllocator,
                                                       ShaderHolder* pShaderHolder,
                                                       const char* pLodSettingName)
    : mMapFresnelCurveIo(
          new CurveIo("aglcurve", "map_fresnel_curve", "Map", "FresnelMap", "LutCurveData")),
      mObjFresnelCurveIo(
          new CurveIo("aglcurve", "obj_fresnel_curve", "Obj", "FresnelObj", "LutCurveData")),
      mUniversalFresnelCurveIo(new CurveIo("aglcurve", "universal_fresnel_curve", "Universal",
                                           "FresnelUniversal", "LutCurveData")),
      mWorldFresnelCurveIo(new CurveIo("aglcurve", "world_fresnel_curve", "World",
                                       "FresnelWorld", "LutCurveData")),
      mThicknessCurveIo(
          new CurveIo("aglcurve", "thickness_curve", "Thickness", "Thickness", "LutCurveData")),
      mIsLoaded(false) {
    StringTmp<256> archiveName("LutCurveData");

    if (pLodSettingName != nullptr) {
        StringTmp<256> archivePath;
        archiveName.format("%s%s", "LutCurveData", pLodSettingName);
        archivePath.format("ObjectData/%s", archiveName.cstr());

        if (!isExistArchive(archivePath.cstr())) {
            archiveName = StringTmp<256>("LutCurveData");
        }
    }

    mSilhouetteCurveIo = new CurveIo("aglcurve", "silhouette_curve", "Silhouette", "Silhouette",
                                     archiveName.cstr());

    agl::utl::IParameterObj* paramObjs[] = {
        mMapFresnelCurveIo->getParamObj(),   mObjFresnelCurveIo->getParamObj(),
        mUniversalFresnelCurveIo->getParamObj(), mWorldFresnelCurveIo->getParamObj(),
        mThicknessCurveIo->getParamObj(),    mSilhouetteCurveIo->getParamObj(),
    };

    createLutCurves(pAllocator, &mFresnelCurves, sFresnelCurveEntries, cFresnelNum, paramObjs);
    createLutCurves(pAllocator, &mThicknessCurves, sThicknessCurveEntries, cThicknessNum,
                    paramObjs);
    createLutCurves(pAllocator, &mSilhouetteCurves, sSilhouetteCurveEntries, cSilhouetteNum,
                    paramObjs);

    mMapFresnelCurveIo->loadResource();
    mObjFresnelCurveIo->loadResource();
    mUniversalFresnelCurveIo->loadResource();
    mWorldFresnelCurveIo->loadResource();
    mThicknessCurveIo->loadResource();
    mSilhouetteCurveIo->loadResource();
    mIsLoaded = true;
}

/**
 * Deletes every look-up curve texture and curve file.
 */
ShaderFresnelTextureKeeper::~ShaderFresnelTextureKeeper() {
    s32 fresnelNum = mFresnelCurves.size();

    for (s32 i = 0; i < fresnelNum; i++) {
        delete mFresnelCurves[i];
    }

    s32 thicknessNum = mThicknessCurves.size();

    for (s32 i = 0; i < thicknessNum; i++) {
        delete mThicknessCurves[i];
    }

    s32 silhouetteNum = mSilhouetteCurves.size();

    for (s32 i = 0; i < silhouetteNum; i++) {
        delete mSilhouetteCurves[i];
    }

    if (mMapFresnelCurveIo != nullptr) {
        delete mMapFresnelCurveIo;
        mMapFresnelCurveIo = nullptr;
    }

    if (mObjFresnelCurveIo != nullptr) {
        delete mObjFresnelCurveIo;
        mObjFresnelCurveIo = nullptr;
    }

    if (mUniversalFresnelCurveIo != nullptr) {
        delete mUniversalFresnelCurveIo;
        mUniversalFresnelCurveIo = nullptr;
    }

    if (mWorldFresnelCurveIo != nullptr) {
        delete mWorldFresnelCurveIo;
        mWorldFresnelCurveIo = nullptr;
    }

    if (mThicknessCurveIo != nullptr) {
        delete mThicknessCurveIo;
        mThicknessCurveIo = nullptr;
    }

    if (mSilhouetteCurveIo != nullptr) {
        delete mSilhouetteCurveIo;
        mSilhouetteCurveIo = nullptr;
    }
}

/**
 * Updates the texture data of every look-up curve after the curve files were loaded.
 */
void ShaderFresnelTextureKeeper::updateLutCurveTex() {
    if (!mIsLoaded) {
        return;
    }

    s32 fresnelNum = mFresnelCurves.size();

    for (s32 i = 0; i < fresnelNum; i++) {
        mFresnelCurves[i]->updateTexData();
    }

    s32 thicknessNum = mThicknessCurves.size();

    for (s32 i = 0; i < thicknessNum; i++) {
        mThicknessCurves[i]->updateTexData();
    }

    s32 silhouetteNum = mSilhouetteCurves.size();

    for (s32 i = 0; i < silhouetteNum; i++) {
        mSilhouetteCurves[i]->updateTexData();
    }

    mIsLoaded = false;
}

/**
 * Activates a fresnel curve texture.
 * @param fresnelId Index of the fresnel curve.
 * @param isUseDisplayList Whether to call the prebuilt display list.
 */
void ShaderFresnelTextureKeeper::activateFresnelTexture(s32 fresnelId,
                                                        bool isUseDisplayList) const {
    const LutCurve* curve = mFresnelCurves[fresnelId];

    if (isUseDisplayList) {
        const agl::DisplayList* displayList = curve->getDisplayList();
        nvnCommandBufferCallCommands(
            agl::driver::getNvnCommandBuffer(GameFrameworkNx::getAglDrawContext()), 1,
            displayList->getHandlePtr());
    } else {
        curve->getSampler().activate(GameFrameworkNx::getAglDrawContext(),
                                     getSamplerLocationFresnelCurve(), -1, false);
    }
}

/**
 * Activates a thickness curve texture.
 * @param thicknessId Index of the thickness curve.
 * @param isUseDisplayList Whether to call the prebuilt display list.
 */
void ShaderFresnelTextureKeeper::activateThicknessCurveTexture(s32 thicknessId,
                                                               bool isUseDisplayList) const {
    const LutCurve* curve = mThicknessCurves[thicknessId];

    if (isUseDisplayList) {
        const agl::DisplayList* displayList = curve->getDisplayList();
        nvnCommandBufferCallCommands(
            agl::driver::getNvnCommandBuffer(GameFrameworkNx::getAglDrawContext()), 1,
            displayList->getHandlePtr());
    } else {
        curve->getSampler().activate(GameFrameworkNx::getAglDrawContext(),
                                     getSamplerLocationThicknessCurve(), -1, false);
    }
}

/**
 * Activates a silhouette curve texture.
 * @param category Index of the silhouette curve.
 * @param rLocation Sampler location to bind the curve to.
 * @param isUseDisplayList Whether to call the prebuilt display list.
 */
void ShaderFresnelTextureKeeper::activateSilhouetteCurveTexture(
    s32 category, const agl::SamplerLocation& rLocation, bool isUseDisplayList) const {
    const LutCurve* curve = mSilhouetteCurves[category];

    if (isUseDisplayList) {
        const agl::DisplayList* displayList = curve->getDisplayList();
        nvnCommandBufferCallCommands(
            agl::driver::getNvnCommandBuffer(GameFrameworkNx::getAglDrawContext()), 1,
            displayList->getHandlePtr());
    } else {
        curve->getSampler().activate(GameFrameworkNx::getAglDrawContext(), rLocation, -1, false);
    }
}

}  // namespace al
