#include <erepo/NetworkStatusObserver.h>

#include <erepo/Data/SendData.h>

namespace erepo {

/**
 * Constructs the observer.
 */
NetworkStatusObserver::NetworkStatusObserver() : _8(0) {}

/**
 * Does nothing.
 * @param pHeap Unused.
 */
void NetworkStatusObserver::initialize(sead::Heap* pHeap) {}

/**
 * Does nothing.
 * @param rArg Unused.
 */
void NetworkStatusObserver::update(const Manager::UpdateArg& rArg) {}

/**
 * Sends the internet connection status.
 * @param rId Reporter id.
 * @return Whether the data was queued.
 */
bool NetworkStatusObserver::report(const StringId& rId)
{
    SendData* data = createSendData_(sead::SafeString("erepo_network_status"), 3, rId.getId(), 0,
                                     StringId(), false);
    if (data) {
        data->addInternetConnectionStatus();
        if (data->requestSave()) {
            return true;
        }
    }
    return false;
}

}  // namespace erepo
