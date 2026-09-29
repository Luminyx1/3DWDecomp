#pragma once

#include <erepo/ObserverBase.h>

namespace erepo {

class UserInfoObserver : public ObserverBase {
public:
    UserInfoObserver();

    void initialize(sead::Heap* pHeap) override;
    const char* getName() const override { return "UserInfo"; }
    bool report(const StringId& rId) override;
};

}  // namespace erepo
