#pragma once

#include <container/seadBuffer.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadDelegate.h>
#include <prim/seadSafeString.h>
#include "common/aglGPUMemAddr.h"
#include "common/aglResBinaryShaderArchive.h"
#include "common/aglResShaderArchive.h"
#include "common/aglShaderCompileInfo.h"
#include "common/aglShaderProgram.h"

namespace sead::hostio {
class Context;
class PropertyEvent;
}  // namespace sead::hostio

namespace agl {

class ShaderProgramArchive;

class ShaderSource : public sead::IDisposer, public sead::hostio::Node {
public:
    ShaderSource();
    ~ShaderSource() override;

    void initialize(ShaderProgramArchive* pArchive, s32 index, ResShaderSource source, bool isUsed,
                    sead::Heap* pHeap);
    void expand();
    void resize(const sead::SafeString& rText, s32 scale);
    void genMessage(sead::hostio::Context* pContext);
    void genCompileButton(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    friend class ShaderProgramArchive;
    friend class ShaderProgramEdit;

    enum Flag {
        cFlag_Dirty = 1 << 0,
        cFlag_Used = 1 << 1,
    };

    s32 mIndex;
    u32 mFlags;
    ShaderProgramArchive* mArchive;
    ResShaderSource mResShaderSource;
    sead::HeapSafeString* mEditText;
    sead::HeapSafeString* mRawText;
    sead::Buffer<bool> mIncludeFlags;
};
static_assert(sizeof(ShaderSource) == 0x60);

class ShaderProgramEdit : public sead::hostio::Node {
public:
    ShaderProgramEdit();
    virtual ~ShaderProgramEdit();

    void initialize(ShaderProgramArchive* pArchive, s32 index, ResShaderProgram program,
                    sead::Heap* pHeap);
    void updateRawText();
    void updateAnalyze();
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

private:
    friend class ShaderProgramArchive;

    struct Stage {
        ShaderCompileInfo mCompileInfo;
        ShaderSource* mSource;
    };
    static_assert(sizeof(Stage) == 0x70);

    ShaderProgram* getProgram_() const;

    u16 mProgramIndex;
    u16 mVariationIndex;
    ShaderProgramArchive* mArchive;
    Stage mStage[cShaderType_Num];
    sead::Buffer<s32> mMacroValueIndex;
    u8 mFlags;
};
static_assert(sizeof(ShaderProgramEdit) == 0x1f0);

class ShaderProgramArchive : public sead::IDisposer, public sead::hostio::Node {
public:
    ShaderProgramArchive();
    ~ShaderProgramArchive() override;

    void destroy();
    void createWithOption(ResBinaryShaderArchive binaryArchive, ResShaderArchive archive,
                          u32 option, sead::Heap* pHeap);
    void updateCompileInfo();
    bool setUp();
    bool setUpFromObjectReflector(bool, bool noForce);
    s32 searchShaderProgramIndex(const sead::SafeString& rName) const;
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    s32 getShaderProgramNum() const { return mShaderPrograms.size(); }
    ShaderProgram& getShaderProgram(s32 index) { return mShaderPrograms[index]; }
    const ShaderProgram& getShaderProgram(s32 index) const { return mShaderPrograms[index]; }

private:
    friend class ShaderSource;
    friend class ShaderProgramEdit;

    enum Flag {
        cFlag_SetUpNoCompile = 1 << 0,
    };

    void setResShaderArchive_(ResShaderArchive archive, sead::Heap* pHeap);
    void destroyResFile_();
    bool setUp_(bool noCompile);

    ResBinaryShaderArchive mResBinaryShaderArchive;
    ResShaderArchive mResShaderArchive;
    sead::Buffer<ShaderProgram> mShaderPrograms;
    ShaderProgram::UpdateListener* mUpdateListener = nullptr;
    sead::IDelegate1<ShaderProgramArchive*>* mSetUpDelegate = nullptr;
    s32 mVariationNum = 0;
    GPUMemVoidAddr mDisplayListBuffer;
    u16 mSetUpCount = 0;
    u16 mFlags = 0;
    sead::Buffer<ShaderProgramEdit> mShaderProgramEdits;
    sead::Buffer<ShaderSource> mShaderSources;
    sead::Buffer<const char*> mSourceTexts;
    sead::Buffer<const char*> mSourceNames;
};
static_assert(sizeof(ShaderProgramArchive) == 0xC0);

}  // namespace agl

namespace agl {

inline ShaderProgram* ShaderProgramEdit::getProgram_() const {
    return mArchive->mShaderPrograms[mProgramIndex].getVariationProgram_(mVariationIndex);
}

}  // namespace agl
