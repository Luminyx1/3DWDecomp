/**
 * @file Layout.h
 * @brief UI Layout implementation.
 */

#pragma once

#include <nn/types.h>
#include <nn/font/font_Util.h>
#include <nn/gfx/gfx_Types.h>
#include <nn/util/util_IntrusiveList.h>
namespace nn::font { template<class T> class TagProcessorBase; }

namespace nn {
namespace ui2d {
class AnimTransform;
class Pane;
class DrawInfo;
class AnimResource;
class ResourceAccessor;
class GroupAnimator;
class GroupArrayAnimator;
struct BuildResultInformation;
struct BuildArgSet;
struct BuildResSet;
struct ResVectorGraphicsTextureList;

class Layout {
public:
    struct PartsBuildDataSet;
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
};
}  // namespace ui2d
}  // namespace nn
