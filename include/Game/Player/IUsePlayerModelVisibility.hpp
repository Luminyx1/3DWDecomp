#pragma once

/// Shows and hides the player's model (implemented by PlayerModelHolder).
class IUsePlayerModelVisibility {
public:
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual bool isHidden() const = 0;
};
