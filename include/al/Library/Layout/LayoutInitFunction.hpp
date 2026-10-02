#pragma once

#include <prim/seadSafeString.h>

namespace nn::font {
struct Rectangle;
}

namespace nn::ui2d {
class DrawInfo;
class GraphicsResource;
}  // namespace nn::ui2d

namespace al {
class CustomTagProcessor;
class LayoutActor;
class LayoutInitInfo;
class LayoutKeeper;
class Resource;

class LayoutAllocatorInScope {
public:
    LayoutAllocatorInScope();
    ~LayoutAllocatorInScope();
};

bool makeLayoutArchivePath(sead::BufferedSafeString* pOut, const sead::SafeString& rName,
                           bool isLocalized);
void makeLayoutResourcePath(sead::BufferedSafeString* pOut, const sead::SafeString& rName,
                            bool isLocalized);
void initLayoutSceneInfo(LayoutActor* pActor, const LayoutInitInfo& rInfo);
void initLayoutEffectKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo, const char* pName);
void initLayoutSeKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo, const char* pName);
void initLayoutBgmKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo, const char* pName);
void initHitReactionKeeper(LayoutActor* pActor, const LayoutInitInfo& rInfo,
                           const Resource* pResource, const char* pName);
void initLayoutMessage(LayoutKeeper* pKeeper, const LayoutInitInfo& rInfo,
                       const Resource* pResource);
void initDrawInfoDefault(nn::ui2d::DrawInfo* pDrawInfo, nn::ui2d::GraphicsResource* pResource);
void initDrawInfo(nn::ui2d::DrawInfo* pDrawInfo, nn::ui2d::GraphicsResource* pResource,
                  const nn::font::Rectangle& rRect);
void initLayoutKeeper(LayoutKeeper* pKeeper, const LayoutInitInfo& rInfo,
                      const Resource* pResource, CustomTagProcessor* pTagProcessor,
                      bool isLocalized);
CustomTagProcessor* createTagProcessor(const LayoutInitInfo& rInfo, const Resource* pResource);
void reallocateTextBoxStringBuffer(LayoutActor* pActor, const Resource* pResource,
                                   const char* pSuffix);
}  // namespace al
