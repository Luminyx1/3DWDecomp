#pragma once

class PlayerActionNode;

/// The graph of the player's actions and the shifts between them.
class PlayerActionGraph {
public:
    PlayerActionNode* getAction() const;
    void checkShift();
};
