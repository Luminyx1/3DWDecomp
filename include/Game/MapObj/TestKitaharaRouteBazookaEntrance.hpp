#pragma once
#include "Project/Block/BlockRailParts.hpp"
class TestKitaharaRouteBazooka;
class TestKitaharaRouteBazookaEntrance : public al::BlockRailParts {
public:
    TestKitaharaRouteBazookaEntrance(const char*, const char*);
    void setInitQT(const sead::Quatf&, const sead::Vector3f&);
    void setHost(TestKitaharaRouteBazooka*);
private:
    u8 mUnreconstructed[0x30];
};
static_assert(sizeof(TestKitaharaRouteBazookaEntrance) == 0x1b0);
