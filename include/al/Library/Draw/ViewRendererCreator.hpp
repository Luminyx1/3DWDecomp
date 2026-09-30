#pragma once

namespace al {
class GraphicsSystemInfo;
class ViewRenderer;

class ViewRendererCreator {
public:
    ViewRenderer* createViewRenderer(GraphicsSystemInfo* pInfo);
    void deleteViewRenderer(ViewRenderer* pRenderer);
};

}  // namespace al
