#include "System/Application.hpp"
#include <heap/seadHeapMgr.h>

/**
 * @brief Initialize and run the application using the first root heap.
 * @param argc Number of startup arguments; zero for the console entry point.
 * @param ppArgv Startup argument array; nullptr when argc is zero.
 * @return Zero after the application run completes.
 */
int AppMain(int argc, char** ppArgv) {
    ApplicationFunction::initialize(argc, ppArgv);
    {
        sead::ScopedCurrentHeapSetter setter(sead::HeapMgr::getRootHeap(0));
        Application::createInstance(nullptr);
    }
    Application::instance()->init(argc, ppArgv);
    Application::instance()->run();
    return 0;
}
