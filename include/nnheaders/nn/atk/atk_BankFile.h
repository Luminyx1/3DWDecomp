#pragma once
#include <attributes.h>
#include <nn/atk/atk_BinaryFileFormat.h>
#include <nn/atk/atk_BinaryFileUtil.h>
#include <nn/atk/atk_WaveSoundFile.h>

namespace nn::atk::detail {
struct WaveId {
    u32 waveArchiveId;
    u32 waveIndex;
};

struct WaveIdTable {
    u32 count;
    WaveId items[1];
};
struct BankFile {
    struct ReferenceTable {
        u32 count;
        Reference items[1];
    };
    struct VelocityRegion {
        struct RegionParameter {
            u8 originalKey, _1[3];
            u8 volume, _5[3];
            u8 pan, _9[3];
            float pitch;
            u8 ignoreNoteOff, keyGroup;
            u16 interpolationType;
            u32 envelopeOffset;
            u32 envelopeReferenceType;
            s32 envelopeReferenceOffset;
            AdshrCurve envelope;
        };
        u32 waveIndex;
        u32 flags;
        u32 values[1];
        NOINLINE int GetOriginalKey() const;
        NOINLINE int GetVolume() const;
        NOINLINE int GetPan() const;
        NOINLINE float GetPitch() const;
        NOINLINE bool IsIgnoreNoteOff() const;
        NOINLINE int GetKeyGroup() const;
        NOINLINE int GetInterpolationType() const;
        NOINLINE const AdshrCurve* GetAdshrCurve() const;
        NOINLINE const RegionParameter* GetRegionParameter() const;
    };
    struct KeyRegion {
        Reference regions;
        NOINLINE const VelocityRegion* GetVelocityRegion(u32 velocity) const;
    };
    struct Instrument {
        Reference regions;
        NOINLINE const KeyRegion* GetKeyRegion(u32 key) const;
    };
    struct InfoBlockBody {
        Reference waveIds, instruments;
        NOINLINE const WaveIdTable* GetWaveIdTable() const;
        NOINLINE const ReferenceTable* GetInstrumentReferenceTable() const;
        NOINLINE const Instrument* GetInstrument(int index) const;
    };
    struct InfoBlock {
        u32 signature, size;
        InfoBlockBody body;
    };
    struct FileHeader : BinaryFileHeader {
        ReferenceWithSize blocks[1];
        NOINLINE const InfoBlock* GetInfoBlock() const;
    };
};
namespace bank {
using file::AtOffset;
using file::GetFloatParameter;
using file::GetParameter;
using file::GetParameterIndex;
const AdshrCurve DefaultAdshrCurve = {127, 127, 127, 127, 127};

/**
 * @brief Finds the first split-table boundary covering a key or velocity.
 * @param pTable Table count followed by its byte-sized upper boundaries.
 * @param value Key or velocity to locate, normally in [0, 127].
 * @return Matching boundary index, or -1 when no boundary covers the value.
 */
inline int FindRegionIndex(const u8* pTable, u32 value) {
    const auto* pCount = reinterpret_cast<const u32*>(pTable);
    for (u32 index = 0; index < *pCount; ++index) {
        if (pTable[4 + static_cast<size_t>(index)] >= value) {
            return index;
        }
    }
    return -1;
}
/**
 * @brief Selects a key or velocity region from a direct, split or indexed table.
 * @tparam T Region record type.
 * @param pBase Instrument or key-region base used by the table reference.
 * @param rReference Table type and unsigned byte displacement from pBase.
 * @param value Key or velocity to select, normally in [0, 127].
 * @return Matching region record, or nullptr when the table type or range is unsupported.
 */
template <typename T>
inline const T* SelectRegion(const void* pBase, const Reference& rReference, u32 value) {
    u16 type = rReference.type;
    const u8* pTable = AtOffset<u8>(pBase, static_cast<u32>(rReference.offset));
    switch (type) {
    case 0x6000: {
        const auto* pReference = reinterpret_cast<const Reference*>(pTable);
        return AtOffset<T>(pTable, pReference->offset);
    }
    case 0x6001: {
        int index = FindRegionIndex(pTable, value);
        if (index == -1) {
            return nullptr;
        }
        u32 count = *reinterpret_cast<const u32*>(pTable);
        size_t displacement =
            ((count + 3) & ~3u) + (sizeof(u32) + sizeof(Reference) * static_cast<ptrdiff_t>(index));
        return AtOffset<T>(pTable, AtOffset<Reference>(pTable, displacement)->offset);
    }
    case 0x6002:
        if (pTable[0] > value) {
            return nullptr;
        }
        if (pTable[1] < value) {
            return nullptr;
        }
        return AtOffset<T>(pTable, AtOffset<Reference>(pTable, 4)[value - pTable[0]].offset);
    default:
        return nullptr;
    }
}

/**
 * @brief Resolves a bank wave index and copies its valid wave identifier.
 * @param pInfo Initialized bank INFO body.
 * @param pOut Non-null destination for the archive and wave indices.
 * @param waveIndex Index within the wave identifier table.
 * @return True if the index exists and refers to a valid wave.
 */
inline bool ReadWaveId(const BankFile::InfoBlockBody* pInfo, WaveId* pOut, u32 waveIndex) {
    const auto* pWaves = pInfo->GetWaveIdTable();
    if (pWaves->count <= waveIndex || pWaves->items[waveIndex].waveIndex == 0xffffffff) {
        return false;
    }
    pOut->waveArchiveId = pWaves->items[waveIndex].waveArchiveId;
    pOut->waveIndex = pWaves->items[waveIndex].waveIndex;
    return true;
}
} // namespace bank

/** @brief Finds the bank INFO block. @return INFO block, or nullptr when its reference is absent or zero. */
inline const BankFile::InfoBlock* BankFile::FileHeader::GetInfoBlock() const {
    for (size_t i = 0; i < blockCount; ++i) {
        if (blocks[i].type == 0x5800) {
            s32 offset = blocks[i].offset;
            return offset != 0 ? file::AtOffset<InfoBlock>(this, offset) : nullptr;
        }
    }
    return nullptr;
}

/** @brief Resolves the INFO block's wave table. @return Referenced wave identifier table. */
inline const WaveIdTable* BankFile::InfoBlockBody::GetWaveIdTable() const {
    return file::AtOffset<WaveIdTable>(this, waveIds.offset);
}

/** @brief Resolves the instrument table. @return Instrument count and relative references. */
inline const BankFile::ReferenceTable* BankFile::InfoBlockBody::GetInstrumentReferenceTable() const {
    return file::AtOffset<ReferenceTable>(this, instruments.offset);
}

/**
 * @brief Resolves an instrument after checking its reference tag and table bounds.
 * @param index Nonnegative instrument index in the reference table.
 * @return Instrument record, or nullptr for a wrong tag or out-of-range index.
 */
inline const BankFile::Instrument* BankFile::InfoBlockBody::GetInstrument(int index) const {
    const auto* pTable = file::AtOffset<ReferenceTable>(this, instruments.offset);
    if (pTable->items[index].type != 0x5900) {
        return nullptr;
    }
    if (static_cast<u32>(index) >= pTable->count) {
        return nullptr;
    }
    return file::AtOffset<Instrument>(pTable, pTable->items[static_cast<u32>(index)].offset);
}

/**
 * @brief Selects an instrument's key region.
 * @param key Note key, normally in [0, 127].
 * @return Selected key region, or nullptr when no region covers the key.
 */
inline const BankFile::KeyRegion* BankFile::Instrument::GetKeyRegion(u32 key) const {
    return bank::SelectRegion<KeyRegion>(this, regions, key);
}

/**
 * @brief Selects a key region's velocity layer.
 * @param velocity Note velocity, normally in [0, 127].
 * @return Selected velocity region, or nullptr when no region covers the velocity.
 */
inline const BankFile::VelocityRegion* BankFile::KeyRegion::GetVelocityRegion(u32 velocity) const {
    return bank::SelectRegion<VelocityRegion>(this, regions, velocity);
}

/** @brief Reads the sample reference key. @return Stored key, or middle C (60) when absent. */
inline int BankFile::VelocityRegion::GetOriginalKey() const {
    return flags & 1 ? file::GetParameter(&flags, 0) : 60;
}
/** @brief Reads instrument gain. @return Stored volume, or full volume (127) when absent. */
inline int BankFile::VelocityRegion::GetVolume() const {
    return flags & 2 ? file::GetParameter(&flags, 1) : 127;
}
/** @brief Reads instrument pan. @return Stored pan, or center (64) when absent. */
inline int BankFile::VelocityRegion::GetPan() const { return flags & 4 ? file::GetParameter(&flags, 2) : 64; }
/** @brief Reads instrument pitch scaling. @return Stored multiplier, or unity when absent. */
inline float BankFile::VelocityRegion::GetPitch() const {
    return flags & 8 ? file::GetFloatParameter(&flags, 3) : 1.0f;
}
/** @brief Checks the note-off behavior. @return Whether the stored note-off-ignore byte is nonzero. */
inline bool BankFile::VelocityRegion::IsIgnoreNoteOff() const {
    return flags & 16 ? static_cast<u8>(file::GetParameter(&flags, 4)) != 0 : false;
}
/** @brief Reads the exclusive key group. @return Stored group field, or zero when absent. */
inline int BankFile::VelocityRegion::GetKeyGroup() const {
    return flags & 16 ? file::GetParameter(&flags, 4) >> 8 : 0;
}
/** @brief Reads the sample interpolation mode. @return Stored mode, or zero when absent. */
inline int BankFile::VelocityRegion::GetInterpolationType() const {
    return flags & 16 ? file::GetParameter(&flags, 4) >> 16 : 0;
}

/** @brief Resolves the optional ADSHR curve. @return Stored curve, or the all-127 default envelope. */
inline const AdshrCurve* BankFile::VelocityRegion::GetAdshrCurve() const {
    u32 index = file::GetParameterIndex(flags, 9);
    if (index == 0) {
        return &bank::DefaultAdshrCurve;
    }
    const auto* pReference = file::AtOffset<Reference>(this, (&flags)[index]);
    return file::AtOffset<AdshrCurve>(pReference, pReference->offset);
}

/** @brief Gets the contiguous common parameter layout. @return Packed parameters only when the flags equal
 * 0x21f. */
inline const BankFile::VelocityRegion::RegionParameter* BankFile::VelocityRegion::GetRegionParameter() const {
    return flags == 0x21f ? reinterpret_cast<const RegionParameter*>(values) : nullptr;
}
} // namespace nn::atk::detail
