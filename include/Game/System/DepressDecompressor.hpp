#pragma once
#include "Project/Draw/dpr.hpp"
#include <resource/seadDecompressor.h>
namespace sead {
class FileHandle;
}
namespace al {
class DepressDecompressor : public sead::Decompressor, public nst::dpr::DepressStreamContext {
  public:
    static u32 GetWorkingSize();
    DepressDecompressor(u8* pWorkBuffer, u32 workSize);
    u8* tryDecompFromDevice(const sead::ResourceMgr::LoadArg& rArg, sead::Resource* pResource, u32* pSize,
                            u32* pAllocSize, bool* pAllocated) override;
    u32 ReadData(s64 offset, s64 size, void* pBuffer) override;

  private:
    u32 mWorkSize;
    u8* mpProvidedWorkBuffer;
    s32 mReadOffset;
    u32 mReadSize;
    s32 mFileOffset;
    sead::FileHandle* mpFile;
};
static_assert(sizeof(DepressDecompressor) == 0xc0);
} // namespace al
