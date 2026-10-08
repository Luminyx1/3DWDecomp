#pragma once

#include "NPC/IUseNekoModeActor.hpp"

class Neko;
class NekoParent;

/**
 * @brief Mode actor of a regular cat that wanders around and can be carried to its parent.
 * @note Only what reconstructed code needs is declared so far.
 */
class NekoNormal : public IUseNekoModeActor {
public:
    NekoNormal(Neko* pHost);

    bool isEnableGoal() const;
    bool isAtGoal() const;
    bool canCollect() const;
    void setNekoParent(const NekoParent* pParent);

    Neko* getHost() const { return mHost; }

private:
    Neko* mHost;  // 0x158
};
