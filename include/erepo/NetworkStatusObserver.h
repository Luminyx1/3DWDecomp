#pragma once

#include <erepo/ObserverBase.h>

namespace erepo {

class NetworkStatusObserver : public ObserverBase {
public:
    NetworkStatusObserver();

    void initialize(sead::Heap* pHeap) override;
    const char* getName() const override { return "NetworkStatus"; }
    void update(const Manager::UpdateArg& rArg) override;
    bool report(const StringId& rId) override;

private:
    s32 _8;
};

}  // namespace erepo
