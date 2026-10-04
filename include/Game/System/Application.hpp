#pragma once
#include <heap/seadDisposer.h>
namespace al {
class SystemKit;
}
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

    /**
     * @brief Access the engine system kit.
     * @return The system kit created by the application.
     */
    al::SystemKit* getSystemKit() const { return mpSystemKit; }

    /**
     * @brief Access the application's framework.
     * @return The framework created by the application.
     */
    sead::Framework* getFramework() const { return mpFramework; }

  private:
    al::SystemKit* mpSystemKit;
    sead::Framework* mpFramework;
};
namespace ApplicationFunction {
void initialize(int argc, char** ppArgv);
}
int AppMain(int argc, char** ppArgv);
static_assert(sizeof(Application) == 0x30);
