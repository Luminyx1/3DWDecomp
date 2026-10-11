#pragma once
#include "Library/Layout/LayoutActor.hpp"
namespace al { class LayoutInitInfo; }
/** @brief Individual player panel in the original entry test. */
class TestAoyagiPlayerEntryItem : public al::LayoutActor {
public:
    TestAoyagiPlayerEntryItem(const al::LayoutInitInfo& rInfo, const char* pName,
                             const char* pPartsName, al::LayoutActor* pParent);
    void setModelFontCurrentCurrent();
    void setColorCharactorModelCurrent();
    /** @brief Selects the controller port for this panel. */
    void setPort(int port) { mPort = port; }
private:
    u8 mUnreconstructed[0x1f];
    int mPort;
};
static_assert(sizeof(TestAoyagiPlayerEntryItem) == 0x148);
