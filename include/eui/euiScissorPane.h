#pragma once

#include <nn/ui2d/ui2d_Pane.h>

namespace eui {

class ScissorPane : public nn::ui2d::Pane {
public:
    ScissorPane();
    ScissorPane(const nn::ui2d::ResPane* pResource, const nn::ui2d::BuildArgSet& rArgs);
    ScissorPane(const ScissorPane& rOther);
    ~ScissorPane() override;
    NN_RUNTIME_TYPEINFO(nn::ui2d::Pane);

    void Draw(nn::ui2d::DrawInfo& rDrawInfo, nn::gfx::CommandBuffer& rCommandBuffer) override;
};

}  // namespace eui
