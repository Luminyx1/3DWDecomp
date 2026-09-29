#pragma once

#include "basis/seadTypes.h"
#include "container/seadBuffer.h"
#include "lms/lms.h"

namespace sead {
class Heap;

class MessageProject {
public:
    struct Color {
        void set(u8 red, u8 green, u8 blue, u8 alpha) {
            b = blue;
            g = green;
            r = red;
            a = alpha;
        }

        u8 a = 0xFF;
        u8 b = 0;
        u8 g = 0;
        u8 r = 0;
    };

    struct Style {
        s32 regionWidth;
        s32 lineNum;
        s32 fontIndex;
        s32 baseColorIndex;
    };

    enum DataType {
        cDataType_U8 = 0x0,
        cDataType_U16 = 0x1,
        cDataType_U32 = 0x2,
        cDataType_S8 = 0x3,
        cDataType_S16 = 0x4,
        cDataType_S32 = 0x5,
        cDataType_F32 = 0x6,
        cDataType_F64 = 0x7,
        cDataType_String = 0x8,
        cDataType_List = 0x9,
        cDataType_Unknown = 0xFF,
    };

    struct AttributeInfo {
        s32 dataType;
        s32 offset;
    };

    virtual ~MessageProject();

    bool initialize(void* pData, Heap* pHeap);
    void finalize();
    const void* getInitializeData() const;

    static void* allocForLibms_(size_t size);
    static void freeForLibms_(void* pPtr);

protected:
    LMSProjFile* mProjFile = nullptr;
    Buffer<Color> mColors;
    Buffer<Style> mStyles;
    Buffer<AttributeInfo> mAttributeInfos;
    s32 mContentsNum = 0;

    static Heap* sHeap;
};
}  // namespace sead
