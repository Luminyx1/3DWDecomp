#pragma once
#include "Library/Nerve/NerveStateBase.hpp"
class TestAoyagiPlayerEntryItem;
/** @brief Runs the entry prompt sequence of an entry test panel. */
class TestAoyagiPlayerEntryStateEntry : public al::NerveStateBase {
public:
    TestAoyagiPlayerEntryStateEntry(const char* pName, TestAoyagiPlayerEntryItem* pHost);
    ~TestAoyagiPlayerEntryStateEntry() override = default;
    void appear() override;
    void exeAppear();
    void exeWait();
    void exeEnd();
    void exeHide();
    bool tryEnd(bool isHidden);
private:
    TestAoyagiPlayerEntryItem* mHost;
    bool mIsHidden = false;
};
static_assert(sizeof(TestAoyagiPlayerEntryStateEntry) == 0x28);
