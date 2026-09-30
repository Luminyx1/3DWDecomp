#include <basis/seadRawPrint.h>
#include <mc/seadCoreInfo.h>

namespace sead
{
u32 CoreInfo::sNumCores = 1;
u32 CoreInfo::sPlatformCoreId[32]{};
CoreId CoreInfo::sCoreIdFromPlatformCoreIdTable[32]{};
#ifdef NNSDK
nn::os::TlsSlot CoreInfo::sCoreNumberTlsSlot{};
#endif

namespace
{
/// Configures CoreInfo during static initialization, after the tables above are set up.
struct CoreInfoInitializer
{
    CoreInfoInitializer() { CoreInfo::configure(); }
};

CoreInfoInitializer sInitializer;
}  // namespace

/**
 * @return how many cores the mask contains
 */
u32 CoreIdMask::countOnBits() const
{
    u32 x = mMask;
    x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x & 0x07070707) + ((x >> 4) & 0x07070707);
    x = (x & 0x000F000F) + ((x >> 8) & 0x000F000F);
    x = (x & 0x1F) + (x >> 16);
    return x;
}

/**
 * Sets up the Switch's three application cores and the TLS slot that stores the current core.
 */
void CoreInfo::configure()
{
#ifdef NNSDK
    sNumCores = 3;
    sPlatformCoreId[0] = 0;
    sPlatformCoreId[1] = 1;
    sPlatformCoreId[2] = 2;

    SEAD_ASSERT(nn::os::GetCurrentCoreNumber() == 0);
    const auto allocResult = nn::os::AllocateTlsSlot(&sCoreNumberTlsSlot, nullptr);
    SEAD_ASSERT(allocResult.IsSuccess());

    for (size_t i = 0; i != sNumCores; ++i)
    {
        const u32 id = sPlatformCoreId[i];
        sCoreIdFromPlatformCoreIdTable[id] = i;
    }
#else
#error "Unknown platform"
#endif
}

/**
 * Prints the core layout (debug builds only).
 */
void CoreInfo::dump()
{
    SEAD_DEBUG_PRINT("* num cores %d\n", sNumCores);
    for (u32 i = 0; i < sNumCores; ++i)
    {
        SEAD_DEBUG_PRINT("  [%d] : %s : PlatformCoreId=%d\n", i, i == 0 ? "Main" : "Sub ", sPlatformCoreId[i]);
    }

    SEAD_DEBUG_PRINT("all mask : %x\n", u32(getMaskAll()));
    SEAD_DEBUG_PRINT("all sub mask : %x\n", u32(getMaskSubAll()));
}

/**
 * @param id core
 * @return the platform affinity mask for the core
 */
u32 CoreInfo::getPlatformMask(CoreId id)
{
    return 1 << getPlatformCoreId(id);
}

}  // namespace sead
