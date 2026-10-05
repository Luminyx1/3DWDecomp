#pragma once
#include "Library/LiveActor/LiveActor.hpp"
class BlockChoice;
class BlockChoiceWatcher : public al::LiveActor {
public:
    BlockChoiceWatcher(const char*);
    ~BlockChoiceWatcher() override;
    void init(const al::ActorInitInfo&) override;
    void exeWait();
    void disappearOtherBlock(int);
    void exeReleasedItem();
private:
    BlockChoice** mBlocks = nullptr;
    int mBlockCount = 0;
    bool mIsReleasedItem = false;
};
