/**
 * @file Layout.h
 * @brief UI Layout implementation.
 */

#pragma once

#include <nn/types.h>
#include <nn/font/font_Util.h>
#include <nn/gfx/gfx_Types.h>
#include <nn/util/util_IntrusiveList.h>
namespace nn::font { template<class T> class TagProcessorBase; struct Rectangle; }
namespace nn::gfx { class DescriptorSlot; class TextureInfo; }

namespace nn {
namespace ui2d {
class AnimTransform;
class Pane;
class DrawInfo;
class AnimResource;
class ResourceAccessor;
class LayoutPaneFactory;
enum RenderTargetTextureLifetime : int;
class GroupAnimator;
class GroupContainer;
class GroupArrayAnimator;
struct BuildResultInformation {
    u64 _0;
    u64 _8;
};
class ControlCreator;
class TextSearcher;
struct BuildArgSet;
struct BuildResSet;
struct ResVectorGraphicsTextureList;

class Layout {
public:
    struct PartsBuildDataSet;

    struct BuildOption {
        u64 _0 = 0;
        u64 _8 = 0;
        u64 _10 = 0;
        u64 _18 = 0;
    };
    NN_RUNTIME_TYPEINFO_BASE();
    Layout();

    virtual ~Layout();

    virtual void DeleteAnimTransform(nn::ui2d::AnimTransform*);
    virtual void BindAnimation(nn::ui2d::AnimTransform*);
    virtual void UnbindAnimation(nn::ui2d::AnimTransform*);
    virtual void UnbindAnimation(nn::ui2d::Pane*);
    virtual void UnbindAllAnimation();

    virtual AnimTransform* BindAnimationAuto(nn::gfx::Device*, const AnimResource&);
    virtual void Animate();
    virtual void UpdateAnimFrame(f32 frame);
    virtual void AnimateAndUpdateAnimFrame(f32 frame);

    virtual void Draw(DrawInfo&, nn::gfx::CommandBuffer&);
    virtual void SetTagProcessor(nn::font::TagProcessorBase<u16>*);
    virtual bool BuildImpl(BuildResultInformation*, nn::gfx::Device*, const void*, ResourceAccessor*, const BuildArgSet&, const PartsBuildDataSet*);
    virtual bool BuildPartsImpl(BuildResultInformation*, nn::gfx::Device*, const void*, const PartsBuildDataSet*, BuildArgSet&, BuildResSet&, u32);
    virtual Layout* DoCreatePartsLayout_(const char*, const PartsBuildDataSet&, const BuildArgSet&);
    virtual GroupAnimator* DoCreateAndSetupGroupAnimator_(nn::gfx::Device*, const char*, const AnimResource&, bool);
    virtual GroupArrayAnimator* DoCreateAndSetupGroupArrayAnimator_(nn::gfx::Device*, const char*, const AnimResource&, bool);
    virtual Pane* BuildPaneObj(BuildResultInformation*, nn::gfx::Device*, u32, const void*, const void*, const BuildArgSet&);
    virtual Layout* BuildPartsLayout(BuildResultInformation*, nn::gfx::Device*, const char*, const PartsBuildDataSet&, const BuildArgSet&);
    virtual void CalculateImpl(DrawInfo&, bool);
    virtual void BuildVectorGraphicsTextureList(BuildResultInformation*, nn::gfx::Device*, const ResVectorGraphicsTextureList*, const char*);
    virtual void CalculateVectorGraphicsTexture(DrawInfo&);

    static void SetAllocator(void* (*)(size_t, size_t, void*), void (*)(void*, void*), void*);
    static void* AllocateMemory(size_t, size_t);
    static void* AllocateMemory(size_t);
    static void FreeMemory(void* src);
    static void SetDynamicTextureInitializationMemoryInfo(int captureCount, int vectorCount, int dynamicCount, int stackCount);
    bool BuildWithName(BuildResultInformation* pResult, nn::gfx::Device* pDevice,
                       ResourceAccessor* pAccessor, ControlCreator* pControlCreator,
                       TextSearcher* pTextSearcher, const BuildOption& rOption, const char* pName,
                       bool isUtf8);
    void Finalize(nn::gfx::Device* pDevice);
    nn::font::Rectangle GetLayoutRect() const;
    GroupAnimator* CreateGroupAnimatorAuto(nn::gfx::Device* device, const char* name, bool enabled);
    const void* GetAnimResourceData(const char* pName) const;

    template <typename T>
    static T* NewArray(int count) {
        void* pMem = Layout::AllocateMemory(sizeof(T) * count);
        if (!pMem) {
            return 0;
        }

        T* const objAry = static_cast<T*>(pMem);

        for (int i = 0; i < count; ++i) {
            new (&objAry[i]) T();
        }

        return objAry;
    }

    template <typename T>
    static void DeleteArray(T objs[], int count) {
        if (objs) {
            for (int i = 0; i < count; ++i) {
                objs[i].~T();
            }
            FreeMemory(objs);
        }
    }

    Pane* GetRootPane() const { return mRootPane; }
    const char* GetName() const { return static_cast<const char*>(_30); }
    GroupContainer* GetGroupContainer() const { return static_cast<GroupContainer*>(_20); }
    ResourceAccessor* GetResourceAccessor() const { return mResourceAccessor; }
    void Calculate(DrawInfo& rDrawInfo, bool isForceGlbMtxDirty = false) { CalculateImpl(rDrawInfo, isForceGlbMtxDirty); }

    nn::util::IntrusiveListNode mAnimTransformList;
    Pane* mRootPane;
    void* _20;
    void* _28;
    void* _30;
    void* _38;
    ResourceAccessor* mResourceAccessor;
    nn::util::IntrusiveListNode _48;
    void* _58;

    static void* g_pAllocateFunction;
    static void* g_pFreeFunction;
    static void* g_pUserDataForAllocator;
    static int g_CaptureTextureShareInfoCountMax;
    static int g_VectorGraphicsTextureShareInfoCountMax;
    static int g_DynamicTextureShareInfoCountMax;
    static int g_DynamicTextureShareInfoPartsStackMax;
    static LayoutPaneFactory* g_pLayoutPaneFactory;
    // Four words of shared random-generator state, initialized from the UI seed.
    static u32 g_Random[4];
    static RenderTargetTextureLifetime (*g_pCreateRenderTargetTextureResourceCallback)(
        nn::gfx::Texture**, nn::gfx::TextureView**, nn::gfx::DescriptorSlot**,
        const Layout*, const nn::gfx::TextureInfo&, void*, RenderTargetTextureLifetime);
    static void (*g_pDestroyRenderTargetTextureResourceCallback)(
        nn::gfx::Texture*, nn::gfx::TextureView*, nn::gfx::DescriptorSlot*,
        const Layout*, void*, RenderTargetTextureLifetime);
    static void* g_pRenderTargetTextureCallbackUserData;
};
}  // namespace ui2d
}  // namespace nn
