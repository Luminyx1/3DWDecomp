#include "System/RootTask.hpp"
#include "System/GameSystem.hpp"

/**
 * @brief Create the root game task.
 * @param rArg Task framework construction parameters.
 */
RootTask::RootTask(const sead::TaskConstructArg& rArg)
    : sead::Task(rArg, "RootTask"), mpGameSystem(nullptr) {}

/**
 * @brief Enter the prepared root task; no additional work is needed.
 */
void RootTask::enter() {}

/**
 * @brief Draw the corresponding game-system view when initialized.
 */
void RootTask::drawTop() {
    if (mpGameSystem != nullptr) {
        mpGameSystem->drawMain();
    }
}

/**
 * @brief Draw the corresponding game-system view when initialized.
 */
void RootTask::drawBtm() {
    if (mpGameSystem != nullptr) {
        mpGameSystem->drawSub();
    }
}

/**
 * @brief Draw the main game view.
 */
void RootTask::draw() { drawTop(); }
