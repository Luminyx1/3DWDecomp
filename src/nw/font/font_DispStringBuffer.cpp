#include <nn/font/font_DispStringBuffer.h>

#include <nn/font/font_Font.h>
#include <nn/gfx/gfx_Buffer.h>
#include <nn/gfx/gfx_BufferInfo.h>
#include <nn/util/util_BitUtil.h>
#include <nn/util/util_VectorApi.h>

namespace nn {
namespace font {

namespace {

const float ColorNormalizeScale = 1.0f / 255.0f;

void MultiplyMatrixT4x3(nn::util::MatrixT4x4fType* pOut, const nn::util::MatrixT4x3fType& rLhs,
                        const nn::util::MatrixT4x4fType& rRhs) {
    float32x4x4_t lhs;
    lhs.val[0] = rLhs._m.val[0];
    lhs.val[1] = rLhs._m.val[1];
    lhs.val[2] = rLhs._m.val[2];
    lhs.val[3] = vsetq_lane_f32(1.0f, vdupq_n_f32(0.0f), 3);
    float32x4x4_t rhs = rRhs._m;
    float32x4x4_t result;
    for (int i = 0; i < 4; i++) {
        result.val[i] = vmulq_laneq_f32(lhs.val[0], rhs.val[i], 0);
        result.val[i] = vfmaq_laneq_f32(result.val[i], lhs.val[1], rhs.val[i], 1);
        result.val[i] = vfmaq_laneq_f32(result.val[i], lhs.val[2], rhs.val[i], 2);
        result.val[i] = vfmaq_laneq_f32(result.val[i], lhs.val[3], rhs.val[i], 3);
    }

    pOut->_m = result;
}

uint8_t MultiplyAlpha(uint32_t a, uint32_t b) {
    return a * b / 255;
}

uint32_t LoadColor(const nn::util::Unorm8x4& rColor) {
    uint32_t value;
    std::memcpy(&value, &rColor, sizeof(value));
    return value;
}

nn::util::Unorm8x4 MultiplyColor(const nn::util::Unorm8x4& rLhs, const nn::util::Unorm8x4& rRhs) {
    const uint32_t value = MultiplyAlpha(rLhs.v[0], rRhs.v[0]) |
                           (MultiplyAlpha(rLhs.v[1], rRhs.v[1]) << 8) |
                           (MultiplyAlpha(rLhs.v[2], rRhs.v[2]) << 16) |
                           (MultiplyAlpha(rLhs.v[3], rRhs.v[3]) << 24);
    nn::util::Unorm8x4 color;
    std::memcpy(&color, &value, sizeof(value));
    return color;
}

nn::util::Unorm8x4 MakeColor(uint32_t rgb, uint8_t alpha) {
    const uint32_t value = (rgb & 0x00ffffff) | (static_cast<uint32_t>(alpha) << 24);
    nn::util::Unorm8x4 color;
    std::memcpy(&color, &value, sizeof(value));
    return color;
}

}  // namespace

/**
 * Constructs an uninitialized buffer.
 */
DispStringBuffer::DispStringBuffer()
    : m_ConstantBufferOffset(0), m_PerCharacterParamOffset(0), m_ShaderVariationFlags(0),
      m_VertexBufferData(), m_CharCountMax(0), m_CharCount(0), m_pCharAttrs(nullptr),
      m_pTextureUseInfoIndices(nullptr), m_pConstantBuffer(nullptr), m_IsShadowEnabled(false),
      m_IsDoubleDrawnBorder(false), m_IsPerCharacterTransformEnabled(false),
      m_IsPerCharacterTransformAutoShadowAlpha(false), m_FontHeight(0.0f) {}

/**
 * Destroys the buffer.
 */
DispStringBuffer::~DispStringBuffer() {}

/**
 * Initializes the buffer with its draw buffer.
 * @param pDevice gfx device
 * @param rArg initialization arguments
 * @return whether initialization succeeded
 */
bool DispStringBuffer::Initialize(nn::gfx::Device* pDevice, const InitializeArg& rArg) {
    if (m_CharCountMax > 0) {
        return false;
    }

    if (rArg.charCountMax < 1) {
        return false;
    }

    if (rArg.pDrawBuffer == nullptr) {
        return false;
    }

    nn::util::BytePtr ptr(rArg.pDrawBuffer);
    m_CharCountMax = rArg.charCountMax;
    m_CharCount = 0;
    m_pCharAttrs = ptr.Get<detail::CharAttribute>();
    m_pTextureUseInfoIndices =
        ptr.Advance(sizeof(detail::CharAttribute) * rArg.charCountMax).Get<uint8_t>();
    m_IsShadowEnabled = rArg.isShadowEnabled;
    m_IsDoubleDrawnBorder = rArg.isDoubleDrawnBorder;
    m_IsPerCharacterTransformEnabled = rArg.isPerCharacterTransformEnabled;
    m_IsPerCharacterTransformAutoShadowAlpha = rArg.isPerCharacterTransformAutoShadowAlpha;
    m_pConstantBuffer = rArg.pConstantBuffer;
    return true;
}

/**
 * Finalizes the buffer.
 * @param pDevice gfx device
 */
void DispStringBuffer::Finalize(nn::gfx::Device* pDevice) {
    if (m_CharCountMax > 0) {
        m_CharCountMax = 0;
    }
}

/**
 * Gets the draw buffer size.
 * @param rArg initialization arguments
 * @return required size
 */
size_t DispStringBuffer::GetRequiredDrawBufferSize(const InitializeArg& rArg) {
    return (sizeof(detail::CharAttribute) + sizeof(uint8_t)) * rArg.charCountMax;
}

/**
 * Gets the constant buffer size.
 * @param pDevice gfx device
 * @param rArg initialization arguments
 * @return required size
 */
size_t DispStringBuffer::GetRequiredConstantBufferSize(nn::gfx::Device* pDevice,
                                                       const InitializeArg& rArg) {
    nn::gfx::BufferInfo info;
    info.SetDefault();
    info.SetGpuAccessFlags(nn::gfx::GpuAccess_ConstantBuffer);

    size_t shaderParamSize =
        nn::util::align_up(sizeof(ShaderParam), nn::gfx::Buffer::GetBufferAlignment(pDevice, info))
        << rArg.isDoubleDrawnBorder;
    size_t perCharacterParamSize =
        rArg.isPerCharacterTransformEnabled ?
            sizeof(detail::VertexShaderCharAttributeWithTransform) :
            sizeof(detail::VertexShaderCharAttribute);
    size_t perCharacterSize =
        nn::util::align_up(perCharacterParamSize * rArg.charCountMax,
                           nn::gfx::Buffer::GetBufferAlignment(pDevice, info));

    return shaderParamSize + perCharacterSize + (rArg.isShadowEnabled ? perCharacterSize : 0);
}

/**
 * Sets the constant buffer.
 * @param pConstantBuffer constant buffer to use
 */
void DispStringBuffer::SetConstantBuffer(GpuBuffer* pConstantBuffer) {
    m_pConstantBuffer = pConstantBuffer;
}

/**
 * Gets the GPU address of the current constant buffer.
 * @param pGpuAddress destination GPU address
 */
void DispStringBuffer::GetConstantBufferGpuAddress(nn::gfx::GpuAddress* pGpuAddress) const {
    *pGpuAddress = m_pConstantBuffer->GetGpuAddress();
}

void DispStringBuffer::BuildConstantBuffer(const nn::util::MatrixT4x4fType& rProjection,
                                           const ConstantBufferAdditionalContent* pContent,
                                           bool isDrawFromRightToLeftEnabled,
                                           bool isOriginToCenterEnabled) {
    static const ConstantBufferAdditionalContent s_DefaultContent;

    if (m_pConstantBuffer == nullptr || m_CharCountMax < 1) {
        return;
    }

    size_t shaderParamSize =
        nn::util::align_up(sizeof(ShaderParam), m_pConstantBuffer->GetBufferAlignment());
    const ConstantBufferAdditionalContent& rContent =
        pContent != nullptr ? *pContent : s_DefaultContent;

    m_ConstantBufferOffset = m_pConstantBuffer->Allocate(shaderParamSize << m_IsDoubleDrawnBorder);

    void* pMapped = m_pConstantBuffer->GetMappedPointer();
    if (pMapped != nullptr) {
        nn::util::BytePtr ptr(pMapped, m_ConstantBufferOffset);
        if (m_IsDoubleDrawnBorder) {
            ConstantBufferAdditionalContent borderContent = rContent;
            std::memcpy(&borderContent.m_InterpolateWhite, &rContent.m_InterpolateBlack,
                        sizeof(nn::util::Float3));
            borderContent.m_InterpolateAlpha = 255;
            std::memcpy(&borderContent.m_ShadowInterpolateWhite,
                        &rContent.m_ShadowInterpolateBlack, sizeof(nn::util::Float3));
            BuildCommonConstantBufferData(*ptr.Get<ShaderParam>(), rProjection, borderContent);
            ptr = nn::util::BytePtr(m_pConstantBuffer->GetMappedPointer(),
                                    shaderParamSize + m_ConstantBufferOffset);
        }

        BuildCommonConstantBufferData(*ptr.Get<ShaderParam>(), rProjection, rContent);
    }

    BuildPerCharacterAttributeConstantBuffer(rContent, isDrawFromRightToLeftEnabled,
                                             isOriginToCenterEnabled);

    m_ShaderVariationFlags = rContent.m_ShaderVariationFlags;
    if (rContent.m_pShadowParam != nullptr) {
        m_ShaderVariationFlags |= ShaderVariationFlag_Shadow;
    }

    if (rContent.m_pPerCharacterTransformInfos != nullptr) {
        m_ShaderVariationFlags |= ShaderVariationFlag_PerCharacterTransform;
    }
}

void DispStringBuffer::BuildCommonConstantBufferData(
    ShaderParam& rShaderParam, const nn::util::MatrixT4x4fType& rProjection,
    const ConstantBufferAdditionalContent& rContent) const {
    if (m_CharCountMax < 1) {
        return;
    }

    nn::util::MatrixT4x4fType mtx;
    if (rContent.m_pViewMatrix != nullptr) {
        MultiplyMatrixT4x3(&mtx, *rContent.m_pViewMatrix, rProjection);
    } else {
        mtx = rProjection;
    }

    if (rContent.m_pLocalMatrix != nullptr) {
        MultiplyMatrixT4x3(&mtx, *rContent.m_pLocalMatrix, mtx);
    }

    rShaderParam.mtx = mtx;

    const nn::util::Float4 interpolateOffset =
        nn::util::MakeFloat4(rContent.m_InterpolateBlack.x, rContent.m_InterpolateBlack.y,
                             rContent.m_InterpolateBlack.z, 0.0f);
    const nn::util::Float4 shadowInterpolateOffset = nn::util::MakeFloat4(
        rContent.m_ShadowInterpolateBlack.x, rContent.m_ShadowInterpolateBlack.y,
        rContent.m_ShadowInterpolateBlack.z, 0.0f);
    const nn::util::Float4 interpolateWidth = nn::util::MakeFloat4(
        rContent.m_InterpolateWhite.x - rContent.m_InterpolateBlack.x,
        rContent.m_InterpolateWhite.y - rContent.m_InterpolateBlack.y,
        rContent.m_InterpolateWhite.z - rContent.m_InterpolateBlack.z,
        rContent.m_InterpolateWhite.w * (rContent.m_InterpolateAlpha * ColorNormalizeScale));
    const nn::util::Float4 shadowInterpolateWidth = nn::util::MakeFloat4(
        rContent.m_ShadowInterpolateWhite.x - rContent.m_ShadowInterpolateBlack.x,
        rContent.m_ShadowInterpolateWhite.y - rContent.m_ShadowInterpolateBlack.y,
        rContent.m_ShadowInterpolateWhite.z - rContent.m_ShadowInterpolateBlack.z,
        rContent.m_ShadowInterpolateWhite.w *
            (rContent.m_ShadowInterpolateAlpha * ColorNormalizeScale));

    rShaderParam.interpolateWidth = interpolateWidth;
    rShaderParam.interpolateOffset = interpolateOffset;
    rShaderParam.shadowInterpolateWidth = shadowInterpolateWidth;
    rShaderParam.shadowInterpolateOffset = shadowInterpolateOffset;
}

void DispStringBuffer::BuildPerCharacterAttributeConstantBuffer(
    const ConstantBufferAdditionalContent& rContent, bool isDrawFromRightToLeftEnabled,
    bool isOriginToCenterEnabled) {
    int charCount = m_CharCount;
    size_t stride = m_IsPerCharacterTransformEnabled ?
                       sizeof(detail::VertexShaderCharAttributeWithTransform) :
                       sizeof(detail::VertexShaderCharAttribute);
    size_t size = stride * charCount;
    if (rContent.m_pShadowParam != nullptr) {
        m_PerCharacterParamOffset = m_pConstantBuffer->Allocate(size * 2);
    } else {
        m_PerCharacterParamOffset = m_pConstantBuffer->Allocate(size);
    }

    const ShadowParameter* pShadowParam = rContent.m_pShadowParam;
    const PerCharacterTransformInfo* pInfos = rContent.m_pPerCharacterTransformInfos;
    ConstantBufferAdditionalContent::PerCharacterTransformCenter center =
        rContent.m_PerCharacterTransformCenter;
    float centerOffset = rContent.m_PerCharacterTransformCenterOffset;

    BuildTextureUseInfos(isDrawFromRightToLeftEnabled);
    BuildPerCharacterParams(m_PerCharacterParamOffset, pInfos, center, centerOffset, pShadowParam,
                            isDrawFromRightToLeftEnabled, isOriginToCenterEnabled);
}

/**
 * Groups the characters by texture.
 * @param isDrawFromRightToLeftEnabled whether characters are drawn right to left
 */
void DispStringBuffer::BuildTextureUseInfos(bool isDrawFromRightToLeftEnabled) {
    uint32_t count = 0;
    uint32_t charCount = m_CharCount;
    for (uint32_t i = 0; i < charCount; i++) {
        const TextureObject* pTexObj = m_pCharAttrs[i].GetTexObj();
        uint8_t index = 0;
        for (; index < count; index++) {
            if (m_VertexBufferData.textureUseInfos[index].pTexObj == pTexObj) {
                break;
            }
        }

        if (index < count) {
            m_VertexBufferData.textureUseInfos[index].useCount++;
        } else {
            TextureUseInfo& rInfo = m_VertexBufferData.textureUseInfos[count];
            rInfo.pTexObj = pTexObj;
            rInfo.useCount = 1;
            rInfo.flags = 0;
            rInfo.flags = pTexObj->IsColorBlackWhiteInterpolationEnabled();
            if (m_pCharAttrs[i].IsBorderEffectEnabled()) {
                rInfo.flags |= 2;
            }

            count++;
            if (count >= TextureUseInfoCountMax) {
                break;
            }
        }

        m_pTextureUseInfoIndices[i] = index;
    }

    m_VertexBufferData.textureUseInfoCount = count;
}

/**
 * Writes the per-character parameters of all characters.
 * @param offset offset of the per-character parameters
 * @param pPerCharacterTransformInfos per-character transform information
 * @param center center of the per-character transform
 * @param centerOffset vertical offset of the transform center
 * @param pShadowParam shadow parameters
 * @param isDrawFromRightToLeftEnabled whether characters are drawn right to left
 * @param isOriginToCenterEnabled whether the origin is the character center
 */
void DispStringBuffer::BuildPerCharacterParams(
    ptrdiff_t offset, const PerCharacterTransformInfo* pPerCharacterTransformInfos,
    ConstantBufferAdditionalContent::PerCharacterTransformCenter center, float centerOffset,
    const ShadowParameter* pShadowParam, bool isDrawFromRightToLeftEnabled,
    bool isOriginToCenterEnabled) {
    void* pMapped = m_pConstantBuffer->GetMappedPointer();

    uint32_t startIndices[TextureUseInfoCountMax] = {};
    uint32_t sum = 0;
    for (int i = 0; i < static_cast<int>(m_VertexBufferData.textureUseInfoCount); i++) {
        startIndices[i] = sum;
        sum += m_VertexBufferData.textureUseInfos[i].useCount;
    }

    uint32_t counts[TextureUseInfoCountMax] = {};
    if (pMapped == nullptr) {
        return;
    }

    int charCount = m_CharCount;
    uint32_t bufferCount = pShadowParam != nullptr ? 2 : 1;
    nn::util::BytePtr base(pMapped);

    if (pShadowParam != nullptr) {
        if (isDrawFromRightToLeftEnabled) {
            for (int i = charCount - 1; i >= 0; i--) {
                BuildShadowBufferPerCharacterParams(offset, pPerCharacterTransformInfos, center,
                                                    centerOffset, pShadowParam, startIndices, base,
                                                    bufferCount, counts, i,
                                                    isOriginToCenterEnabled);
            }
        } else {
            for (int i = 0; i < charCount; i++) {
                BuildShadowBufferPerCharacterParams(offset, pPerCharacterTransformInfos, center,
                                                    centerOffset, pShadowParam, startIndices, base,
                                                    bufferCount, counts, i,
                                                    isOriginToCenterEnabled);
            }
        }
    }

    if (isDrawFromRightToLeftEnabled) {
        for (int i = charCount - 1; i >= 0; i--) {
            BuildCharacterBufferPerCharacterParams(offset, pPerCharacterTransformInfos, center,
                                                   centerOffset, startIndices, base, bufferCount,
                                                   counts, i, isOriginToCenterEnabled);
        }
    } else {
        for (int i = 0; i < charCount; i++) {
            BuildCharacterBufferPerCharacterParams(offset, pPerCharacterTransformInfos, center,
                                                   centerOffset, startIndices, base, bufferCount,
                                                   counts, i, isOriginToCenterEnabled);
        }
    }
}

void DispStringBuffer::BuildShadowBufferPerCharacterParams(
    ptrdiff_t offset, const PerCharacterTransformInfo* pPerCharacterTransformInfos,
    ConstantBufferAdditionalContent::PerCharacterTransformCenter center, float centerOffset,
    const ShadowParameter* pShadowParam, const uint32_t* pStartIndices, nn::util::BytePtr base,
    uint32_t bufferCount, uint32_t* pCounts, int index, bool isOriginToCenterEnabled) {
    const detail::CharAttribute& rAttr = m_pCharAttrs[index];
    uint8_t texIndex = m_pTextureUseInfoIndices[index];
    uint32_t vertexIndex = pStartIndices[texIndex] * bufferCount + pCounts[texIndex]++;
    auto pVertex = static_cast<detail::VertexShaderCharAttributeWithTransform*>(
        detail::CalculateVertexShaderCharAttributePtr(base, offset, vertexIndex,
                                                      pPerCharacterTransformInfos != nullptr));

    float width = rAttr.pos.x * pShadowParam->scale.x;
    float height = rAttr.pos.y * pShadowParam->scale.y;
    float x = isOriginToCenterEnabled ? width * -0.5f : rAttr.pos.z;
    x += pShadowParam->offset.x;
    float y = (rAttr.pos.y - height) + (rAttr.pos.w - pShadowParam->offset.y);
    float italicOffset =
        pShadowParam->scale.x * pShadowParam->italicRatio + static_cast<float>(rAttr.italicOffset);
    float italicRatio = rAttr.pos.y * italicOffset / m_FontHeight;

    const uint32_t topColor = LoadColor(pShadowParam->topColor);
    const uint32_t bottomColor = LoadColor(pShadowParam->bottomColor);
    uint8_t topAlpha = MultiplyAlpha(topColor >> 24, rAttr.shadowAlpha);
    uint8_t bottomAlpha = MultiplyAlpha(bottomColor >> 24, rAttr.shadowAlpha);

    if (pPerCharacterTransformInfos == nullptr) {
        pVertex->VertexShaderCharAttribute::Set(
            x, y, width, height, rAttr.tex, MakeColor(topColor, topAlpha),
            MakeColor(bottomColor, bottomAlpha), ~rAttr.sheetIndex, italicRatio, italicOffset);
        return;
    }

    const PerCharacterTransformInfo& rInfo = pPerCharacterTransformInfos[index];
    nn::util::Float4 rotateX;
    nn::util::Float4 rotateY;
    nn::util::Float4 translate;
    CalculatePerCharacterTransform(rotateX, rotateY, translate, x, y, width, height, italicRatio,
                                   italicOffset, rInfo, center, centerOffset,
                                   isOriginToCenterEnabled, -pShadowParam->offset.y);

    if (m_IsPerCharacterTransformAutoShadowAlpha) {
        topAlpha = MultiplyAlpha(topAlpha, rInfo.lt.v[3]);
        bottomAlpha = MultiplyAlpha(bottomAlpha, rInfo.lb.v[3]);
    }

    pVertex->Set(x, y, width, height, rAttr.tex, MakeColor(topColor, topAlpha),
                 MakeColor(bottomColor, bottomAlpha), ~rAttr.sheetIndex, italicRatio, italicOffset,
                 rotateX, rotateY, translate);
}

void DispStringBuffer::BuildCharacterBufferPerCharacterParams(
    ptrdiff_t offset, const PerCharacterTransformInfo* pPerCharacterTransformInfos,
    ConstantBufferAdditionalContent::PerCharacterTransformCenter center, float centerOffset,
    const uint32_t* pStartIndices, nn::util::BytePtr base, uint32_t bufferCount, uint32_t* pCounts,
    int index, bool isOriginToCenterEnabled) {
    const detail::CharAttribute& rAttr = m_pCharAttrs[index];
    uint8_t texIndex = m_pTextureUseInfoIndices[index];
    uint32_t vertexIndex = pStartIndices[texIndex] * bufferCount + pCounts[texIndex]++;
    auto pVertex = static_cast<detail::VertexShaderCharAttributeWithTransform*>(
        detail::CalculateVertexShaderCharAttributePtr(base, offset, vertexIndex,
                                                      pPerCharacterTransformInfos != nullptr));

    float width = rAttr.pos.x;
    float height = rAttr.pos.y;
    float x = isOriginToCenterEnabled ? width * -0.5f : rAttr.pos.z;
    float italicOffset = static_cast<float>(rAttr.italicOffset);
    float italicRatio = height * italicOffset / m_FontHeight;
    float y = rAttr.pos.w;

    if (pPerCharacterTransformInfos == nullptr) {
        pVertex->VertexShaderCharAttribute::Set(x, y, width, height, rAttr.tex, rAttr.color[0],
                                                rAttr.color[1], rAttr.sheetIndex, italicRatio,
                                                italicOffset);
        return;
    }

    const PerCharacterTransformInfo& rInfo = pPerCharacterTransformInfos[index];
    nn::util::Float4 rotateX;
    nn::util::Float4 rotateY;
    nn::util::Float4 translate;
    CalculatePerCharacterTransform(rotateX, rotateY, translate, x, y, width, height, italicRatio,
                                   italicOffset, rInfo, center, centerOffset,
                                   isOriginToCenterEnabled, 0.0f);

    pVertex->Set(x, y, width, height, rAttr.tex, MultiplyColor(rAttr.color[0], rInfo.lt),
                 MultiplyColor(rAttr.color[1], rInfo.lb), rAttr.sheetIndex,
                 italicRatio, italicOffset, rotateX, rotateY, translate);
}

void DispStringBuffer::CalculatePerCharacterTransform(
    nn::util::Float4& rRotateMatrixAndCenterX, nn::util::Float4& rRotateMatrixAndCenterY,
    nn::util::Float4& rTranslate, float x, float y, float width, float height, float italicRatio,
    float italicOffset, const PerCharacterTransformInfo& rInfo,
    ConstantBufferAdditionalContent::PerCharacterTransformCenter center, float centerOffset,
    bool isOriginToCenterEnabled, float shadowOffsetY) const {
    float centerX = (x + x + width + italicOffset) * 0.5f;
    float centerY = y;
    if (!isOriginToCenterEnabled) {
        switch (center) {
        case ConstantBufferAdditionalContent::PerCharacterTransformCenter_Center:
            centerY = (y + y + height) * 0.5f;
            break;
        case ConstantBufferAdditionalContent::PerCharacterTransformCenter_Bottom:
            centerY = y + height;
            break;
        default:
            centerY = 0.0f;
            break;
        }

        centerY -= centerOffset;
    }

    const float cosX = rInfo.rotationCos[0];
    const float cosY = rInfo.rotationCos[1];
    const float cosZ = rInfo.rotationCos[2];
    const float sinX = rInfo.rotationSin[0];
    const float sinY = rInfo.rotationSin[1];
    const float sinZ = rInfo.rotationSin[2];

    const float m00 = cosY * cosZ;
    const float m01 = cosY * sinZ;
    const float m02 = -sinY;
    const float m10 = sinX * sinY * cosZ - cosX * sinZ;
    const float m11 = sinX * sinY * sinZ + cosX * cosZ;
    const float m12 = sinX * cosY;

    rRotateMatrixAndCenterX.x = m00 * rInfo.scale[0];
    rRotateMatrixAndCenterX.y = m01 * rInfo.scale[0];
    rRotateMatrixAndCenterX.z = m02 * rInfo.scale[0];
    rRotateMatrixAndCenterX.w = centerX;
    rRotateMatrixAndCenterY.x = m10 * rInfo.scale[1];
    rRotateMatrixAndCenterY.y = m11 * rInfo.scale[1];
    rRotateMatrixAndCenterY.z = m12 * rInfo.scale[1];
    rRotateMatrixAndCenterY.w = centerY;
    rTranslate.x = rInfo.translation[0];
    rTranslate.y = isOriginToCenterEnabled ? rInfo.translation[1] - centerY + shadowOffsetY :
                                             rInfo.translation[1];
    rTranslate.z = rInfo.translation[2];
}

/**
 * Sets the font height.
 * @param fontHeight font height
 */
void DispStringBuffer::SetFontHeight(float fontHeight) {
    m_FontHeight = fontHeight;
}

bool DispStringBuffer::CompareCopiedInstanceTest(const DispStringBuffer& rOther) const {
    if (m_CharCountMax != rOther.m_CharCountMax) {
        return false;
    }

    if (m_IsShadowEnabled != rOther.m_IsShadowEnabled) {
        return false;
    }

    if (m_IsDoubleDrawnBorder != rOther.m_IsDoubleDrawnBorder) {
        return false;
    }

    if (m_IsPerCharacterTransformEnabled != rOther.m_IsPerCharacterTransformEnabled) {
        return false;
    }

    if (m_IsPerCharacterTransformAutoShadowAlpha !=
        rOther.m_IsPerCharacterTransformAutoShadowAlpha) {
        return false;
    }

    if (m_FontHeight != rOther.m_FontHeight) {
        return false;
    }

    return true;
}

}  // namespace font
}  // namespace nn
