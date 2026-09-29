#pragma once

#include "nn/gfx/detail/gfx_Declare.h"
#include "nn/gfx/gfx_DataAccessorConverter.h"
#include "nn/gfx/gfx_Types.h"

namespace nn {

namespace font {

template <typename TGfxObject>
inline bool IsInitialized(const TGfxObject& obj) {
    return nn::gfx::AccessorToData(obj)->state != TGfxObject::DataType::State_NotInitialized;
}

template <typename T>
inline void FinalizeIfNecessary(T& obj, nn::gfx::Device* pDevice) {
    if (IsInitialized(obj)) {
        obj.Finalize(pDevice);
    }
}

template <typename T>
inline void FinalizeIfNecessary(T& obj, bool isInitialized,
                                void (T::*finalizeFunc)(nn::gfx::Device*),
                                nn::gfx::Device* pDevice) {
    if (isInitialized) {
        (obj.*finalizeFunc)(pDevice);
    }
}

namespace detail {

class RuntimeTypeInfo {
public:
    const RuntimeTypeInfo* m_ParentTypeInfo;

    explicit RuntimeTypeInfo(const RuntimeTypeInfo* parent) : m_ParentTypeInfo(parent) {}
    bool IsDerivedFrom(const RuntimeTypeInfo*) const;
};

}  // namespace detail

// todo: figure out where to put this
#define NN_RUNTIME_TYPEINFO_BASE()                                                                 \
    static const nn::font::detail::RuntimeTypeInfo* GetRuntimeTypeInfoStatic() {                   \
        static const nn::font::detail::RuntimeTypeInfo s_TypeInfo(nullptr);                        \
        return &s_TypeInfo;                                                                        \
    }                                                                                              \
                                                                                                   \
    virtual const nn::font::detail::RuntimeTypeInfo* GetRuntimeTypeInfo() const {                  \
        return GetRuntimeTypeInfoStatic();                                                         \
    }

#define NN_RUNTIME_TYPEINFO(BASE)                                                                  \
    static const nn::font::detail::RuntimeTypeInfo* GetRuntimeTypeInfoStatic() {                   \
        static const nn::font::detail::RuntimeTypeInfo s_TypeInfo(                                 \
            BASE::GetRuntimeTypeInfoStatic());                                                     \
        return &s_TypeInfo;                                                                        \
    }                                                                                              \
                                                                                                   \
    virtual const nn::font::detail::RuntimeTypeInfo* GetRuntimeTypeInfo() const {                  \
        return GetRuntimeTypeInfoStatic();                                                         \
    }
}  // namespace font
};  // namespace nn