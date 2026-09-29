#pragma once

#include <nn/ui2d/ui2d_Pane.h>

namespace eui {

class DynamicCapturePane : public nn::ui2d::Pane {
public:
    void freeDynamicTexture();

    nn::util::IntrusiveListNode m_CaptureLink;
};

}  // namespace eui
