#include "utility/aglParameter.h"
#include <codec/seadHashCRC32.h>
#include <gfx/seadColor.h>
#include <math/seadQuatCalcCommon.h>
#include <math/seadVector.h>
#include <prim/seadFormatPrint.h>
#include <prim/seadMemUtil.h>
#include <prim/seadStringUtil.h>
#include "utility/aglParameterObj.h"
#include "utility/aglParameterStringMgr.h"

namespace agl::utl
{

static constexpr const char* sParameterTypeNames[] = {
    "bool",       "f32",       "int",    "vec2",   "vec3",       "vec4",          "color",
    "string32",   "string64",  "curve1", "curve2", "curve3",     "curve4",        "buffer_int",
    "buffer_f32", "string256", "quat",   "u32",    "buffer_u32", "buffer_binary", "stringRef",
};

/**
 * Returns the parameter's name (empty in release builds).
 * @return the parameter name
 */
sead::SafeString ParameterBase::getParameterName() const
{
#ifdef SEAD_DEBUG
    return mName;
#else
    return sead::SafeString::cEmptyString;
#endif
}

/**
 * Returns the parameter's label (empty in release builds).
 * @return the parameter label
 */
sead::SafeString ParameterBase::getLabel() const
{
#ifdef SEAD_DEBUG
    return mLabel;
#else
    return sead::SafeString::cEmptyString;
#endif
}

/**
 * Returns the parameter's meta string (empty in release builds).
 * @return the parameter meta string
 */
sead::SafeString ParameterBase::getMeta() const
{
#ifdef SEAD_DEBUG
    return mMeta;
#else
    return sead::SafeString::cEmptyString;
#endif
}

/**
 * Returns the XML tag used for parameters.
 * @return the tag name
 */
const char* ParameterBase::getTagName()
{
    return "param";
}

const char* ParameterBase::getAttributeNameString()
{
    return "name";
}

const char* ParameterBase::getAttributeTypeString()
{
    return "type";
}

const char* ParameterBase::getAttributeValueString()
{
    return "value";
}

const char* ParameterBase::getParameterTypeName(ParameterType type)
{
    return sParameterTypeNames[u32(type)];
}

/**
 * Checks whether a resource of the given type can be applied to this parameter.
 * @param type resource parameter type
 * @return whether the type is compatible
 */
bool ParameterBase::isSafeType(ParameterType type) const
{
    if (getParameterType() == type)
    {
        return true;
    }

    constexpr std::pair<ParameterType, ParameterType> pairs[] = {
        {ParameterType::String64, ParameterType::String32},
        {ParameterType::String32, ParameterType::String64},
        {ParameterType::String256, ParameterType::String32},
        {ParameterType::String256, ParameterType::String64},
        {ParameterType::String32, ParameterType::String256},
        {ParameterType::String64, ParameterType::String256},
    };

    for (const auto pair : pairs)
    {
        const auto current = getParameterType();
        if (type == pair.first && current == pair.second)
        {
            return true;
        }
    }

    if (getParameterType() == ParameterType::StringRef &&
        (type == ParameterType::String32 || type == ParameterType::String64 ||
         type == ParameterType::String256))
    {
        return true;
    }

    return false;
}

/**
 * Verifies a resource type against this parameter (always succeeds in release builds).
 * @param type resource parameter type
 * @return true
 */
bool ParameterBase::verifyType(ParameterType type) const
{
    return true;
}

/**
 * Copies another parameter's value if its type and name hash match.
 * @param rOther parameter to copy from
 * @return whether the value was copied
 */
bool ParameterBase::copy(const ParameterBase& rOther)
{
    if (getParameterType() != rOther.getParameterType() || mNameHash != rOther.mNameHash)
    {
        return false;
    }

    copyUnsafe(rOther);
    return true;
}

/**
 * Copies another parameter's value without checking the type.
 * @param rOther parameter to copy from
 */
void ParameterBase::copyUnsafe(const ParameterBase& rOther)
{
    if (rOther.getParameterType() == ParameterType::StringRef)
    {
        auto* source = static_cast<const sead::SafeString*>(rOther.typePtr());
        auto* dest = static_cast<sead::SafeString*>(typePtr());
        *dest = *source;
        return;
    }

    auto* dest = ptrT<u8>();
    auto* src = rOther.ptrT<u8>();
    const s32 n = size();
    for (s32 i = 0; i < n; ++i)
    {
        *dest = *src;
        ++dest;
        ++src;
    }
}

/**
 * Sets this f32 parameter to the linear interpolation of two parameters.
 * @param rParam1 value at t = 0
 * @param rParam2 value at t = 1
 * @param t interpolation factor
 */
template <>
void ParameterBase::copyLerp_<f32>(const ParameterBase& rParam1, const ParameterBase& rParam2,
                                   f32 t)
{
    *ptrT<f32>() = sead::lerp(*rParam1.ptrT<f32>(), *rParam2.ptrT<f32>(), t);
}

/**
 * Sets this quaternion parameter to the spherical interpolation of two parameters.
 * @param rParam1 value at t = 0
 * @param rParam2 value at t = 1
 * @param t interpolation factor
 */
template <>
void ParameterBase::copyLerp_<sead::Quatf>(const ParameterBase& rParam1,
                                           const ParameterBase& rParam2, f32 t)
{
    sead::QuatCalcCommon<f32>::slerpTo(*ptrT<sead::Quatf>(), *rParam1.ptrT<sead::Quatf>(),
                                       *rParam2.ptrT<sead::Quatf>(), t);
}

template <typename T>
static void lerpVec_(T& v_dest, const T& v1, const T& v2, f32 t)
{
    for (size_t i = 0; i < v_dest.e.size(); ++i)
    {
        v_dest.e[i] = sead::lerp(v1.e[i], v2.e[i], t);
    }
}

template <typename T>
static void copyLerpVec_(ParameterBase& dest, const ParameterBase& param1,
                         const ParameterBase& param2, f32 t)
{
    lerpVec_(*dest.ptrT<T>(), *param1.ptrT<T>(), *param2.ptrT<T>(), t);
}

bool ParameterBase::copyLerp(const ParameterBase& param1, const ParameterBase& param2, f32 t)
{
    if (getParameterType() != param1.getParameterType() || mNameHash != param1.mNameHash)
    {
        return false;
    }

    if (getParameterType() != param2.getParameterType() || mNameHash != param2.mNameHash)
    {
        return false;
    }

    switch (getParameterType())
    {
    case ParameterType::Bool:
    case ParameterType::Int:
    case ParameterType::String32:
    case ParameterType::String64:
    case ParameterType::String256:
    case ParameterType::U32:
        copyUnsafe(param1);
        return true;
    case ParameterType::F32:
        copyLerp_<f32>(param1, param2, t);
        return true;
    case ParameterType::Vec2:
        copyLerpVec_<sead::Vector2f>(*this, param1, param2, t);
        return true;
    case ParameterType::Vec3:
        copyLerpVec_<sead::Vector3f>(*this, param1, param2, t);
        return true;
    case ParameterType::Vec4:
        copyLerpVec_<sead::Vector4f>(*this, param1, param2, t);
        return true;
    case ParameterType::Color:
    {
        auto& color = *ptrT<sead::Color4f>();
        color.setLerp(*param1.ptrT<sead::Color4f>(), *param2.ptrT<sead::Color4f>(), t);
        return true;
    }
    case ParameterType::Curve1:
    case ParameterType::Curve2:
    case ParameterType::Curve3:
    case ParameterType::Curve4:
    case ParameterType::BufferInt:
    case ParameterType::BufferF32:
    case ParameterType::BufferU32:
    case ParameterType::BufferBinary:
    case ParameterType::StringRef:
    case ParameterType::Special:
        return true;
    case ParameterType::Quat:
        copyLerp_<sead::Quatf>(param1, param2, t);
        return true;
    default:
        SEAD_ASSERT_MSG(false, "%d", int(getParameterType()));
        return true;
    }
}

static void applyResourceSimple_(ParameterBase& param, const ResParameter& res)
{
    void* dest = param.ptr();
    const void* src = res.getData<void>();

    const size_t data_size = param.size();
    const size_t res_data_size = res.getDataSize();
    const auto copy_size = data_size < res_data_size ? data_size : res_data_size;

    sead::MemUtil::copy(dest, src, copy_size);
}

/**
 * Applies a resource parameter's value to this parameter.
 * @param res resource parameter
 */
void ParameterBase::applyResource(ResParameter res)
{
#ifdef SEAD_DEBUG
    if (!verifyType(ParameterType(res.ptr()->getType())))
    {
        return;
    }
#endif

    if (getParameterType() == ParameterType::Bool)
    {
        const bool value = *res.getData<u32>() != 0;
        *ptrT<bool>() = value;
    }
    else if (isBinaryInternalBuffer())
    {
        if (getParameterType() == ParameterType::StringRef)
        {
            *static_cast<sead::SafeString*>(typePtr()) = res.getData<char>();
        }
        else
        {
            applyResourceSimple_(*this, res);
        }
    }

    postApplyResource_(res.getData<void>(), res.getDataSize());
}

void ParameterBase::applyResource(ResParameter res, f32 t)
{
#ifdef SEAD_DEBUG
    if (!verifyType(ParameterType(res.ptr()->getType())))
    {
        return;
    }
#endif

    switch (getParameterType())
    {
    case ParameterType::Bool:
        *ptrT<bool>() = *res.getData<u32>() != 0;
        break;
    case ParameterType::F32:
    {
        f32 x;
        sead::MemUtil::copy(&x, res.getData<void>(), size());
        *ptrT<f32>() += x * t;
        break;
    }
    case ParameterType::Int:
    case ParameterType::String32:
    case ParameterType::String64:
    case ParameterType::String256:
    case ParameterType::U32:
        applyResourceSimple_(*this, res);
        break;
    case ParameterType::Vec2:
    {
        sead::Vector2f vec;
        sead::MemUtil::copy(&vec, res.getData<void>(), size());
        *ptrT<sead::Vector2f>() += vec * t;
        break;
    }
    case ParameterType::Vec3:
    {
        sead::Vector3f vec;
        sead::MemUtil::copy(&vec, res.getData<void>(), size());
        *ptrT<sead::Vector3f>() += vec * t;
        break;
    }
    case ParameterType::Vec4:
    {
        sead::Vector4f vec;
        sead::MemUtil::copy(&vec, res.getData<void>(), size());
        *ptrT<sead::Vector4f>() += vec * t;
        break;
    }
    case ParameterType::Color:
    {
        sead::Color4f color;
        sead::MemUtil::copy(&color, res.getData<void>(), size());
        *ptrT<sead::Color4f>() += color * sead::Color4f{t, t, t, t};
        break;
    }
    case ParameterType::Curve1:
    case ParameterType::Curve2:
    case ParameterType::Curve3:
    case ParameterType::Curve4:
    case ParameterType::Special:
        break;
    case ParameterType::BufferInt:
    case ParameterType::BufferF32:
    case ParameterType::BufferU32:
    case ParameterType::BufferBinary:
        if (isBinaryInternalBuffer())
        {
            applyResourceSimple_(*this, res);
        }
        break;
    case ParameterType::Quat:
    {
        sead::Quatf quat;
        sead::MemUtil::copy(&quat, res.getData<void>(), size());
        auto* target = ptrT<sead::Quatf>();
        sead::QuatCalcCommon<f32>::slerpTo(*target, *target, quat, t);
        break;
    }
    case ParameterType::StringRef:
        *static_cast<sead::SafeString*>(typePtr()) = res.getData<char>();
        break;
    default:
        SEAD_ASSERT_MSG(false, "%d", int(getParameterType()));
        break;
    }

    postApplyResource_(res.getData<void>(), res.getDataSize());
}

/**
 * Checks whether this parameter's type supports interpolation.
 * @return whether the parameter can be interpolated
 */
bool ParameterBase::isInterpolatable() const
{
    const auto type = getParameterType();
    return type == ParameterType::F32 || type == ParameterType::Vec2 ||
           type == ParameterType::Vec3 || type == ParameterType::Vec4 ||
           type == ParameterType::Color || type == ParameterType::Quat;
}

void ParameterBase::genMessageParameter(sead::hostio::Context* pContext,
                                        const sead::SafeString& rLabel)
{
    const char* name = getParameterName().cstr();

    switch (getParameterType())
    {
    case ParameterType::Bool:
        typePtr();
        break;
    case ParameterType::F32:
        typePtr();
        break;
    case ParameterType::Int:
        typePtr();
        break;
    case ParameterType::Vec2:
        typePtr();
        break;
    case ParameterType::Vec3:
        typePtr();
        break;
    case ParameterType::Vec4:
        typePtr();
        break;
    case ParameterType::Color:
        typePtr();
        break;
    case ParameterType::String32:
        typePtr();
        break;
    case ParameterType::String64:
        typePtr();
        break;
    case ParameterType::String256:
        typePtr();
        break;
    case ParameterType::Quat:
        typePtr();
        break;
    case ParameterType::U32:
        typePtr();
        break;
    case ParameterType::StringRef:
    {
        const auto* string = static_cast<const sead::SafeString*>(typePtr());
        sead::FormatFixedSafeString<1024> meta("%s (unmodifiable) [%s]", name, string->cstr());
        break;
    }
    default:
        break;
    }
}

/**
 * Writes this parameter's binary representation.
 * @param pBinary destination buffer
 * @return number of bytes written
 */
size_t ParameterBase::binarize(void* pBinary) const
{
    SEAD_ASSERT(pBinary != nullptr);

    size_t binary_size;
    if (getParameterType() != ParameterType::Bool)
    {
        binary_size = calcBinarizeSize();
        sead::MemUtil::copy(pBinary, ptr(), binary_size);
    }
    else
    {
        binary_size = sizeof(u32);
        *static_cast<u32*>(pBinary) = *ptrT<bool>();
    }
    return binary_size;
}

bool ParameterBase::makeZero()
{
    switch (getParameterType())
    {
    case ParameterType::F32:
        *ptrT<f32>() = 0;
        return true;
    case ParameterType::Vec2:
        *ptrT<sead::Vector2f>() = {0, 0};
        return true;
    case ParameterType::Vec3:
        *ptrT<sead::Vector3f>() = {0, 0, 0};
        return true;
    case ParameterType::Vec4:
        *ptrT<sead::Vector4f>() = {0, 0, 0, 0};
        return true;
    case ParameterType::Color:
        *ptrT<sead::Color4f>() = {0, 0, 0, 0};
        return true;
    case ParameterType::Quat:
    {
        auto* quat = ptrT<sead::Quatf>();
        quat->z = 0;
        quat->w = 1;
        quat->x = quat->y = 0;
        return true;
    }
    case ParameterType::Bool:
    case ParameterType::Int:
    case ParameterType::String32:
    case ParameterType::String64:
    case ParameterType::Curve1:
    case ParameterType::Curve2:
    case ParameterType::Curve3:
    case ParameterType::Curve4:
    case ParameterType::BufferInt:
    case ParameterType::BufferF32:
    case ParameterType::String256:
    case ParameterType::U32:
    case ParameterType::BufferU32:
    case ParameterType::BufferBinary:
    case ParameterType::StringRef:
    case ParameterType::Special:
        return false;
    }
    return false;
}

/**
 * Generates host IO messages for the direction (empty in release builds).
 * @param pContext host IO context
 */
void ParameterDirection3f::genMessageParameter(sead::hostio::Context* pContext) {}

ParameterBase* ParameterBase::createByTypeName(const sead::SafeString& name,
                                               const sead::SafeString& bufferSize)
{
    if (name.isEqual("bool"))
    {
        return new Parameter<bool>;
    }
    if (name.isEqual("f32"))
    {
        return new Parameter<f32>;
    }
    if (name.isEqual("int"))
    {
        return new Parameter<s32>;
    }
    if (name.isEqual("u32"))
    {
        return new Parameter<u32>;
    }
    if (name.isEqual("vec2"))
    {
        return new Parameter<sead::Vector2f>;
    }
    if (name.isEqual("vec3"))
    {
        return new Parameter<sead::Vector3f>;
    }
    if (name.isEqual("vec4"))
    {
        return new Parameter<sead::Vector4f>;
    }
    if (name.isEqual("color"))
    {
        return new Parameter<sead::Color4f>;
    }
    if (name.isEqual("quat"))
    {
        return new Parameter<sead::Quatf>;
    }
    if (name.isEqual("string32"))
    {
        return new Parameter<sead::FixedSafeString<32>>;
    }
    if (name.isEqual("string64"))
    {
        return new Parameter<sead::FixedSafeString<64>>;
    }
    if (name.isEqual("string256"))
    {
        return new Parameter<sead::FixedSafeString<256>>;
    }
    if (name.isEqual("stringRef"))
    {
        return new Parameter<sead::SafeString>;
    }
    if (name.isEqual("curve1"))
    {
        return new ParameterCurve<1>;
    }
    if (name.isEqual("curve2"))
    {
        return new ParameterCurve<2>;
    }
    if (name.isEqual("curve3"))
    {
        return new ParameterCurve<3>;
    }
    if (name.isEqual("curve4"))
    {
        return new ParameterCurve<4>;
    }
    if (name.isEqual("buffer_int"))
    {
        ParameterBuffer<s32>* buffer = new ParameterBuffer<s32>;
        u32 size =
            sead::StringUtil::parseS32(bufferSize, sead::StringUtil::CardinalNumber::BaseAuto);
        buffer->allocateBuffer(nullptr, size);
        return buffer;
    }
    if (name.isEqual("buffer_f32"))
    {
        ParameterBuffer<f32>* buffer = new ParameterBuffer<f32>;
        u32 size =
            sead::StringUtil::parseS32(bufferSize, sead::StringUtil::CardinalNumber::BaseAuto);
        buffer->allocateBuffer(nullptr, size);
        return buffer;
    }
    if (name.isEqual("buffer_u32"))
    {
        ParameterBuffer<u32>* buffer = new ParameterBuffer<u32>;
        u32 size =
            sead::StringUtil::parseS32(bufferSize, sead::StringUtil::CardinalNumber::BaseAuto);
        buffer->allocateBuffer(nullptr, size);
        return buffer;
    }
    if (name.isEqual("buffer_binary"))
    {
        ParameterBuffer<u8>* buffer = new ParameterBuffer<u8>;
        u32 size =
            sead::StringUtil::parseS32(bufferSize, sead::StringUtil::CardinalNumber::BaseAuto);
        buffer->allocateBuffer(nullptr, (u32)sead::Mathf::ceil(size * 0.25f) * 4);
        return buffer;
    }
    return nullptr;
}

u32 ParameterBase::calcHash(const sead::SafeString& key)
{
    return sead::HashCRC32::calcStringHash(key);
}

ParameterBase::ParameterBase()
{
    initializeListNode("default", "parameter", "", nullptr);
}

ParameterBase::ParameterBase(const sead::SafeString& name, const sead::SafeString& label,
                             IParameterObj* param_obj)
{
    initializeListNode(name, label, "", param_obj);
}

ParameterBase::ParameterBase(const sead::SafeString& name, const sead::SafeString& label,
                             const sead::SafeString& meta, IParameterObj* param_obj)
{
    initializeListNode(name, label, meta, param_obj);
}

void ParameterBase::initializeListNode(const sead::SafeString& name, const sead::SafeString& label,
                                       const sead::SafeString& meta, IParameterObj* param_obj)
{
    mNext = nullptr;

#ifdef SEAD_DEBUG
    if (ParameterStringMgr::instance())
    {
        mName = ParameterStringMgr::instance()->appendString(name);
        mLabel = ParameterStringMgr::instance()->appendString(label);
        mMeta = ParameterStringMgr::instance()->appendString(meta);
    }
    else
    {
        mName = nullptr;
        mLabel = nullptr;
        mMeta = nullptr;
    }
#endif

    mNameHash = calcHash(name);

    if (param_obj)
    {
        param_obj->pushBackListNode(this);
    }
}

}  // namespace agl::utl
