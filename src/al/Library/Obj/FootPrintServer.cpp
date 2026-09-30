#include "Library/Obj/FootPrintServer.hpp"

#include "Library/LiveActor/Util/ActorFlagUtil.hpp"
#include "Project/Obj/FootPrint.hpp"

namespace al {
/**
 * Creates the foot prints.
 * @param rInfo actor init info
 * @param pArchiveName foot print archive name
 * @param num foot print count
 */
FootPrintServer::FootPrintServer(const ActorInitInfo& rInfo, const char* pArchiveName, s32 num) {
    mFootPrints = new sead::PtrArray<FootPrint>();
    mFootPrints->allocBuffer(num, nullptr);

    for (s32 i = 0; i < mFootPrints->capacity(); i++) {
        mFootPrints->pushBack(new FootPrint(rInfo, pArchiveName));
    }
}

/**
 * Finds a dead foot print.
 * @return dead foot print, or null
 */
FootPrint* FootPrintServer::findDeadFootPrint() {
    for (s32 i = 0; i < mFootPrints->size(); i++) {
        if (isDead(mFootPrints->at(i))) {
            return mFootPrints->at(i);
        }
    }

    return nullptr;
}
}  // namespace al
