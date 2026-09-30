#include "Project/Controller/PadDataArcReader.hpp"

#include <prim/seadEndian.h>
#include <prim/seadSafeString.h>

#include "Library/Controller/PadDataPack.hpp"
#include "Library/Resource/Resource.hpp"
#include "Library/Resource/ResourceFunction.hpp"

namespace al {
/**
 * Creates a reader for recorded pad data in an archive.
 * @param pPath archive path
 */
PadDataArcReader::PadDataArcReader(const char* pPath) : mPath(pPath) {
    findOrCreateResource(pPath, nullptr);
}

/**
 * Creates a reader for recorded pad data in an archive and loads a resource.
 * @param pPath archive path
 * @param pResourceName name of the recorded data, without extension
 */
PadDataArcReader::PadDataArcReader(const char* pPath, const char* pResourceName) : mPath(pPath) {
    readResource(pResourceName);
}

/**
 * Loads the recorded pad data of a resource from the archive.
 * @param pResourceName name of the recorded data, without extension
 */
void PadDataArcReader::readResource(const char* pResourceName) {
    mCursorFrame = 0;
    Resource* resource = findOrCreateResource(mPath, nullptr);
    sead::FixedSafeString<256> fileName;
    fileName.format("%s.bin", pResourceName);
    mDataFrames = static_cast<PadDataPack*>(resource->getOtherFile(fileName, nullptr));
    checkEnd();
    PadDataPack* frame = mDataFrames;

    while (frame->trig != 0xffffffff) {
        frame++;
    }

    mTotalFrame = frame - mDataFrames;
}

/**
 * Marks the reader as ended if the current frame is the terminator.
 */
void PadDataArcReader::checkEnd() {
    if (mDataFrames[mCursorFrame].trig == 0xffffffff) {
        mIsEnd = true;
    }
}

/**
 * Reads the next frame of pad data.
 * @param pFrameData frame data to fill
 */
void PadDataArcReader::read(PadDataPack* pFrameData) {
    if (mIsEnd) {
        return;
    }

    const u32* src = reinterpret_cast<const u32*>(&mDataFrames[mCursorFrame++]);
    u32* dst = reinterpret_cast<u32*>(pFrameData);

    for (s32 i = 0; i < 6; i++) {
        dst[i] = sead::Endian::swapU32(src[i]);
    }

    checkEnd();
}
}  // namespace al
