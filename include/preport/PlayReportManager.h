#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

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
    bool EventBegin(KeyEventType type, int eventId, int option, bool isForce);
    void EventEnd();
    void KeySetValue(Key key, int value);
    void KeySetValue(Key key, s64 value);
    void KeySetValue(Key key, sead::SafeString& rValue);
};

} // namespace preport
