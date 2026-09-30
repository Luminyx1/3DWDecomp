#include <nn/ui2d/ui2d_ArcExtractor.h>
#include <nn/font/font_ResFont.h>
#include <nn/util/util_BinaryFormat.h>
#include <nn/util.h>
#include <nn/util/util_StringUtil.h>
#include <cstring>

namespace nn::ui2d {
// reverse requests byte swapping; value is the serialized 16-bit archive field.
static inline s16 Read16(bool reverse, s16 value) {
    return reverse ? __builtin_bswap16(value) : value;
}

// value is the serialized 32-bit field; endian identifies the archive byte order.
static inline u32 Read32(const s32& value, EndianTypes endian) {
    return endian == EndianTypes_Big ? __builtin_bswap32(value) : value;
}

// order contains the two-byte marker from the archive header.
static inline EndianTypes GetEndian(u16 order) {
    u32 second = order >> 8, first = order & 0xff;
    if (first == 0xff && second == 0xfe) return EndianTypes_Little;
    if (first == 0xfe && second == 0xff) return EndianTypes_Big;
    return EndianTypes_Little;
}

ArcExtractor::ArcExtractor()
    : m_pArchiveBlockHeader(nullptr), m_pFATBlockHeader(nullptr), m_pFNTBlock(nullptr),
      m_pFATEntries(nullptr), m_FATEntryCount(0), m_pDataBlock(nullptr), m_EndianType(EndianTypes_Little) {}
// archive points to the complete SARC file whose contents remain caller-owned.
ArcExtractor::ArcExtractor(const void* archive) : ArcExtractor() { PrepareArchive(archive); }
ArcExtractor::~ArcExtractor() = default;

// archive supplies the SARC header, allocation table, names, and resource data.
bool ArcExtractor::PrepareArchive(const void* archive) {
    if (!archive) return false;
    auto* header = static_cast<const ArchiveBlockHeader*>(archive);
    m_pArchiveBlockHeader = header;
    if (std::strncmp(header->signature, "SARC", 4)) return false;
    m_EndianType = GetEndian(header->byteOrder);
    if (Read16(m_EndianType == EndianTypes_Big, header->version) != 0x100) return false;
    if (Read16(m_EndianType == EndianTypes_Big, header->headerSize) != sizeof(ArchiveBlockHeader)) return false;
    auto* fat = reinterpret_cast<const FATBlockHeader*>(header + 1);
    m_pFATBlockHeader = fat;
    if (std::strncmp(fat->signature, "SFAT", 4)) return false;
    if (Read16(m_EndianType == EndianTypes_Big, fat->headerSize) != sizeof(FATBlockHeader)) return false;
    if (static_cast<u16>(Read16(m_EndianType == EndianTypes_Big, fat->fileCount)) >= 0x4000) return false;
    m_FATEntryCount = Read16(m_EndianType == EndianTypes_Big, fat->fileCount);
    auto* bytes = static_cast<const char*>(archive);
    m_pFATEntries = reinterpret_cast<FATEntry*>(const_cast<char*>(bytes) +
        Read16(m_EndianType == EndianTypes_Big, header->headerSize) + Read16(m_EndianType == EndianTypes_Big, fat->headerSize));
    auto* fnt = reinterpret_cast<const FNTBlockHeader*>(bytes +
        Read16(m_EndianType == EndianTypes_Big, header->headerSize) + Read16(m_EndianType == EndianTypes_Big, fat->headerSize) +
        Read16(m_EndianType == EndianTypes_Big, fat->fileCount) * sizeof(FATEntry));
    if (std::strncmp(fnt->signature, "SFNT", 4)) return false;
    if (Read16(m_EndianType == EndianTypes_Big, fnt->headerSize) != sizeof(FNTBlockHeader)) return false;
    m_pFNTBlock = reinterpret_cast<const char*>(fnt + 1);
    if (static_cast<s32>(Read32(header->dataBlockOffset, m_EndianType)) < m_pFNTBlock - bytes) return false;
    m_pDataBlock = reinterpret_cast<const u8*>(bytes + static_cast<s32>(Read32(header->dataBlockOffset, m_EndianType)));
    return true;
}

// archive is accepted for symmetry; SARC itself contains no relocatable pointers.
void ArcExtractor::Relocate(const void* archive) {}
// archive contains embedded texture, shader, or font resources to unrelocate.
void ArcExtractor::Unrelocate(const void* archive) {
    ArcExtractor extractor(archive);
    int count = extractor.GetFileCount();
    for (u32 index = 0; index < count; ++index) {
        const FATEntry* entry = &extractor.m_pFATEntries[index];
        u32 offset = extractor.m_EndianType == EndianTypes_Big
            ? __builtin_bswap32(entry->dataStartOffset) : entry->dataStartOffset;
        auto* file = reinterpret_cast<nn::util::BinaryFileHeader*>(const_cast<u8*>(extractor.m_pDataBlock) + offset);
        u64 signature = *reinterpret_cast<u64*>(file);
        if (signature == 0x58544e42 || signature == 0x48534e42) {
            if (file->IsRelocated()) {
                file->GetRelocationTable()->Unrelocate();
                file->SetRelocated(false);
            }
        }

        if (*reinterpret_cast<u64*>(file) == 0x544e4646) nn::font::ResFont::Unrelocate(file);
    }
}

int ArcExtractor::GetFileCount() const { return m_FATEntryCount < 0 ? 0 : m_FATEntryCount; }
// info optionally receives the resource offset and size; entryId selects a FAT entry.
void* ArcExtractor::GetFileFast(ArcFileInfo* info, int entryId) {
    if (entryId < 0) return nullptr;
    if (entryId >= m_FATEntryCount) return nullptr;
    u32 start = Read32(m_pFATEntries[entryId].dataStartOffset, m_EndianType);
    if (info) {
        u32 end = Read32(m_pFATEntries[entryId].dataEndOffset, m_EndianType);
        if (end < start) return nullptr;
        info->m_StartOffset = start;
        info->m_Length = end - start;
    }

    return const_cast<u8*>(m_pDataBlock) + start;
}

// entries/count describe the sorted FAT; hash is the desired resource-name hash.
static inline int FindEntry(const ArcExtractor::FATEntry* entries, int count, u32 hash, EndianTypes endian) {
    int low = 0, high = count, middle = high / 2;
    while (Read32(entries[middle].hash, endian) != hash) {
        if (Read32(entries[middle].hash, endian) < hash) {
            if (low == middle) return -1;
            low = middle;
        } else {
            if (high == middle) return -1;
            high = middle;
        }

        middle = (low + high) / 2;
    }

    return middle;
}

// path is the archive-relative resource name; the result is its FAT index or -1.
int ArcExtractor::ConvertPathToEntryId(const char* path) const {
    u32 key = Read32(m_pFATBlockHeader->hashKey, m_EndianType), hash = 0;
    for (const char* c = path; *c; ++c) hash = hash * key + static_cast<u8>(*c);
    int middle = FindEntry(m_pFATEntries, m_FATEntryCount, hash, m_EndianType);
    if (middle == -1) return -1;
    u32 name = Read32(m_pFATEntries[middle].nameOffset, m_EndianType);
    if (!name) return middle;
    int index = middle - int(name >> 24) + 1;
    for (; index < m_FATEntryCount; ++index) {
        if (Read32(m_pFATEntries[index].hash, m_EndianType) != hash) return -1;
        u32 offset = Read32(m_pFATEntries[index].nameOffset, m_EndianType) & 0xffffff;
        if (m_pFNTBlock + offset > reinterpret_cast<const char*>(m_pDataBlock)) return -1;
        if (std::strcmp(path, m_pFNTBlock + offset * 4) == 0) break;
    }

    return index;
}

// entryId is the starting index and receives the next index; entries receives at
// most count names, substituting a hexadecimal hash for entries without names.
int ArcExtractor::ReadEntry(int* entryId, ArcEntry* entries, int count) const {
    int index = *entryId, read = 0;
    for (; (read < count) & (index < Read16(m_EndianType == EndianTypes_Big, m_pFATBlockHeader->fileCount)); ++read, index = *entryId + read) {
        u32 name = Read32(m_pFATEntries[index].nameOffset, m_EndianType);
        if (!name) {
            nn::util::SNPrintf(entries[read].name, sizeof(entries[read].name), "%08x",
                               Read32(m_pFATEntries[index].hash, m_EndianType));
        } else {
            u32 offset = name & 0xffffff;
            if (m_pFNTBlock + offset > reinterpret_cast<const char*>(m_pDataBlock)) {
                entries[read].name[0] = 0;
            } else {
                nn::util::Strlcpy(entries[read].name, m_pFNTBlock + offset * 4, 256);
            }
        }
    }

    *entryId = index;
    return read;
}
}
