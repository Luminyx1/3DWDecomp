#pragma once

#include <nn/types.h>

namespace Vessel::Emulator::Virtual {

/**
 * @brief A component of an emulated platform (CPU, PPU, sound unit, ...).
 *
 * Units talk to each other through System, Access and Process calls that carry a UArgument.
 */
class CPlatformUnit {
public:
    /// Operations passed to Access.
    enum Operation {
        cOperation_Read = 0,
        cOperation_Write = 1,
    };

    /// Results returned by System, Access and Process.
    enum Result {
        cResult_Success = 0,
        cResult_Unhandled = 4,
    };

    /**
     * @brief Payload of a System, Access or Process call; which view is valid depends on the call.
     */
    union UArgument {
        /// Access: a bus transfer.
        struct {
            u64 mSize;
            u64 mCount;
            u64 mAddress;
            u64 mReserved;
            void* mData;
        } mAccess;

        /// Process: the events that happened since the last call.
        struct {
            u64 mEvents;
            u64 mReserved;
            u64* mCycles;
        } mProcess;

        /// System: a command with a value.
        struct {
            u64 mCommand;
            u64 mCount;
            u64 mValue;
            u64 mReserved;
        } mSystem;

        u64 mRaw[8];
    };

    /**
     * @brief Identification data of a unit.
     */
    struct SDescriptor {
        char mName[16];
        u64 mIndex;
        u64 mFlags;
        u8 mKind;
    };

    /**
     * @brief Construct a unit with a cleared descriptor.
     * @param index The unit index.
     * @param flags The unit flags.
     */
    CPlatformUnit(u64 index, u64 flags) {
        mDescriptor = {};
        mDescriptor.mIndex = index;
        mDescriptor.mFlags = flags;
    }

    /// Destroy the unit, clearing its descriptor.
    virtual ~CPlatformUnit() { mDescriptor = {}; }

    virtual void Initialize(CPlatformUnit* pParent);
    virtual void Finalize();
    virtual int Notify(CPlatformUnit* pUnit, u64 channel, u64 operation, void* pArgument);
    virtual int SystemAttach(CPlatformUnit* pUnit, u64 channel, u64 operation,
                             UArgument* pArgument);
    virtual int SystemDetach(CPlatformUnit* pUnit, u64 channel, u64 operation,
                             UArgument* pArgument);
    virtual int System(CPlatformUnit* pUnit, u64 channel, u64 operation, UArgument* pArgument);
    virtual int Access(CPlatformUnit* pUnit, u64 channel, u64 operation, UArgument* pArgument);
    virtual int Process(CPlatformUnit* pUnit, u64 channel, u64 operation, UArgument* pArgument);
    virtual bool ExportContent(void* pBuffer, u64 bufferSize, u64& rSize);
    virtual bool ImportContent(const void* pBuffer, u64 bufferSize, u64& rSize);
    virtual bool InferContentSize(u64& rSize);

    SDescriptor mDescriptor;
};

static_assert(sizeof(CPlatformUnit) == 0x30, "CPlatformUnit size");

}  // namespace Vessel::Emulator::Virtual
