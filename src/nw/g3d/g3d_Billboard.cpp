#include <nn/g3d/g3d_Billboard.h>
#include <nn/util/util_VectorApi.h>

namespace nn::g3d {
namespace {
using Matrix = Billboard::Matrix;
/**
 * @brief Construct a three-component vector with zero padding.
 * @param x First component.
 * @param y Second component.
 * @param z Third component.
 * @return Padded vector containing the specified components.
 */
inline float32x4_t Vector(float x, float y, float z) {
    util::Vector3fType value;
    util::VectorSet(&value, x, y, z);
    return value._v;
}
/**
 * @brief Sum four lanes into a duplicated two-lane result.
 * @param value Squared vector components; padding must be zero.
 * @return Pair of identical squared lengths.
 */
inline float32x2_t SumPair(float32x4_t value) {
    float32x2_t sum = vadd_f32(vget_high_f32(value), vget_low_f32(value));
    return vpadd_f32(sum, sum);
}
/**
 * @brief Measure a padded vector's squared length.
 * @param value Three-component vector with zero padding.
 * @return Squared length duplicated into two lanes.
 */
inline float32x2_t LengthSquared(float32x4_t value) { return SumPair(vmulq_f32(value, value)); }
/**
 * @brief Refine a reciprocal square root estimate twice.
 * @param length Squared length replicated across all lanes.
 * @return Refined reciprocal square root; zero inputs require masking by the caller.
 */
inline float32x4_t ReciprocalSqrt(float32x4_t length) {
    float32x4_t inverse = vrsqrteq_f32(length);
    inverse = vmulq_f32(inverse, vrsqrtsq_f32(inverse, vmulq_f32(inverse, length)));
    return vmulq_f32(inverse, vrsqrtsq_f32(inverse, vmulq_f32(length, inverse)));
}
/**
 * @brief Normalize a vector using two reciprocal-square-root refinements.
 * @param value Vector whose squared length is supplied separately.
 * @param length Squared length of value replicated across all four lanes.
 * @return Unit vector; a zero squared length produces a zero vector.
 */
inline float32x4_t Normalize(float32x4_t value, float32x4_t length) {
    float32x4_t inverse = ReciprocalSqrt(length);
    return vreinterpretq_f32_u32(
        vandq_u32(vreinterpretq_u32_f32(vmulq_f32(value, inverse)), vmvnq_u32(vceqzq_f32(length))));
}
/**
 * @brief Normalize a direction, selecting a default axis for a degenerate input.
 * @param value Direction with zero padding.
 * @param fallback Unit axis used when the squared length is not positive.
 * @return Normalized direction or fallback.
 */
inline float32x4_t NormalizeOr(float32x4_t value, float32x4_t fallback) {
    float32x2_t squaredLength = LengthSquared(value);
    float32x4_t normalized = Normalize(value, vcombine_f32(squaredLength, squaredLength));
    return vget_lane_f32(squaredLength, 0) > 0 ? normalized : fallback;
}
/**
 * @brief Transform a direction by the linear part of an affine matrix.
 * @param value Direction to transform, with its padding ignored.
 * @param rMatrix Transform whose first three rows provide the linear basis.
 * @return Transformed direction.
 */
inline float32x4_t TransformDirection(float32x4_t value, const Matrix& rMatrix) {
    float32x4_t result = vmulq_laneq_f32(rMatrix._m.val[0], value, 0);
    result = vfmaq_laneq_f32(result, rMatrix._m.val[1], value, 1);
    return vfmaq_laneq_f32(result, rMatrix._m.val[2], value, 2);
}
/**
 * @brief Transform a world position into view space.
 * @param value Position to transform.
 * @param rMatrix Affine world-to-view transform.
 * @return Position after rotation and translation.
 */
inline float32x4_t TransformPosition(float32x4_t value, const Matrix& rMatrix) {
    return vaddq_f32(rMatrix._m.val[3], TransformDirection(value, rMatrix));
}
/**
 * @brief Compute a padded three-dimensional cross product.
 * @param first Left-hand vector with zero padding.
 * @param second Right-hand vector with zero padding.
 * @return Cross product of first and second, with zero padding.
 */
inline float32x4_t Cross(float32x4_t first, float32x4_t second) {
    uint8x8_t yz = {4, 5, 6, 7, 8, 9, 10, 11};
    uint8x8_t xw = {0, 1, 2, 3, 12, 13, 14, 15};
    uint8x8_t zx = {8, 9, 10, 11, 0, 1, 2, 3};
    uint8x8_t yw = {4, 5, 6, 7, 12, 13, 14, 15};
    auto a = vreinterpretq_u8_f32(first);
    auto b = vreinterpretq_u8_f32(second);
    float32x2_t ayz = vreinterpret_f32_u8(vqtbl1_u8(a, yz));
    float32x2_t axw = vreinterpret_f32_u8(vqtbl1_u8(a, xw));
    float32x2_t bzx = vreinterpret_f32_u8(vqtbl1_u8(b, zx));
    float32x2_t byw = vreinterpret_f32_u8(vqtbl1_u8(b, yw));
    float32x4_t left = vcombine_f32(vmul_f32(ayz, bzx), vmul_f32(axw, byw));
    float32x4_t rightA = vreinterpretq_f32_u8(vcombine_u8(vqtbl1_u8(a, zx), vqtbl1_u8(a, yw)));
    float32x4_t rightB = vreinterpretq_f32_u8(vcombine_u8(vqtbl1_u8(b, yz), vqtbl1_u8(b, xw)));
    return vfmsq_f32(left, rightA, rightB);
}
/**
 * @brief Build an orthonormal basis facing a viewpoint while retaining a preferred up direction.
 * @param pOutput Matrix receiving the basis; translation is retained.
 * @param up Preferred up direction in view space.
 * @param facing Unit direction towards the camera.
 */
inline void FaceViewpoint(Matrix* pOutput, float32x4_t up, float32x4_t facing) {
    float32x4_t right = Cross(up, facing);
    float32x2_t length = LengthSquared(right);
    if (!(vget_lane_f32(length, 0) > 0)) {
        right = Vector(facing[2], 0, -facing[0]);
        length = LengthSquared(right);
    }
    if (vget_lane_f32(length, 0) > 0) {
        right = Normalize(right, vcombine_f32(length, length));
        up = Cross(facing, right);
    } else {
        right = Vector(1, 0, 0);
        up = Vector(0, 0, -facing[1]);
    }
    pOutput->_m.val[0] = right;
    pOutput->_m.val[1] = up;
    pOutput->_m.val[2] = facing;
}
/**
 * @brief Set a screen-parallel basis using the projected up direction.
 * @param pOutput Destination whose translation is retained and padding is cleared.
 * @param rUp Preferred up direction; its depth and padding are discarded.
 */
ALWAYS_INLINE inline void FaceScreen(Matrix* pOutput, const float32x4_t& rUp) {
    float32x4_t up = vcombine_f32(vget_low_f32(rUp), vdup_n_f32(0));
    pOutput->_m.val[2] = Vector(0, 0, 1);
    up = NormalizeOr(up, Vector(0, 1, 0));
    pOutput->_m.val[0] = Vector(up[1], -up[0], 0);
    pOutput->_m.val[1] = vcombine_f32(vget_low_f32(up), vdup_n_f32(0));
    float32x4_t translation = pOutput->_m.val[3];
    float32x2_t high = vset_lane_f32(0, vget_high_f32(translation), 1);
    pOutput->_m.val[3] = vcombine_f32(vget_low_f32(translation), high);
}
} // namespace
/**
 * @brief Face the screen while preserving the projected world-space up direction.
 * @param pOutput Destination matrix with an existing translation.
 * @param rView World-to-view transform.
 * @param rWorld World transform supplying the local up axis.
 */
void Billboard::CalculateWorld(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld) {
    FaceScreen(pOutput, TransformDirection(rWorld._m.val[1], rView));
}
/**
 * @brief Face the camera position while preserving the world-space up direction.
 * @param pOutput Destination matrix whose translation is retained.
 * @param rView World-to-view transform.
 * @param rWorld World transform supplying up and position.
 */
void Billboard::CalculateWorldViewpoint(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld) {
    float32x4_t position = rWorld._m.val[3];
    float32x4_t up = TransformDirection(rWorld._m.val[1], rView);
    float32x4_t facing = vmulq_f32(TransformPosition(position, rView), float32x4_t{-1, -1, -1, 0});
    facing = NormalizeOr(facing, Vector(0, 0, 1));
    FaceViewpoint(pOutput, up, facing);
}
/**
 * @brief Face the screen by rotating about the transformed local Y axis.
 * @param pOutput Destination matrix whose translation is retained.
 * @param rView World-to-view transform.
 * @param rWorld World transform supplying the local Y axis.
 */
void Billboard::CalculateYAxis(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld) {
    float32x4_t originalUp = TransformDirection(rWorld._m.val[1], rView);
    float32x2_t length = LengthSquared(originalUp);
    float32x4_t up = Normalize(originalUp, vcombine_f32(length, length));
    float32x4_t right = Vector(originalUp[1], -originalUp[0], 0);
    bool valid = vget_lane_f32(length, 0) > 0;
    up = valid ? up : Vector(0, 1, 0);
    right = valid ? right : Vector(1, 0, 0);
    length = LengthSquared(right);
    float32x4_t facing;
    if (vget_lane_f32(length, 0) > 0) {
        right = Normalize(right, vcombine_f32(length, length));
        facing = Cross(right, up);
    } else {
        right = Vector(1, 0, 0);
        facing = Vector(0, -up[2], 0);
    }
    pOutput->_m.val[0] = right;
    pOutput->_m.val[1] = up;
    pOutput->_m.val[2] = facing;
}
/**
 * @brief Face the camera position by rotating about the transformed local Y axis.
 * @param pOutput Destination matrix whose translation is retained.
 * @param rView World-to-view transform.
 * @param rWorld World transform supplying the local Y axis and position.
 */
void Billboard::CalculateYAxisViewpoint(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld) {
    float32x4_t upAxis = rWorld._m.val[1];
    float32x4_t position = rWorld._m.val[3];
    float32x4_t up = TransformDirection(upAxis, rView);
    float32x4_t facing = vmulq_f32(TransformPosition(position, rView), float32x4_t{-1, -1, -1, 0});
    up = NormalizeOr(up, Vector(0, 1, 0));
    float32x4_t right = Cross(up, facing);
    float32x2_t length = LengthSquared(right);
    if (vget_lane_f32(length, 0) > 0) {
        right = Normalize(right, vcombine_f32(length, length));
        facing = Cross(right, up);
    } else {
        right = Vector(up[1], -up[0], 0);
        length = LengthSquared(right);
        if (vget_lane_f32(length, 0) > 0) {
            right = Normalize(right, vcombine_f32(length, length));
            facing = Cross(right, up);
        } else {
            right = Vector(1, 0, 0);
            facing = Vector(0, -up[2], 0);
        }
    }
    pOutput->_m.val[0] = right;
    pOutput->_m.val[1] = up;
    pOutput->_m.val[2] = facing;
}
/**
 * @brief Face the screen using the combined transform's projected up axis.
 * @param pOutput Destination matrix with an existing translation.
 * @param rWorldView Combined transform supplying the projected up direction.
 */
void Billboard::CalculateScreen(Matrix* pOutput, const Matrix& rWorldView) {
    FaceScreen(pOutput, rWorldView._m.val[1]);
}
/**
 * @brief Face the camera position using a screen-projected preferred up axis.
 * @param pOutput Destination matrix whose translation is retained.
 * @param rView World-to-view transform.
 * @param rWorld World transform supplying the position.
 * @param rWorldView Combined transform whose up axis is projected onto the screen.
 */
void Billboard::CalculateScreenViewpoint(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld,
                                         const Matrix& rWorldView) {
    float32x4_t facing = vmulq_f32(TransformPosition(rWorld._m.val[3], rView), float32x4_t{-1, -1, -1, 0});
    facing = NormalizeOr(facing, Vector(0, 0, 1));
    float32x4_t up = vcombine_f32(vget_low_f32(rWorldView._m.val[1]), vdup_n_f32(0));
    FaceViewpoint(pOutput, up, facing);
}
/**
 * @brief Calculate the selected billboard basis and restore the world's axis scales.
 * @param mode Resource billboard mode, from 0x20000 through 0x70000 in 0x10000 steps.
 * @param pOutput Destination with its translation already initialized.
 * @param rView World-to-view transform.
 * @param rWorld World transform supplying position, up direction and axis scales.
 * @param rWorldView Combined world/view transform for screen-oriented modes.
 */
void Billboard::Calculate(u32 mode, Matrix* pOutput, const Matrix& rView, const Matrix& rWorld,
                          const Matrix& rWorldView) {
    static CalculateFunction functions[] = {CalculateWorld,  CalculateWorldViewpoint,
                                            CalculateScreen, CalculateScreenViewpoint,
                                            CalculateYAxis,  CalculateYAxisViewpoint};
    float32x2_t length = LengthSquared(rWorld._m.val[0]);
    float32x4_t xScale = vsqrtq_f32(vcombine_f32(length, length));
    length = LengthSquared(rWorld._m.val[1]);
    float32x4_t yScale = vsqrtq_f32(vcombine_f32(length, length));
    length = LengthSquared(rWorld._m.val[2]);
    float32x4_t zScale = vsqrtq_f32(vcombine_f32(length, length));
    functions[(mode - 0x20000) >> 16](pOutput, rView, rWorld, rWorldView);
    pOutput->_m.val[0] = vmulq_laneq_f32(pOutput->_m.val[0], xScale, 0);
    pOutput->_m.val[1] = vmulq_laneq_f32(pOutput->_m.val[1], yScale, 0);
    pOutput->_m.val[2] = vmulq_laneq_f32(pOutput->_m.val[2], zScale, 0);
}
} // namespace nn::g3d
