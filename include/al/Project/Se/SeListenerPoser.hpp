#pragma once

#include "Project/Se/SeListener.hpp"

namespace al {

class SeListenerPoser : public ISeListenerPoser {
public:
    SeListenerPoser(const sead::SafeString& rName, const sead::SafeString& rGroupName);

    const sead::SafeString& getName() const override { return mName; }

    static bool tryCalcViewMatrix(sead::Matrix34f* pMtx, const sead::Vector3f& rPos, const sead::Vector3f& rAt,
                                  const sead::Vector3f& rUp);

private:
    sead::SafeString mName;
};

static_assert(sizeof(SeListenerPoser) == 0x18);

}  // namespace al
