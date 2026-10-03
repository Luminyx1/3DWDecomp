#include <nn/g3d/g3d_ShapeObj.h>
#include <type_traits>
#include <cstring>
#include <nn/util/util_MatrixApi.h>

namespace nn::g3d {
namespace {
/**
 * @brief Locate a typed vertex attribute within a byte-addressed buffer.
 * @tparam T Attribute component or vector type.
 * @param pBuffer Buffer containing the requested attribute.
 * @param offset Byte offset aligned for T and within the buffer.
 * @return Pointer to the selected attribute.
 */
template <class T> inline T* VertexAttribute(void* pBuffer, ptrdiff_t offset) {
    return reinterpret_cast<T*>(static_cast<char*>(pBuffer) + offset);
}
/**
 * @brief Apply a blend weight to a scalar or vector vertex attribute.
 * @tparam T Source scalar or Float3/Float4 vector type.
 * @param value Source component or vector value.
 * @param weight Blend weight multiplied into the source component.
 * @return Weighted value, truncated to a signed integer for integer formats.
 */
template <class T> inline auto WeightedComponent(T value, float weight) {
    if constexpr (std::is_same<T, util::Float3>::value) {
        return util::Float3{{value.x * weight, value.y * weight, value.z * weight}};
    } else if constexpr (std::is_same<T, util::Float4>::value) {
        return util::Float4{{value.x * weight, value.y * weight, value.z * weight, value.w * weight}};
    } else if constexpr (std::is_integral<T>::value) {
        return static_cast<int>(value * weight);
    } else {
        return value * weight;
    }
}
/**
 * @brief Blend one packed vector attribute per vertex.
 * @tparam T Float3 or Float4 packed vector representation.
 * @tparam add Whether to accumulate weighted vectors instead of replacing the destination.
 * @param pDestination Writable vertex buffer covering every output vector.
 * @param destinationOffset Byte offset of the first destination vector, aligned for T.
 * @param destinationStride Byte distance between destination vectors.
 * @param pSource Vertex buffer containing all source vectors.
 * @param sourceOffset Byte offset of the first source vector, aligned for T.
 * @param sourceStride Byte distance between source vectors.
 * @param weight Weight applied to each source vector.
 * @param vertexCount Number of vectors to process; zero does no work.
 */
template <class T, bool add>
inline void BlendVectors(void* pDestination, ptrdiff_t destinationOffset, ptrdiff_t destinationStride,
                         void* pSource, ptrdiff_t sourceOffset, ptrdiff_t sourceStride, float weight,
                         unsigned vertexCount) {
    char* pSourceVertex = VertexAttribute<char>(pSource, sourceOffset);
    char* pDestinationVertex = VertexAttribute<char>(pDestination, destinationOffset);
    for (unsigned vertex = 0; vertex < vertexCount; ++vertex) {
        T* pOutput = VertexAttribute<T>(pDestinationVertex, 0);
        const T* pInput = VertexAttribute<T>(pSourceVertex, 0);
        T weighted = WeightedComponent(*pInput, weight);
        if constexpr (add) {
            T original = *pOutput;
            weighted.x += original.x;
            weighted.y += original.y;
            weighted.z += original.z;
            if constexpr (std::is_same<T, util::Float4>::value) {
                weighted.w += original.w;
            }
        }
        *pOutput = weighted;
        pDestinationVertex += destinationStride;
        pSourceVertex += sourceStride;
    }
}
} // namespace

struct ShapeObj::Impl {
    /**
     * @brief Implement replacement or accumulation for one vertex attribute representation.
     * @tparam T Scalar component representation or Float3/Float4 vector type.
     * @tparam add Whether to accumulate into the destination instead of replacing it.
     */
    template <class T, bool add> struct BlendShapeImpl {
        /**
         * @brief Blend strided vertex attributes into a destination buffer.
         * @param pDestination Writable vertex buffer covering all addressed components.
         * @param destinationOffset Byte offset of the first destination attribute, aligned for T.
         * @param destinationStride Byte distance between consecutive destination vertices.
         * @param pSource Source vertex buffer covering all addressed components.
         * @param sourceOffset Byte offset of the first source attribute, aligned for T.
         * @param sourceStride Byte distance between consecutive source vertices.
         * @param weight Weight applied to each source component before accumulation or assignment.
         * @param componentCount Scalar components per vertex; nonpositive values do no work. Ignored for
         * vectors.
         * @param vertexCount Number of vertices to process; zero does no work.
         */
        static void Execute(void* pDestination, ptrdiff_t destinationOffset, ptrdiff_t destinationStride,
                            void* pSource, ptrdiff_t sourceOffset, ptrdiff_t sourceStride, float weight,
                            int componentCount, unsigned vertexCount) {
            if constexpr (std::is_same<T, util::Float3>::value || std::is_same<T, util::Float4>::value) {
                BlendVectors<T, add>(pDestination, destinationOffset, destinationStride, pSource,
                                     sourceOffset, sourceStride, weight, vertexCount);
            } else {
                char* pDestinationVertex = VertexAttribute<char>(pDestination, destinationOffset);
                char* pSourceVertex = VertexAttribute<char>(pSource, sourceOffset);
                for (unsigned vertex = 0; vertex < vertexCount; ++vertex) {
                    T* pOutput = VertexAttribute<T>(pDestinationVertex, 0);
                    const T* pInput = VertexAttribute<T>(pSourceVertex, 0);
                    for (int component = 0; component < componentCount; ++component, ++pOutput, ++pInput) {
                        if constexpr (add) {
                            *pOutput += WeightedComponent(*pInput, weight);
                        } else {
                            *pOutput = WeightedComponent(*pInput, weight);
                        }
                    }
                    pDestinationVertex += destinationStride;
                    pSourceVertex += sourceStride;
                }
            }
        }
    };
};

template struct ShapeObj::Impl::BlendShapeImpl<unsigned char, false>;
template struct ShapeObj::Impl::BlendShapeImpl<unsigned char, true>;
template struct ShapeObj::Impl::BlendShapeImpl<signed char, false>;
template struct ShapeObj::Impl::BlendShapeImpl<signed char, true>;
template struct ShapeObj::Impl::BlendShapeImpl<unsigned short, false>;
template struct ShapeObj::Impl::BlendShapeImpl<unsigned short, true>;
template struct ShapeObj::Impl::BlendShapeImpl<short, false>;
template struct ShapeObj::Impl::BlendShapeImpl<short, true>;
template struct ShapeObj::Impl::BlendShapeImpl<float, false>;
template struct ShapeObj::Impl::BlendShapeImpl<float, true>;

template struct ShapeObj::Impl::BlendShapeImpl<util::Float3, false>;
template struct ShapeObj::Impl::BlendShapeImpl<util::Float3, true>;
template struct ShapeObj::Impl::BlendShapeImpl<util::Float4, false>;
template struct ShapeObj::Impl::BlendShapeImpl<util::Float4, true>;

/** @brief Reset all blend weights and clear the flags recording modified weights. */
void ShapeObj::ClearBlendWeights() {
    int count = m_pRes->ToData().keyShapeCount;
    for (int i = 0; i < count; ++i) {
        m_pBlendWeights[i] = 0.0f;
    }
    std::memset(m_pBlendWeightFlags, 0, ((count + 31) / 32) * sizeof(u32));
    m_Flag &= ~2;
}
/**
 * @brief Query the graphics backend's alignment for a shape uniform block.
 * @param pDevice Initialized graphics device providing buffer requirements.
 * @return Required alignment in bytes for the 256-byte shape block.
 */
size_t ShapeObj::GetBlockBufferAlignment(gfx::Device* pDevice) const {
    gfx::BufferInfo info;
    std::memset(&info, 0, sizeof(info));
    info.SetDefault();
    info.SetSize(256);
    info.SetGpuAccessFlags(0x10);
    return BufferImpl::GetBufferAlignment(pDevice, info);
}
/**
 * @brief Calculate aligned storage for every view and buffered shape uniform block.
 * @param pDevice Initialized graphics device providing buffer alignment.
 * @return Total shape-block storage size in bytes, excluding dynamic vertex buffers.
 */
size_t ShapeObj::CalculateShapeBlockBufferSize(gfx::Device* pDevice) const {
    size_t alignment = GetBlockBufferAlignment(pDevice);
    return ((256 + alignment - 1) & -alignment) * m_ShapeBlockCount * m_BufferingCount;
}
/**
 * @brief Access one buffered dynamic vertex buffer.
 * @param vertexBufferIndex Vertex-buffer index within the shape's vertex resource.
 * @param bufferIndex Buffering index below the initialized buffering count.
 * @return Selected buffer, or nullptr when the vertex buffer is not dynamic.
 */
const gfx::Buffer* ShapeObj::GetDynamicVertexBuffer(int vertexBufferIndex, int bufferIndex) const {
    if (m_ppDynamicVertexBuffers == nullptr) {
        return nullptr;
    }
    gfx::Buffer* pBuffers = m_ppDynamicVertexBuffers[vertexBufferIndex];
    if (pBuffers == nullptr) {
        return nullptr;
    }
    return &pBuffers[bufferIndex];
}
/**
 * @brief Check whether a vertex attribute uses a dynamic vertex buffer.
 * @param attributeIndex Attribute index within the shape's vertex resource.
 * @return True if the attribute's buffer has dynamic storage assigned.
 */
bool ShapeObj::IsDynamicVertexAttr(int attributeIndex) const {
    if (m_ppDynamicVertexBuffers == nullptr) {
        return false;
    }
    const ResVertex* pVertex = m_pRes->ToData().pVertex.Get();
    int bufferIndex = pVertex->ToData().pAttribArray.Get()[attributeIndex].bufferIndex;
    return m_ppDynamicVertexBuffers[bufferIndex] != nullptr;
}
/**
 * @brief Upload a shape transform, skinning count and user data to a buffered uniform block.
 * @param viewIndex View index below the initialized view count; ignored for view-independent shapes.
 * @param rWorld Transform to pack into the shape block.
 * @param bufferIndex Buffering index below the initialized buffering count; buffers must be set up.
 */
void ShapeObj::CalculateShape(int viewIndex, const util::Matrix4x3fType& rWorld, int bufferIndex) {
    if (m_ShapeBlockCount == 0) {
        return;
    }
    struct ShapeBlock {
        util::FloatColumnMajor4x3 world;
        u32 vertexSkinCount;
        u32 reserved[3];
        u32 userArea[48];
    };
    BufferImpl* pBuffer = GetShapeBlock(viewIndex, bufferIndex);
    auto* pBlock = static_cast<ShapeBlock*>(pBuffer->Map());
    int vertexSkinCount = m_pRes->GetVertexSkinCount();
    util::MatrixStore(&pBlock->world, rWorld);
    pBlock->vertexSkinCount = vertexSkinCount;
    int wordCount = m_UserAreaSize / sizeof(u32);
    std::memcpy(pBlock->userArea, m_pUserArea, wordCount * sizeof(u32));
    pBuffer->FlushMappedRange(0, sizeof(ShapeBlock));
    pBuffer->Unmap();
}

/**
 * @brief Calculate aligned storage for all buffered dynamic vertex streams.
 * @param pDevice Initialized graphics device providing buffer alignment requirements.
 * @return Required storage bytes, or zero when no dynamic vertex buffers are assigned.
 */
size_t ShapeObj::CalculateDynamicVertexBufferSize(gfx::Device* pDevice) const {
    size_t total = 0;
    if (m_ppDynamicVertexBuffers != nullptr) {
        const ResVertex* pVertex = m_pRes->ToData().pVertex.Get();
        gfx::BufferInfo info;
        std::memset(&info, 0, sizeof(info));
        int count = pVertex->ToData().bufferCount;
        for (int i = 0; i < count; ++i) {
            if (m_ppDynamicVertexBuffers[i] == nullptr) {
                continue;
            }
            info = *pVertex->GetBufferInfo(i);
            info.SetGpuAccessFlags(info.GetGpuAccessFlags() | 0x40);
            size_t size = info.GetSize();
            size_t alignment = BufferImpl::GetBufferAlignment(pDevice, info);
            total += ((size + alignment - 1) & -alignment) * m_BufferingCount;
        }
    }
    return total;
}
/**
 * @brief Calculate combined storage for shape uniform blocks and dynamic vertex buffers.
 * @param pDevice Initialized graphics device providing buffer alignment requirements.
 * @return Total GPU memory-pool storage requirement in bytes.
 */
size_t ShapeObj::CalculateBlockBufferSize(gfx::Device* pDevice) const {
    size_t shapeSize = CalculateShapeBlockBufferSize(pDevice);
    return shapeSize + CalculateDynamicVertexBufferSize(pDevice);
}

} // namespace nn::g3d
