#include "common/aglShaderProgramArchive.h"
#include <cstring>
#include <heap/seadHeap.h>
#include <hostio/seadHostIOPropertyEvent.h>
#include <nvn/nvn_FuncPtrInline.h>
#include <time/seadTickTime.h>
#include "detail/aglFileIOMgr.h"
#include "detail/aglPrivateResource.h"
#include "detail/aglRootNode.h"
#include "detail/aglShaderTextUtil.h"
#include "shader_text/aglShaderTextPreprocessor.h"

namespace agl {

namespace {

bool isEventTarget(const sead::hostio::PropertyEvent* pEvent, const char* pTarget)
{
    return !(pEvent->getType() & 2) && pEvent->getId() < pTarget + 1 && pEvent->getId() >= pTarget;
}

// TODO: add the event field at 0x20 to sead::hostio::PropertyEvent
s64 getEventValue(const sead::hostio::PropertyEvent* pEvent)
{
    return *reinterpret_cast<const s64*>(reinterpret_cast<const u8*>(pEvent) + 0x20);
}

}  // namespace

/**
 * Constructs an empty shader program archive.
 */
ShaderProgramArchive::ShaderProgramArchive()
{
    detail::RootNode::setNodeMeta(this, "Icon = LAYOUT, Security = agl_shader");
}

/**
 * Destroys the shader program archive.
 */
ShaderProgramArchive::~ShaderProgramArchive()
{
    destroy();
}

/**
 * Releases the shader programs, the resource files and the display list buffer.
 */
void ShaderProgramArchive::destroy()
{
    destroyResFile_();
    mShaderPrograms.freeBuffer();

    if (mResBinaryShaderArchive.isValid()) {
        mResBinaryShaderArchive.cleanUp();
        mResBinaryShaderArchive = nullptr;
    }

    if (mDisplayListBuffer.isValid()) {
        mDisplayListBuffer.deleteGPUMemBlock();
        mDisplayListBuffer.invalidate();
    }

    mSetUpCount = 0;
}

/**
 * Creates the shader programs from a binary shader archive and a shader source archive.
 * @param binaryArchive archive with the compiled shader binaries
 * @param archive archive with the shader sources
 * @param option creation option flags
 * @param pHeap heap to allocate from
 */
void ShaderProgramArchive::createWithOption(ResBinaryShaderArchive binaryArchive,
                                            ResShaderArchive archive, u32 option,
                                            sead::Heap* pHeap)
{
    mResBinaryShaderArchive = binaryArchive;

    if (option & 4) {
        mFlags |= cFlag_SetUpNoCompile;
    } else {
        mFlags &= ~cFlag_SetUpNoCompile;
    }

    if (binaryArchive.isValid()) {
        mResBinaryShaderArchive.setUp(true);
        mShaderPrograms.tryAllocBuffer(mResBinaryShaderArchive.getResBinaryShaderProgramNum(),
                                       pHeap);

        ResBinaryShaderProgramArray programs =
            mResBinaryShaderArchive.getResBinaryShaderProgramArray();
        for (auto it = programs.begin(), end = programs.end(); it != end; ++it) {
            const s32 index = it.getIndex();
            ShaderProgram& program = mShaderPrograms[index];
            ResBinaryShaderProgram res_program(&(*it));
            program.initialize(res_program, pHeap);

            const u64 address = nvnBufferGetAddress(&mResBinaryShaderArchive.ptr()->mBuffer);

            for (s32 i = 0; i <= program.mVariation->mVariationBuffer.mPrograms.size(); i++) {
                ShaderProgram* variation = program.getVariationProgram_(i);
                variation->mBufferAddress = address;

                for (s32 type = 0; type < cShaderType_Num; type++) {
                    Shader* shader = variation->getShader(static_cast<ShaderType>(type));
                    const s32 binary_index =
                        mResBinaryShaderArchive.isBinaryPerProgram() ?
                            index :
                            res_program.getShaderBinaryIndex(i, static_cast<ShaderType>(type));
                    if (binary_index == -1) {
                        continue;
                    }

                    ResShaderBinary binary =
                        mResBinaryShaderArchive.getResShaderBinaryArray().get(binary_index);
                    shader->setBinary(binary.getInfo());

                    const u32 raw = binary.ref().mDataOffset;
                    Shader::BinaryInfo info = {};
                    info._0 = raw >> 20;
                    info._6 = (raw >> 6) & 0x3f;
                    info._7 = raw & 0x3f;
                    info._2 = raw >> 12;
                    shader->setBinaryInfo(info);
                }
            }
        }
    }

    setResShaderArchive_(archive, pHeap);

    for (auto& program : mShaderPrograms) {
        program.reserveSetUpAllVariation();
        mVariationNum += program.mVariation->mVariationBuffer.mPrograms.size() + 1;
    }
}

/**
 * Sets up the shader sources and program editors from a shader source archive.
 * @param archive archive with the shader sources
 * @param pHeap heap to allocate from
 */
// NON_MATCHING: used-flag loop compare widths, inlined ShaderSource::initialize index addressing
void ShaderProgramArchive::setResShaderArchive_(ResShaderArchive archive, sead::Heap* pHeap)
{
    destroyResFile_();

    if (!archive.isValid()) {
        return;
    }

    mResShaderArchive = archive;
    mResShaderArchive.setUp();

    if (!mResBinaryShaderArchive.isValid()) {
        mShaderPrograms.tryAllocBuffer(mResShaderArchive.getResShaderProgramNum(), pHeap);
    }

    mShaderSources.tryAllocBuffer(mResShaderArchive.getResShaderSourceNum(), pHeap);
    mSourceTexts.tryAllocBuffer(mShaderSources.size(), pHeap);
    mSourceNames.tryAllocBuffer(mShaderSources.size(), pHeap);
    mShaderProgramEdits.tryAllocBuffer(mShaderPrograms.size(), pHeap);

    ResShaderProgramArray programs = mResShaderArchive.getResShaderProgramArray();
    ResShaderSourceArray sources = mResShaderArchive.getResShaderSourceArray();

    bool used[1024];
    const u32 source_num = sources.getNum();

    for (u64 i = 0; i < source_num; i++) {
        used[i] = false;

        for (auto it = programs.constBegin(), end = programs.constEnd(); it != end; ++it) {
            ResShaderProgram program(&(*it));

            if (static_cast<u32>(program.getSourceIndex(cShaderType_Vertex)) == i ||
                static_cast<u32>(program.getSourceIndex(cShaderType_Fragment)) == i ||
                static_cast<u32>(program.getSourceIndex(cShaderType_Geometry)) == i ||
                static_cast<u32>(program.getSourceIndex(cShaderType_Compute)) == i) {
                used[i] = true;
                break;
            }
        }
    }

    for (auto it = sources.begin(), end = sources.end(); it != end; ++it) {
        const s32 index = it.getIndex();
        mShaderSources[index].initialize(this, index, &(*it), used[index], pHeap);
    }

    sead::TickTime time;

    for (auto it = programs.begin(), end = programs.end(); it != end; ++it) {
        const s32 index = it.getIndex();
        mShaderPrograms[index].initialize(ResShaderProgram(&(*it)), pHeap);
        mShaderProgramEdits[index].initialize(this, index, &(*it), pHeap);
    }

    updateCompileInfo();
}

/**
 * Releases the shader sources and program editors created from the shader source archive.
 */
// NON_MATCHING: prologue is shrink-wrapped past the validity check
void ShaderProgramArchive::destroyResFile_()
{
    if (mResShaderArchive.isValid()) {
        mShaderProgramEdits.freeBuffer();
        mSourceTexts.freeBuffer();
        mSourceNames.freeBuffer();
        mShaderSources.freeBuffer();
        mResShaderArchive = nullptr;
    }
}

/**
 * Sets up a shader source of the archive.
 * @param pArchive archive that owns the source
 * @param index index of the source in the archive
 * @param source shader source resource
 * @param isUsed whether the source is used by a shader program directly
 * @param pHeap heap to allocate from
 */
// NON_MATCHING: source name store uses a selected pointer instead of a selected index
void ShaderSource::initialize(ShaderProgramArchive* pArchive, s32 index, ResShaderSource source,
                              bool isUsed, sead::Heap* pHeap)
{
    mArchive = pArchive;
    mIndex = static_cast<u16>(index);
    mResShaderSource = source;

    if (isUsed) {
        mFlags |= cFlag_Used;
    } else {
        mFlags &= ~cFlag_Used;
    }

    mArchive->mSourceNames[mIndex] = source.getName();
    mArchive->mSourceTexts[mIndex] = source.getText();

    mIncludeFlags.tryAllocBuffer(mArchive->mShaderSources.size(), pHeap);

    for (auto& flag : mIncludeFlags) {
        flag = false;
    }

    detail::RootNode::setNodeMeta(this, "Icon = NOTE");
}

/**
 * Sets up the compile information of a shader program from the shader source archive.
 * @param pArchive archive that owns the program
 * @param index index of the program in the archive
 * @param program shader program resource
 * @param pHeap heap to allocate from
 */
// NON_MATCHING: source index compare is sign-extended, heap kept in a register instead of spilled
void ShaderProgramEdit::initialize(ShaderProgramArchive* pArchive, s32 index,
                                   ResShaderProgram program, sead::Heap* pHeap)
{
    mArchive = pArchive;
    mProgramIndex = index;
    ShaderProgram& shader_program = pArchive->mShaderPrograms[mProgramIndex];

    for (s32 i = 0; i < cShaderType_Num; i++) {
        const ShaderType type = static_cast<ShaderType>(i);

        if (*reinterpret_cast<const u16*>(&program.ref().mSourceIndex[type]) == 0xffff) {
            mStage[i].mSource = nullptr;
            continue;
        }

        mStage[i].mSource = &mArchive->mShaderSources[program.getSourceIndex(type)];
        ResShaderMacroArray macros = program.getResShaderMacroArray(type);
        mStage[i].mCompileInfo.create(
            macros.getNum(), shader_program.mVariation->mVariationBuffer.mMacros.size(), pHeap);
        mStage[i].mCompileInfo.setName(mStage[i].mSource->mResShaderSource.getName());

        for (auto it = macros.constBegin(), end = macros.constEnd(); it != end; ++it) {
            ResShaderMacro macro(&(*it));
            mStage[i].mCompileInfo.mMacroName.pushBack(macro.getName());
            mStage[i].mCompileInfo.mMacroValue.pushBack(macro.getValue());
        }

        shader_program.getShader(type)->setCompileInfo(&mStage[i].mCompileInfo);
    }

    const s32 macro_num = shader_program.mVariation->mVariationBuffer.mMacros.size();

    if (macro_num > 0) {
        mMacroValueIndex.tryAllocBuffer(macro_num, pHeap);

        for (auto& value : mMacroValueIndex) {
            value = 0;
        }
    }

    detail::RootNode::setNodeMeta(this, "Icon = CIRCLE_ORENGE");
}

/**
 * Expands the includes of dirty shader sources and updates the programs using them.
 */
void ShaderProgramArchive::updateCompileInfo()
{
    sead::TickTime time;

    for (auto& source : mShaderSources) {
        if (source.mFlags & ShaderSource::cFlag_Dirty) {
            source.expand();
        }
    }

    for (auto& edit : mShaderProgramEdits) {
        edit.updateRawText();
    }

    for (auto& source : mShaderSources) {
        source.mFlags &= ~ShaderSource::cFlag_Dirty;
    }
}

/**
 * Sets up all shader programs of the archive.
 * @return whether every shader program was set up
 */
bool ShaderProgramArchive::setUp()
{
    updateCompileInfo();
    return setUp_(mFlags & cFlag_SetUpNoCompile);
}

/**
 * Sets up all shader programs of the archive.
 * @param noCompile whether to only register the update listener instead of compiling
 * @return whether every shader program was set up
 */
bool ShaderProgramArchive::setUp_(bool noCompile)
{
    mSetUpCount++;
    sead::TickTime time;

    for (auto& edit : mShaderProgramEdits) {
        edit.updateAnalyze();
    }

    for (auto& program : mShaderPrograms) {
        program.mVariation->mListener = mUpdateListener;

        if (!noCompile && program.setUpAllVariation(false) != 0) {
            return false;
        }
    }

    sead::TickTime time_end;

    if (mSetUpDelegate != nullptr) {
        mSetUpDelegate->invoke(this);
    }

    return true;
}

/**
 * Sets up all shader programs of the archive after an edit from the host.
 * @param noForce whether to compile the programs regardless of the archive option
 * @return whether every shader program was set up
 */
bool ShaderProgramArchive::setUpFromObjectReflector(bool, bool noForce)
{
    updateCompileInfo();
    return setUp_(noForce ? false : (mFlags & cFlag_SetUpNoCompile) != 0);
}

/**
 * Expands the includes of the shader source into its raw text.
 */
void ShaderSource::expand()
{
    if (!(mFlags & cFlag_Used)) {
        return;
    }

    if (mRawText) {
        delete mRawText;
        mRawText = nullptr;
    }

    bool used[1024];
    ShaderProgramArchive* archive = mArchive;
    mRawText = detail::ShaderTextUtil::createRawText(
        archive->mSourceTexts[mIndex], archive->mSourceNames.getBufferPtr(),
        archive->mSourceTexts.getBufferPtr(), archive->mShaderSources.size(), used,
        detail::PrivateResource::instance()->getShaderTextHeap());

    s32 index = 0;

    for (auto& source : mArchive->mShaderSources) {
        source.mIncludeFlags[mIndex] = used[index];
        index++;
    }
}

/**
 * Reallocates the edit text of the shader source and copies a text into it.
 * @param rText text to copy into the edit text
 * @param scale factor applied to the current source text length for the new buffer size
 */
void ShaderSource::resize(const sead::SafeString& rText, s32 scale)
{
    const s32 textLength = mResShaderSource.ref().mTextLen;
    const s32 length =
        sead::SafeString(mArchive->mSourceTexts[mIndex]).calcLength() * scale + 1;
    const s32 size = length <= textLength ? textLength : length;

    if (u32(size) + sizeof(sead::HeapSafeString) >
        detail::PrivateResource::instance()->getDebugHeap()->getMaxAllocatableSize(8)) {
        return;
    }

    sead::HeapSafeString* old_text = mEditText;
    mEditText = new (detail::PrivateResource::instance()->getDebugHeap())
        sead::HeapSafeString(detail::PrivateResource::instance()->getDebugHeap(), size);
    mEditText->copy(rText);

    if (old_text) {
        delete old_text;
    }

    mArchive->mSourceTexts[mIndex] = mEditText->cstr();
}

/**
 * Passes the expanded source text of dirty shader sources to the compile information.
 */
void ShaderProgramEdit::updateRawText()
{
    for (s32 type = 0; type < cShaderType_Num; type++) {
        Stage& stage = mStage[type];

        if (stage.mSource != nullptr && (stage.mSource->mFlags & ShaderSource::cFlag_Dirty)) {
            stage.mCompileInfo.setSource(stage.mSource->mRawText);
            mArchive->mShaderPrograms[mProgramIndex].reserveSetUpAllVariation();
        }
    }
}

/**
 * Revalidates the edited shader program if it was modified.
 */
void ShaderProgramEdit::updateAnalyze()
{
    if (!(mFlags & 1)) {
        return;
    }

    ShaderProgram* program = getProgram_();
    program->mFlags |= 10;
    program->validate_();
}

/**
 * Searches for a shader program by name.
 * @param rName name of the shader program
 * @return index of the shader program, or -1 if it was not found
 */
s32 ShaderProgramArchive::searchShaderProgramIndex(const sead::SafeString& rName) const
{
    s32 index = 0;

    for (const auto& program : mShaderPrograms) {
        if (program.mVariation->mName == rName) {
            return index;
        }

        index++;
    }

    return -1;
}

/**
 * Generates the host message of the archive (no output in release builds).
 * @param pContext host communication context
 */
void ShaderProgramArchive::genMessage(sead::hostio::Context* pContext)
{
    {
        sead::FormatFixedSafeString<1024> str(
            "Resource size (.sharc/.sharcb) %d/%d[byte]",
            mResShaderArchive.isValid() ? mResShaderArchive.ref().mFileSize : 0,
            mResBinaryShaderArchive.isValid() ? mResBinaryShaderArchive.ref().mFileSize : 0);
    }

    if (mDisplayListBuffer.getMemoryBlock() != nullptr) {
        sead::FormatFixedSafeString<1024> str("DisplayList : %d[byte]",
                                              mDisplayListBuffer.getMemoryBlock()->getSize());
    }

    {
        sead::FormatFixedSafeString<1024> str(
            "program num   : %d\nvariation num : %d\nsource num    : %d",
            mShaderPrograms.size(), mVariationNum, mShaderSources.size());
    }
}

/**
 * Handles a property event sent from the host.
 * @param pEvent property event
 */
void ShaderProgramArchive::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    const uintptr_t id = reinterpret_cast<uintptr_t>(pEvent->getId());

    switch (id) {
    case 100000:
    case 100002:
        if (id != 100000) {
            for (auto& source : mShaderSources) {
                source.mFlags |= ShaderSource::cFlag_Dirty;
            }
        }

        setUpFromObjectReflector(false, !(mFlags & cFlag_SetUpNoCompile));
        break;
    case 100004:
        for (auto& program : mShaderPrograms) {
            program.dump();
        }

        break;
    case 100010:
        for (auto& source : mShaderSources) {
            source.resize(*source.mEditText, 1);
        }

        break;
    }
}

/**
 * Handles a property event of the shader program editor sent from the host.
 * @param pEvent property event
 */
// NON_MATCHING: range checks get merged (ccmp) instead of jump-threaded to the id switch; block layout
void ShaderProgramEdit::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    const uintptr_t id = reinterpret_cast<uintptr_t>(pEvent->getId());
    const u32 num = mArchive->mShaderPrograms.size();

    if (id >= 101014 && id < num * 4 + 101014) {
    } else if (id >= 102014 && id < num * 4 + 1000 + 102014) {
        const bool is_nvn = id >= num * 4 + 102014;
        const s32 local = id - (is_nvn ? 103014 : 102014);
        const s32 program_index = local / 4;
        const s32 type = local % 4;

        sead::BufferedSafeString* source =
            detail::PrivateResource::instance()->getShaderSourceBuffer(type);
        ShaderProgram* program =
            mArchive->mShaderPrograms[program_index].getVariationProgram_(mVariationIndex);
        program->calcCompileSource(ShaderType(type), source,
                                   is_nvn ? ShaderCompileInfo::cTarget_NVNBinary :
                                            ShaderCompileInfo::cTarget_GL);

        detail::FileIOMgr::DialogArg arg;
        arg.mFilter = "sh";
        arg.mFileName =
            mArchive->mSourceNames[mArchive->mResShaderArchive.getResShaderProgramArray()
                                       .get(program_index)
                                       .getSourceIndex(ShaderType(type))];
        detail::FileIOMgr::instance()->save(source->cstr(), source->calcLength(), arg);
    } else if (id >= 104014 && id < num * 4 + 104014) {
        sead::Heap* heap = detail::PrivateResource::instance()->getDebugHeap();
        const u32 program_index = (id - 104014) / 4;
        const u32 type = (id - 104014) % 4;

        shtxt::Preprocessor preprocessor(heap, heap);
        sead::BufferedSafeString* source =
            detail::PrivateResource::instance()->getShaderSourceBuffer(type);
        ShaderProgram* program =
            mArchive->mShaderPrograms[program_index].getVariationProgram_(mVariationIndex);
        program->calcCompileSource(ShaderType(type), source, ShaderCompileInfo::cTarget_NVNBinary);

        preprocessor.initialize(source->cstr());
        preprocessor.preprocess(0x7a3, 0, 0);
        const s32 length = preprocessor.calcConstructLength();
        sead::HeapSafeString text(heap, length + 1);
        preprocessor.construct(&text);
        preprocessor.finalize();

        detail::FileIOMgr::DialogArg arg;
        arg.mFilter = "sh";
        arg.mFileName =
            mArchive->mSourceNames[mArchive->mResShaderArchive.getResShaderProgramArray()
                                       .get(program_index)
                                       .getSourceIndex(ShaderType(type))];
        detail::FileIOMgr::instance()->save(text.cstr(), length, arg);
    } else if (id >= 105014 && id < num * 4 + 105014) {
        sead::Heap* heap = detail::PrivateResource::instance()->getDebugHeap();
        const u32 program_index = (id - 105014) / 4;
        const u32 type = (id - 105014) % 4;

        shtxt::Preprocessor preprocessor(heap, heap);
        sead::BufferedSafeString* source =
            detail::PrivateResource::instance()->getShaderSourceBuffer(type);
        ShaderProgram* program =
            mArchive->mShaderPrograms[program_index].getVariationProgram_(mVariationIndex);
        program->calcCompileSource(ShaderType(type), source, ShaderCompileInfo::cTarget_NVNBinary);

        preprocessor.initialize(source->cstr());
        preprocessor.preprocess(0x7a7, 0, 0);
        const s32 length = preprocessor.calcConstructLength();
        sead::HeapSafeString text(heap, length + 1);
        preprocessor.construct(&text);
        preprocessor.finalize();

        detail::FileIOMgr::DialogArg arg;
        arg.mFilter = "sh";
        arg.mFileName =
            mArchive->mSourceNames[mArchive->mResShaderArchive.getResShaderProgramArray()
                                       .get(program_index)
                                       .getSourceIndex(ShaderType(type))];
        detail::FileIOMgr::instance()->save(text.cstr(), length, arg);
    } else if (id >= 106014 && id < num * 4 + 106014) {
        mArchive->mShaderPrograms[id - 106014].validate_();
        updateAnalyze();
    } else if (id == 100015) {
        sead::FormatFixedSafeString<1024> str(
            "File = %%AGL_ROOT%%/tools/temporary/shader_analyze, Verb = open");
    } else if (id == 100013) {
        const ShaderProgram* program = mArchive->mShaderPrograms[mProgramIndex].getVariation(0);
        s32 index = 0;
        s32 i = 0;

        for (const auto& macro : program->mVariation->mVariationBuffer.mMacros) {
            index += mMacroValueIndex(i) * macro.mStride;
            i++;
        }

        mVariationIndex = index;
    }
}

/**
 * Constructs an empty shader program editor.
 */
ShaderProgramEdit::ShaderProgramEdit() : mProgramIndex(0), mVariationIndex(0), mArchive(nullptr)
{
    mFlags = 0;
}

/**
 * Destroys the shader program editor.
 */
ShaderProgramEdit::~ShaderProgramEdit()
{
    mMacroValueIndex.freeBuffer();
}

/**
 * Constructs an empty shader source.
 */
ShaderSource::ShaderSource()
    : mFlags(cFlag_Dirty), mArchive(nullptr), mEditText(nullptr), mRawText(nullptr)
{
}

/**
 * Destroys the shader source.
 */
ShaderSource::~ShaderSource()
{
    if (mRawText) {
        delete mRawText;
        mRawText = nullptr;
    }

    if (mEditText) {
        delete mEditText;
        mEditText = nullptr;
    }

    mIncludeFlags.freeBuffer();
}

/**
 * Generates the host message of the shader source (no output in release builds).
 * @param pContext host communication context
 */
void ShaderSource::genMessage(sead::hostio::Context* pContext)
{
    if (mEditText) {
        sead::FormatFixedSafeString<1024> str(
            "Font = FixedPitch, EditorExtension = sh, IsReadOnly = False, Encode=%s",
            detail::ShaderTextUtil::isUTF8(mEditText->cstr()) ? "UTF8" : "SJIS");
    } else {
        const sead::BufferedSafeString text(const_cast<char*>(mResShaderSource.getText()),
                                            mResShaderSource.ref().mTextLen);
        sead::FormatFixedSafeString<1024> str(
            "Font = FixedPitch, EditorExtension = sh, IsReadOnly = True, IsEnable=False, "
            "Encode=%s",
            detail::ShaderTextUtil::isUTF8(text.cstr()) ? "UTF8" : "SJIS");
    }
}

/**
 * Handles a property event of the shader source sent from the host.
 * @param pEvent property event
 */
void ShaderSource::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent)
{
    if (!mEditText) {
        if (reinterpret_cast<uintptr_t>(pEvent->getId()) == 100012) {
            resize(sead::SafeString(mResShaderSource.getText()), 2);
        }

        return;
    }

    if (isEventTarget(pEvent, mEditText->cstr())) {
        mFlags |= cFlag_Dirty;
    }

    if (isEventTarget(pEvent, mEditText->cstr()) && getEventValue(pEvent) == 1) {
        goto compile;
    }

    switch (reinterpret_cast<uintptr_t>(pEvent->getId())) {
    case 100007: {
        detail::FileIOMgr::DialogArg arg;
        arg.mFileName = sead::SafeString(sead::SafeString::cEmptyString);
        arg.mFilter = "sh";
        detail::FileIOMgr::instance()->save(mEditText->cstr(), mEditText->calcLength(), arg);
        break;
    }
    case 100008:
        if (mEditText) {
            delete mEditText;
        }

        mEditText = nullptr;
        mArchive->mSourceTexts[mIndex] = mResShaderSource.getText();
        mFlags |= cFlag_Dirty;
        [[fallthrough]];
    case 100001:
    case 100006:
    compile:
        if (mFlags & cFlag_Dirty) {
            ShaderProgramArchive* archive = mArchive;
            s32 index = 0;

            for (auto& source : archive->mShaderSources) {
                if (mIncludeFlags[index]) {
                    source.mFlags |= cFlag_Dirty;
                }

                index++;
            }

            mArchive->setUpFromObjectReflector(
                false, reinterpret_cast<uintptr_t>(pEvent->getId()) == 100006);
        }

        break;
    case 100011:
        resize(*mEditText, 2);
        break;
    }
}

/**
 * Generates the compile button of the shader source (no output in release builds).
 * @param pContext host communication context
 */
void ShaderSource::genCompileButton(sead::hostio::Context* pContext) {}

}  // namespace agl
