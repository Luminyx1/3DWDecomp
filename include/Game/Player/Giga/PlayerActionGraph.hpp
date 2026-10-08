#pragma once

class PlayerAction;

/// The graph of the player's actions and the shifts between them.
class PlayerActionGraph {
public:
    const PlayerAction* getAction() const;
    void checkShift();
};
