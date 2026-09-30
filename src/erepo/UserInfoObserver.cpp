#include <erepo/UserInfoObserver.h>

#include <nn/time.h>
#include <prim/seadEnvUtil.h>

#include <erepo/Data/SendData.h>

namespace erepo {

/**
 * Constructs the observer.
 */
UserInfoObserver::UserInfoObserver() {}

/**
 * Does nothing.
 * @param pHeap Unused.
 */
void UserInfoObserver::initialize(sead::Heap* pHeap) {}

/**
 * Sends the region, language and time zone.
 * @param rId Report id.
 * @return Whether the data was queued.
 */
bool UserInfoObserver::report(const StringId& rId)
{
    SendData* data = createSendData_(sead::SafeString("erepo_region"), 4, 0, 0, rId, true);

    if (!data) {
        return false;
    }

    data->addData(sead::FixedSafeString<63>(sead::SafeString("Region")),
                  static_cast<s32>(sead::EnvUtil::getRegion()));
    data->addData(sead::FixedSafeString<63>(sead::SafeString("Language")),
                  static_cast<s32>(sead::EnvUtil::getLanguage()));

    nn::time::CalendarTime calendarTime;
    nn::time::CalendarAdditionalInfo additionalInfo;
    nn::time::PosixTime posixTime;
    nn::time::StandardUserSystemClock::GetCurrentTime(&posixTime);
    nn::time::ToCalendarTime(&calendarTime, &additionalInfo, posixTime);

    data->addData(sead::FixedSafeString<63>(sead::SafeString("StandardTimeName")),
                  additionalInfo.timeZone.standardTimeName);
    data->addData(sead::FixedSafeString<63>(sead::SafeString("UtcOffsetSeconds")),
                  additionalInfo.timeZone.utcOffset);
    return data->requestSave();
}

}  // namespace erepo
