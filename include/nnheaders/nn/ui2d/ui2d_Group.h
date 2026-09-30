#pragma once

#include <nn/util/util_IntrusiveList.h>

namespace nn::ui2d {
class Pane;
struct ResGroup {
    u32 signature;
    u32 size;
    char name[34];
    u16 paneCount;
};
struct PaneLink {
    nn::util::IntrusiveListNode link;
    Pane* pane;
};
class Group {
public:
    Group();
    explicit Group(const char* name);
    Group(const ResGroup* resource, Pane* root);
    Group(const Group& source, Pane* root);
    virtual ~Group();
    void AppendPane(Pane* pane);
    bool CompareCopiedInstanceTest(const Group& source) const;

    nn::util::IntrusiveListNode mLink;
    using PaneList = nn::util::IntrusiveList<PaneLink,
        nn::util::IntrusiveListMemberNodeTraits<PaneLink, &PaneLink::link>>;
    PaneList mPanes;
    const char* mName;
    bool mUserAllocated;
};
class GroupContainer {
public:
    ~GroupContainer();
    void AppendGroup(Group* group);
    Group* FindGroupByName(const char* name);
    const Group* FindGroupByName(const char* name) const;

    using List = nn::util::IntrusiveList<Group,
        nn::util::IntrusiveListMemberNodeTraits<Group, &Group::mLink>>;
    List mGroups;
};
}
