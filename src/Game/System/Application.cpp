#include "System/Application.hpp"
#include "System/RootTask.hpp"
#include <framework/seadFramework.h>
#include <framework/seadTaskMgr.h>

/**
 * @brief Create an application before framework initialization.
 */
Application::Application() : mpUnknown20(nullptr), mpFramework(nullptr) {}

/**
 * @brief Access the framework's root game task.
 * @return The root task; requires an initialized framework.
 */
RootTask* Application::getRootTask() const {
    return static_cast<RootTask*>(mpFramework->mTaskMgr->mRootTask);
}

/**
 * @brief Handle the pre-swap callback, which performs no work in this build.
 */
void Application::preSwapBufferCallback() {}
