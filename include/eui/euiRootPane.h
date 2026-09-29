#pragma once

#include <nn/ui2d/ui2d_Pane.h>

namespace eui {
class LayoutEx;

class RootPane : public nn::ui2d::Pane {
public:
    explicit RootPane(LayoutEx* pLayout);
    RootPane(const nn::ui2d::ResPane* pResource, const nn::ui2d::BuildArgSet& rArgs);
    RootPane(const RootPane& rOther, LayoutEx* pLayout);
    ~RootPane() override;
    NN_RUNTIME_TYPEINFO(nn::ui2d::Pane);

    LayoutEx* m_pLayout;
};

}  // namespace eui
