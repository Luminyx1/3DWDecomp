#include "System/GameSystem.hpp"
#include "System/Application.hpp"
#include "System/RootTask.hpp"
#include "Library/Sequence/Sequence.hpp"

/**
 * @brief Create the game system before subsystem initialization.
 */
GameSystem::GameSystem()
    : al::NerveExecutor("\u30b2\u30fc\u30e0\u30b7\u30b9\u30c6\u30e0"), mpSequence(nullptr), mpInfo(nullptr),
      mpAudio(nullptr), mpUnknown30(nullptr), mpUnknown38(nullptr), mpUnknown40(nullptr) {}

/**
 * @brief Handle the unused secondary view.
 */
void GameSystem::drawSub() {}

/**
 * @brief Update the active game sequence when one exists.
 */
void GameSystem::exePlay() {
    if (mpSequence != nullptr) {
        mpSequence->update();
    }
}

/**
 * @brief Access the application's game system.
 * @return The game system owned by the initialized root task.
 */
GameSystem* GameSystemFunction::getGameSystem() {
    return Application::instance()->getRootTask()->getGameSystem();
}
