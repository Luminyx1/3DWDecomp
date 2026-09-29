#pragma once

#include <basis/seadTypes.h>
#include <container/seadBuffer.h>
#include <container/seadPtrArray.h>
#include <container/seadSafeArray.h>
#include <gfx/seadColor.h>
#include <hostio/seadHostIONode.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>
#include <prim/seadBitFlag.h>
#include <prim/seadSafeString.h>

#include "common/aglGPUMemBlock.h"
#include "common/aglIndexStream.h"
#include "common/aglTextureData.h"
#include "common/aglTextureSampler.h"
#include "common/aglVertexBuffer.h"
#include "effect/aglOccludedEffect.h"
#include "environment/aglEnvObjBuffer.h"
#include "utility/aglParameter.h"
#include "utility/aglParameterIO.h"
#include "utility/aglParameterList.h"
#include "utility/aglParameterObj.h"

namespace sead {
class FileDevice;
class Heap;
class Viewport;
namespace hostio {
class Context;
class PropertyEvent;
}  // namespace hostio
}  // namespace sead

namespace nn::g3d {
struct ResExternalFileData;
}  // namespace nn::g3d

namespace agl {
class DrawContext;
class RenderBuffer;
class RenderTargetDepth;
class VertexAttribute;
}  // namespace agl

namespace agl::fx {

class OccludedEffectMgr : public sead::hostio::Node, public utl::IParameterIO {
public:
    static constexpr s32 cTypeMax = 128;

    struct Vtx {
        sead::Vector2f mPos;
        sead::Vector2f mTexCoord;
    };
    static_assert(sizeof(Vtx) == 0x10);

    class CreateArg {
    public:
        CreateArg() = default;
        virtual ~CreateArg() = default;

        s32 searchPresetTypeIndex(const OfxBase::PresetBase* pPreset, s32 type) const
        {
            s32 num = mPresetNum[type];
            for (s32 i = 0; i < num; i++)
            {
                if (sead::DynamicCast<OfxBase::PresetBase>(getObj(mPresetTypeId[type], i)) !=
                        nullptr &&
                    sead::DynamicCast<OfxBase::PresetBase>(getObj(mPresetTypeId[type], i))
                            ->getEnvObjName() == pPreset->getEnvObjName())
                {
                    return i;
                }
            }
            return -1;
        }

        s32 searchOfxType(s32 typeId) const
        {
            for (s32 i = 0; i < mTypeNum; i++)
            {
                if (mOfxTypeId[i] == typeId)
                {
                    return i;
                }
            }
            return -1;
        }

        s32 searchPresetType(s32 typeId) const
        {
            for (s32 i = 0; i < mTypeNum; i++)
            {
                if (mPresetTypeId[i] == typeId)
                {
                    return i;
                }
            }
            return -1;
        }

        env::EnvObj* getObj(s32 type, s32 index) const
        {
            if (index < 0)
            {
                return nullptr;
            }
            return mEnvObjBuffer->tryGetObj(type, index);
        }

        OfxBase* getInstance(s32 type, s32 index) const
        {
            return sead::DynamicCast<OfxBase>(getObjRef_(mOfxTypeId[type], index));
        }

        OfxBase::PresetBase* getPreset(s32 type, s32 index) const
        {
            return sead::DynamicCast<OfxBase::PresetBase>(getObjRef_(mPresetTypeId[type], index));
        }

        env::EnvObj* getObjRef_(const s32& rTypeId, s32 index) const
        {
            return index >= 0 ? mEnvObjBuffer->tryGetObj(rTypeId, index) : nullptr;
        }

        inline OfxBase::PresetBase* getPreset(s32 index) const
        {
            s32 type = 0;
            for (; type < mTypeNum; type++)
            {
                if (index < mPresetNum[type])
                {
                    break;
                }
                index -= mPresetNum[type];
            }
            if (type >= mTypeNum)
            {
                return nullptr;
            }
            return sead::DynamicCast<OfxBase::PresetBase>(getObj(mPresetTypeId[type], index));
        }

        s32 mViewNum = 1;
        s32 mTextureNum = 11;
        s32 mResTextureNum = 20;
        sead::FixedSafeString<256> mResPath{""};
        void* _130 = nullptr;
        env::EnvObjBuffer* mEnvObjBuffer = nullptr;
        s32 mTypeNum = 0;
        s32 mInstanceMenuNum = 0;
        s32 mTotalPresetNum = 0;
        sead::SafeArray<s32, cTypeMax> mOfxTypeId;
        sead::SafeArray<s32, cTypeMax> mPresetTypeId;
        sead::SafeArray<s32, cTypeMax> mOfxNum;
        sead::SafeArray<s32, cTypeMax> mPresetNum;
    };
    static_assert(sizeof(CreateArg) == 0x950);

    class MenuNode_Instance : public sead::hostio::Node {
    public:
        explicit MenuNode_Instance(OccludedEffectMgr* pMgr) : mMgr(pMgr) {}

        OccludedEffectMgr* mMgr;
    };

    class MenuNode_Preset : public sead::hostio::Node {};

    class MenuNode_PresetRoot : public sead::hostio::Node {};

    class TextureInfo : public utl::IParameterObj {
    public:
        class Placement {
        public:
            Placement(s32 index, TextureInfo* pInfo);

            s32 mIndex;
            s32 mResIndex = -1;
            utl::Parameter<sead::FixedSafeString<64>> mRefTexName;
            utl::Parameter<bool> mIsQuarter;
        };
        static_assert(sizeof(Placement) == 0x98);

        explicit TextureInfo(s32 index) : mIndex(index), mPlacement(index, this) {}
        ~TextureInfo() override = default;

        s32 mIndex;
        sead::FixedSafeString<256> mName{""};
        TextureSampler mSampler;
        bool mIsValid = false;
        Placement mPlacement;
    };
    static_assert(sizeof(TextureInfo) == 0x360);

    class Resource {
    public:
        struct ResTexInfo {
            TextureSampler mSampler;
            TextureData mTextureData;
            sead::FixedSafeString<256> mName{""};
        };
        static_assert(sizeof(ResTexInfo) == 0x3b0);

        ~Resource();

        void release();
        void initialize(s32 num, sead::Heap* pHeap);
        bool setupWithBinary(void* pBinary, const sead::SafeString& rName, bool isOwned);
        bool setupWithFile(sead::FileDevice* pDevice, const sead::SafeString& rPath,
                           sead::Heap* pHeap);

        sead::FixedSafeString<512> mPath;
        void* mResFile = nullptr;
        s32 mTexNum = 0;
        sead::Buffer<ResTexInfo> mResTexInfo;
        const nn::g3d::ResExternalFileData* mSettingFile = nullptr;
        sead::FixedSafeString<256> mName;
        bool mIsValid = false;
        bool mIsExternal;
    };
    static_assert(sizeof(Resource) == 0x360);

    struct VtxStream {
        GPUMemBlock<Vtx> mVertexBlock;
        VertexBuffer mVertexBuffer;
        GPUMemBlock<u16> mIndexBlock;
        IndexStream mIndexStream;
    };
    static_assert(sizeof(VtxStream) == 0x1f0);

    OccludedEffectMgr();
    ~OccludedEffectMgr() override;

    void initialize(const CreateArg& rArg, sead::Heap* pHeap);
    bool loadBinary(void* pBinary, u32 size, const sead::SafeString& rName, bool isOwned);
    bool loadFile(const sead::SafeString& rPath, sead::Heap* pHeap);
    void releaseResource();
    void calc();
    void calcView(s32 viewIndex, const sead::Matrix34f& rViewMtx, const sead::Matrix44f& rProjMtx,
                  f32 near, f32 far, f32 fovy, f32 aspect, const sead::Vector2f& rOffset);
    void updateGPU();
    void updateViewGPU(s32 viewIndex, const RenderBuffer& rRenderBuffer) const;
    void draw(DrawContext* pDrawContext, s32 viewIndex, const RenderBuffer& rRenderBuffer,
              const sead::Viewport& rViewport, const RenderTargetDepth& rDepth) const;
    void drawDebug(DrawContext* pDrawContext, s32 viewIndex) const;
    void setEnableAll(bool enable);
    bool saveSetting(s32 type, const sead::SafeString& rName,
                     sead::BufferedSafeString* pOut) const;
    void loadSetting(const void* pData, u32 size, const sead::SafeString* pName, bool x);
    OfxBase::PresetBase* searchPresetByName(s32 type, const sead::SafeString& rName) const;
    OfxBase::PresetBase* getPresetByIndex(s32 type, s32 index) const;
    void loadSettingFromFile();
    void getVertexAttrQuad(VertexAttribute* pAttr, sead::Heap* pHeap) const;
    void getVertexAttrQuadDouble(VertexAttribute* pAttr, sead::Heap* pHeap) const;
    void getVertexAttrOctagon(VertexAttribute* pAttr, sead::Heap* pHeap) const;
    void getVertexAttrOctagonDouble(VertexAttribute* pAttr, sead::Heap* pHeap) const;
    bool setInstanceParameterAll(s32 type, const sead::SafeString& rPresetName,
                                 const utl::ParameterBase& rParam);
    void updateInstancePresetAll(s32 type, const sead::SafeString& rPresetName);
    void setInstanceDebugDrawAll(s32 type, const sead::SafeString& rPresetName, bool enable);
    void setInstanceDebugDrawColorAll(s32 type, const sead::SafeString& rPresetName,
                                      const sead::Color4f& rColor0, const sead::Color4f& rColor1,
                                      const sead::Color4f& rColor2);
    void genMessageTextureSelect(sead::hostio::Context* pContext, s32* pIndex,
                                 const char* pLabel);
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);
    void genMessageMenuInstance(sead::hostio::Context* pContext);
    void listenPropertyEventMenuInstance(const sead::hostio::PropertyEvent* pEvent);

    const CreateArg& getCreateArg() const { return mCreateArg; }
    void setDirty() { mFlag.set(1); }
    void setPresetDirty() { mFlag.set(2); }

private:
    void createVtxStream_(sead::Heap* pHeap);
    void loadOriginalResource_();
    void constructHostIO_();
    void initTexturePlacement_();
    void updateTexturePlacement_();
    bool mountRawDir_(bool isSkip, bool isLoadSetting, bool unused);
    bool isDrawable_() const { return mResState != 0 && mStatus != 0 && mStatus != 1 && mStatus != 3; }
    const Resource& getCurrRes_() const;

public:
    CreateArg mCreateArg;
    utl::ParameterList mTextureList;
    sead::Buffer<utl::ParameterList> mPresetList;
    MenuNode_Instance mMenuNodeInstance{this};
    MenuNode_Preset mMenuNodePreset;
    sead::Buffer<MenuNode_PresetRoot> mMenuNodePresetRoot;
    sead::BitFlag32 mFlag;
    sead::PtrArray<OfxBase> mMenuInstance;
    sead::FixedSafeString<256> _c60;
    sead::FixedSafeString<256> _d78;
    sead::PtrArray<TextureInfo> mTextureInfo;
    Resource mResource[2];
    sead::FixedSafeString<1024> mRawDir;
    sead::FixedSafeString<1024> mSettingPath;
    sead::FixedSafeString<1024> _1d90;
    s32 mResState = 0;
    u32 mStatus = 0;
    sead::BufferedSafeString* mRawText = nullptr;
    s32 _21b8 = -1;
    VtxStream mVtxQuad;
    VtxStream mVtxQuadDouble;
    VtxStream mVtxOctagon;
    VtxStream mVtxOctagonDouble;
};
static_assert(sizeof(OccludedEffectMgr) == 0x2980);

}  // namespace agl::fx
