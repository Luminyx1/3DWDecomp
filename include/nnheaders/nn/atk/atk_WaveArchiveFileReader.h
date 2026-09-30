#pragma once
#include <nn/atk/atk_WaveArchiveFile.h>

namespace nn::atk::detail {
class WaveArchiveFileReader {
public:
    WaveArchiveFileReader();
    WaveArchiveFileReader(const void* file, bool individualLoad);
    void Initialize(const void* file, bool individualLoad);
    bool HasIndividualLoadTable() const;
    void Finalize();
    void InitializeFileTable();
    u32 GetWaveFileCount() const;
    const void* GetWaveFile(u32 index) const;
    u32 GetWaveFileSize(u32 index) const;
    u32 GetWaveFileOffsetFromFileHead(u32 index) const;
    const void* SetWaveFile(u32 index, const void* file);

private:
    const WaveArchiveFile::FileHeader* mHeader;
    const WaveArchiveFile::InfoBlockBody* mInfo;
    const void** mFileTable;
    bool mInitialized;
};
static_assert(sizeof(WaveArchiveFileReader) == 0x20, "WaveArchiveFileReader size");
}
