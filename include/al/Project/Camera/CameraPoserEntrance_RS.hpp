#pragma once

#include "Project/Camera/Main/CameraPoser_RS.hpp"

namespace al {
/// The camera used when the player enters a stage.
class CameraPoserEntrance_RS : public CameraPoser_RS {
public:
    CameraPoserEntrance_RS(const char* pName);

    void initParam(f32, f32, f32, const sead::Vector3f&);
    void initParam(f32, const sead::Vector3f&, const sead::Vector3f&);
    void initLookAtPosDirect(const sead::Vector3f&);
    void setStartPos(sead::Vector3f&);

    void start(const CameraStartInfo& rInfo) override;
    void update() override;
    void end() override;
    void loadParam(const ByamlIter& rIter) override;
    bool isEnableRotateByPad() const override;
    void movement() override;

    void exeKeepByFlag();
    void exeKeepInAir();
    void exeWait();

    void* mParam;       // _148
    void* _150;         // _150
    void* _158;         // _158
    void* _160;         // _160
    void* _168;         // _168
    void* _170;         // _170
    s32 _178;           // _178
    u8 _17C[0x198 - 0x17C];  // _17C
};
}  // namespace al
