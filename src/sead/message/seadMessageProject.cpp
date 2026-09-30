#include "message/seadMessageProject.h"

#include "heap/seadHeap.h"

namespace sead {

Heap* MessageProject::sHeap = nullptr;

/**
 * Destroys the message project.
 */
MessageProject::~MessageProject() = default;

// NON_MATCHING: 99%, order of two loads and of the color byte stores
/**
 * Opens an MSBP file and caches its colors, styles and attribute layout.
 * @param pData the MSBP file
 * @param pHeap heap for the caches and the LibMessageStudio allocations
 * @return true.
 */
bool MessageProject::initialize(void* pData, Heap* pHeap) {
    sHeap = pHeap;
    LMS_SetMemFuncs(allocForLibms_, freeForLibms_);
    mProjFile = LMS_InitProject(static_cast<const char*>(pData));
    LMS_SetMemFuncs(nullptr, nullptr);
    sHeap = nullptr;

    s32 colorNum = LMS_GetColorNum(mProjFile);
    if (colorNum > 0) {
        mColors.tryAllocBuffer(colorNum, pHeap);
        for (auto it = mColors.begin(), end = mColors.end(); it != end; ++it) {
            LMSColor color;
            LMS_GetColor(mProjFile, it.getIndex(), &color);
            it->set(color.r, color.g, color.b, color.a);
        }
    }

    s32 styleNum = LMS_GetStyleNum(mProjFile);
    if (styleNum > 0) {
        mStyles.tryAllocBuffer(styleNum, pHeap);
        for (auto it = mStyles.begin(), end = mStyles.end(); it != end; ++it) {
            it->regionWidth = LMS_GetRegionWidth(mProjFile, it.getIndex());
            it->lineNum = LMS_GetLineNum(mProjFile, it.getIndex());
            s32 fontIndex = LMS_GetFontIndex(mProjFile, it.getIndex());
            it->fontIndex = fontIndex < 0 ? -1 : fontIndex;
            s32 baseColorIndex = LMS_GetBaseColorIndex(mProjFile, it.getIndex());
            it->baseColorIndex = baseColorIndex < 0 ? -1 : baseColorIndex;
        }
    }

    s32 attrNum = LMS_GetAttrNum(mProjFile);
    if (attrNum > 0) {
        mAttributeInfos.tryAllocBuffer(attrNum, pHeap);
        for (auto it = mAttributeInfos.begin(), end = mAttributeInfos.end(); it != end; ++it) {
            it->dataType = LMS_GetAttrType(mProjFile, it.getIndex());
            it->offset = LMS_GetAttrOffset(mProjFile, it.getIndex());
        }
    }

    s32 contentsNum = LMS_GetContentsNum(mProjFile);
    mContentsNum = contentsNum < 0 ? 0 : contentsNum;
    return true;
}

/**
 * Allocation callback handed to LibMessageStudio.
 * @param size number of bytes
 * @return the allocated memory.
 */
void* MessageProject::allocForLibms_(size_t size) {
    return new (sHeap, 8) u8[size];
}

/**
 * Free callback handed to LibMessageStudio.
 * @param pPtr memory from allocForLibms_
 */
void MessageProject::freeForLibms_(void* pPtr) {
    delete[] static_cast<u8*>(pPtr);
}

/**
 * Frees the caches and closes the MSBP file.
 */
void MessageProject::finalize() {
    mStyles.freeBuffer();
    mColors.freeBuffer();
    mAttributeInfos.freeBuffer();
    LMS_SetMemFuncs(nullptr, freeForLibms_);
    LMS_CloseProject(mProjFile);
    mProjFile = nullptr;
    mContentsNum = 0;
    LMS_SetMemFuncs(nullptr, nullptr);
}

/**
 * @return the MSBP data passed to initialize, or nullptr.
 */
const void* MessageProject::getInitializeData() const {
    if (mProjFile) {
        return mProjFile->commonInfo.pResource;
    }

    return nullptr;
}

}  // namespace sead
