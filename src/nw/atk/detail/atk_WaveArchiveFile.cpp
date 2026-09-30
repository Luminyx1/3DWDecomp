#include <nn/atk/atk_WaveArchiveFile.h>

namespace nn::atk::detail {
// type identifies one of the archive's two block references.
const ReferenceWithSize* WaveArchiveFile::FileHeader::GetReferenceBy(u16 type) const {
    if (blocks[0].type == type) return &blocks[0];
    if (blocks[1].type == type) return &blocks[1];
    return nullptr;
}
const WaveArchiveFile::InfoBlock* WaveArchiveFile::FileHeader::GetInfoBlock() const {
    return reinterpret_cast<const InfoBlock*>(reinterpret_cast<const u8*>(this) + GetInfoBlockOffset());
}
u32 WaveArchiveFile::FileHeader::GetInfoBlockOffset() const { return GetReferenceBy(0x6800)->offset; }
u32 WaveArchiveFile::FileHeader::GetInfoBlockSize() const { return GetReferenceBy(0x6800)->size; }
const WaveArchiveFile::FileBlock* WaveArchiveFile::FileHeader::GetFileBlock() const {
    return reinterpret_cast<const FileBlock*>(reinterpret_cast<const u8*>(this) + GetFileBlockOffset());
}
u32 WaveArchiveFile::FileHeader::GetFileBlockOffset() const { return GetReferenceBy(0x6801)->offset; }
u32 WaveArchiveFile::FileHeader::GetFileBlockSize() const { return GetReferenceBy(0x6801)->size; }
}
