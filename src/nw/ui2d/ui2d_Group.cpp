#include <nn/ui2d/ui2d_Group.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/ui2d/ui2d_Pane.h>
#include <new>

namespace nn::ui2d {
Group::Group() : mName(""), mUserAllocated(false) {}
// name remains caller-owned and identifies this group in its container.
Group::Group(const char* name) : mName(name), mUserAllocated(false) {}
// resource supplies the group name and pane-name array; root is searched recursively.
Group::Group(const ResGroup* resource, Pane* root) : mName(resource->name), mUserAllocated(false) {
    const auto* names = reinterpret_cast<const char (*)[24]>(resource + 1);

    for (u32 i = 0; i < resource->paneCount; ++i) {
        Pane* pane = root->FindPaneByName(names[i], true);

        if (pane) AppendPane(pane);
    }
}

// source supplies group membership; root optionally replaces each pane by its
// namesake in another pane tree. A null root retains the original pane pointers.
Group::Group(const Group& source, Pane* root) : mName(source.mName), mUserAllocated(false) {
    if (root) {
        for (const auto& link : source.mPanes)
            AppendPane(root->FindPaneByName(link.pane->mPanelName, true));
    } else {
        for (const auto& link : source.mPanes) AppendPane(link.pane);
    }
}

// pane is referenced by a newly allocated link; the group does not own the pane.
void Group::AppendPane(Pane* pane) {
    void* memory = Layout::AllocateMemory(sizeof(PaneLink));

    if (!memory) return;
    auto* link = new (memory) PaneLink;
    link->pane = pane;
    mPanes.push_back(*link);
}

Group::~Group() {
    for (auto it = mPanes.begin(); it != mPanes.end();) {
        auto current = it++;
        mPanes.erase(current);
        Layout::FreeMemory(&*current);
    }
}

// source is accepted by the copy-test interface; groups impose no extra checks.
bool Group::CompareCopiedInstanceTest(const Group& source) const { return true; }
GroupContainer::~GroupContainer() {
    for (auto it = mGroups.begin(); it != mGroups.end();) {
        auto current = it++;
        mGroups.erase(current);
        Group* group = &*current;

        if (!group->mUserAllocated) {
            group->~Group();
            Layout::FreeMemory(group);
        }
    }
}

// group is linked into the container; its allocation flag controls destruction.
void GroupContainer::AppendGroup(Group* group) { mGroups.push_back(*group); }
// left/right are names compared through the serialized group's 32-byte limit.
static inline bool SameName(const char* left, const char* right) {
    for (size_t i = 0; i < 32; ++i) {
        if (left[i] != right[i]) return false;

        if (!left[i]) break;
    }

    return true;
}

// name selects the first matching group in insertion order.
Group* GroupContainer::FindGroupByName(const char* name) {
    for (auto& group : mGroups) if (SameName(group.mName, name)) return &group;
    return nullptr;
}

// name selects a group without granting mutable access to the container's data.
const Group* GroupContainer::FindGroupByName(const char* name) const {
    for (const auto& group : mGroups) if (SameName(group.mName, name)) return &group;
    return nullptr;
}
}
