#pragma once

#include "Library/Message/IUseMessageSystem.hpp"
#include "Library/Nerve/NerveExecutor.hpp"
#include "Library/Scene/ISceneObj.hpp"

namespace al {
class LayoutInitInfo;
}  // namespace al

/**
 * Scrolls the staff roll (job/name lines and images) shown during the ending.
 * @note Only the members used so far are declared.
 */
class StaffRollLayoutHolder : public al::NerveExecutor,
                              public al::IUseMessageSystem,
                              public al::ISceneObj {
public:
    StaffRollLayoutHolder(const al::LayoutInitInfo& rInfo, bool isSingleMode);
    ~StaffRollLayoutHolder() override;

    const al::MessageSystem* getMessageSystem() const override;
    const char* getSceneObjName() const override;

    void update();
    bool isEnd() const;
    void setup();
    void startAppear();
    void startFin();

private:
    u8 _20[0x70 - 0x20];
};

static_assert(sizeof(StaffRollLayoutHolder) == 0x70);
