#pragma once

class PlayerBindEndParam;

/// How the last bind (being held, carried, ...) ended (implemented by PlayerBindEndParamGetter).
class IUsePlayerBindEndParamGetter {
public:
    virtual bool isBindEndOnGround() const = 0;
    virtual bool isBindEndSquat() const = 0;
    virtual bool isBindEndDeathMapCode() const = 0;
    virtual const PlayerBindEndParam* getBindEndParam() const = 0;
};
