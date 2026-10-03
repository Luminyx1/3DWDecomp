#pragma once

#include <basis/seadTypes.h>
#include <common/aglGPUMemBlock.h>
#include <common/aglTextureData.h>
#include <common/aglTextureSampler.h>
#include <container/seadPtrArray.h>
#include <hostio/seadHostIOCurve.h>
#include <prim/seadSafeString.h>
#include <utility/aglParameter.h>
#include <utility/aglParameterCurve.hpp>
#include <utility/aglParameterIO.h>
#include <utility/aglParameterObj.h>

namespace agl {
class DisplayList;
class DrawContext;
class SamplerLocation;
class ShaderProgram;

namespace utl {
class DynamicTextureAllocator;
}  // namespace utl
}  // namespace agl

namespace sead {
class Heap;
}  // namespace sead

namespace al {
class GpuMemAllocator;

/**
 * @brief Scoped access to the dynamic texture allocator, optionally holding the draw context lock.
 */
class DynamicTexAlloc {
public:
    DynamicTexAlloc(bool isLock);
    ~DynamicTexAlloc();

    agl::utl::DynamicTextureAllocator* getAlloc() const;
    void freeTex(agl::TextureData* pTexture);

private:
    bool mIsLock;
};

s32 calcMaxMipLevelNum(s32 size);
const agl::TextureSampler* getWhite2DSampler();
const agl::TextureSampler* getBlack2DSampler();
const agl::TextureSampler* getRed2DSampler();
const agl::TextureSampler* getGreen2DSampler();
const agl::TextureSampler* getBlue2DSampler();
const agl::TextureSampler* getBlackCubeSampler();
const agl::TextureData& getWhite2DTexture();
const agl::TextureData& getBlack2DTexture();
const agl::TextureData& getRed2DTexture();
const agl::TextureData& getGreen2DTexture();
const agl::TextureData& getBlue2DTexture();
const agl::TextureData& getBlackCubeTexture();

/**
 * @brief A texture loaded from an archive, with its optional sampler.
 */
struct TextureInfo {
    TextureInfo() : mName("No Name") {}

    bool operator<(const TextureInfo& rOther) const { return mName < rOther.mName; }

    agl::TextureData* mTextureData = nullptr;
    agl::TextureSampler* mSampler = nullptr;
    sead::FixedSafeString<256> mName;
};

static_assert(sizeof(TextureInfo) == 0x128);

/**
 * @brief Name-sorted list of textures loaded from an archive.
 */
class TextureInfoArray : public sead::PtrArray<TextureInfo> {
public:
    TextureInfo* findTexture(const char* pName) const;
    s32 findTextureIndex(const char* pName) const;
};

agl::SamplerLocation* makeSamplerLoc(const agl::ShaderProgram& rProgram, const char* pName);
void InitializeAGLTextureNoResSetup(agl::TextureData* pTextureData, void* pResFile,
                                    const sead::SafeString& rName);
void loadTextureInfoArray(TextureInfoArray* pArray, const char* pArchiveName,
                          const char* pFileName, const char* pUnused, bool isCreateSampler);
void tryLoadTextureInfoArray(TextureInfoArray* pArray, const char* pArchiveName,
                             const char* pFileName, const char* pUnused, bool isCreateSampler);
void freeTextureInfo(TextureInfo* pInfo);

/**
 * @brief Small look-up texture whose image lives in its own GPU memory block.
 */
class LutTexture {
public:
    LutTexture(agl::TextureFormat format, u32 width, u32 height, u32 depth);
    ~LutTexture();

    void create();
    void createDisplayList(GpuMemAllocator* pAllocator, agl::DrawContext* pDrawContext,
                           const agl::SamplerLocation& rLocation, sead::Heap* pHeap);
    void initTexData();
    void initCore();
    void storeU16(u16 value, s32 index);
    void reinit();

    const agl::TextureSampler& getSampler() const { return mSampler; }

    const agl::DisplayList* getDisplayList() const { return mDisplayList; }

protected:
    agl::TextureData mTextureData;
    agl::TextureSampler mSampler;
    agl::DisplayList* mDisplayList = nullptr;
    u32 mWidth;
    u32 mHeight;
    u32 mDepth;
    agl::TextureFormat mFormat;
    agl::GPUMemBlock<u8> mMemBlock;
};

static_assert(sizeof(LutTexture) == 0x2e8);

/**
 * @brief Look-up texture filled from a curve.
 */
class LutCurve : public LutTexture {
public:
    LutCurve(const sead::SafeString& rName, const sead::SafeString& rLabel,
             agl::utl::IParameterObj* pParamObj)
        : LutTexture(agl::TextureFormat::cTextureFormat_R32_float, 64, 0, 0),
          mCurve(rName, rLabel, pParamObj) {}

    void updateTexData();

private:
    agl::utl::ParameterCurve<1> mCurve;
};

/**
 * @brief Parameter file holding a curve object.
 */
class CurveIo {
public:
    CurveIo(const char* pName, const char* pObjName, const char* pParamName, const char* pFileName,
            const char* pArchiveName);

    void loadResource();

    agl::utl::ParameterObj* getParamObj() { return &mParamObj; }

private:
    agl::utl::IParameterIO mParamIO;
    agl::utl::ParameterObj mParamObj;
    const char* mObjName;
    const char* mParamName;
    const char* mFileName;
    const char* mArchiveName;
};

static_assert(sizeof(CurveIo) == 0x270);

/**
 * @brief Binds a sampler to a shader program's sampler slot of the given name.
 */
class ShaderSamplerSetter {
public:
    ShaderSamplerSetter(agl::DrawContext* pDrawContext, const agl::ShaderProgram* pProgram,
                        const agl::TextureSampler* pSampler, const char* pName, bool isUnused);
};

}  // namespace al
