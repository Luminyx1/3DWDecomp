#include "utility/aglResParameter.h"
#include <basis/seadRawPrint.h>
#include <container/seadTreeMap.h>
#include <math/seadVector.h>
#include <prim/seadPtrUtil.h>
#include <heap/seadHeap.h>
#include <prim/seadMemUtil.h>
#include <prim/seadStringUtil.h>
#include "detail/aglPrivateResource.h"
#include "utility/aglParameter.h"

namespace agl::utl
{

/**
 * Computes the size in bytes of this parameter's payload.
 * @return payload size in bytes, 0 for an unknown type
 */
size_t ResParameter::getDataSize() const
{
    switch (ParameterType(ptr()->getType()))
    {
    case ParameterType::Bool:
    case ParameterType::F32:
    case ParameterType::Int:
    case ParameterType::U32:
        return 4;
    case ParameterType::Vec2:
        return sizeof(sead::Vector2f);
    case ParameterType::Vec3:
        return sizeof(sead::Vector3f);
    case ParameterType::Vec4:
    case ParameterType::Color:
    case ParameterType::Quat:
        return sizeof(sead::Vector4f);
    case ParameterType::String32:
    case ParameterType::String64:
    case ParameterType::String256:
    case ParameterType::StringRef:
        return sead::SafeString(getData<char>()).calcLength() + 1;
    case ParameterType::Curve1:
        return 1 * 0x80;
    case ParameterType::Curve2:
        return 2 * 0x80;
    case ParameterType::Curve3:
        return 3 * 0x80;
    case ParameterType::Curve4:
        return 4 * 0x80;
    case ParameterType::BufferBinary:
        return getBufferSize();
    case ParameterType::BufferInt:
    case ParameterType::BufferF32:
    case ParameterType::BufferU32:
        return 4 * getBufferSize();
    default:
        SEAD_ASSERT_MSG(false, "illigal type:%d", ptr()->getType());
        return 0;
    }
}

s32 ResParameterObj::searchIndex(u32 param_hash) const
{
    for (s32 i = 0; i < getNum(); ++i)
    {
        if (getResParameter(i).getParameterNameHash() == param_hash)
        {
            return i;
        }
    }

    return -1;
}

s32 ResParameterList::searchListIndex(u32 list_hash) const
{
    for (s32 i = 0; i < getResParameterListNum(); ++i)
    {
        if (getResParameterList(i).getParameterListNameHash() == list_hash)
        {
            return i;
        }
    }

    return -1;
}

s32 ResParameterList::searchObjIndex(u32 obj_hash) const
{
    for (s32 i = 0; i < getResParameterObjNum(); ++i)
    {
        if (getResParameterObj(i).getParameterObjNameHash() == obj_hash)
        {
            return i;
        }
    }

    return -1;
}

static void getName_(sead::BufferedSafeString* pName, u32 hash,
                     const sead::TreeMap<u32, const char*>* pNameTable)
{
    if (pNameTable)
    {
        if (const auto* node = pNameTable->find(hash))
        {
            pName->copy(node->value());
            return;
        }
    }

    pName->format("0x%08x", hash);
}

void ResParameterList::dump(s32 indent, const sead::TreeMap<u32, const char*>* pNameTable) const
{
    sead::FixedSafeString<32> name;
    getName_(&name, getParameterListNameHash(), pNameTable);

    for (auto it = listBegin(), end = listEnd(); it != end; ++it)
    {
        (*it).dump(indent + 1, pNameTable);
    }

    for (auto obj_it = objBegin(), obj_end = objEnd(); obj_it != obj_end; ++obj_it)
    {
        const ResParameterObj obj = *obj_it;
        getName_(&name, obj.getParameterObjNameHash(), pNameTable);

        for (s32 i = 0; i < obj.getNum(); ++i)
        {
            const ResParameter param = obj.getResParameter(i);
            getName_(&name, param.getParameterNameHash(), pNameTable);

            switch (ParameterType(param.ptr()->getType()))
            {
            case ParameterType::BufferInt:
            {
                ParameterBuffer<s32> buffer;
                buffer.postApplyResource_(param.getData<void>(), param.getDataSize());
                break;
            }
            case ParameterType::BufferF32:
            {
                ParameterBuffer<f32> buffer;
                buffer.postApplyResource_(param.getData<void>(), param.getDataSize());
                break;
            }
            case ParameterType::BufferU32:
            {
                ParameterBuffer<u32> buffer;
                buffer.postApplyResource_(param.getData<void>(), param.getDataSize());
                break;
            }
            case ParameterType::BufferBinary:
            {
                ParameterBuffer<u8> buffer;
                buffer.postApplyResource_(param.getData<void>(), param.getDataSize());
                break;
            }
            default:
                break;
            }
        }
    }
}

/**
 * Reads the payload as a string pointer if the parameter is a string type.
 * @param pOut receives the string pointer
 * @return whether the parameter is a string type
 */
template <>
bool ResParameter::copyData<const char*>(const char** pOut) const
{
    switch (ParameterType(ptr()->getType()))
    {
    case ParameterType::String32:
    case ParameterType::String64:
    case ParameterType::String256:
    case ParameterType::StringRef:
        *pOut = getData<char>();
        return true;
    default:
        return false;
    }
}

/**
 * Reads the payload as a bool if the parameter is a bool.
 * @param pOut receives the value
 * @return whether the parameter is a bool
 */
template <>
bool ResParameter::copyData<bool>(bool* pOut) const
{
    if (ParameterType(ptr()->getType()) != ParameterType::Bool)
    {
        return false;
    }

    *pOut = *getData<u32>() != 0;
    return true;
}

/**
 * Returns the supported AAMP version.
 * @return the version number
 */
u32 ResParameterArchiveData::getVersion()
{
    return 2;
}

/**
 * Returns the AAMP file signature.
 * @return the signature as a little-endian u32
 */
u32 ResParameterArchiveData::getSignature()
{
    return 0x504D4141;
}

ResParameterArchive::ResParameterArchive(const void* pData)
{
    mpData = static_cast<const ResParameterArchiveData*>(pData);
    if (!pData)
    {
        return;
    }

    auto* data = const_cast<ResParameterArchiveData*>(mpData);
    const bool is_little_endian = data->flags.isOn(ResParameterArchiveFlag::LittleEndian);
    bool is_utf8;
    if (is_little_endian)
    {
        if (data->flags.isOn(ResParameterArchiveFlag::Utf8))
        {
            return;
        }

        is_utf8 = false;
    }
    else
    {
        ModifyEndianU32(false, data, sizeof(ResParameterArchiveData));
        data = const_cast<ResParameterArchiveData*>(mpData);
        is_utf8 = data->flags.isOn(ResParameterArchiveFlag::Utf8);
    }

    u8* lists = reinterpret_cast<u8*>(data) + sizeof(ResParameterArchiveData) + data->offset_to_pio;
    const size_t lists_size = data->num_lists * sizeof(ResParameterListData);
    u8* objs = lists + lists_size;
    const size_t objs_size = data->num_objects * sizeof(ResParameterObjData);
    u8* params = objs + objs_size;
    const size_t params_size = data->num_parameters * sizeof(ResParameterData);
    u8* data_section = params + params_size;
    const size_t data_size = data->data_section_size;
    char* strings = reinterpret_cast<char*>(data_section + data_size);
    const u32 string_size = data->string_section_size;
    u8* unk = reinterpret_cast<u8*>(strings + string_size);

    if (!is_little_endian)
    {
        const size_t size = lists_size + objs_size + params_size + data_size;
        if (size != 0)
        {
            ModifyEndianU32(false, lists, size);
        }

        const u32 unk_size = mpData->unk_section_size;
        if (unk_size != 0)
        {
            u32 offset = 0;
            do
            {
                u32* entry = reinterpret_cast<u32*>(unk + offset);
                ModifyEndianU32(false, entry, sizeof(u32));
                offset += *entry;
            } while (offset < unk_size);
        }

        const_cast<ResParameterArchiveData*>(mpData)->flags.set(
            ResParameterArchiveFlag::LittleEndian);
    }

    if (is_utf8 || mpData->string_section_size == 0)
    {
        return;
    }

    if (string_size != 0)
    {
        do
        {
            const s32 length = sead::SafeString(strings).calcLength();
            if (length > 0)
            {
                const s32 utf16_length = length + 1;
                sead::Heap* heap = detail::PrivateResource::instance()->getWorkHeap();
                auto* utf16 = new (heap) char16[utf16_length];
                const s32 utf8_length = utf16_length * 2;
                auto* utf8 = new (heap) char[utf8_length];
                sead::StringUtil::convertSjisToUtf16(utf16, utf16_length, strings, -1);
                sead::StringUtil::convertUtf16ToUtf8(utf8, utf8_length, utf16, -1);
                const s32 converted_length = sead::SafeString(utf8).calcLength() + 1;
                sead::MemUtil::copy(strings, utf8,
                                    converted_length < utf16_length ? converted_length :
                                                                      utf16_length);
                heap->free(utf16);
                heap->free(utf8);
            }

            strings += (length + 4) & ~3;
        } while (strings < reinterpret_cast<char*>(unk));
    }

    const_cast<ResParameterArchiveData*>(mpData)->flags.set(ResParameterArchiveFlag::Utf8);
}

}  // namespace agl::utl
