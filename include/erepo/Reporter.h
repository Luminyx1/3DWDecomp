#pragma once

#include <erepo/Data/AtomicBitFlag.h>
#include <erepo/Manager.h>
#include <erepo/Types.h>

namespace erepo {

class ObserverBase;
class SaveData;

class Reporter {
public:
    SEAD_ENUM(ETiming, cDaily, cStartup)

    static constexpr u32 cStartupReportId = 0xa4795224;
    static constexpr u32 cDailyReportId = 0xcadb6581;

    explicit Reporter(ObserverBase* pObserver) : mObserver(pObserver) {}

    void initialize(sead::Heap* pHeap);
    const char* getName() const;
    void load();
    void save(SaveData* pData);
    void update(const Manager::UpdateArg& rArg);
    bool report(const StringId& rId);

    void setTimingOn(ETiming timing) { mTimings.setOn(timing); }

private:
    ObserverBase* mObserver;
    AtomicBitFlag<ETiming> mTimings;
};

}  // namespace erepo
