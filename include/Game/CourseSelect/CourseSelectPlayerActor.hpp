#pragma once
#include "Player/Normal/PlayerActor.hpp"

// Course-map player interface; the derived actor's fields remain unrecovered.
class CourseSelectPlayerActor : public PlayerActor {
public:
    bool isReceivePush() const;
};
