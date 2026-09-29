#pragma once

#include <atomic>
#include <basis/seadTypes.h>
#include <thread/seadAtomic.h>

namespace erepo {

template <typename Enum>
class AtomicBitFlag {
public:
    AtomicBitFlag() : mBits(sead::AtomicDirectInitTag{}, 0) {}
    explicit AtomicBitFlag(u32 bits) : mBits(bits) {}

    bool isOn(Enum bit) const { return mBits.isBitOn(bit); }
    bool isOff(Enum bit) const { return !isOn(bit); }
    /// Returns true if the bit was off before.
    bool setOn(Enum bit)
    {
        const u32 mask = 1u << bit;
        return (getRaw().fetch_or(mask, std::memory_order_relaxed) & mask) == 0;
    }
    /// Returns true if the bit was on before.
    bool setOff(Enum bit)
    {
        const u32 mask = 1u << bit;
        return (getRaw().fetch_and(~mask, std::memory_order_relaxed) & mask) != 0;
    }
    bool testAndClear(Enum bit) { return setOff(bit); }

    u32 getDirect() const { return mBits.load(); }
    std::atomic<u32>& getRaw() { return mBits.getRaw(); }
    const std::atomic<u32>& getRaw() const { return mBits.getRaw(); }

private:
    struct Bits : sead::Atomic<u32> {
        using sead::Atomic<u32>::Atomic;
        using sead::Atomic<u32>::AtomicBase;
        std::atomic<u32>& getRaw() { return this->mValue; }
        const std::atomic<u32>& getRaw() const { return this->mValue; }
    };

    Bits mBits;
};

}  // namespace erepo
