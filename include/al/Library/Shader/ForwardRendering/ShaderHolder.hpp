#pragma once

#include <basis/seadTypes.h>
#include <heap/seadDisposer.h>

namespace agl {
class ShaderProgram;
class ShaderProgramArchive;
}  // namespace agl

namespace nn::g3d {
class ResShadingModel;
struct ResShaderArchive;
}  // namespace nn::g3d

namespace sead {
class Heap;
}

namespace al {

class ShaderHolder {
public:
    class SingletonDisposer_ : public sead::IDisposer {
    public:
        using sead::IDisposer::IDisposer;
        virtual ~SingletonDisposer_();

        static SingletonDisposer_* sStaticDisposer;
    };

    static ShaderHolder* instance() { return sInstance; }

    static ShaderHolder* createInstance(sead::Heap* pHeap);
    static void deleteInstance();

    static ShaderHolder* sInstance;

    static constexpr s32 cArchiveNumMax = 38;

    ShaderHolder();
    virtual ~ShaderHolder() {}

    void init();
    void initAndLoadAll(const char* pArchiveName, sead::Heap* pHeap);
    void loadAll(const char* pArchiveName, sead::Heap* pHeap);
    void initAndLoadAllFromDir(const char* pDirName, sead::Heap* pHeap);
    void load(const char* pArchiveName, const char* pName, sead::Heap* pHeap, u32 option);
    agl::ShaderProgram* tryGetShaderProgram(const char* pName) const;
    agl::ShaderProgram* getShaderProgram(const char* pName) const;
    nn::g3d::ResShadingModel* getShadingModel(const char* pName) const;
    nn::g3d::ResShadingModel* getShadingModelUber(const char* pName) const;
    void setupShaderArchives();
    void cleanupShaderArchives();

    ShaderHolder(const ShaderHolder&) = delete;
    ShaderHolder& operator=(const ShaderHolder&) = delete;

private:
    friend class SingletonDisposer_;

    u32 mSingletonDisposerBuf_[sizeof(SingletonDisposer_) / sizeof(u32)];
    u8 mProgramArchiveNum = 0;
    u8 mShaderArchiveNum = 0;
    u8 mUberShaderArchiveNum = 0;
    agl::ShaderProgramArchive* mProgramArchives[cArchiveNumMax] = {};
    nn::g3d::ResShaderArchive* mShaderArchives[cArchiveNumMax] = {};
    nn::g3d::ResShaderArchive* mUberShaderArchives[cArchiveNumMax] = {};
    bool mIsInitialized = false;
};

static_assert(sizeof(ShaderHolder) == 0x3c8);

}  // namespace al
