#pragma once
#include <nn/types.h>

namespace nn::g3d {
class BindFuncTable {
public:
    struct StringLength {
        size_t length;
        const char* content;
    };
    enum Type { Light, DistanceAttenuation, AngleAttenuation, Fog };
    int lengths[4];
    StringLength* strings[4];
};
}
