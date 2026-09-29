#pragma once

#include <hostio/seadHostIONode.h>
#include <prim/seadSafeString.h>

namespace agl::detail {

class RootNode {
public:
    static void setNodeMeta(sead::hostio::Node* pNode, const sead::SafeString& rMeta);
};

}  // namespace agl::detail
