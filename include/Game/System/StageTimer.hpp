#pragma once

/** @brief Stage countdown interface used by the stage-entry timer event. */
class StageTimer {
public:
    bool tryStartHurryUp();
};
