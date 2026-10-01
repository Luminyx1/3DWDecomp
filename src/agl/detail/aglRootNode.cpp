#include "detail/aglRootNode.h"

#include "common/aglDisplayList.h"
#include "detail/aglPrivateResource.h"
#include "utility/aglDevTools.h"

namespace agl::detail {

SEAD_SINGLETON_DISPOSER_IMPL(RootNode)

/**
 * Constructs the root node with an empty meta suffix.
 */
RootNode::RootNode() : mMetaSuffix(sead::SafeString::cEmptyString) {}

/**
 * Destroys the root node.
 */
RootNode::~RootNode() = default;

/**
 * Sets the meta suffix appended to every node meta string.
 * @param pHeap heap (unused)
 * @param rMetaSuffix suffix, prefixed with a comma when not empty
 */
void RootNode::initialize(sead::Heap* pHeap, const sead::SafeString& rMetaSuffix)
{
    if (rMetaSuffix == sead::SafeString::cEmptyString)
    {
        mMetaSuffix = sead::SafeString::cEmptyString;
    }
    else
    {
        mMetaSuffix = ",";
        mMetaSuffix.append(rMetaSuffix);
    }

    setNodeMeta(this, "Icon = DRAW, Order = -1");
}

/**
 * Builds the meta string of a host IO node (only when a debug heap exists).
 * @param pNode node to set the meta of
 * @param rMeta meta string
 */
void RootNode::setNodeMeta(sead::hostio::Node* pNode, const sead::SafeString& rMeta)
{
    if (sInstance != nullptr && PrivateResource::instance()->getDebugHeap() != nullptr)
    {
        sead::FormatFixedSafeString<1024> meta("%s%s", rMeta.cstr(), sInstance->mMetaSuffix.cstr());
    }
}

/**
 * Appends a node below the agl root node.
 * @param rName name of the node
 * @param pNode node to append
 */
void RootNode::appendChildAGL(const sead::SafeString& rName, sead::hostio::Node* pNode)
{
    if (pNode != nullptr)
    {
        setNodeMeta(pNode, "Icon=NOTE");
    }
}

/**
 * Generates the host IO message with the agl version and the global tool settings.
 * @param pContext host IO context
 */
void RootNode::genMessage(sead::hostio::Context* pContext)
{
    sead::FormatFixedSafeString<1024> version("Version:%d.%d.%d.%d", 0, 3, 0, 0);
    DisplayList::genMessage(pContext);
    utl::DevTools::genMessage(pContext);
}

/**
 * Handles a host IO property event (empty in release builds).
 * @param pEvent property event
 */
void RootNode::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

}  // namespace agl::detail
