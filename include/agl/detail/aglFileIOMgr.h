#pragma once

#include <container/seadBuffer.h>
#include <heap/seadDisposer.h>
#include <hostio/seadHostIONode.h>
#include <prim/seadBitFlag.h>
#include <prim/seadSafeString.h>
#include <thread/seadCriticalSection.h>

namespace sead {
namespace hostio {
class Context;
class FileInfo;
class PropertyEvent;
}  // namespace hostio
class Heap;
class NinHostIOFileDevice;
class XmlDocument;
}  // namespace sead

namespace agl::detail {

class FileIOMgr : public sead::hostio::Node {
    SEAD_SINGLETON_DISPOSER(FileIOMgr)
public:
    class CreateArg {
    public:
        CreateArg();

        bool mUseCheckout;
    };

    class DialogArg {
    public:
        DialogArg();

        sead::SafeString mFilter = sead::SafeString::cEmptyString;
        sead::SafeString mFileName = sead::SafeString::cEmptyString;
        sead::SafeString mId = "agl_default";
        sead::SafeString mPath = sead::SafeString::cEmptyString;
        sead::BufferedSafeString* mOutPath = nullptr;
        s32 mAlignment = 0x20;
        bool mSkipCheckout = false;
        u32 mUserData = 0;
    };
    static_assert(sizeof(DialogArg) == 0x58);

    struct File {
        u8* mData;
        u32 mSize;
        void* _10;
        u8 mUserData;
    };
    static_assert(sizeof(File) == 0x20);

    FileIOMgr();
    virtual ~FileIOMgr();

    void initialize(const CreateArg& rArg, sead::Heap* pHeap);
    void setCheckoutCommandPath(const sead::SafeString& rPath);
    bool save(const sead::XmlDocument& rDocument, const DialogArg& rArg, u32 bufferSize);
    bool showDialog(sead::hostio::FileInfo* pInfo, const sead::SafeString& rMode,
                    const sead::SafeString& rId, const sead::SafeString& rFilter,
                    const sead::SafeString& rFileName) const;
    void checkout_(const sead::SafeString& rPath) const;
    void showErrorDialog_(const sead::SafeString& rPath) const;
    bool save(const void* pData, u32 size, const DialogArg& rArg);
    s32 load(const DialogArg& rArg);
    void close(s32 handle);
    void genMessage(sead::hostio::Context* pContext);
    void listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent);

    const File& getFile(s32 handle) const { return mFiles[handle]; }

private:
    sead::NinHostIOFileDevice* mDevice = nullptr;
    sead::Buffer<File> mFiles;
    sead::CriticalSection mCS;
    sead::BitFlag32 mFlags;
    sead::FixedSafeString<256> mCheckoutCommandPath;
};
static_assert(sizeof(FileIOMgr) == 0x1a0);

}  // namespace agl::detail
