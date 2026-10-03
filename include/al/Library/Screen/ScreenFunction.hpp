#pragma once

#include <basis/seadTypes.h>
#include <math/seadMatrix.h>
#include <math/seadVector.h>

namespace sead {
class Camera;
class Projection;
class Viewport;
}  // namespace sead

namespace al {
class IUseCamera;
class SceneCameraInfo;

s32 getDisplayWidth();
s32 getDisplayHeight();
s32 getDebugMenuDisplayWidth();
s32 getDebugMenuDisplayHeight();
u32 getSubDisplayWidth();
u32 getSubDisplayHeight();
u32 getLayoutDisplayWidth();
u32 getLayoutDisplayHeight();
s32 getVirtualDisplayWidth();
s32 getVirtualDisplayHeight();
void getDisplayViewport(sead::Viewport& rViewport);
void getSubDisplayViewport(sead::Viewport& rViewport);
sead::Viewport* getDisplayViewport();
sead::Viewport* getSubDisplayViewport();
bool isInScreen(const sead::Vector2f& rPos, f32 margin);

void calcScreenPosFromLayoutPos(sead::Vector2f* pOut, const sead::Vector2f& rLayoutPos);
void calcLayoutPosFromScreenPos(sead::Vector2f* pOut, const sead::Vector2f& rScreenPos);

bool calcWorldPosFromScreen(sead::Vector3f* pOut, const sead::Vector2f& rScreenPos,
                            const sead::Matrix34f& rCameraMtx, f32 distance);
void calcWorldPosFromScreenPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                               const sead::Vector2f& rScreenPos, f32 distance);
void calcWorldPosFromLayoutPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                               const sead::Vector2f& rLayoutPos, f32 distance);
void calcWorldPosFromScreenPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                               const sead::Vector2f& rScreenPos, const sead::Vector3f& rDepthPos);
void calcWorldPosFromLayoutPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                               const sead::Vector2f& rLayoutPos, const sead::Vector3f& rDepthPos);
void calcWorldPosFromScreenPosSub(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                  const sead::Vector2f& rScreenPos, f32 distance);
void calcWorldPosFromLayoutPosSub(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                  const sead::Vector2f& rLayoutPos, f32 distance);
void calcWorldPosFromScreenPosSub(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                  const sead::Vector2f& rScreenPos,
                                  const sead::Vector3f& rDepthPos);
void calcWorldPosFromLayoutPosSub(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                  const sead::Vector2f& rLayoutPos,
                                  const sead::Vector3f& rDepthPos);

void calcScreenPosFromWorldPos(sead::Vector2f* pOut, const IUseCamera* pCamera,
                               const sead::Vector3f& rPos, s32 viewIndex);
void calcLayoutPosFromWorldPos(sead::Vector2f* pOut, const IUseCamera* pCamera,
                               const sead::Vector3f& rPos, s32 viewIndex);
void calcScreenPosFromWorldPosSub(sead::Vector2f* pOut, const IUseCamera* pCamera,
                                  const sead::Vector3f& rPos);
void calcLayoutPosFromWorldPosSub(sead::Vector2f* pOut, const IUseCamera* pCamera,
                                  const sead::Vector3f& rPos);
void calcScreenPosFromWorldPosSubClampInScreen(sead::Vector2f* pOut, const IUseCamera* pCamera,
                                               const sead::Vector3f& rPos);
void calcLayoutPosFromWorldPosSubClampInScreen(sead::Vector2f* pOut, const IUseCamera* pCamera,
                                               const sead::Vector3f& rPos);
void calcLayoutPosFromWorldPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                               const sead::Vector3f& rPos);
void calcLayoutPosFromWorldPosWithClampOutRange(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                                const sead::Vector3f& rPos, f32 range,
                                                s32 viewIndex);
void calcLayoutPosFromWorldPosWithClampOutRange(sead::Vector3f* pOut,
                                                const SceneCameraInfo* pCameraInfo,
                                                const sead::Vector3f& rPos, f32 range,
                                                s32 viewIndex);
void calcDisplayPosFromWorldPosWithClampOutRange(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                                 const sead::Vector3f& rPos, f32 range,
                                                 s32 viewIndex);
void calcDisplayPosFromWorldPosWithClampOutRange(sead::Vector3f* pOut,
                                                 const SceneCameraInfo* pCameraInfo,
                                                 const sead::Vector3f& rPos, f32 range,
                                                 s32 viewIndex);

f32 calcWorldRadiusFromLayoutRadius(const IUseCamera* pCamera, f32 distance, f32 radius);
f32 calcLayoutRadiusFromWorldRadius(const sead::Vector3f& rPos, const IUseCamera* pCamera,
                                    f32 radius);
f32 calcScreenRadiusFromWorldRadius(const sead::Vector3f& rPos, const IUseCamera* pCamera,
                                    f32 radius);
f32 calcScreenRadiusFromWorldRadiusSub(const sead::Vector3f& rPos, const IUseCamera* pCamera,
                                       f32 radius);

bool calcCameraPosToWorldPosDirFromScreenPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                             const sead::Vector2f& rScreenPos, f32 distance);
bool calcCameraPosToWorldPosDirFromScreenPos(sead::Vector3f* pOut,
                                             const SceneCameraInfo* pCameraInfo,
                                             const sead::Vector2f& rScreenPos, f32 distance,
                                             s32 viewIndex);
bool calcCameraPosToWorldPosDirFromScreenPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                             const sead::Vector2f& rScreenPos,
                                             const sead::Vector3f& rDepthPos);
bool calcCameraPosToWorldPosDirFromScreenPos(sead::Vector3f* pOut,
                                             const SceneCameraInfo* pCameraInfo,
                                             const sead::Vector2f& rScreenPos,
                                             const sead::Vector3f& rDepthPos, s32 viewIndex);
void calcCameraPosToWorldPosDirFromScreenPosSub(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                                const sead::Vector2f& rScreenPos, f32 distance);
void calcCameraPosToWorldPosDirFromScreenPosSub(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                                const sead::Vector2f& rScreenPos,
                                                const sead::Vector3f& rDepthPos);

void calcLineCameraToWorldPosFromScreenPos(sead::Vector3f* pStart, sead::Vector3f* pLine,
                                           const IUseCamera* pCamera,
                                           const sead::Vector2f& rScreenPos, f32 near, f32 far);
void calcLineCameraToWorldPosFromScreenPos(sead::Vector3f* pStart, sead::Vector3f* pLine,
                                           const IUseCamera* pCamera,
                                           const sead::Vector2f& rScreenPos);
void calcLineCameraToWorldPosFromScreenPosSub(sead::Vector3f* pStart, sead::Vector3f* pLine,
                                              const IUseCamera* pCamera,
                                              const sead::Vector2f& rScreenPos, f32 near,
                                              f32 far);
void calcLineCameraToWorldPosFromScreenPosSub(sead::Vector3f* pStart, sead::Vector3f* pLine,
                                              const IUseCamera* pCamera,
                                              const sead::Vector2f& rScreenPos);

void calcWorldPosFromLayoutPos(sead::Vector3f* pOut, const SceneCameraInfo* pCameraInfo,
                               const sead::Vector2f& rLayoutPos, f32 distance, s32 viewIndex);
void calcWorldPosFromLayoutPos(sead::Vector3f* pOut, const SceneCameraInfo* pCameraInfo,
                               const sead::Vector2f& rLayoutPos, const sead::Vector3f& rDepthPos,
                               s32 viewIndex);
void calcWorldPosFromScreenPos(sead::Vector3f* pOut, const SceneCameraInfo* pCameraInfo,
                               const sead::Vector2f& rScreenPos, f32 distance, s32 viewIndex);
void calcWorldPosFromScreenPos(sead::Vector3f* pOut, const SceneCameraInfo* pCameraInfo,
                               const sead::Vector2f& rScreenPos, const sead::Vector3f& rDepthPos,
                               s32 viewIndex);
void calcLayoutPosFromWorldPos(sead::Vector2f* pOut, const SceneCameraInfo* pCameraInfo,
                               const sead::Vector3f& rPos, s32 viewIndex);
void calcLayoutPosFromWorldPos(sead::Vector3f* pOut, const SceneCameraInfo* pCameraInfo,
                               const sead::Vector3f& rPos, s32 viewIndex);
void calcLineCameraToWorldPosFromScreenPos(sead::Vector3f* pStart, sead::Vector3f* pLine,
                                           const SceneCameraInfo* pCameraInfo,
                                           const sead::Vector2f& rScreenPos, f32 near, f32 far,
                                           s32 viewIndex);
void calcLineCameraToWorldPosFromScreenPos(sead::Vector3f* pStart, sead::Vector3f* pLine,
                                           const SceneCameraInfo* pCameraInfo,
                                           const sead::Vector2f& rScreenPos, s32 viewIndex);
}  // namespace al

namespace ScreenFunction {
void calcWorldPositionFromCenterScreen(sead::Vector3f* pOut, const sead::Vector2f& rCenterPos,
                                       const sead::Vector3f& rDepthPos,
                                       const sead::Camera& rCamera,
                                       const sead::Projection& rProjection,
                                       const sead::Viewport& rViewport);
}  // namespace ScreenFunction
