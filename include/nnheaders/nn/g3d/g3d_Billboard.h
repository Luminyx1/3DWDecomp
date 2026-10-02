#pragma once
#include <nn/util/util_MathTypes.h>
#include <attributes.h>

namespace nn::g3d {
class Billboard {
  public:
    using Matrix = nn::util::Matrix4x3fType;
    using CalculateFunction = void (*)(Matrix*, const Matrix&, const Matrix&, const Matrix&);
    static void CalculateWorld(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld);
    static NOINLINE void CalculateWorldViewpoint(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld);
    static void CalculateYAxis(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld);
    static NOINLINE void CalculateYAxisViewpoint(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld);
    static void CalculateScreen(Matrix* pOutput, const Matrix& rWorldView);
    static void CalculateScreenViewpoint(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld,
                                         const Matrix& rWorldView);
    static void Calculate(u32 mode, Matrix* pOutput, const Matrix& rView, const Matrix& rWorld,
                          const Matrix& rWorldView);
    /**
     * @brief Adapt the World billboard calculation to the common dispatch signature.
     * @param pOutput Destination matrix; existing translation is retained.
     * @param rView World-to-view transform.
     * @param rWorld World transform supplying the local up axis and position.
     * @param rWorldView Unused for this billboard mode.
     */
    static void CalculateWorld(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld,
                               const Matrix& rWorldView) {
        CalculateWorld(pOutput, rView, rWorld);
    }
    /**
     * @brief Adapt the WorldViewpoint billboard calculation to the common dispatch signature.
     * @param pOutput Destination matrix; existing translation is retained.
     * @param rView World-to-view transform.
     * @param rWorld World transform supplying the local up axis and position.
     * @param rWorldView Unused for this billboard mode.
     */
    static void CalculateWorldViewpoint(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld,
                                        const Matrix& rWorldView) {
        CalculateWorldViewpoint(pOutput, rView, rWorld);
    }
    /**
     * @brief Adapt the Screen billboard calculation to the common dispatch signature.
     * @param pOutput Destination matrix; existing translation is retained.
     * @param rView Unused for screen-aligned billboards.
     * @param rWorld Unused for screen-aligned billboards.
     * @param rWorldView Combined world/view transform supplying the projected up direction.
     */
    static void CalculateScreen(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld,
                                const Matrix& rWorldView) {
        CalculateScreen(pOutput, rWorldView);
    }
    /**
     * @brief Adapt the YAxis billboard calculation to the common dispatch signature.
     * @param pOutput Destination matrix; existing translation is retained.
     * @param rView World-to-view transform.
     * @param rWorld World transform supplying the local up axis and position.
     * @param rWorldView Unused for this billboard mode.
     */
    static void CalculateYAxis(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld,
                               const Matrix& rWorldView) {
        CalculateYAxis(pOutput, rView, rWorld);
    }
    /**
     * @brief Adapt the YAxisViewpoint billboard calculation to the common dispatch signature.
     * @param pOutput Destination matrix; existing translation is retained.
     * @param rView World-to-view transform.
     * @param rWorld World transform supplying the local up axis and position.
     * @param rWorldView Unused for this billboard mode.
     */
    static void CalculateYAxisViewpoint(Matrix* pOutput, const Matrix& rView, const Matrix& rWorld,
                                        const Matrix& rWorldView) {
        CalculateYAxisViewpoint(pOutput, rView, rWorld);
    }
};
} // namespace nn::g3d
