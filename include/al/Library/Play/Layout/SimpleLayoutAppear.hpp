#pragma once

#include "Library/Layout/LayoutActor.hpp"

namespace al {
class LayoutInitInfo;

class SimpleLayoutAppear : public LayoutActor {
public:
    SimpleLayoutAppear(const char* pName, const char* pLayoutName, const LayoutInitInfo& rInfo,
                       const char* pArchiveName);

    void appear() override;
};
}  // namespace al
