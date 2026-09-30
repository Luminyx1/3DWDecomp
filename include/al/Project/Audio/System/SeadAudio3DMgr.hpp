#pragma once

#include <audio/seadAudio3DMgrNin.h>
#include <math/seadVector.h>

namespace al {
class SeadAudio3DMgr : public sead::Audio3DMgrNin {
public:
    const sead::Vector3f& getListenerPosition() const;
};
}  // namespace al
