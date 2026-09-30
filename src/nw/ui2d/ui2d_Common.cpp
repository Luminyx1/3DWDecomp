#include <nn/ui2d/ui2d_Common.h>
#include <nn/ui2d/ui2d_Layout.h>
#include <nn/util/util_MathTypes.h>
#include <cstring>

namespace nn::ui2d::detail {
void TexCoordArray::Initialize() { mCapacity = 0; mSize = 0; mCoords = nullptr; }
void TexCoordArray::Free() {
    if (mCoords) { Layout::FreeMemory(mCoords); mCoords = nullptr; mCapacity = 0; mSize = 0; }
}

// size is the requested number of four-corner texture-coordinate sets.
void TexCoordArray::Reserve(s32 size) {
    if (mCapacity >= size) return;
    Free();
    mCoords = static_cast<nn::util::Float2 (*)[4]>(Layout::AllocateMemory(sizeof(*mCoords) * static_cast<u32>(size)));
    mCoords[0][0] = {0, 0};
    if (mCoords) mCapacity = size;
}

// size becomes the active set count; newly exposed sets cover the full texture.
void TexCoordArray::SetSize(s32 size) {
    if (!mCoords || mCapacity < size) return;
    for (int i = mSize; i < size; ++i) {
        mCoords[i][0] = {0, 0}; mCoords[i][1] = {1, 0};
        mCoords[i][2] = {0, 1}; mCoords[i][3] = {1, 1};
    }

    mSize = size;
}

// out receives the four corner coordinates from the set at index.
void TexCoordArray::GetCoord(nn::util::Float2* out, s32 index) const {
    for (int i = 0; i < 4; ++i) out[i] = mCoords[index][i];
}

// index selects the set; coords supplies its four corner coordinates.
void TexCoordArray::SetCoord(s32 index, const nn::util::Float2* coords) {
    for (int i = 0; i < 4; ++i) mCoords[index][i] = coords[i];
}

// source supplies count packed coordinate sets in resource format.
void TexCoordArray::Copy(const void* source, s32 count) {
    if (mSize < static_cast<u8>(count)) mSize = count;
    auto* coords = static_cast<const nn::util::Float2*>(source);
    for (int i = 0; i < count; ++i)
        for (int j = 0; j < 4; ++j) {
            mCoords[i][j].x = coords[i * 4 + j].x;
            mCoords[i][j].y = coords[i * 4 + j].y;
        }
}

// other is the copy whose allocated capacity and active coordinates are compared.
bool TexCoordArray::CompareCopiedInstanceTest(const TexCoordArray& other) const {
    if (mCapacity != other.mCapacity || mSize != other.mSize) return false;
    for (int i = 0; i < mSize; ++i)
        for (int j = 0; j < 4; ++j)
            if (std::memcmp(&mCoords[i][j], &other.mCoords[i][j], sizeof(nn::util::Float2))) return false;
    return true;
}
}
