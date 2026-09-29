#pragma once

namespace nn::gfx::detail {
template <class T>
class Caster;
template <class T>
class DataContainer;
}  // namespace nn::gfx::detail

#include "nn/gfx/detail/gfx_DataContainer.h"

namespace nn {
namespace gfx {
template <typename TAccessor>
inline detail::Caster<typename TAccessor::DataType> AccessorToData(TAccessor& accessor) {
    return accessor.ToData();
}

template <typename TAccessor>
inline detail::Caster<volatile typename TAccessor::DataType>
AccessorToData(volatile TAccessor& accessor) {
    return accessor.ToData();
}

template <typename TAccessor>
inline detail::Caster<volatile const typename TAccessor::DataType>
AccessorToData(volatile const TAccessor& accessor) {
    return accessor.ToData();
}

template <typename TAccessor>
inline detail::Caster<const typename TAccessor::DataType>
AccessorToData(const TAccessor& accessor) {
    return accessor.ToData();
}

template <typename TAccessor>
inline detail::Caster<typename TAccessor::DataType> AccessorToData(TAccessor* pAccessor) {
    return pAccessor->ToData();
}

template <typename TAccessor>
inline detail::Caster<const typename TAccessor::DataType>
AccessorToData(const TAccessor* pAccessor) {
    return pAccessor->ToData();
}

template <typename TAccessor>
inline detail::Caster<volatile typename TAccessor::DataType>
AccessorToData(volatile TAccessor* pAccessor) {
    return pAccessor->ToData();
}

template <typename TAccessor>
inline detail::Caster<volatile const typename TAccessor::DataType>
AccessorToData(volatile const TAccessor* pAccessor) {
    return pAccessor->ToData();
}

template <typename TData>
inline detail::Caster<detail::DataContainer<TData>> DataToAccessor(TData& data) {
    return detail::DataContainer<TData>::DataToAccessor(data);
}

template <typename TData>
inline detail::Caster<const detail::DataContainer<TData>> DataToAccessor(const TData& data) {
    return detail::DataContainer<TData>::DataToAccessor(data);
}

template <typename TData>
inline detail::Caster<volatile detail::DataContainer<TData>> DataToAccessor(volatile TData& data) {
    return detail::DataContainer<TData>::DataToAccessor(data);
}

template <typename TData>
inline detail::Caster<volatile const detail::DataContainer<TData>>
DataToAccessor(volatile const TData& data) {
    return detail::DataContainer<TData>::DataToAccessor(data);
}

template <typename TData>
inline detail::Caster<detail::DataContainer<TData>> DataToAccessor(TData* pData) {
    return detail::DataContainer<TData>::DataToAccessor(*pData);
}

template <typename TData>
inline detail::Caster<const detail::DataContainer<TData>> DataToAccessor(const TData* pData) {
    return detail::DataContainer<TData>::DataToAccessor(*pData);
}

template <typename TData>
inline detail::Caster<volatile detail::DataContainer<TData>> DataToAccessor(volatile TData* pData) {
    return detail::DataContainer<TData>::DataToAccessor(*pData);
}

template <typename TData>
inline detail::Caster<volatile const detail::DataContainer<TData>>
DataToAccessor(volatile const TData* pData) {
    return detail::DataContainer<TData>::DataToAccessor(*pData);
}

};  // namespace gfx
};  // namespace nn