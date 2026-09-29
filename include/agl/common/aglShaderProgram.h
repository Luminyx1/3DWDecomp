#pragma once

#include <container/seadBuffer.h>
#include <prim/seadDelegate.h>
#include <nvn/nvn.h>
#include <prim/seadSafeString.h>
#include "common/aglDisplayList.h"
#include "common/aglResShaderArchive.h"
#include "common/aglResBinaryShaderProgram.h"
#include "common/aglShader.h"
#include "common/aglShaderCompileInfo.h"
#include "common/aglShaderEnum.h"
#include "common/aglShaderLocation.h"

namespace sead {
class Heap;
}

namespace agl {

class DrawContext;
class ShaderStorageBlock;
class UniformSymbol;
class UniformBlockSymbol;
class ShaderStorageBlockSymbol;

class SamplerSymbol {
public:
    sead::FixedSafeString<64> mName;
    ShaderLocation mLocation;
};
static_assert(sizeof(SamplerSymbol) == 0x60);
class AttributeSymbol;
class BufferVariableSymbol;

class ShaderProgram {
public:
    class VariationBuffer {
    public:
        struct Macro {
            sead::SafeString mName;
            sead::SafeString mID;
            sead::Buffer<sead::SafeString> mValues;
            u16 mStride;
        };
        static_assert(sizeof(Macro) == 0x38);

        VariationBuffer();
        virtual ~VariationBuffer();

        void initialize(s32 macroNum, sead::Heap* pHeap);
        void createMacro(s32 index, const sead::SafeString& rName, const sead::SafeString& rID,
                         s32 valueNum, sead::Heap* pHeap);
        void setMacroValue(s32 macroIndex, s32 valueIndex, const sead::SafeString& rValue);
        void create(sead::Heap* pHeap);
        s32 getMacroAndValueArray(s32 programIndex, const char** pMacros,
                                  const char** pValues) const;
        s32 searchShaderProgramIndex(s32 macroNum, const char* const* pMacros,
                                     const char* const* pValues, s32 baseIndex) const;
        const char* searchMacroValue(s32 programIndex, const char* pName) const;
        const sead::SafeString& searchMacroName(const sead::SafeString& rID) const;
        const sead::SafeString& searchMacroID(const sead::SafeString& rName) const;
        s32 searchMacroIndex(const sead::SafeString& rName) const;

        ShaderProgram* mProgram;
        sead::Buffer<ShaderProgram> mPrograms;
        sead::Buffer<Macro> mMacros;
    };
    static_assert(sizeof(VariationBuffer) == 0x30);

    struct UpdateListener {
        void* _0;
        sead::IDelegate1<const ShaderProgram*>* getDelegate() {
            return reinterpret_cast<sead::IDelegate1<const ShaderProgram*>*>(&_8);
        }
        void* _8;
    };

    struct Variation {
        sead::SafeString mName;
        VariationBuffer mVariationBuffer;
        UpdateListener* mListener;
        s32 mShaderMode;
        const ResShaderVariationArrayData* mDefaultVariationArray;
        const ResShaderUniformBlockArray::DataType* mUniformBlockArray;
        s32 mRegisterUniformBlockLocation;
        void* _68;
    };
    static_assert(sizeof(Variation) == 0x70);

    ShaderProgram();
    virtual ~ShaderProgram();

    void cleanUp();
    void destroyLocationBuffers() {
        destroyAttribute();
        destroyUniform();
        destroySamplerLocation();
        destroyImageLocation();
        destroyUniformBlock();
        destroyShaderStorageBlock();
    }

    void initializeVariation(const sead::SafeString& rName, s32 macroNum, sead::Heap* pHeap);
    void createVariationMacro(s32 index, const sead::SafeString& rName,
                              const sead::SafeString& rID, s32 valueNum, sead::Heap* pHeap);
    void setVariationMacroValue(s32 macroIndex, s32 valueIndex, const sead::SafeString& rValue);
    void createVariation(sead::Heap* pHeap);
    void initialize_(const sead::SafeString& rName, ResArray<ResShaderVariation> variations,
                     sead::Heap* pHeap);
    void initialize(ResShaderProgram program, sead::Heap* pHeap);
    void initialize(ResBinaryShaderProgram program, sead::Heap* pHeap);

    void createAttribute(s32 num, sead::Heap* pHeap);
    void setAttributeName(s32 index, const sead::SafeString& rName);
    void destroyAttribute();
    void createUniform(s32 num, sead::Heap* pHeap);
    void setUniformName(s32 index, const sead::SafeString& rName);
    void destroyUniform();
    void createSamplerLocation(s32 num, sead::Heap* pHeap);
    void setSamplerLocationName(s32 index, const sead::SafeString& rName);
    void destroySamplerLocation();
    void createImageLocation(s32 num, sead::Heap* pHeap);
    void setImageLocationName(s32 index, const sead::SafeString& rName);
    void destroyImageLocation();
    void createUniformBlock(s32 num, sead::Heap* pHeap);
    void setUniformBlockName(s32 index, const sead::SafeString& rName);
    void destroyUniformBlock();
    void createShaderStorageBlock(s32 num, sead::Heap* pHeap);
    void setShaderStorageBlockName(s32 index, const sead::SafeString& rName);
    void destroyShaderStorageBlock();

    const Shader& getShader(ShaderType type) const;
    Shader* getShader(ShaderType type);
    void setShaderGX2_(DrawContext* pDrawContext) const;
    s32 validate_() const;
    s32 forceValidate_(bool) const;
    void compileOffline_() const;
    void updateLocation() const;
    void updateUniformLocation() const;
    void updateUniformBlockLocation() const;
    void updateShaderStorageBlockLocation() const;
    void updateAttributeLocation() const;
    void updateSamplerLocation() const;
    void updateImageLocation() const;
    void activate(DrawContext* pDrawContext, bool) const;
    s32 setUpAllVariation(bool force);
    void reserveSetUpAllVariation();
    void dispatchCompute(DrawContext* pDrawContext, s32 x, s32 y, s32 z) const;
    void dispatchComputeIndirect(DrawContext* pDrawContext, const ShaderStorageBlock& rBlock,
                                 s32 index, long offset) const;
    static u32 calcHash(const void* pData, u32 size);
    s32 getUniformSymbol(UniformSymbol* pSymbol, s32 index, bool) const;
    s32 getUniformBlockSymbol(UniformBlockSymbol* pSymbol, s32 index) const;
    s32 getShaderStorageBlockSymbol(ShaderStorageBlockSymbol* pSymbol, s32 index) const;
    s32 getSamplerSymbol(SamplerSymbol* pSymbol, s32 index) const;
    s32 getAttributeSymbol(AttributeSymbol* pSymbol, s32 index) const;
    s32 getBufferVariableSymbol(BufferVariableSymbol* pSymbol, s32 index) const;
    bool calcCompileSource(ShaderType type, sead::BufferedSafeString* pSource,
                           ShaderCompileInfo::Target target) const;
    bool calcCompileSourceNoVariation(ShaderType type, sead::BufferedSafeString* pSource,
                                      ShaderCompileInfo::Target target) const;
    void dump() const;
    bool hasStage(ShaderType type) const;

    const void* getRegisterUniformArray() const { return mVariation->mUniformBlockArray; }

    const ShaderProgram* getVariation(s32 index) const
    {
        VariationBuffer& buffer = mVariation->mVariationBuffer;
        return index <= 0 ? buffer.mProgram : &buffer.mPrograms[index - 1];
    }
    const ShaderProgram* searchVariationShaderProgram(s32 macroNum, const char* const* pMacros,
                                                      const char* const* pValues) const
    {
        s32 index = mVariation->mVariationBuffer.searchShaderProgramIndex(macroNum, pMacros,
                                                                          pValues, mVariationIndex) -
                    1;
        VariationBuffer& buffer = mVariation->mVariationBuffer;
        return index < 0 ? buffer.mProgram : &buffer.mPrograms[index];
    }
    s32 getVariationProgramNum() const { return mVariation->mVariationBuffer.mPrograms.size(); }
    u16 getVariationMacroStride(s32 macroIndex) const
    {
        return mVariation->mVariationBuffer.mMacros[macroIndex].mStride;
    }
    const UniformLocation& getUniformLocation(s32 index) const { return mUniformLocation[index]; }
    const SamplerLocation& getSamplerLocation(s32 index) const { return mSamplerLocation[index]; }
    const SamplerLocation& getSamplerLocationValidate(s32 index) const
    {
        validate_();
        return mSamplerLocation[index];
    }
    const UniformBlockLocation& getUniformBlockLocation(s32 index) const
    {
        return mUniformBlockLocation[index];
    }

private:
    friend class ShaderProgramArchive;
    friend class ShaderProgramEdit;

    void setUpForVariation_() const;

    ShaderProgram* getVariationProgram_(s32 index) const {
        VariationBuffer& buffer = mVariation->mVariationBuffer;
        return index == 0 ? buffer.mProgram : &buffer.mPrograms[index - 1];
    }

    enum Flag {
        cFlag_Initialized = 1 << 0,
        cFlag_ReserveSetUp = 1 << 1,
    };

    Variation* mVariation;
    mutable u8 mFlags;
    u16 mVariationIndex;
    DisplayList mDisplayList;
    mutable sead::Buffer<AttributeLocation> mAttributeLocation;
    mutable sead::Buffer<UniformLocation> mUniformLocation;
    mutable sead::Buffer<UniformBlockLocation> mUniformBlockLocation;
    mutable sead::Buffer<ShaderStorageBlockLocation> mShaderStorageBlockLocation;
    mutable sead::Buffer<SamplerLocation> mSamplerLocation;
    mutable sead::Buffer<ImageLocation> mImageLocation;
    VertexShader mVertexShader;
    FragmentShader mFragmentShader;
    GeometryShader mGeometryShader;
    ComputeShader mComputeShader;
    mutable NVNprogram mProgram;
    u64 mBufferAddress;
    mutable u16 mStageFlags;
};
static_assert(sizeof(ShaderProgram) == 0x428);

}  // namespace agl
