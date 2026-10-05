#pragma once

class IGoalObj {
public:
    virtual bool isGoal() const = 0;
    virtual bool isEndGoalDemo() const = 0;
    virtual bool isUseResult() const = 0;
    virtual bool isRetireGoal() const = 0;
};
