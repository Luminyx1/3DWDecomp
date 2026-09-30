#include "Library/Play/Layout/SimpleLayoutAppear.hpp"

#include "Library/Layout/LayoutActionFunction.hpp"
#include "Library/Layout/LayoutInitInfo.hpp"

namespace al {
/**
 * Creates a layout that plays its appear action when it appears.
 * @param pName actor name
 * @param pLayoutName layout name
 * @param rInfo layout init info
 * @param pArchiveName archive name, or nullptr to use the layout name
 */
SimpleLayoutAppear::SimpleLayoutAppear(const char* pName, const char* pLayoutName,
                                       const LayoutInitInfo& rInfo, const char* pArchiveName)
    : LayoutActor(pName) {
    initLayoutActor(this, rInfo, pLayoutName, pArchiveName);
}

/**
 * Starts the appear action and makes the layout appear.
 */
void SimpleLayoutAppear::appear() {
    startAction(this, "Appear");
    LayoutActor::appear();
}
}  // namespace al
