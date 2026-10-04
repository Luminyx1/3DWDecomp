#pragma once
#include <framework/seadTask.h>
class GameSystem;
class RootTask : public sead::Task {
  public:
    explicit RootTask(const sead::TaskConstructArg& rArg);
    void prepare() override;
    void enter() override;
    void calc() override;

    /**
     * @brief Draw the main game view.
     */
    void draw() override { drawTop(); }

    virtual void drawTop();
    virtual void drawBtm();

    /**
     * @brief Access the game system owned by the root task.
     * @return The game system, or nullptr before preparation.
     */
    GameSystem* getGameSystem() const { return mpGameSystem; }

  private:
    GameSystem* mpGameSystem;
};
static_assert(sizeof(RootTask) == 0x208);
