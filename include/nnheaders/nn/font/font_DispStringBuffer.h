#pragma once

#include <attributes.h>
#include <cstring>
#include <nn/font/font_GpuBuffer.h>
#include <nn/gfx/gfx_Device.h>
#include <nn/gfx/gfx_GpuAddress.h>
#include <nn/types.h>
#include <nn/util/util_BytePtr.h>
#include <nn/util/util_MathTypes.h>

namespace nn {
namespace font {

class TextureObject;

struct PerCharacterTransformInfo {
    float scale[2];
    float rotationCos[3];
    float rotationSin[3];
    float translation[3];
    nn::util::Unorm8x4 lt;
    nn::util::Unorm8x4 lb;
};

struct ShadowParameter {
    nn::util::Unorm8x4 topColor;
    nn::util::Unorm8x4 bottomColor;
    nn::util::Float2 offset;
    nn::util::Float2 scale;
    float italicRatio;
};

struct ConstantBufferAdditionalContent {
    enum PerCharacterTransformCenter {
        PerCharacterTransformCenter_Center,
        PerCharacterTransformCenter_Bottom,
    };

    ConstantBufferAdditionalContent()
        : m_pShadowParam(nullptr), m_pPerCharacterTransformInfos(nullptr),
          m_PerCharacterTransformCenter(PerCharacterTransformCenter_Center),
          m_PerCharacterTransformCenterOffset(0.0f), m_pViewMatrix(nullptr),
          m_pLocalMatrix(nullptr), m_InterpolateAlpha(255), m_ShadowInterpolateAlpha(255),
          m_ShaderVariationFlags(0) {
        m_InterpolateBlack.x = 0.0f;
        m_InterpolateBlack.y = 0.0f;
        m_InterpolateBlack.z = 0.0f;
        m_InterpolateBlack.w = 0.0f;
        m_InterpolateWhite.x = 1.0f;
        m_ShadowInterpolateBlack.x = 0.0f;
        m_ShadowInterpolateWhite.x = 1.0f;
        m_InterpolateWhite.y = 1.0f;
        m_ShadowInterpolateBlack.y = 0.0f;
        m_ShadowInterpolateWhite.y = 1.0f;
        m_InterpolateWhite.z = 1.0f;
        m_ShadowInterpolateBlack.z = 0.0f;
        m_ShadowInterpolateWhite.z = 1.0f;
        m_InterpolateWhite.w = 1.0f;
        m_ShadowInterpolateBlack.w = 0.0f;
        m_ShadowInterpolateWhite.w = 1.0f;
    }

    const ShadowParameter* m_pShadowParam;
    const PerCharacterTransformInfo* m_pPerCharacterTransformInfos;
    PerCharacterTransformCenter m_PerCharacterTransformCenter;
    float m_PerCharacterTransformCenterOffset;
    const nn::util::MatrixT4x3fType* m_pViewMatrix;
    const nn::util::MatrixT4x3fType* m_pLocalMatrix;
    nn::util::Float4 m_InterpolateBlack;
    nn::util::Float4 m_InterpolateWhite;
    uint8_t m_InterpolateAlpha;
    nn::util::Float4 m_ShadowInterpolateBlack;
    nn::util::Float4 m_ShadowInterpolateWhite;
    uint8_t m_ShadowInterpolateAlpha;
    uint32_t m_ShaderVariationFlags;
};

namespace detail {

struct CharAttribute {
    nn::util::Float4 pos;
    nn::util::Unorm8x4 color[2];
    nn::util::Float4 tex;
    uintptr_t pTexObjAndFlag;
    int16_t italicOffset;
    uint8_t sheetIndex;
    uint8_t shadowAlpha;

    const TextureObject* GetTexObj() const {
        return reinterpret_cast<const TextureObject*>(pTexObjAndFlag & ~static_cast<uintptr_t>(1));
    }
    bool IsBorderEffectEnabled() const { return (pTexObjAndFlag & 1) != 0; }
};

// The original binary has out-of-line copies of these only in font_CharWriter.cpp, which defines
// NN_FONT_DETAIL_EMIT_OUT_OF_LINE before including this header; every other user inlines them.
#ifdef NN_FONT_DETAIL_EMIT_OUT_OF_LINE
#define NN_FONT_DETAIL_EMIT USED
#else
#define NN_FONT_DETAIL_EMIT
#endif

struct VertexShaderCharAttribute {
    nn::util::Float4 posAndSize;
    nn::util::Float4 texCoord;
    nn::util::Unorm8x4 color[2];
    int sheetIndex;
    float italicOffset;
    float translate[3];
    float fontHeight;

    NN_FONT_DETAIL_EMIT void Set(float x, float y, float width, float height,
                                 const nn::util::Float4& rTexCoord,
                                 const nn::util::Unorm8x4& rColorTop,
                                 const nn::util::Unorm8x4& rColorBottom, int sheetIndex_,
                                 float italicOffset_, float fontHeight_) {
        posAndSize.x = width;
        posAndSize.y = height;
        posAndSize.z = x;
        posAndSize.w = y;
        texCoord = rTexCoord;
        color[0] = rColorTop;
        color[1] = rColorBottom;
        sheetIndex = sheetIndex_;
        italicOffset = italicOffset_;
        fontHeight = fontHeight_;
    }
};

struct VertexShaderCharAttributeWithTransform : VertexShaderCharAttribute {
    nn::util::Float4 rotateMatrixAndCenterX;
    nn::util::Float4 rotateMatrixAndCenterY;

    NN_FONT_DETAIL_EMIT void Set(float x, float y, float width, float height,
                                 const nn::util::Float4& rTexCoord,
                                 const nn::util::Unorm8x4& rColorTop,
                                 const nn::util::Unorm8x4& rColorBottom, int sheetIndex_,
                                 float italicOffset_, float fontHeight_,
                                 const nn::util::Float4& rRotateMatrixAndCenterX,
                                 const nn::util::Float4& rRotateMatrixAndCenterY,
                                 const nn::util::Float4& rTranslate) {
        posAndSize.x = width;
        posAndSize.y = height;
        posAndSize.z = x;
        posAndSize.w = y;
        texCoord = rTexCoord;
        color[0] = rColorTop;
        color[1] = rColorBottom;
        sheetIndex = sheetIndex_;
        italicOffset = italicOffset_;
        rotateMatrixAndCenterX = rRotateMatrixAndCenterX;
        rotateMatrixAndCenterY = rRotateMatrixAndCenterY;
        std::memcpy(translate, &rTranslate, sizeof(translate));
        fontHeight = fontHeight_;
    }
};

NN_FONT_DETAIL_EMIT inline void* CalculateVertexShaderCharAttributePtr(
    nn::util::BytePtr base, ptrdiff_t offset, uint32_t index,
    bool isPerCharacterTransformEnabled) {
    const size_t stride = isPerCharacterTransformEnabled ?
                              sizeof(VertexShaderCharAttributeWithTransform) :
                              sizeof(VertexShaderCharAttribute);
    return base.Advance(offset + stride * index).Get();
}

#undef NN_FONT_DETAIL_EMIT

}  // namespace detail

class DispStringBuffer {
public:
    static const int TextureUseInfoCountMax = 8;

    enum ShaderVariationFlag {
        ShaderVariationFlag_BorderPass = 1 << 0,
        ShaderVariationFlag_Shadow = 1 << 1,
        ShaderVariationFlag_PerCharacterTransform = 1 << 2,
    };

    struct InitializeArg {
        InitializeArg()
            : pDrawBuffer(nullptr), pConstantBuffer(nullptr), charCountMax(0),
              isShadowEnabled(false), isDoubleDrawnBorder(false),
              isPerCharacterTransformEnabled(false),
              isPerCharacterTransformAutoShadowAlpha(false) {}

        void* pDrawBuffer;
        GpuBuffer* pConstantBuffer;
        int charCountMax;
        bool isShadowEnabled;
        bool isDoubleDrawnBorder;
        bool isPerCharacterTransformEnabled;
        bool isPerCharacterTransformAutoShadowAlpha;
    };

    struct ShaderParam {
        nn::util::MatrixT4x4fType mtx;
        nn::util::Float4 interpolateWidth;
        nn::util::Float4 interpolateOffset;
        nn::util::Float4 shadowInterpolateWidth;
        nn::util::Float4 shadowInterpolateOffset;
    };

    struct TextureUseInfo {
        const TextureObject* pTexObj;
        uint32_t useCount;
        uint32_t flags;
    };

    struct VertexBufferData {
        TextureUseInfo textureUseInfos[TextureUseInfoCountMax];
        uint32_t textureUseInfoCount;
    };

    DispStringBuffer();
    ~DispStringBuffer();

    bool Initialize(nn::gfx::Device* pDevice, const InitializeArg& rArg);
    void Finalize(nn::gfx::Device* pDevice);

    static size_t GetRequiredDrawBufferSize(const InitializeArg& rArg);
    static size_t GetRequiredConstantBufferSize(nn::gfx::Device* pDevice,
                                                const InitializeArg& rArg);

    void SetConstantBuffer(GpuBuffer* pConstantBuffer);
    void GetConstantBufferGpuAddress(nn::gfx::GpuAddress* pGpuAddress) const;

    void BuildConstantBuffer(const nn::util::MatrixT4x4fType& rProjection,
                             const ConstantBufferAdditionalContent* pContent,
                             bool isDrawFromRightToLeftEnabled, bool isOriginToCenterEnabled);
    void BuildCommonConstantBufferData(ShaderParam& rShaderParam,
                                       const nn::util::MatrixT4x4fType& rProjection,
                                       const ConstantBufferAdditionalContent& rContent) const;
    void BuildPerCharacterAttributeConstantBuffer(const ConstantBufferAdditionalContent& rContent,
                                                  bool isDrawFromRightToLeftEnabled,
                                                  bool isOriginToCenterEnabled);
    void BuildTextureUseInfos(bool isDrawFromRightToLeftEnabled);
    void BuildPerCharacterParams(
        ptrdiff_t offset, const PerCharacterTransformInfo* pPerCharacterTransformInfos,
        ConstantBufferAdditionalContent::PerCharacterTransformCenter center, float centerOffset,
        const ShadowParameter* pShadowParam, bool isDrawFromRightToLeftEnabled,
        bool isOriginToCenterEnabled);
    void BuildShadowBufferPerCharacterParams(
        ptrdiff_t offset, const PerCharacterTransformInfo* pPerCharacterTransformInfos,
        ConstantBufferAdditionalContent::PerCharacterTransformCenter center, float centerOffset,
        const ShadowParameter* pShadowParam, const uint32_t* pStartIndices,
        nn::util::BytePtr base, uint32_t bufferCount, uint32_t* pCounts, int index,
        bool isOriginToCenterEnabled);
    void BuildCharacterBufferPerCharacterParams(
        ptrdiff_t offset, const PerCharacterTransformInfo* pPerCharacterTransformInfos,
        ConstantBufferAdditionalContent::PerCharacterTransformCenter center, float centerOffset,
        const uint32_t* pStartIndices, nn::util::BytePtr base, uint32_t bufferCount,
        uint32_t* pCounts, int index, bool isOriginToCenterEnabled);
    void CalculatePerCharacterTransform(
        nn::util::Float4& rRotateMatrixAndCenterX, nn::util::Float4& rRotateMatrixAndCenterY,
        nn::util::Float4& rTranslate, float x, float y, float width, float height,
        float italicOffset, float fontHeight, const PerCharacterTransformInfo& rInfo,
        ConstantBufferAdditionalContent::PerCharacterTransformCenter center, float centerOffset,
        bool isOriginToCenterEnabled, float shadowOffsetY) const;
    void SetFontHeight(float fontHeight);
    bool CompareCopiedInstanceTest(const DispStringBuffer& rOther) const;

    uint32_t m_ConstantBufferOffset;
    uint32_t m_PerCharacterParamOffset;
    uint32_t m_ShaderVariationFlags;
    VertexBufferData m_VertexBufferData;
    int m_CharCountMax;
    int m_CharCount;
    detail::CharAttribute* m_pCharAttrs;
    uint8_t* m_pTextureUseInfoIndices;
    GpuBuffer* m_pConstantBuffer;
    bool m_IsShadowEnabled;
    bool m_IsDoubleDrawnBorder;
    bool m_IsPerCharacterTransformEnabled;
    bool m_IsPerCharacterTransformAutoShadowAlpha;
    float m_FontHeight;
};

}  // namespace font
}  // namespace nn
