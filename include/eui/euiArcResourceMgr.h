#pragma once

#include <basis/seadTypes.h>

namespace eui {
class ArcResourceMgr {
public:
    ArcResourceMgr();

    static void finalizeInitializedShaderResource(void* pArchive);
    static void reinitializeShaderResource(void* pArchive);

private:
    u8 _0[0x20];
};
}  // namespace eui
