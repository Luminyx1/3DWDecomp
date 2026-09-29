#pragma once

#include "nn/gfx/gfx_DataAccessorConverter.h"

namespace nn::gfx::detail {

template <class T>
class CasterBase {
protected:
    T* m_Value;

public:
    explicit CasterBase(T* value) : m_Value(value) {}

    T* operator->() { return m_Value; }

    operator bool();
    operator bool() const;
    bool IsNull();

    template <class U>
    operator U*() {
        return static_cast<U*>(m_Value);
    }
};

template <class T>
class Caster : public CasterBase<T> {
public:
    explicit Caster(T* value) : CasterBase<T>(value) {}

    template <class U>
    operator U&() {
        return static_cast<U&>(*CasterBase<T>::m_Value);
    }
};

template <class T>
class Caster<const T> : public CasterBase<const T> {
public:
    explicit Caster(const T* value) : CasterBase<const T>(value) {}

    template <class U>
    operator const U&() {
        return static_cast<const U&>(*CasterBase<const T>::m_Value);
    }
};

template <class T>
class Caster<volatile T> : public CasterBase<volatile T> {
public:
    explicit Caster(volatile T* value) : CasterBase<volatile T>(value) {}
};

template <class T>
class Caster<const volatile T> : public CasterBase<const volatile T> {
public:
    explicit Caster(const volatile T* value) : CasterBase<const volatile T>(value) {}
};

template <typename TData>
class DataContainer : protected TData {
public:
    typedef TData DataType;

    DataContainer() : TData() {}

    Caster<DataType> ToData() { return Caster<DataType>(static_cast<DataType*>(this)); }

    Caster<const DataType> ToData() const {
        return Caster<const DataType>(static_cast<const DataType*>(this));
    }

    Caster<volatile DataType> ToData() volatile {
        return Caster<volatile DataType>(static_cast<volatile DataType*>(this));
    }

    Caster<volatile const DataType> ToData() volatile const {
        return Caster<volatile const DataType>(static_cast<volatile const DataType*>(this));
    }

    static Caster<DataContainer<DataType>> DataToAccessor(DataType* pData) {
        return Caster<DataContainer<DataType>>(static_cast<DataContainer<DataType>*>(pData));
    }

    static Caster<const DataContainer<DataType>> DataToAccessor(const DataType* pData) {
        return Caster<const DataContainer<DataType>>(
            static_cast<const DataContainer<DataType>*>(pData));
    }

    static Caster<volatile DataContainer<DataType>> DataToAccessor(volatile DataType* pData) {
        return Caster<volatile DataContainer<DataType>>(
            static_cast<volatile DataContainer<DataType>*>(pData));
    }

    static Caster<volatile const DataContainer<DataType>>
    DataToAccessor(volatile const DataType* pData) {
        return Caster<volatile const DataContainer<DataType>>(
            static_cast<volatile const DataContainer<DataType>*>(pData));
    }

    static Caster<DataContainer<DataType>> DataToAccessor(DataType& data) {
        return DataToAccessor(&data);
    }

    static Caster<const DataContainer<DataType>> DataToAccessor(const DataType& data) {
        return DataToAccessor(&data);
    }

    static Caster<volatile DataContainer<DataType>> DataToAccessor(volatile DataType& data) {
        return DataToAccessor(&data);
    }

    static Caster<volatile const DataContainer<DataType>>
    DataToAccessor(volatile const DataType& data) {
        return DataToAccessor(&data);
    }
};

}  // namespace nn::gfx::detail