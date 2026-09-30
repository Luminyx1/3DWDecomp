#pragma once

#include <basis/seadTypes.h>
#include <message/seadMessageProject.h>

extern const char* sProjectDataPath;

namespace al {
class MessageProjectEx : public sead::MessageProject {
public:
    MessageProjectEx();
    ~MessageProjectEx() override = default;

    void init();
    void finalize();
    const char* getTagGroupNameByIndex(s32 groupIndex) const;
    const char* getTagNameByIndex(s32 groupIndex, s32 tagIndex) const;
    const char* getTagParamNameByIndex(s32 groupIndex, s32 tagIndex, s32 paramIndex) const;
    s32 getTagGroupNum() const;
    s32 getTagNum(s32 groupIndex) const;
    s32 getTagParamNum(s32 groupIndex, s32 tagIndex) const;
};
}  // namespace al
