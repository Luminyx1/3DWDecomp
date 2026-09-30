#include <erepo/ObserverBase.h>

#include <erepo/Data/SendData.h>
#include <erepo/Manager.h>

namespace erepo {

namespace {
sead::FixedSafeString<63> sSendLogName(sead::SafeString("SendKibana"));
}

/**
 * Creates send data and tags it for logging when logging is enabled.
 * @param rEventId Play report event id.
 * @param dataNum Number of data entries.
 * @param arrayNum Number of arrays.
 * @param structNum Number of structs.
 * @param rId Reporter id.
 * @param isNothrow Whether creation must not fail.
 * @return Created send data, or nullptr.
 */
SendData* ObserverBase::createSendData_(const sead::FixedSafeString<31>& rEventId, s32 dataNum,
                                        s32 arrayNum, s32 structNum, const StringId& rId,
                                        bool isNothrow)
{
    SendData* data =
        SendData::CreateSendData(rEventId, dataNum, arrayNum, structNum, rId, isNothrow);
    if (data && Manager::instance() && Manager::instance()->isFlagOn(Manager::EFlag::cEnableLog)) {
        data->addLog(sSendLogName, true);
    }

    return data;
}

/**
 * Creates send data from a creation argument.
 * @param rArg Creation argument.
 * @return Created send data, or nullptr.
 */
SendData* ObserverBase::createSendData_(const SendDataBase::CreateArg& rArg)
{
    SendData* data = SendData::CreateSendData(rArg.eventId, rArg.dataNum, rArg.arrayNum,
                                              rArg.structNum, rArg.reporterId, rArg.isNothrow);
    if (data && Manager::instance() && Manager::instance()->isFlagOn(Manager::EFlag::cEnableLog)) {
        data->addLog(sSendLogName, true);
    }

    return data;
}

}  // namespace erepo
