#pragma once
namespace nn::ui2d {
class Group;
class GroupContainer {
public:
    Group* FindGroupByName(const char* pName);
};
}
