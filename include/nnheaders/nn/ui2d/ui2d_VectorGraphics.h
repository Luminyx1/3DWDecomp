#pragma once

#include <cstdio>
#include "nn/ui2d/ui2d_DrawInfo.h"
#include "nn/ui2d/ui2d_Resources.h"
#include "nn/ui2d/ui2d_Types.h"
#include "nn/util/util_VectorApi.h"

namespace nn {
namespace ui2d {
namespace detail {

struct StrokeSegmentVertexInfo {
    nn::util::Float2 start;
    nn::util::Float2* pCurvePoints;
    uint32_t count;
    bool ignore;
};

class ReservedVectorGraphicsSceneMemory {
public:
    ReservedVectorGraphicsSceneMemory();
    ~ReservedVectorGraphicsSceneMemory();

    void Initialize(size_t);
    void Finalize();
    size_t GetReservedSize() const;
    size_t GetAllocatedSize() const;
    void* Allocate(size_t);
    void* Allocate(size_t, size_t);
};

class VectorGraphicsShapePathProcessor {
public:
    enum TrimDirection { TrimDirection_Right, TrimDirection_Left, TrimDirection_Max };

    virtual ~VectorGraphicsShapePathProcessor() {}
    virtual void Initialize() = 0;
    virtual void Finalize() = 0;
    virtual BnvgShapePathType GetType() const = 0;
    virtual void EvaluateParams(DrawInfo& drawInfo, float time) = 0;
    virtual const nn::util::Float2 GetPosition() const { return nn::util::MakeFloat2(0.0f, 0.0f); }
    virtual uint32_t GetControlPointCount() const = 0;
    virtual uint32_t CalculatePathDivideVertexCount(int) const = 0;
    virtual uint32_t CalculateVertexCount() const = 0;
    virtual bool IsPathClosed() const = 0;
    virtual bool IsPathTrimed() const { return m_TrimStart > 0.0f || m_TrimEnd < 1.0f; }
    virtual void SetTrimParams(float, float);
    virtual int GenerateAndWritePathVertex(nn::util::Float2**, int*, uint32_t**, int*,
                                           StrokeSegmentVertexInfo*) = 0;
    virtual TrimDirection TrimPathDirection() const { return TrimDirection_Right; }

    float m_TrimStart;
    float m_TrimEnd;
    float m_PathLength;
    bool m_ShapeAnimated;
    ReservedVectorGraphicsSceneMemory* m_pReservedMemory;
};
};  // namespace detail
};  // namespace ui2d
};  // namespace nn