#pragma once

#include <basis/seadTypes.h>
#include <prim/seadSafeString.h>

class GameDataHolder;

namespace erepo {
class EControllerStyle;
}  // namespace erepo

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
    bool Initialize(GameDataHolder* pHolder, int num);
    void Update();
    void UpdateStyle(erepo::EControllerStyle style);
    void requestSaveData();
    bool EventBegin(KeyEventType type, int eventId, int option, bool isForce);
    void EventEnd();
    void KeySetValue(Key key, int value);
    void KeySetValue(Key key, f32 value);
    void KeySetValue(Key key, s64 value);
    void KeySetValue(Key key, sead::SafeString& rValue);
    void KeySetValue(Key key, int* pValues, int num);
    void KeySetValue(Key key, f32* pValues, int num);
    void sendNetworkStatus();
    void addSessionId();
};

} // namespace preport
