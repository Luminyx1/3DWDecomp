#pragma once

#include "Library/Execute/IUseExecutor.hpp"

namespace al {
class ExecuteDirector;
class GraphicsSystemInfo;

class AtmosScatterDrawer : public IUseExecutor {
public:
    AtmosScatterDrawer(ExecuteDirector* pExecuteDirector, GraphicsSystemInfo* pGraphicsSystemInfo);

    void execute() override {}
    void draw() const override;

private:
    GraphicsSystemInfo* mGraphicsSystemInfo;
};
}  // namespace al
