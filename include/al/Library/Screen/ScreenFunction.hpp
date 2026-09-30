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
}  // namespace al
