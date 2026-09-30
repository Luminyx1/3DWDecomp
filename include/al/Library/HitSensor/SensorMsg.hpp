#pragma once

#include <prim/seadRuntimeTypeInfo.h>

namespace al {
class SensorMsg {
    SEAD_RTTI_BASE(SensorMsg)

public:
    virtual ~SensorMsg() = default;
};
}  // namespace al

#define SENSOR_MSG(Type)                                                                           \
    class SensorMsg##Type : public al::SensorMsg {                                                 \
        SEAD_RTTI_OVERRIDE(SensorMsg##Type, al::SensorMsg)                                         \
    public:                                                                                        \
        SensorMsg##Type() = default;                                                               \
        ~SensorMsg##Type() override = default;                                                     \
    }

#define SENSOR_MSG_WITH_DATA(Type, DataType, DataName)                                             \
    class SensorMsg##Type : public al::SensorMsg {                                                 \
        SEAD_RTTI_OVERRIDE(SensorMsg##Type, al::SensorMsg)                                         \
    public:                                                                                        \
        SensorMsg##Type(DataType data) : m##DataName(data) {}                                      \
        ~SensorMsg##Type() override = default;                                                     \
        DataType get##DataName() const { return m##DataName; }                                     \
                                                                                                   \
    private:                                                                                       \
        DataType m##DataName;                                                                      \
    }
