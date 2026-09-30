#pragma once

namespace agl {
class DrawContext;
}

namespace al {
struct ExecuteSystemInitInfo {
    ExecuteSystemInitInfo();

    agl::DrawContext* drawContext;
};
}  // namespace al
