#pragma once

#include <basis/seadTypes.h>

namespace preport {

/**
 * @brief Identifier of a value recorded in a play-report event.
 */
enum Key : s32 {};

/**
 * @brief Kind of a play-report event.
 */
enum KeyEventType : s32 {};

class PlayReportManager {
  public:
    void requestSaveData();
};

} // namespace preport
