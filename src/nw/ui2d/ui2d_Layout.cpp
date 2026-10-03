#include <nn/ui2d/ui2d_Layout.h>
#include <nn/ui2d/ui2d_AnimResource.h>
#include <nn/ui2d/ui2d_ResourceAccessor.h>
#include <nn/ui2d/ui2d_DrawInfo.h>
#include <nn/ui2d/ui2d_Group.h>
#include <nn/ui2d/ui2d_TextBox.h>
#include <nn/font/font_TagProcessorBase.h>
#include <nn/util/util_StringUtil.h>

namespace nn::ui2d {
/**
 * @brief Install allocation callbacks shared by all UI layouts.
 * @param pAllocate Allocation callback receiving byte size, alignment and pArgument.
 * @param pFree Deallocation callback receiving an allocation and pArgument.
 * @param pArgument Opaque context forwarded to both callbacks; may be nullptr.
 */
void Layout::SetAllocator(AllocateFunction pAllocate, FreeFunction pFree, void* pArgument) {
    g_pAllocateFunction = pAllocate;
    g_pFreeFunction = pFree;
    g_pUserDataForAllocator = pArgument;
}

/**
 * @brief Allocate layout storage using the configured callback.
 * @param size Requested byte count.
 * @param alignment Required byte alignment passed to the allocator.
 * @return Allocator result, possibly nullptr on failure.
 */
void* Layout::AllocateMemory(size_t size, size_t alignment) {
    return g_pAllocateFunction(size, alignment, g_pUserDataForAllocator);
}

/**
 * @brief Allocate layout storage with the default four-byte alignment.
 * @param size Requested byte count.
 * @return Allocator result, possibly nullptr on failure.
 */
void* Layout::AllocateMemory(size_t size) { return AllocateMemory(size, 4); }
/**
 * @brief Release layout storage using the configured callback.
 * @param pMemory Allocation returned by the layout allocator; null handling is allocator-defined.
 */
void Layout::FreeMemory(void* pMemory) { g_pFreeFunction(pMemory, g_pUserDataForAllocator); }

/**
 * @brief Configure shared texture-record capacities for subsequent layout builds.
 * @param captureCount Maximum number of capture-texture sharing records.
 * @param vectorCount Maximum number of vector-graphics texture sharing records.
 * @param dynamicCount Maximum number of dynamic-texture sharing records.
 * @param stackCount Maximum nested parts-layout stack depth during texture initialization.
 */
void Layout::SetDynamicTextureInitializationMemoryInfo(int captureCount, int vectorCount, int dynamicCount,
                                                       int stackCount) {
    g_CaptureTextureShareInfoCountMax = captureCount;
    g_VectorGraphicsTextureShareInfoCountMax = vectorCount;
    g_DynamicTextureShareInfoCountMax = dynamicCount;
    g_DynamicTextureShareInfoPartsStackMax = stackCount;
}

/** @brief Construct an empty layout with initialized lists and zero dimensions. */
Layout::Layout()
    : mRootPane(nullptr), _20(nullptr), _30(nullptr), _38(nullptr), mResourceAccessor(nullptr),
      mDynamicTextureList(nullptr) {
    mLayoutSize = {};
}
/** @brief Destroy the layout shell; owned resources must first be released through Finalize. */
Layout::~Layout() = default;

/**
 * @brief Allocate and register a default animation transform owned by this layout.
 * @tparam T Concrete animation transform type to construct.
 * @return New transform, or nullptr when allocation fails.
 */
template <class T> T* Layout::CreateAnimTransform() {
    void* pMemory = AllocateMemory(sizeof(T));
    T* pTransform = pMemory != nullptr ? new (pMemory) T : nullptr;
    if (pTransform != nullptr) {
        mAnimTransformList.LinkPrev(&pTransform->m_Link);
    }
    return pTransform;
}
/**
 * @brief Construct an animation transform and initialize it from a parsed resource.
 * @tparam T Concrete animation transform type to construct.
 * @param pDevice Graphics device used for the animation's resources.
 * @param rResource Parsed animation data; must contain an animation block to create a transform.
 * @return New transform, or nullptr when the animation block is absent or allocation fails.
 */
template <class T> T* Layout::CreateAnimTransform(nn::gfx::Device* pDevice, const AnimResource& rResource) {
    const auto* pBlock = rResource.mAnimation;
    if (pBlock == nullptr) {
        return nullptr;
    }
    T* pTransform = CreateAnimTransform<T>();
    if (pTransform != nullptr) {
        pTransform->SetResource(pDevice, mResourceAccessor, pBlock);
    }
    return pTransform;
}
/**
 * @brief Resolve a named animation and construct an initialized transform.
 * @tparam T Concrete animation transform type to construct.
 * @param pDevice Graphics device used for the animation's resources.
 * @param pName Animation tag appended to the layout name to resolve its resource.
 * @return New transform, or nullptr when the resource lacks animation data or allocation fails.
 */
template <class T> T* Layout::CreateAnimTransform(nn::gfx::Device* pDevice, const char* pName) {
    AnimResource resource;
    resource.Set(GetAnimResourceData(pName));
    return CreateAnimTransform<T>(pDevice, resource);
}
/**
 * @brief Allocate a basic transform and register it with this layout.
 * @return New transform, or nullptr when allocation fails.
 */
AnimTransformBasic* Layout::CreateAnimTransformBasic() { return CreateAnimTransform<AnimTransformBasic>(); }
/**
 * @brief Parse animation data and create its basic transform.
 * @param pDevice Graphics device used for animation resources.
 * @param pData Serialized animation file read by AnimResource::Set.
 * @return New transform, or nullptr when its animation block is absent or allocation fails.
 */
AnimTransformBasic* Layout::CreateAnimTransformBasic(nn::gfx::Device* pDevice, const void* pData) {
    AnimResource resource;
    resource.Set(pData);
    return CreateAnimTransform<AnimTransformBasic>(pDevice, resource);
}
/**
 * @brief Create a basic transform from parsed animation data.
 * @param pDevice Graphics device used for animation resources.
 * @param rResource Parsed animation resource containing the animation block.
 * @return New transform, or nullptr when its animation block is absent or allocation fails.
 */
AnimTransformBasic* Layout::CreateAnimTransformBasic(nn::gfx::Device* pDevice,
                                                     const AnimResource& rResource) {
    return CreateAnimTransform<AnimTransformBasic>(pDevice, rResource);
}
/**
 * @brief Create a basic transform from a named animation resource.
 * @param pDevice Graphics device used for animation resources.
 * @param pName Animation tag appended to the layout name.
 * @return New transform, or nullptr when its animation block is absent or allocation fails.
 */
AnimTransformBasic* Layout::CreateAnimTransformBasic(nn::gfx::Device* pDevice, const char* pName) {
    return CreateAnimTransform<AnimTransformBasic>(pDevice, pName);
}
/**
 * @brief Look up the animation file associated with this layout and a tag name.
 * @param pName Animation tag appended to the layout name.
 * @return Resource data returned by the layout's resource accessor, possibly nullptr.
 */
const void* Layout::GetAnimResourceData(const char* pName) const {
    char path[136];
    nn::util::SNPrintf(path, sizeof(path), "%s_%s.bflan", GetName(), pName);
    return mResourceAccessor->FindResourceByName(0x616e696d, path);
}
/**
 * @brief Construct an animator for a single pane.
 * @param pDevice Graphics device used for animation resources.
 * @param pName Tag identifying a valid animation resource; allocation must succeed.
 * @param pPane Pane to bind without recursively binding children.
 * @param enabled Initial animation activation state.
 * @return Newly constructed and bound pane animator.
 */
PaneAnimator* Layout::CreatePaneAnimator(nn::gfx::Device* pDevice, const char* pName, Pane* pPane,
                                         bool enabled) {
    AnimResource resource;
    resource.Set(GetAnimResourceData(pName));
    auto* pAnimator = CreateAnimTransform<PaneAnimator>(pDevice, resource);
    pAnimator->Setup(pPane, enabled);
    return pAnimator;
}
/**
 * @brief Construct an animator for the panes in a group.
 * @param pDevice Graphics device used for animation resources.
 * @param pName Tag identifying a valid animation resource; allocation must succeed.
 * @param pGroup Group containing the animation targets.
 * @param enabled Initial animation activation state.
 * @return Newly constructed and bound group animator.
 */
GroupAnimator* Layout::CreateGroupAnimator(nn::gfx::Device* pDevice, const char* pName, Group* pGroup,
                                           bool enabled) {
    AnimResource resource;
    resource.Set(GetAnimResourceData(pName));
    auto* pAnimator = CreateAnimTransform<GroupAnimator>(pDevice, resource);
    pAnimator->Setup(pGroup, enabled);
    return pAnimator;
}
/**
 * @brief Construct an animator bound to one group named by its animation resource.
 * @param pDevice Graphics device used for animation resources.
 * @param pName Tag identifying a valid animation resource; allocation must succeed.
 * @param index Group index passed to GroupAnimator::Setup.
 * @param enabled Initial animation activation state.
 * @return Newly constructed group animator.
 */
GroupAnimator* Layout::CreateGroupAnimatorWithIndex(nn::gfx::Device* pDevice, const char* pName, int index,
                                                    bool enabled) {
    AnimResource resource;
    resource.Set(GetAnimResourceData(pName));
    auto* pAnimator = CreateAnimTransform<GroupAnimator>(pDevice, resource);
    pAnimator->Setup(resource, GetGroupContainer(), index, enabled);
    return pAnimator;
}
/**
 * @brief Construct an animator for the first group in a parsed resource.
 * @param pDevice Graphics device used for animation resources.
 * @param pName Unused tag name; rResource already identifies the animation.
 * @param rResource Valid animation resource; allocation must succeed.
 * @param enabled Initial animation activation state.
 * @return Newly constructed group animator.
 */
GroupAnimator* Layout::DoCreateAndSetupGroupAnimator_(nn::gfx::Device* pDevice, const char* pName,
                                                      const AnimResource& rResource, bool enabled) {
    auto* pAnimator = CreateAnimTransform<GroupAnimator>(pDevice, rResource);
    pAnimator->Setup(rResource, GetGroupContainer(), 0, enabled);
    return pAnimator;
}
/**
 * @brief Construct an animator with inline storage for every group in the resource.
 * @param pDevice Graphics device used for animation resources.
 * @param pName Unused tag name; rResource already identifies the animation.
 * @param rResource Parsed resource supplying animation data and group names.
 * @param enabled Initial animation activation state.
 * @return New group-array animator, or nullptr if animation data or allocation is unavailable.
 */
GroupArrayAnimator* Layout::DoCreateAndSetupGroupArrayAnimator_(nn::gfx::Device* pDevice, const char* pName,
                                                                const AnimResource& rResource, bool enabled) {
    const auto* pBlock = rResource.mAnimation;
    if (pBlock == nullptr) {
        return nullptr;
    }
    void* pMemory = AllocateMemory(sizeof(GroupArrayAnimator) + rResource.GetGroupCount() * sizeof(Group*));
    auto* pAnimator = pMemory != nullptr ? new (pMemory) GroupArrayAnimator : nullptr;
    if (pAnimator != nullptr) {
        pAnimator->SetResource(pDevice, mResourceAccessor, pBlock);
        mAnimTransformList.LinkPrev(&pAnimator->m_Link);
        pAnimator->Setup(rResource, GetGroupContainer(), reinterpret_cast<Group**>(pAnimator + 1), enabled);
    }
    return pAnimator;
}
/**
 * @brief Create a group-array animator through the layout's factory hook.
 * @param pDevice Graphics device used for animation resources.
 * @param rResource Parsed resource supplying animation data and group names.
 * @param enabled Initial animation activation state.
 * @return Animator returned by the factory hook, possibly nullptr.
 */
GroupArrayAnimator* Layout::CreateGroupArrayAnimator(nn::gfx::Device* pDevice, const AnimResource& rResource,
                                                     bool enabled) {
    return DoCreateAndSetupGroupArrayAnimator_(pDevice, nullptr, rResource, enabled);
}
/**
 * @brief Resolve a named resource and create an animator for all its groups.
 * @param pDevice Graphics device used for animation resources.
 * @param pName Animation tag appended to the layout name.
 * @param enabled Initial animation activation state.
 * @return Animator returned by the factory hook, possibly nullptr.
 */
GroupArrayAnimator* Layout::CreateGroupArrayAnimator(nn::gfx::Device* pDevice, const char* pName,
                                                     bool enabled) {
    AnimResource resource;
    resource.Set(GetAnimResourceData(pName));
    return DoCreateAndSetupGroupArrayAnimator_(pDevice, pName, resource, enabled);
}
/**
 * @brief Select a single-group or group-array animator based on the resource's group count.
 * @param pDevice Graphics device used for animation resources.
 * @param pName Animation tag appended to the layout name.
 * @param enabled Initial animation activation state.
 * @return Animator created by the corresponding factory hook.
 */
Animator* Layout::CreateGroupAnimatorAuto(nn::gfx::Device* pDevice, const char* pName, bool enabled) {
    AnimResource resource;
    resource.Set(GetAnimResourceData(pName));
    if (resource.GetGroupCount() > 1) {
        return DoCreateAndSetupGroupArrayAnimator_(pDevice, pName, resource, enabled);
    }
    return DoCreateAndSetupGroupAnimator_(pDevice, pName, resource, enabled);
}
/**
 * @brief Bind an animation recursively to the root pane when one exists.
 * @param pTransform Animation transform to bind; must be non-null when the layout has a root pane.
 */
void Layout::BindAnimation(AnimTransform* pTransform) {
    if (mRootPane != nullptr) {
        pTransform->BindPane(mRootPane, true);
    }
}
/**
 * @brief Remove every binding from a transform.
 * @param pTransform Non-null animation transform to unbind.
 */
void Layout::UnbindAnimation(AnimTransform* pTransform) { pTransform->UnbindAll(); }
/**
 * @brief Remove a pane's bindings from all transforms owned by the layout.
 * @param pPane Pane whose animation bindings should be removed.
 */
void Layout::UnbindAnimation(Pane* pPane) {
    for (auto& rTransform : GetAnimTransformList()) {
        rTransform.UnbindPane(pPane);
    }
}
/** @brief Remove every binding from all transforms owned by the layout. */
void Layout::UnbindAllAnimation() {
    for (auto& rTransform : GetAnimTransformList()) {
        rTransform.UnbindAll();
    }
}
/**
 * @brief Unlink, destroy and free a transform allocated by this layout.
 * @param pTransform Transform belonging to the animation list; must not be nullptr.
 */
void Layout::DeleteAnimTransform(AnimTransform* pTransform) {
    using Iterator = AnimTransformList::iterator;
    GetAnimTransformList().erase(Iterator(&pTransform->m_Link));
    if (pTransform != nullptr) {
        pTransform->~AnimTransform();
        FreeMemory(pTransform);
    }
}
/**
 * @brief Draw the root pane between draw-info setup and cleanup calls.
 * @param rDrawInfo Rendering state configured for this layout during drawing.
 * @param rCommands Command buffer receiving the pane draw commands.
 */
void Layout::Draw(DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommands) {
    if (mRootPane != nullptr) {
        rDrawInfo.ConfigureBeforeDrawing(this);
        mRootPane->Draw(rDrawInfo, rCommands);
        rDrawInfo.ConfigureAfterDrawing();
    }
}
/** @brief Apply owned animations, then recursively animate all nested parts layouts. */
void Layout::Animate() {
    for (auto& rTransform : GetAnimTransformList()) {
        rTransform.Animate();
    }
    for (auto& rParts : GetPartsList()) {
        rParts.m_pLayout->Animate();
    }
}
/**
 * @brief Advance owned animation frames and recursively update nested parts layouts.
 * @param frame Elapsed animation step passed to each transform and nested layout.
 */
void Layout::UpdateAnimFrame(float frame) {
    for (auto& rTransform : GetAnimTransformList()) {
        rTransform.UpdateFrame(frame);
    }
    for (auto& rParts : GetPartsList()) {
        rParts.m_pLayout->UpdateAnimFrame(frame);
    }
}
/**
 * @brief Apply each animation before advancing its frame, then recurse through parts layouts.
 * @param frame Elapsed animation step passed to each transform and nested layout.
 */
void Layout::AnimateAndUpdateAnimFrame(float frame) {
    for (auto& rTransform : GetAnimTransformList()) {
        rTransform.Animate();
        rTransform.UpdateFrame(frame);
    }
    for (auto& rParts : GetPartsList()) {
        rParts.m_pLayout->AnimateAndUpdateAnimFrame(frame);
    }
}
/**
 * @brief Return the layout bounds centered around the origin, with positive Y pointing upward.
 * @return Rectangle spanning half the layout's width and height on each side of the origin.
 */
nn::font::Rectangle Layout::GetLayoutRect() const {
    const float width = mLayoutSize.x;
    const float height = mLayoutSize.y;
    const float left = width * -0.5f;
    const float top = height * 0.5f;
    const float right = width * 0.5f;
    const float bottom = height * -0.5f;
    nn::font::Rectangle rectangle;
    rectangle.top = top;
    rectangle.left = left;
    rectangle.right = right;
    rectangle.bottom = bottom;
    return rectangle;
}
/**
 * @brief Calculate global matrices from the root pane using this layout's drawing context.
 * @param rDrawInfo View and layout state used to initialize pane calculation.
 * @param forceDirty Whether to force global matrices to be recalculated.
 */
void Layout::CalculateGlobalMatrix(DrawInfo& rDrawInfo, bool forceDirty) {
    if (mRootPane != nullptr) {
        Pane::CalculateContext context;
        context.Set(rDrawInfo, this);
        mRootPane->CalculateGlobalMatrix(context, forceDirty);
    }
}

namespace {
/**
 * @brief Destroy an object and release its storage through the layout allocator.
 * @tparam T Allocated object type; its destructor may dispatch virtually.
 * @param pObject Object to destroy, or nullptr to do nothing.
 */
template <class T> inline void DeleteLayoutObject(T* pObject) {
    if (pObject != nullptr) {
        pObject->~T();
        Layout::FreeMemory(pObject);
    }
}
} // namespace
/**
 * @brief Release owned groups, panes, animations and the dynamic-texture pointer array.
 * @param pDevice Graphics device used to finalize the owned root pane's resources.
 */
void Layout::Finalize(nn::gfx::Device* pDevice) {
    if (mDynamicTextureList != nullptr) {
        if (mDynamicTextureList->pTextures != nullptr) {
            FreeMemory(mDynamicTextureList->pTextures);
        }
        DeleteLayoutObject(mDynamicTextureList);
        mDynamicTextureList = nullptr;
    }
    GetPartsList().clear();
    DeleteLayoutObject(GetGroupContainer());
    _20 = nullptr;
    if (mRootPane != nullptr && !(mRootPane->mFlags & 8)) {
        mRootPane->Finalize(pDevice);
        DeleteLayoutObject(mRootPane);
        mRootPane = nullptr;
    }
    auto& rAnimations = GetAnimTransformList();
    for (auto iter = rAnimations.begin(); iter != rAnimations.end();) {
        Layout::DeleteAnimTransform(&*iter++);
    }
    mLayoutSize = {};
    _30 = nullptr;
    _38 = nullptr;
    mResourceAccessor = nullptr;
}
/**
 * @brief Resolve a layout file and build it when the resource accessor finds it.
 * @param pResult Optional build-result storage forwarded to Build.
 * @param pDevice Graphics device used for layout resources.
 * @param pAccessor Resource accessor used to resolve the file and build dependencies.
 * @param pControlCreator Control factory forwarded to Build.
 * @param pTextSearcher Text resolver forwarded to Build.
 * @param rOption Build options forwarded without modification.
 * @param pName Complete layout resource filename to resolve.
 * @param isUtf8 Whether the layout's text resources use UTF-8.
 * @return Build's result, or false when the named resource is absent.
 */
bool Layout::BuildWithName(BuildResultInformation* pResult, nn::gfx::Device* pDevice,
                           ResourceAccessor* pAccessor, ControlCreator* pControlCreator,
                           TextSearcher* pTextSearcher, const BuildOption& rOption, const char* pName,
                           bool isUtf8) {
    const void* pData = pAccessor->FindResourceByName(0x626c7974, pName);
    if (pData == nullptr) {
        return false;
    }
    return Build(pResult, pDevice, pAccessor, pControlCreator, pTextSearcher, pData, rOption, isUtf8);
}
/**
 * @brief Look up a layout resource after appending its file extension.
 * @param pName Layout resource basename, without the .bflyt extension.
 * @return Resource data returned by the accessor, possibly nullptr.
 */
const void* Layout::GetLayoutResourceData(const char* pName) const {
    char path[72];
    nn::util::SNPrintf(path, sizeof(path), "%s.bflyt", pName);
    return mResourceAccessor->FindResourceByName(0x626c7974, path);
}
/**
 * @brief Acquire a named shader through the layout's resource accessor.
 * @param pDevice Graphics device used to initialize shader resources.
 * @param pName Shader name forwarded to the resource accessor.
 * @return Shader supplied by the accessor, possibly nullptr.
 */
ShaderInfo* Layout::AcquireArchiveShader(nn::gfx::Device* pDevice, const char* pName) const {
    return mResourceAccessor->AcquireShader(pDevice, pName);
}
/**
 * @brief Acquire an archive shader selected by its signature and variation keys.
 * @param pDevice Graphics device used to initialize shader resources.
 * @param signature Shader archive signature forwarded to the accessor.
 * @param keyCount Number of variation keys in pKeys.
 * @param pKeys Array of keyCount variation keys.
 * @return Shader supplied by the accessor, possibly nullptr.
 */
ShaderInfo* Layout::AcquireArchiveShader(nn::gfx::Device* pDevice, u32 signature, size_t keyCount,
                                         const u32* pKeys) const {
    return mResourceAccessor->AcquireArchiveShader(pDevice, signature, keyCount, pKeys);
}
/**
 * @brief Count capture textures in this layout and all nested parts layouts.
 * @return Combined capture-texture count.
 */
int Layout::CalculateCaptureTextureCountRecursive() const {
    int count = 0;
    for (const auto& rParts : GetPartsList()) {
        count += rParts.m_pLayout->CalculateCaptureTextureCountRecursive();
    }
    return count + (mDynamicTextureList != nullptr ? mDynamicTextureList->captureCount : 0);
}
/**
 * @brief Count vector-graphics textures in this layout and all nested parts layouts.
 * @return Combined vector-graphics texture count.
 */
int Layout::CalculateVectorGraphicsTextureCountRecursive() const {
    int count = 0;
    for (const auto& rParts : GetPartsList()) {
        count += rParts.m_pLayout->CalculateVectorGraphicsTextureCountRecursive();
    }
    return count + (mDynamicTextureList != nullptr ? mDynamicTextureList->vectorGraphicsCount : 0);
}
/**
 * @brief Leave vector-graphics texture-list creation to derived layouts.
 * @param pResult Unused build-result storage.
 * @param pDevice Unused graphics device.
 * @param pResources Unused serialized vector-graphics texture list.
 * @param pName Unused layout resource name.
 */
void Layout::BuildVectorGraphicsTextureList(BuildResultInformation* pResult, nn::gfx::Device* pDevice,
                                            const ResVectorGraphicsTextureList* pResources,
                                            const char* pName) {}
/**
 * @brief Calculate the pane tree and vector graphics within this layout's drawing context.
 * @param rDrawInfo Drawing state temporarily associated with this layout.
 * @param forceDirty Whether pane calculations must refresh global matrices.
 */
void Layout::CalculateImpl(DrawInfo& rDrawInfo, bool forceDirty) {
    if (mRootPane != nullptr) {
        Pane::CalculateContext context;
        context.Set(rDrawInfo, this);
        rDrawInfo.m_pLayoutInformation =
            reinterpret_cast<const Pane::CalculateContext::LayoutInformation*>(this);
        mRootPane->Calculate(rDrawInfo, context, forceDirty);
        CalculateVectorGraphicsTexture(rDrawInfo);
        rDrawInfo.m_pLayoutInformation = nullptr;
    }
}

namespace {
/**
 * @brief Check a pane's runtime hierarchy before accessing its text-box interface.
 * @param pPane Pane whose runtime type is checked; may be nullptr.
 * @return Text-box interface, or nullptr for a null or incompatible pane.
 */
inline TextBox* GetTextBox(Pane* pPane) {
    const auto* pWanted = TextBox::GetRuntimeTypeInfoStatic();
    if (pPane == nullptr) {
        return nullptr;
    }
    const auto* pType = pPane->GetRuntimeTypeInfo();
    while (pType != nullptr) {
        if (pType == pWanted) {
            return static_cast<TextBox*>(pPane);
        }
        pType = pType->m_ParentTypeInfo;
    }
    return nullptr;
}
/**
 * @brief Compare a supplied name against the fixed-width pane name field.
 * @param pName Null-terminated requested pane name.
 * @param pPaneName Pane name occupying at most 24 bytes.
 * @return True when the strings agree through a terminator or all 24 bytes.
 */
inline bool MatchesPaneName(const char* pName, const char* pPaneName) {
    for (size_t i = 0; i < 24; ++i) {
        if (pName[i] != pPaneName[i]) {
            return false;
        }
        if (pName[i] == '\0') {
            return true;
        }
    }
    return true;
}
/**
 * @brief Find a directly registered parts pane while preserving list constness.
 * @tparam TList Mutable or const parts-list type.
 * @param rParts List of parts panes belonging to one layout.
 * @param pName Requested pane name, compared through at most 24 bytes.
 * @return Matching parts pane, or nullptr when no registered pane has that name.
 */
template <class TList>
inline auto FindNamedParts(TList& rParts, const char* pName) -> decltype(&*rParts.begin()) {
    for (auto& rPane : rParts) {
        if (MatchesPaneName(pName, rPane.GetName())) {
            return &rPane;
        }
    }
    return nullptr;
}
} // namespace
void SetTagProcessorRecursive(Pane* pPane, nn::font::TagProcessorBase<u16>* pProcessor) asm("sub_710059ACC0");
/**
 * @brief Assign a tag processor to text boxes throughout a pane tree.
 * @param pPane Non-null root of the pane subtree to traverse.
 * @param pProcessor Tag processor to assign; nullptr selects the default text handling.
 */
void SetTagProcessorRecursive(Pane* pPane, nn::font::TagProcessorBase<u16>* pProcessor) {
    auto* pTextBox = GetTextBox(pPane);
    if (pTextBox != nullptr) {
        pTextBox->SetTagProcessor(pProcessor);
    }
    using PaneNodeTraits =
        nn::util::IntrusiveListMemberNodeTraits<detail::PaneBase, &detail::PaneBase::m_Link>;
    for (auto* pNode = pPane->m_Children.GetNext(); pNode != &pPane->m_Children; pNode = pNode->GetNext()) {
        auto* pChild = static_cast<Pane*>(&PaneNodeTraits::GetItem(*pNode));
        SetTagProcessorRecursive(pChild, pProcessor);
    }
}
/**
 * @brief Assign a tag processor to every text box beneath the layout's root pane.
 * @param pProcessor Tag processor to assign; the layout must have a root pane.
 */
void Layout::SetTagProcessor(nn::font::TagProcessorBase<u16>* pProcessor) {
    SetTagProcessorRecursive(mRootPane, pProcessor);
}
/**
 * @brief Find a parts pane directly registered with this layout.
 * @param pName Requested pane name, compared through at most 24 bytes.
 * @return Matching parts pane, or nullptr when no registered pane has that name.
 */
Parts* Layout::FindPartsPaneByName(const char* pName) { return FindNamedParts(GetPartsList(), pName); }
/**
 * @brief Find a parts pane without modifying this layout.
 * @param pName Requested pane name, compared through at most 24 bytes.
 * @return Matching parts pane, or nullptr when no registered pane has that name.
 */
const Parts* Layout::FindPartsPaneByName(const char* pName) const {
    return FindNamedParts(GetPartsList(), pName);
}
} // namespace nn::ui2d
