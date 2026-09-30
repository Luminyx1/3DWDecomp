#include <erepo/Reporter.h>

#include <erepo/ObserverBase.h>

namespace erepo {

/**
 * Initializes the observer.
 * @param pHeap Heap for the observer.
 */
void Reporter::initialize(sead::Heap* pHeap)
{
    mObserver->initialize(pHeap);
}

/**
 * Gets the observer name.
 * @return Observer name.
 */
const char* Reporter::getName() const
{
    return mObserver->getName();
}

/**
 * Lets the observer load from the save data info.
 */
void Reporter::load()
{
    mObserver->load();
}

/**
 * Lets the observer save into the save data info.
 * @param pData Unused save data.
 */
void Reporter::save(SaveData* pData)
{
    mObserver->save(pData);
}

/**
 * Updates the observer.
 * @param rArg Frame time and system message.
 */
void Reporter::update(const Manager::UpdateArg& rArg)
{
    mObserver->update(rArg);
}

/**
 * Sends the observer's report if it takes part in the given report timing.
 * @param rId Report id.
 * @return Whether a report was queued.
 */
bool Reporter::report(const StringId& rId)
{
    if (rId == StringId(cDailyReportId)) {
        if (!mTimings.isOn(ETiming::cDaily)) {
            return false;
        }
    } else if (rId == StringId(cStartupReportId)) {
        if (!mTimings.isOn(ETiming::cStartup)) {
            return false;
        }
    }

    return mObserver->report(rId);
}

}  // namespace erepo
