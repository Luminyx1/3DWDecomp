#pragma once

/// An action that can report that it finished.
class IUsePlayerActionEnd {
public:
    virtual bool isEnd() const = 0;
};
