#pragma once
#include <heap/seadDisposer.h>
namespace sead {
class Framework;
}
class RootTask;
class Application {
    SEAD_SINGLETON_DISPOSER(Application)
  public:
    Application();
    void init(int argc, char** ppArgv);
    void run();
    RootTask* getRootTask() const;
    void preSwapBufferCallback();

  private:
    void* mpUnknown20;
    sead::Framework* mpFramework;
};
namespace ApplicationFunction {
void initialize(int argc, char** ppArgv);
}
int AppMain(int argc, char** ppArgv);
static_assert(sizeof(Application) == 0x30);
