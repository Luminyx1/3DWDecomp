#include "Library/Screen/ScreenFunction.hpp"

#include <gfx/seadCamera.h>
#include <gfx/seadProjection.h>
#include <gfx/seadViewport.h>
#include <math/seadMathCalcCommon.h>

#include "Library/Camera/CameraUtil.hpp"
#include "Library/Math/MathUtil.hpp"
#include "Project/Camera/Core/CameraUtil.hpp"

namespace al {
namespace {

/**
 * Returns the width of the display used by a view.
 * @param viewIndex view index (1 is the sub display)
 * @return width in pixels
 */
inline f32 getViewDisplayWidth(s32 viewIndex) {
    return viewIndex == 1 ? getSubDisplayWidth() : getDisplayWidth();
}

/**
 * Returns the height of the display used by a view.
 * @param viewIndex view index (1 is the sub display)
 * @return height in pixels
 */
inline f32 getViewDisplayHeight(s32 viewIndex) {
    return viewIndex == 1 ? getSubDisplayHeight() : getDisplayHeight();
}

/**
 * Converts a position relative to the screen center to a world position at a fixed distance.
 * @param pOut world position
 * @param rCenterPos position relative to the screen center
 * @param distance distance from the camera (ignored if not positive)
 * @param rCamera camera
 * @param rProjection projection
 * @param rViewport viewport
 */
inline void calcWorldPositionFromCenterScreenByDistance(sead::Vector3f* pOut,
                                                        const sead::Vector2f& rCenterPos,
                                                        f32 distance,
                                                        const sead::Camera& rCamera,
                                                        const sead::Projection& rProjection,
                                                        const sead::Viewport& rViewport) {
    sead::Vector2f screenPos(rCenterPos.x / rViewport.getHalfSizeX(),
                             -rCenterPos.y / rViewport.getHalfSizeY());
    sead::Vector3f cameraPos;
    rProjection.screenPosToCameraPos(&cameraPos, screenPos);

    if (distance > 0.0f) {
        cameraPos *= -distance / cameraPos.z;
    }

    rCamera.cameraPosToWorldPosByMatrix(pOut, cameraPos);
}

}  // namespace
/**
 * Returns the width of the display.
 * @return width in pixels
 */
s32 getDisplayWidth() {
    return 1920;
}

/**
 * Returns the height of the display.
 * @return height in pixels
 */
s32 getDisplayHeight() {
    return 1080;
}

/**
 * Returns the width of the debug menu display.
 * @return width in pixels
 */
s32 getDebugMenuDisplayWidth() {
    return 1280;
}

/**
 * Returns the height of the debug menu display.
 * @return height in pixels
 */
s32 getDebugMenuDisplayHeight() {
    return 720;
}

/**
 * Returns the width of the sub display.
 * @return width in pixels
 */
u32 getSubDisplayWidth() {
    return 1280;
}

/**
 * Returns the height of the sub display.
 * @return height in pixels
 */
u32 getSubDisplayHeight() {
    return 720;
}

/**
 * Returns the width of the layout display.
 * @return width in layout units
 */
u32 getLayoutDisplayWidth() {
    return 1280;
}

/**
 * Returns the height of the layout display.
 * @return height in layout units
 */
u32 getLayoutDisplayHeight() {
    return 720;
}

/**
 * Returns the width of the virtual display.
 * @return width in pixels
 */
s32 getVirtualDisplayWidth() {
    return 1920;
}

/**
 * Returns the height of the virtual display.
 * @return height in pixels
 */
s32 getVirtualDisplayHeight() {
    return 1080;
}

/**
 * Sets a viewport to the display rectangle.
 * @param rViewport viewport to set
 */
void getDisplayViewport(sead::Viewport& rViewport) {
    rViewport.set(0.0f, 0.0f, 1920.0f, 1080.0f);
}

/**
 * Sets a viewport to the sub display rectangle.
 * @param rViewport viewport to set
 */
void getSubDisplayViewport(sead::Viewport& rViewport) {
    rViewport.set(0.0f, 0.0f, 1280.0f, 720.0f);
}

/**
 * Creates a viewport covering the display.
 * @return the new viewport
 */
sead::Viewport* getDisplayViewport() {
    auto* viewport = new sead::Viewport();
    getDisplayViewport(*viewport);
    return viewport;
}

/**
 * Creates a viewport covering the sub display.
 * @return the new viewport
 */
sead::Viewport* getSubDisplayViewport() {
    auto* viewport = new sead::Viewport();
    getSubDisplayViewport(*viewport);
    return viewport;
}

/**
 * Checks whether a display position is on screen.
 * @param rPos display position
 * @param margin margin allowed outside the screen
 * @return whether the position is on screen
 */
bool isInScreen(const sead::Vector2f& rPos, f32 margin) {
    if (rPos.x < -margin || rPos.y < -margin || rPos.x > margin + getDisplayWidth() ||
        rPos.y > margin + getDisplayHeight()) {
        return false;
    }

    return true;
}

/**
 * Converts a layout position to a screen position.
 * @param pOut screen position
 * @param rLayoutPos layout position
 */
void calcScreenPosFromLayoutPos(sead::Vector2f* pOut, const sead::Vector2f& rLayoutPos) {
    pOut->x = rLayoutPos.x + 640.0f;
    pOut->y = 360.0f - rLayoutPos.y;
}

/**
 * Converts a screen position to a layout position.
 * @param pOut layout position
 * @param rScreenPos screen position
 */
void calcLayoutPosFromScreenPos(sead::Vector2f* pOut, const sead::Vector2f& rScreenPos) {
    pOut->x = rScreenPos.x + -640.0f;
    pOut->y = 360.0f - rScreenPos.y;
}

/**
 * Converts a screen position to a world position using an orthonormal camera matrix.
 * @param pOut world position (may be null)
 * @param rScreenPos screen position
 * @param rCameraMtx camera (view) matrix
 * @param distance distance from the camera (a default distance is used if negative)
 * @return always true
 */
bool calcWorldPosFromScreen(sead::Vector3f* pOut, const sead::Vector2f& rScreenPos,
                            const sead::Matrix34f& rCameraMtx, f32 distance) {
    f32 depth = distance >= 0.0f ? distance : 869.1169f;
    f32 scale = depth / 869.1169f;
    sead::Vector3f cameraPos(scale * (rScreenPos.x - 960.0f), -(scale * (rScreenPos.y - 540.0f)),
                             -depth);

    if (pOut == nullptr) {
        return true;
    }

    sead::Matrix34f invMtx;
    invMtx.m[0][0] = rCameraMtx.m[0][0];
    invMtx.m[0][1] = rCameraMtx.m[1][0];
    invMtx.m[0][2] = rCameraMtx.m[2][0];
    invMtx.m[0][3] = -rCameraMtx.m[0][0] * rCameraMtx.m[0][3] -
                     rCameraMtx.m[1][0] * rCameraMtx.m[1][3] -
                     rCameraMtx.m[2][0] * rCameraMtx.m[2][3];
    invMtx.m[1][0] = rCameraMtx.m[0][1];
    invMtx.m[1][1] = rCameraMtx.m[1][1];
    invMtx.m[1][2] = rCameraMtx.m[2][1];
    invMtx.m[1][3] = -rCameraMtx.m[0][1] * rCameraMtx.m[0][3] -
                     rCameraMtx.m[1][1] * rCameraMtx.m[1][3] -
                     rCameraMtx.m[2][1] * rCameraMtx.m[2][3];
    invMtx.m[2][0] = rCameraMtx.m[0][2];
    invMtx.m[2][1] = rCameraMtx.m[1][2];
    invMtx.m[2][2] = rCameraMtx.m[2][2];
    invMtx.m[2][3] = -rCameraMtx.m[0][2] * rCameraMtx.m[0][3] -
                     rCameraMtx.m[1][2] * rCameraMtx.m[1][3] -
                     rCameraMtx.m[2][2] * rCameraMtx.m[2][3];
    pOut->setMul(invMtx, cameraPos);
    return true;
}

/**
 * Converts a screen position to a world position at a fixed distance from the camera.
 * @param pOut world position
 * @param pCamera camera user
 * @param rScreenPos screen position
 * @param distance distance from the camera (ignored if not positive)
 */
void calcWorldPosFromScreenPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                               const sead::Vector2f& rScreenPos, f32 distance) {
    sead::Vector2f layoutPos(rScreenPos.x - getDisplayWidth() * 0.5f,
                             rScreenPos.y - getDisplayHeight() * 0.5f);
    calcWorldPosFromLayoutPos(pOut, pCamera, layoutPos, distance);
}

/**
 * Converts a layout position to a world position at a fixed distance from the camera.
 * @param pOut world position
 * @param pCamera camera user
 * @param rLayoutPos layout position
 * @param distance distance from the camera (ignored if not positive)
 */
void calcWorldPosFromLayoutPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                               const sead::Vector2f& rLayoutPos, f32 distance) {
    sead::Viewport viewport(0.0f, 0.0f, getLayoutDisplayWidth(), getLayoutDisplayHeight());
    const sead::LookAtCamera* camera = getCameraLookAtCamera(pCamera);
    const sead::PerspectiveProjection* projection = getCameraProjection(pCamera);
    calcWorldPositionFromCenterScreenByDistance(pOut, rLayoutPos, distance, *camera, *projection,
                                                viewport);
}

/**
 * Converts a screen position to a world position at the depth of another world position.
 * @param pOut world position
 * @param pCamera camera user
 * @param rScreenPos screen position
 * @param rDepthPos world position giving the depth
 */
void calcWorldPosFromScreenPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                               const sead::Vector2f& rScreenPos, const sead::Vector3f& rDepthPos) {
    sead::Vector2f layoutPos(rScreenPos.x - getDisplayWidth() * 0.5f,
                             rScreenPos.y - getDisplayHeight() * 0.5f);
    calcWorldPosFromLayoutPos(pOut, pCamera, layoutPos, rDepthPos);
}

/**
 * Converts a layout position to a world position at the depth of another world position.
 * @param pOut world position
 * @param pCamera camera user
 * @param rLayoutPos layout position
 * @param rDepthPos world position giving the depth
 */
void calcWorldPosFromLayoutPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                               const sead::Vector2f& rLayoutPos, const sead::Vector3f& rDepthPos) {
    sead::Viewport viewport(0.0f, 0.0f, getLayoutDisplayWidth(), getLayoutDisplayHeight());
    const sead::LookAtCamera* camera = getCameraLookAtCamera(pCamera);
    const sead::PerspectiveProjection* projection = getCameraProjection(pCamera);
    ScreenFunction::calcWorldPositionFromCenterScreen(pOut, rLayoutPos, rDepthPos, *camera,
                                                      *projection, viewport);
}

/**
 * Converts a sub screen position to a world position at a fixed distance from the sub camera.
 * @param pOut world position
 * @param pCamera camera user
 * @param rScreenPos sub screen position
 * @param distance distance from the camera (ignored if not positive)
 */
void calcWorldPosFromScreenPosSub(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                  const sead::Vector2f& rScreenPos, f32 distance) {
    sead::Vector2f layoutPos(rScreenPos.x - getSubDisplayWidth() * 0.5f,
                             rScreenPos.y - getSubDisplayHeight() * 0.5f);
    calcWorldPosFromLayoutPosSub(pOut, pCamera, layoutPos, distance);
}

/**
 * Converts a layout position to a world position at a fixed distance from the sub camera.
 * @param pOut world position
 * @param pCamera camera user
 * @param rLayoutPos layout position
 * @param distance distance from the camera (ignored if not positive)
 */
void calcWorldPosFromLayoutPosSub(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                  const sead::Vector2f& rLayoutPos, f32 distance) {
    sead::Viewport viewport(0.0f, 0.0f, getLayoutDisplayWidth(), getLayoutDisplayHeight());
    const sead::LookAtCamera* camera = getCameraLookAtCameraSub(pCamera);
    const sead::PerspectiveProjection* projection = getCameraProjectionSub(pCamera);
    calcWorldPositionFromCenterScreenByDistance(pOut, rLayoutPos, distance, *camera, *projection,
                                                viewport);
}

/**
 * Converts a sub screen position to a world position at the depth of another world position.
 * @param pOut world position
 * @param pCamera camera user
 * @param rScreenPos sub screen position
 * @param rDepthPos world position giving the depth
 */
void calcWorldPosFromScreenPosSub(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                  const sead::Vector2f& rScreenPos,
                                  const sead::Vector3f& rDepthPos) {
    sead::Vector2f layoutPos(rScreenPos.x - getSubDisplayWidth() * 0.5f,
                             rScreenPos.y - getSubDisplayHeight() * 0.5f);
    calcWorldPosFromLayoutPosSub(pOut, pCamera, layoutPos, rDepthPos);
}

/**
 * Converts a layout position to a world position at the depth of another world position, using
 * the sub camera.
 * @param pOut world position
 * @param pCamera camera user
 * @param rLayoutPos layout position
 * @param rDepthPos world position giving the depth
 */
void calcWorldPosFromLayoutPosSub(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                  const sead::Vector2f& rLayoutPos,
                                  const sead::Vector3f& rDepthPos) {
    sead::Viewport viewport(0.0f, 0.0f, getLayoutDisplayWidth(), getLayoutDisplayHeight());
    const sead::LookAtCamera* camera = getCameraLookAtCameraSub(pCamera);
    const sead::PerspectiveProjection* projection = getCameraProjectionSub(pCamera);
    ScreenFunction::calcWorldPositionFromCenterScreen(pOut, rLayoutPos, rDepthPos, *camera,
                                                      *projection, viewport);
}

}  // namespace al

namespace ScreenFunction {

/**
 * Converts a position relative to the screen center to a world position at the depth of another
 * world position.
 * @param pOut world position
 * @param rCenterPos position relative to the screen center
 * @param rDepthPos world position giving the depth
 * @param rCamera camera
 * @param rProjection projection
 * @param rViewport viewport
 */
void calcWorldPositionFromCenterScreen(sead::Vector3f* pOut, const sead::Vector2f& rCenterPos,
                                       const sead::Vector3f& rDepthPos,
                                       const sead::Camera& rCamera,
                                       const sead::Projection& rProjection,
                                       const sead::Viewport& rViewport) {
    sead::Vector2f screenPos;
    sead::Vector3f cameraPos;
    sead::Vector3f depthPos;
    rCamera.worldPosToCameraPosByMatrix(&depthPos, rDepthPos);
    f32 depth = depthPos.z;

    sead::Vector2f centerPos = rCenterPos;
    screenPos.x = centerPos.x / rViewport.getHalfSizeX();
    screenPos.y = -centerPos.y / rViewport.getHalfSizeY();
    rProjection.screenPosToCameraPos(&cameraPos, screenPos);

    if (depth < 0.0f) {
        cameraPos *= depth / cameraPos.z;
    }

    rCamera.cameraPosToWorldPosByMatrix(pOut, cameraPos);
}

}  // namespace ScreenFunction

namespace al {

/**
 * Converts a world position to a screen position.
 * @param pOut screen position
 * @param pCamera camera user
 * @param rPos world position
 * @param viewIndex view index (unused)
 */
void calcScreenPosFromWorldPos(sead::Vector2f* pOut, const IUseCamera* pCamera,
                               const sead::Vector3f& rPos, s32 viewIndex) {
    calcLayoutPosFromWorldPos(pOut, pCamera, rPos, viewIndex);
    calcScreenPosFromLayoutPos(pOut, *pOut);
}

/**
 * Converts a world position to a layout position.
 * @param pOut layout position
 * @param pCamera camera user
 * @param rPos world position
 * @param viewIndex view index (unused)
 */
void calcLayoutPosFromWorldPos(sead::Vector2f* pOut, const IUseCamera* pCamera,
                               const sead::Vector3f& rPos, s32 viewIndex) {
    sead::Viewport viewport(0.0f, 0.0f, getLayoutDisplayWidth(), getLayoutDisplayHeight());
    getCameraLookAtCamera(pCamera)->projectByMatrix(pOut, rPos, *getCameraProjection(pCamera),
                                                    viewport);
}

/**
 * Converts a world position to a sub screen position.
 * @param pOut sub screen position
 * @param pCamera camera user
 * @param rPos world position
 */
void calcScreenPosFromWorldPosSub(sead::Vector2f* pOut, const IUseCamera* pCamera,
                                  const sead::Vector3f& rPos) {
    calcLayoutPosFromWorldPosSub(pOut, pCamera, rPos);
    calcScreenPosFromLayoutPos(pOut, *pOut);
}

/**
 * Converts a world position to a layout position using the sub camera.
 * @param pOut layout position
 * @param pCamera camera user
 * @param rPos world position
 */
void calcLayoutPosFromWorldPosSub(sead::Vector2f* pOut, const IUseCamera* pCamera,
                                  const sead::Vector3f& rPos) {
    sead::Viewport viewport(0.0f, 0.0f, getLayoutDisplayWidth(), getLayoutDisplayHeight());
    getCameraLookAtCameraSub(pCamera)->projectByMatrix(pOut, rPos,
                                                       *getCameraProjectionSub(pCamera), viewport);
}

/**
 * Converts a world position to a sub screen position, clamped to the screen edges.
 * @param pOut sub screen position
 * @param pCamera camera user
 * @param rPos world position
 */
void calcScreenPosFromWorldPosSubClampInScreen(sead::Vector2f* pOut, const IUseCamera* pCamera,
                                               const sead::Vector3f& rPos) {
    calcLayoutPosFromWorldPosSubClampInScreen(pOut, pCamera, rPos);
    calcScreenPosFromLayoutPos(pOut, *pOut);
}

/**
 * Converts a world position to a layout position using the sub camera, clamped to the screen
 * edges.
 * @param pOut layout position
 * @param pCamera camera user
 * @param rPos world position
 */
void calcLayoutPosFromWorldPosSubClampInScreen(sead::Vector2f* pOut, const IUseCamera* pCamera,
                                               const sead::Vector3f& rPos) {
    sead::Viewport viewport(0.0f, 0.0f, getLayoutDisplayWidth(), getLayoutDisplayHeight());
    sead::Vector3f cameraPos;
    getCameraLookAtCameraSub(pCamera)->worldPosToCameraPosByMatrix(&cameraPos, rPos);

    if (cameraPos.z > 0.0f) {
        cameraPos.x = -cameraPos.x;
        cameraPos.y = -cameraPos.y;
    }

    sead::Vector2f layoutPos = sead::Vector2f::zero;
    getCameraProjectionSub(pCamera)->project(&layoutPos, cameraPos, viewport);

    f32 rateX = sead::Mathf::abs(layoutPos.x) / (getLayoutDisplayWidth() * 0.5f);
    f32 rateY = sead::Mathf::abs(layoutPos.y) / (getLayoutDisplayHeight() * 0.5f);
    f32 rate = sead::Mathf::max(rateX, rateY);

    if (rate <= 1.0f) {
        pOut->set(layoutPos);
    } else {
        pOut->set(layoutPos * (1.0f / rate));
    }
}

/**
 * Converts a world position to a layout position plus camera space depth.
 * @param pOut layout position (x, y) and camera space depth (z)
 * @param pCamera camera user
 * @param rPos world position
 */
void calcLayoutPosFromWorldPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                               const sead::Vector3f& rPos) {
    sead::Viewport viewport(0.0f, 0.0f, getLayoutDisplayWidth(), getLayoutDisplayHeight());
    sead::Vector3f cameraPos;
    getCameraLookAtCamera(pCamera)->worldPosToCameraPosByMatrix(&cameraPos, rPos);

    sead::Vector2f layoutPos;
    getCameraProjection(pCamera)->project(&layoutPos, cameraPos, viewport);
    pOut->set(layoutPos.x, layoutPos.y, cameraPos.z);
}

/**
 * Converts a world position to a layout position plus camera space depth, pushing positions too
 * close to the camera plane out to a minimum depth.
 * @param pOut layout position (x, y) and camera space depth (z)
 * @param pCamera camera user
 * @param rPos world position
 * @param range minimum absolute camera space depth
 * @param viewIndex view index (unused)
 */
void calcLayoutPosFromWorldPosWithClampOutRange(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                                const sead::Vector3f& rPos, f32 range,
                                                s32 viewIndex) {
    calcLayoutPosFromWorldPosWithClampOutRange(pOut, getSceneCameraInfo(pCamera), rPos, range,
                                               viewIndex);
}

/**
 * Converts a world position to a layout position plus camera space depth, pushing positions too
 * close to the camera plane out to a minimum depth.
 * @param pOut layout position (x, y) and camera space depth (z)
 * @param pCameraInfo scene camera info
 * @param rPos world position
 * @param range minimum absolute camera space depth
 * @param viewIndex view index (unused)
 */
void calcLayoutPosFromWorldPosWithClampOutRange(sead::Vector3f* pOut,
                                                const SceneCameraInfo* pCameraInfo,
                                                const sead::Vector3f& rPos, f32 range,
                                                s32 viewIndex) {
    sead::Viewport viewport(0.0f, 0.0f, getLayoutDisplayWidth(), getLayoutDisplayHeight());
    sead::Vector3f cameraPos;
    getCameraLookAtCamera(pCameraInfo)->worldPosToCameraPosByMatrix(&cameraPos, rPos);

    f32 minRange = -range;
    if (cameraPos.z > minRange && cameraPos.z < range) {
        cameraPos.z = cameraPos.z > 0.0f ? range : minRange;
    }

    sead::Vector2f layoutPos;
    getCameraProjection(pCameraInfo)->project(&layoutPos, cameraPos, viewport);
    pOut->set(layoutPos.x, layoutPos.y, cameraPos.z);
}

/**
 * Converts a world position to a display position plus camera space depth, pushing positions too
 * close to the camera plane out to a minimum depth.
 * @param pOut display position (x, y) and camera space depth (z)
 * @param pCamera camera user
 * @param rPos world position
 * @param range minimum absolute camera space depth
 * @param viewIndex view index (1 is the sub display)
 */
void calcDisplayPosFromWorldPosWithClampOutRange(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                                 const sead::Vector3f& rPos, f32 range,
                                                 s32 viewIndex) {
    calcDisplayPosFromWorldPosWithClampOutRange(pOut, getSceneCameraInfo(pCamera), rPos, range,
                                                viewIndex);
}

/**
 * Converts a world position to a display position plus camera space depth, pushing positions too
 * close to the camera plane out to a minimum depth.
 * @param pOut display position (x, y) and camera space depth (z)
 * @param pCameraInfo scene camera info
 * @param rPos world position
 * @param range minimum absolute camera space depth
 * @param viewIndex view index (1 is the sub display)
 */
void calcDisplayPosFromWorldPosWithClampOutRange(sead::Vector3f* pOut,
                                                 const SceneCameraInfo* pCameraInfo,
                                                 const sead::Vector3f& rPos, f32 range,
                                                 s32 viewIndex) {
    sead::Viewport viewport(0.0f, 0.0f, getViewDisplayWidth(viewIndex),
                            getViewDisplayHeight(viewIndex));
    sead::Vector3f cameraPos;
    getCameraLookAtCamera(pCameraInfo)->worldPosToCameraPosByMatrix(&cameraPos, rPos);

    f32 minRange = -range;
    if (cameraPos.z > minRange && cameraPos.z < range) {
        cameraPos.z = cameraPos.z > 0.0f ? range : minRange;
    }

    sead::Vector2f displayPos;
    getCameraProjection(pCameraInfo)->project(&displayPos, cameraPos, viewport);
    pOut->set(displayPos.x, displayPos.y, cameraPos.z);
}

/**
 * Converts a layout radius to a world radius at a fixed distance from the camera.
 * @param pCamera camera user
 * @param distance distance from the camera
 * @param radius layout radius
 * @return world radius
 */
f32 calcWorldRadiusFromLayoutRadius(const IUseCamera* pCamera, f32 distance, f32 radius) {
    sead::Viewport viewport(0.0f, 0.0f, getLayoutDisplayWidth(), getLayoutDisplayHeight());
    sead::Vector3f worldPos;
    const sead::LookAtCamera* camera = getCameraLookAtCamera(pCamera);
    const sead::PerspectiveProjection* projection = getCameraProjection(pCamera);
    calcWorldPositionFromCenterScreenByDistance(&worldPos, sead::Vector2f(radius, 0.0f), distance,
                                                *camera, *projection, viewport);
    return worldPos.x;
}

/**
 * Converts a world radius around a world position to a layout radius.
 * @param rPos world position
 * @param pCamera camera user
 * @param radius world radius
 * @return layout radius
 */
f32 calcLayoutRadiusFromWorldRadius(const sead::Vector3f& rPos, const IUseCamera* pCamera,
                                    f32 radius) {
    sead::Viewport viewport(0.0f, 0.0f, getLayoutDisplayWidth(), getLayoutDisplayHeight());
    sead::Vector3f cameraPos;
    getCameraLookAtCamera(pCamera)->worldPosToCameraPosByMatrix(&cameraPos, rPos);
    cameraPos.x = radius;
    cameraPos.y = 0.0f;

    sead::Vector2f layoutPos;
    getCameraProjection(pCamera)->project(&layoutPos, cameraPos, viewport);
    return layoutPos.x;
}

/**
 * Converts a world radius around a world position to a screen radius.
 * @param rPos world position
 * @param pCamera camera user
 * @param radius world radius
 * @return screen radius
 */
f32 calcScreenRadiusFromWorldRadius(const sead::Vector3f& rPos, const IUseCamera* pCamera,
                                    f32 radius) {
    sead::Viewport viewport(0.0f, 0.0f, getDisplayWidth(), getDisplayHeight());
    sead::Vector3f cameraPos;
    getCameraLookAtCamera(pCamera)->worldPosToCameraPosByMatrix(&cameraPos, rPos);
    cameraPos.x = radius;
    cameraPos.y = 0.0f;

    sead::Vector2f screenPos;
    getCameraProjection(pCamera)->project(&screenPos, cameraPos, viewport);
    return screenPos.x;
}

/**
 * Converts a world radius around a world position to a sub screen radius.
 * @param rPos world position
 * @param pCamera camera user
 * @param radius world radius
 * @return sub screen radius
 */
f32 calcScreenRadiusFromWorldRadiusSub(const sead::Vector3f& rPos, const IUseCamera* pCamera,
                                       f32 radius) {
    sead::Viewport viewport(0.0f, 0.0f, getSubDisplayWidth(), getSubDisplayHeight());
    sead::Vector3f cameraPos;
    getCameraLookAtCameraSub(pCamera)->worldPosToCameraPosByMatrix(&cameraPos, rPos);
    cameraPos.x = radius;
    cameraPos.y = 0.0f;

    sead::Vector2f screenPos;
    getCameraProjectionSub(pCamera)->project(&screenPos, cameraPos, viewport);
    return screenPos.x;
}

/**
 * Calculates the direction from the camera to the world position under a screen position.
 * @param pOut normalized direction
 * @param pCamera camera user
 * @param rScreenPos screen position
 * @param distance distance from the camera (ignored if not positive)
 * @return whether the direction could be normalized
 */
bool calcCameraPosToWorldPosDirFromScreenPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                             const sead::Vector2f& rScreenPos, f32 distance) {
    return calcCameraPosToWorldPosDirFromScreenPos(pOut, getSceneCameraInfo(pCamera), rScreenPos,
                                                   distance, 0);
}

/**
 * Calculates the direction from the camera to the world position under a screen position.
 * @param pOut normalized direction
 * @param pCameraInfo scene camera info
 * @param rScreenPos screen position
 * @param distance distance from the camera (ignored if not positive)
 * @param viewIndex view index (1 is the sub display)
 * @return whether the direction could be normalized
 */
bool calcCameraPosToWorldPosDirFromScreenPos(sead::Vector3f* pOut,
                                             const SceneCameraInfo* pCameraInfo,
                                             const sead::Vector2f& rScreenPos, f32 distance,
                                             s32 viewIndex) {
    sead::Vector3f worldPos;
    calcWorldPosFromScreenPos(&worldPos, pCameraInfo, rScreenPos, distance, viewIndex);
    pOut->setSub(worldPos, getCameraPos(pCameraInfo));
    return tryNormalizeOrZero(pOut);
}

/**
 * Calculates the direction from the camera to the world position under a screen position.
 * @param pOut normalized direction
 * @param pCamera camera user
 * @param rScreenPos screen position
 * @param rDepthPos world position giving the depth
 * @return whether the direction could be normalized
 */
bool calcCameraPosToWorldPosDirFromScreenPos(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                             const sead::Vector2f& rScreenPos,
                                             const sead::Vector3f& rDepthPos) {
    return calcCameraPosToWorldPosDirFromScreenPos(pOut, getSceneCameraInfo(pCamera), rScreenPos,
                                                   rDepthPos, 0);
}

/**
 * Calculates the direction from the camera to the world position under a screen position.
 * @param pOut normalized direction
 * @param pCameraInfo scene camera info
 * @param rScreenPos screen position
 * @param rDepthPos world position giving the depth
 * @param viewIndex view index (1 is the sub display)
 * @return whether the direction could be normalized
 */
bool calcCameraPosToWorldPosDirFromScreenPos(sead::Vector3f* pOut,
                                             const SceneCameraInfo* pCameraInfo,
                                             const sead::Vector2f& rScreenPos,
                                             const sead::Vector3f& rDepthPos, s32 viewIndex) {
    sead::Vector3f worldPos;
    calcWorldPosFromScreenPos(&worldPos, pCameraInfo, rScreenPos, rDepthPos, viewIndex);
    pOut->setSub(worldPos, getCameraPos(pCameraInfo));
    return tryNormalizeOrZero(pOut);
}

/**
 * Calculates the direction from the sub camera to the world position under a sub screen
 * position.
 * @param pOut normalized direction
 * @param pCamera camera user
 * @param rScreenPos sub screen position
 * @param distance distance from the camera (ignored if not positive)
 */
void calcCameraPosToWorldPosDirFromScreenPosSub(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                                const sead::Vector2f& rScreenPos, f32 distance) {
    sead::Vector3f worldPos;
    calcWorldPosFromScreenPosSub(&worldPos, pCamera, rScreenPos, distance);
    pOut->setSub(worldPos, getCameraPosSub(pCamera));
    normalizeOrZero(pOut);
}

/**
 * Calculates the direction from the sub camera to the world position under a sub screen
 * position.
 * @param pOut normalized direction
 * @param pCamera camera user
 * @param rScreenPos sub screen position
 * @param rDepthPos world position giving the depth
 */
void calcCameraPosToWorldPosDirFromScreenPosSub(sead::Vector3f* pOut, const IUseCamera* pCamera,
                                                const sead::Vector2f& rScreenPos,
                                                const sead::Vector3f& rDepthPos) {
    sead::Vector3f worldPos;
    calcWorldPosFromScreenPosSub(&worldPos, pCamera, rScreenPos, rDepthPos);
    pOut->setSub(worldPos, getCameraPosSub(pCamera));
    normalizeOrZero(pOut);
}

/**
 * Calculates the camera ray segment going through a screen position.
 * @param pStart start of the segment (at the near distance)
 * @param pLine segment vector (from near to far distance)
 * @param pCamera camera user
 * @param rScreenPos screen position
 * @param near near distance
 * @param far far distance
 */
void calcLineCameraToWorldPosFromScreenPos(sead::Vector3f* pStart, sead::Vector3f* pLine,
                                           const IUseCamera* pCamera,
                                           const sead::Vector2f& rScreenPos, f32 near, f32 far) {
    sead::Vector3f dir;
    calcCameraPosToWorldPosDirFromScreenPos(&dir, pCamera, rScreenPos, 0.0f);
    pStart->setScaleAdd(near, dir, getCameraPos(pCamera));
    *pLine = dir * (far - near);
}

/**
 * Calculates the camera ray segment going through a screen position, between the camera's near
 * and far clip distances.
 * @param pStart start of the segment
 * @param pLine segment vector
 * @param pCamera camera user
 * @param rScreenPos screen position
 */
void calcLineCameraToWorldPosFromScreenPos(sead::Vector3f* pStart, sead::Vector3f* pLine,
                                           const IUseCamera* pCamera,
                                           const sead::Vector2f& rScreenPos) {
    calcLineCameraToWorldPosFromScreenPos(pStart, pLine, pCamera, rScreenPos,
                                          getCameraNear(pCamera), getCameraFar(pCamera));
}

/**
 * Calculates the sub camera ray segment going through a sub screen position.
 * @param pStart start of the segment (at the near distance)
 * @param pLine segment vector (from near to far distance)
 * @param pCamera camera user
 * @param rScreenPos sub screen position
 * @param near near distance
 * @param far far distance
 */
void calcLineCameraToWorldPosFromScreenPosSub(sead::Vector3f* pStart, sead::Vector3f* pLine,
                                              const IUseCamera* pCamera,
                                              const sead::Vector2f& rScreenPos, f32 near,
                                              f32 far) {
    sead::Vector3f dir;
    calcCameraPosToWorldPosDirFromScreenPosSub(&dir, pCamera, rScreenPos, 0.0f);
    pStart->setScaleAdd(near, dir, getCameraPosSub(pCamera));
    *pLine = dir * (far - near);
}

/**
 * Calculates the sub camera ray segment going through a sub screen position, between the
 * camera's near and far clip distances.
 * @param pStart start of the segment
 * @param pLine segment vector
 * @param pCamera camera user
 * @param rScreenPos sub screen position
 */
void calcLineCameraToWorldPosFromScreenPosSub(sead::Vector3f* pStart, sead::Vector3f* pLine,
                                              const IUseCamera* pCamera,
                                              const sead::Vector2f& rScreenPos) {
    calcLineCameraToWorldPosFromScreenPosSub(pStart, pLine, pCamera, rScreenPos,
                                             getCameraNear(pCamera), getCameraFar(pCamera));
}

/**
 * Converts a display-centered position to a world position at a fixed distance from the camera.
 * @param pOut world position
 * @param pCameraInfo scene camera info
 * @param rLayoutPos position relative to the display center
 * @param distance distance from the camera (ignored if not positive)
 * @param viewIndex view index (1 is the sub display)
 */
void calcWorldPosFromLayoutPos(sead::Vector3f* pOut, const SceneCameraInfo* pCameraInfo,
                               const sead::Vector2f& rLayoutPos, f32 distance, s32 viewIndex) {
    sead::Viewport viewport(0.0f, 0.0f, getViewDisplayWidth(viewIndex),
                            getViewDisplayHeight(viewIndex));
    const sead::LookAtCamera* camera = getCameraLookAtCamera(pCameraInfo);
    const sead::PerspectiveProjection* projection = getCameraProjection(pCameraInfo);
    calcWorldPositionFromCenterScreenByDistance(pOut, rLayoutPos, distance, *camera, *projection,
                                                viewport);
}

/**
 * Converts a display-centered position to a world position at the depth of another world
 * position.
 * @param pOut world position
 * @param pCameraInfo scene camera info
 * @param rLayoutPos position relative to the display center
 * @param rDepthPos world position giving the depth
 * @param viewIndex view index (1 is the sub display)
 */
void calcWorldPosFromLayoutPos(sead::Vector3f* pOut, const SceneCameraInfo* pCameraInfo,
                               const sead::Vector2f& rLayoutPos, const sead::Vector3f& rDepthPos,
                               s32 viewIndex) {
    sead::Viewport viewport(0.0f, 0.0f, getViewDisplayWidth(viewIndex),
                            getViewDisplayHeight(viewIndex));
    const sead::LookAtCamera* camera = getCameraLookAtCamera(pCameraInfo);
    const sead::PerspectiveProjection* projection = getCameraProjection(pCameraInfo);
    ScreenFunction::calcWorldPositionFromCenterScreen(pOut, rLayoutPos, rDepthPos, *camera,
                                                      *projection, viewport);
}

/**
 * Converts a display position to a world position at a fixed distance from the camera.
 * @param pOut world position
 * @param pCameraInfo scene camera info
 * @param rScreenPos display position
 * @param distance distance from the camera (ignored if not positive)
 * @param viewIndex view index (1 is the sub display)
 */
void calcWorldPosFromScreenPos(sead::Vector3f* pOut, const SceneCameraInfo* pCameraInfo,
                               const sead::Vector2f& rScreenPos, f32 distance, s32 viewIndex) {
    sead::Vector2f layoutPos(rScreenPos.x - getViewDisplayWidth(viewIndex) * 0.5f,
                             rScreenPos.y - getViewDisplayHeight(viewIndex) * 0.5f);
    calcWorldPosFromLayoutPos(pOut, pCameraInfo, layoutPos, distance, viewIndex);
}

/**
 * Converts a display position to a world position at the depth of another world position.
 * @param pOut world position
 * @param pCameraInfo scene camera info
 * @param rScreenPos display position
 * @param rDepthPos world position giving the depth
 * @param viewIndex view index (1 is the sub display)
 */
void calcWorldPosFromScreenPos(sead::Vector3f* pOut, const SceneCameraInfo* pCameraInfo,
                               const sead::Vector2f& rScreenPos, const sead::Vector3f& rDepthPos,
                               s32 viewIndex) {
    sead::Vector2f layoutPos(rScreenPos.x - getViewDisplayWidth(viewIndex) * 0.5f,
                             rScreenPos.y - getViewDisplayHeight(viewIndex) * 0.5f);
    calcWorldPosFromLayoutPos(pOut, pCameraInfo, layoutPos, rDepthPos, viewIndex);
}

/**
 * Converts a world position to a layout position.
 * @param pOut layout position
 * @param pCameraInfo scene camera info
 * @param rPos world position
 * @param viewIndex view index (unused)
 */
void calcLayoutPosFromWorldPos(sead::Vector2f* pOut, const SceneCameraInfo* pCameraInfo,
                               const sead::Vector3f& rPos, s32 viewIndex) {
    sead::Viewport viewport(0.0f, 0.0f, getLayoutDisplayWidth(), getLayoutDisplayHeight());
    getCameraLookAtCamera(pCameraInfo)
        ->projectByMatrix(pOut, rPos, *getCameraProjection(pCameraInfo), viewport);
}

/**
 * Converts a world position to a layout position plus camera space depth.
 * @param pOut layout position (x, y) and camera space depth (z)
 * @param pCameraInfo scene camera info
 * @param rPos world position
 * @param viewIndex view index (unused)
 */
void calcLayoutPosFromWorldPos(sead::Vector3f* pOut, const SceneCameraInfo* pCameraInfo,
                               const sead::Vector3f& rPos, s32 viewIndex) {
    sead::Viewport viewport(0.0f, 0.0f, getLayoutDisplayWidth(), getLayoutDisplayHeight());
    sead::Vector3f cameraPos;
    getCameraLookAtCamera(pCameraInfo)->worldPosToCameraPosByMatrix(&cameraPos, rPos);

    sead::Vector2f layoutPos;
    getCameraProjection(pCameraInfo)->project(&layoutPos, cameraPos, viewport);
    pOut->set(layoutPos.x, layoutPos.y, cameraPos.z);
}

/**
 * Calculates the camera ray segment going through a display position.
 * @param pStart start of the segment (at the near distance)
 * @param pLine segment vector (from near to far distance)
 * @param pCameraInfo scene camera info
 * @param rScreenPos display position
 * @param near near distance
 * @param far far distance
 * @param viewIndex view index (1 is the sub display)
 */
void calcLineCameraToWorldPosFromScreenPos(sead::Vector3f* pStart, sead::Vector3f* pLine,
                                           const SceneCameraInfo* pCameraInfo,
                                           const sead::Vector2f& rScreenPos, f32 near, f32 far,
                                           s32 viewIndex) {
    sead::Vector3f dir;
    calcCameraPosToWorldPosDirFromScreenPos(&dir, pCameraInfo, rScreenPos, 0.0f, viewIndex);
    pStart->setScaleAdd(near, dir, getCameraPos(pCameraInfo));
    *pLine = dir * (far - near);
}

/**
 * Calculates the camera ray segment going through a display position, between the camera's near
 * and far clip distances.
 * @param pStart start of the segment
 * @param pLine segment vector
 * @param pCameraInfo scene camera info
 * @param rScreenPos display position
 * @param viewIndex view index (1 is the sub display)
 */
void calcLineCameraToWorldPosFromScreenPos(sead::Vector3f* pStart, sead::Vector3f* pLine,
                                           const SceneCameraInfo* pCameraInfo,
                                           const sead::Vector2f& rScreenPos, s32 viewIndex) {
    calcLineCameraToWorldPosFromScreenPos(pStart, pLine, pCameraInfo, rScreenPos,
                                          getCameraNear(pCameraInfo), getCameraFar(pCameraInfo),
                                          viewIndex);
}

}  // namespace al
