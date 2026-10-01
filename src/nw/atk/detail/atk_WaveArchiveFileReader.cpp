#include <nn/atk/atk_WaveArchiveFileReader.h>

namespace nn::atk::detail {
WaveArchiveFileReader::WaveArchiveFileReader()
    : mHeader(nullptr), mInfo(nullptr), mFileTable(nullptr), mInitialized(false) {}
// file supplies the FWAR resource; individualLoad enables its writable address table when present.
WaveArchiveFileReader::WaveArchiveFileReader(const void* file, bool individualLoad)
    : WaveArchiveFileReader() {
    Initialize(file, individualLoad);
}

// file supplies the archive; individualLoad selects addresses from its FWTA table rather than embedded data.
void WaveArchiveFileReader::Initialize(const void* file, bool individualLoad) {
    auto* header = static_cast<const WaveArchiveFile::FileHeader*>(file);

    if (header == nullptr || header->signature != 0x52415746 || header->byteOrder != 0xfeff || header->version != 0x10000) return;
    mHeader = header;
    const auto* info = mHeader->GetInfoBlock();
    mInitialized = true;
    mInfo = &info->body;
    mFileTable = nullptr;

    if (individualLoad && HasIndividualLoadTable()) {
        mFileTable = reinterpret_cast<const void**>(const_cast<u8*>(static_cast<const u8*>(file)) + mHeader->GetFileBlockOffset() + 4);
    }
}

bool WaveArchiveFileReader::HasIndividualLoadTable() const {
    if (!mInitialized) return false;
    auto* base = reinterpret_cast<const u8*>(mHeader);
    return *reinterpret_cast<const u32*>(base + mHeader->GetFileBlockOffset()) == 0x54415746;
}

void WaveArchiveFileReader::Finalize() {
    if (!mInitialized) return;
    mInitialized = false;
    mHeader = nullptr;
    mInfo = nullptr;
    mFileTable = nullptr;
}

void WaveArchiveFileReader::InitializeFileTable() {
    for (size_t i = 0; i < GetWaveFileCount(); ++i) mFileTable[i] = nullptr;
}

u32 WaveArchiveFileReader::GetWaveFileCount() const { return mInitialized ? mInfo->count : 0; }
// index selects a wave; invalid indices or an uninitialized reader return null.
const void* WaveArchiveFileReader::GetWaveFile(u32 index) const {
    if (!mInitialized) return nullptr;

    if (mInfo->count <= index) return nullptr;

    if (mFileTable != nullptr) return mFileTable[index];
    u32 offset = mInfo->waves[index].offset;
    return mHeader->GetFileBlock()->data + offset;
}

// index must identify a wave; an uninitialized reader reports zero bytes.
u32 WaveArchiveFileReader::GetWaveFileSize(u32 index) const {
    return mInitialized ? mInfo->waves[index].size : 0;
}

// index must identify a wave; the returned offset is relative to the archive header.
u32 WaveArchiveFileReader::GetWaveFileOffsetFromFileHead(u32 index) const {
    if (!mInitialized) return 0;
    u32 offset = mHeader->GetFileBlockOffset();
    return offset + mInfo->waves[index].offset + 8;
}

// index selects a table entry and file supplies its loaded wave address; return the previous address.
const void* WaveArchiveFileReader::SetWaveFile(u32 index, const void* file) {
    if (!mInitialized) return nullptr;

    if (mFileTable == nullptr) return nullptr;

    if (mInfo->count <= index) return nullptr;
    const void* previous = mFileTable[index];
    mFileTable[index] = file;
    return previous;
}
}
