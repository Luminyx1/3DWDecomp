#pragma once

#include "LuigiBros/Emulator/PlatformUnit.hpp"

namespace Vessel::Emulator::Virtual::PlatformHvc {

/**
 * @brief Base of the units of the emulated Famicom (HVC) platform.
 */
class CPlatformHvcUnit : public CPlatformUnit {
public:
    /// Mapper numbers stored in _SEntity::mMapper.
    enum Mapper {
        cMapper_Mmc2 = 9,
        cMapper_Mmc4 = 10,
    };

    /// Offsets of the PPU and I/O registers inside _SEntity::mMemory.
    enum MemoryOffset {
        cMemory_PpuRegister = 0x303800,
        cMemory_PpuLatch = 0x303808,
        cMemory_IoRegister = 0x303810,
        cMemory_IoShadow = 0x303a10,
        cMemory_Oam = 0x304410,
        cMemory_Palette = 0x304510,
    };

    /**
     * @brief One entry of a bus page table.
     */
    struct _SPage {
        u8 mType;
        u8 mLimit;
        u8 _02[6];
        u64 mOffset;
        u8 _10;
        u8 mLine;
        u8 _12[0xe];
    };

    static_assert(sizeof(_SPage) == 0x20, "_SPage size");

    /// Scroll position forced on one scanline (values above 0x1ff mean "no override").
    struct _SLineScroll {
        u16 mX;
        u16 mY;
    };

    /**
     * @brief Per-title settings of the emulated cartridge.
     */
    struct _STitleSetting {
        u8 _00[0x30];
        u64 mTitleId;
    };

    /**
     * @brief Per-title rendering parameters.
     */
    struct _STitleRender {
        u8 _00[0x32];
        u8 mVisibleTop;
        u8 mVisibleBottom;
        u8 mSpriteLimit;
        u8 mIsSprite0HitAlways;
        _SLineScroll mLineScroll[240];
    };

    /**
     * @brief State of the emulated machine shared by all of its units.
     */
    struct _SEntity {
        /// Whether the cartridge uses the MMC2 or MMC4 character latch.
        bool IsMmc2orMmc4() const {
            return mMapper == cMapper_Mmc2 || mMapper == cMapper_Mmc4;
        }

        /**
         * @brief Get a pointer into PPU address space.
         * @param address The PPU address.
         * @return The backing memory of that address.
         */
        u8* GetPpuPointer(u64 address) const {
            return mMemory + mPpuPages[address >> 10].mOffset + (address & 0x3ff);
        }

        u8 mMapper;
        u8 _01[7];
        _SPage mCpuPages[16];
        _SPage mPpuPages[80];
        u64 _c08;
        u64 mFrameClock;
        u64 _c18;
        u64 mScanlineClock;
        u8 _c28[0x1648 - 0xc28];
        u64 mDmaCycles;
        u8 _1650[0x1718 - 0x1650];
        u16 mResetRequest;
        u16 _171a;
        u16 mInterruptRequest;
        u8 _171e[0x1728 - 0x171e];
        u8* mMemory;
        u8* mFrameBuffer;
        u8 _1738[0x1758 - 0x1738];
        _STitleSetting* mTitleSetting;
        _STitleRender* mTitleRender;
    };

    static_assert(__builtin_offsetof(_SEntity, mPpuPages) == 0x208, "_SEntity::mPpuPages");
    static_assert(__builtin_offsetof(_SEntity, mScanlineClock) == 0xc20, "_SEntity::mScanlineClock");
    static_assert(__builtin_offsetof(_SEntity, mTitleRender) == 0x1760, "_SEntity::mTitleRender");

    /**
     * @brief Construct a unit of the given kind.
     * @param kind The unit kind.
     * @param index The unit index.
     * @param flags The unit flags.
     */
    CPlatformHvcUnit(u8 kind, u64 index, u64 flags) : CPlatformUnit(index, flags) {
        mParent = nullptr;
        _38 = false;
        mDescriptor.mKind = kind;
        mEntity = nullptr;
    }

    ~CPlatformHvcUnit() override {}

    void Initialize(CPlatformUnit* pParent) override;
    void Finalize() override;
    int Notify(CPlatformUnit* pUnit, u64 channel, u64 operation, void* pArgument) override;
    int SystemAttach(CPlatformUnit* pUnit, u64 channel, u64 operation,
                     UArgument* pArgument) override;
    int SystemDetach(CPlatformUnit* pUnit, u64 channel, u64 operation,
                     UArgument* pArgument) override;
    int System(CPlatformUnit* pUnit, u64 channel, u64 operation, UArgument* pArgument) override;
    int Access(CPlatformUnit* pUnit, u64 channel, u64 operation, UArgument* pArgument) override;
    int Process(CPlatformUnit* pUnit, u64 channel, u64 operation, UArgument* pArgument) override;
    bool ExportContent(void* pBuffer, u64 bufferSize, u64& rSize) override;
    bool ImportContent(const void* pBuffer, u64 bufferSize, u64& rSize) override;
    bool InferContentSize(u64& rSize) override;

    virtual int _Prologue() = 0;
    virtual int _Epilogue() = 0;
    virtual void _SerializeCoreExport(void* pBuffer) = 0;
    virtual void _SerializeCoreImport(const void* pBuffer) = 0;
    virtual u64 _SerializeCoreInferSize() = 0;
    virtual u8 _SerializeTagContent() = 0;
    virtual u8 _SerializeTagSpecies() = 0;
    virtual u16 _SerializeTagVersion() = 0;
    virtual const char* _SerializeTagComment() = 0;

    CPlatformUnit* mParent;
    bool _38;
    _SEntity* mEntity;
};

static_assert(sizeof(CPlatformHvcUnit) == 0x48, "CPlatformHvcUnit size");

}  // namespace Vessel::Emulator::Virtual::PlatformHvc
