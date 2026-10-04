#pragma once

#include <basis/seadTypes.h>
#include <container/seadPtrArray.h>

namespace al {
class HitSensor;
}  // namespace al

class BindPuppeteer;

/**
 * @brief A fixed-capacity list of bind puppeteers, one per player that can be bound.
 * @note Only what reconstructed code needs is declared so far.
 */
class BindPuppeteerGroup {
public:
    using PuppeteerArray = sead::PtrArray<BindPuppeteer>;

    BindPuppeteerGroup(const char* pName, s32 maxNum);

    void update();
    void registerPuppeteer(BindPuppeteer* pPuppeteer);
    void insertPuppeteer(s32 index, BindPuppeteer* pPuppeteer);
    bool isEndBindAll() const;
    BindPuppeteer* getPuppeteer(s32 index) const;
    BindPuppeteer* getPuppeteerByPlayerIndex(const al::HitSensor* pPlayerSensor) const;
    void erasePuppeteer(BindPuppeteer* pPuppeteer);

    /**
     * @brief Count the registered puppeteers.
     * @return Number of puppeteers in the group.
     */
    s32 getPuppeteerNum() const { return mPuppeteers.size(); }

    /**
     * @brief Get the capacity of the group.
     * @return Maximum number of puppeteers.
     */
    s32 getPuppeteerNumMax() const { return mPuppeteers.capacity(); }

    /// Forget every registered puppeteer (the puppeteers themselves are kept alive).
    void clearPuppeteer() { mPuppeteers.clear(); }

private:
    const char* mName;
    PuppeteerArray mPuppeteers;
};

static_assert(sizeof(BindPuppeteerGroup) == 0x18);
