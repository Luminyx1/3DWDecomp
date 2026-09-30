#include <eui/euiMultiArcResourceAccessor.h>
namespace eui {
// pArchives supplies resource archives; pFonts supplies fonts shared by the layouts.
MultiArcResourceAccessor::MultiArcResourceAccessor(const ArcResourceMgr* pArchives, const FontMgr* pFonts)
    : mArchives(pArchives), mFonts(pFonts) {}
}
