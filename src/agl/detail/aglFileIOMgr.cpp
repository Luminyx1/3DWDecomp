#include "detail/aglFileIOMgr.h"

#include <filedevice/nin/seadNinHostIOFileDevice.h>
#include <filedevice/seadFileDeviceMgr.h>
#include <hostio/seadHostIOFileInfo.h>
#include <prim/seadScopedLock.h>
#include <stream/seadBufferStream.h>
#include <stream/seadFileDeviceStream.h>
#include <xml/seadXmlDocument.h>

#include "detail/aglPrivateResource.h"

namespace agl::detail {

SEAD_SINGLETON_DISPOSER_IMPL(FileIOMgr)

/**
 * Constructs a create argument with checkout disabled.
 */
FileIOMgr::CreateArg::CreateArg() : mUseCheckout(false) {}

/**
 * Constructs a dialog argument with empty strings and the default dialog id.
 */
FileIOMgr::DialogArg::DialogArg() = default;

/**
 * Constructs the manager with the default checkout command path.
 */
FileIOMgr::FileIOMgr()
    : mCheckoutCommandPath(sead::SafeString("%AGL_ROOT%/tools/bat/perforce_save.bat")) {}

/**
 * Frees the file table.
 */
FileIOMgr::~FileIOMgr() {
    mFiles.freeBuffer();
}

/**
 * Allocates the file table, mounts the host PC file device and stores the checkout option.
 * @param rArg creation options
 * @param pHeap heap used for the file table and device, or nullptr to skip allocation
 */
void FileIOMgr::initialize(const CreateArg& rArg, sead::Heap* pHeap) {
    if (pHeap) {
        mFiles.tryAllocBuffer(0x1000, pHeap);
        for (auto& file : mFiles) {
            file.mData = nullptr;
            file.mSize = 0;
            file.mUserData = 0;
            file._10 = nullptr;
        }

        mDevice = new (pHeap) sead::NinHostIOFileDevice();
        sead::FileDeviceMgr::instance()->mount(mDevice, "agl_hostpc");
    }

    mFlags.change(1, rArg.mUseCheckout);
}

/**
 * Sets the command used to check files out before saving.
 * @param rPath command path
 */
void FileIOMgr::setCheckoutCommandPath(const sead::SafeString& rPath) {
    mCheckoutCommandPath = rPath;
}

/**
 * Saves an XML document to the path in the dialog argument (or one chosen via a dialog).
 * @param rDocument document to save
 * @param rArg dialog argument
 * @param bufferSize size of the write buffer
 * @return whether the document was saved
 */
bool FileIOMgr::save(const sead::XmlDocument& rDocument, const DialogArg& rArg, u32 bufferSize) {
    sead::ScopedLock<sead::CriticalSection> lock(&mCS);

    sead::hostio::FileInfo info;
    if (rArg.mPath == sead::SafeString::cEmptyString) {
        const char* mode = mFlags.isOn(1) ? "SaveAcceptReadOnlyFile" : "Save";
        if (!showDialog(&info, mode, rArg.mId, rArg.mFilter, rArg.mFileName)) {
            return false;
        }
    } else {
        info.mPath = rArg.mPath;
    }

    if (!rArg.mSkipCheckout) {
        checkout_(info.mPath);
    }

    sead::FileHandle handle;
    sead::FileDevice* device =
        mDevice->tryOpen(&handle, info.mPath, sead::FileDevice::cFileOpenFlag_WriteOnly, 0);
    if (!device) {
        showErrorDialog_(info.mPath);
        return false;
    }

    sead::Heap* heap = PrivateResource::instance()->getDebugHeap();
    u8* buffer = new (heap, 0x40) u8[bufferSize];
    sead::FileDeviceWriteStream fileStream(&handle, sead::Stream::Modes::Binary);
    sead::BufferWriteStream stream(&fileStream, buffer, bufferSize);
    rDocument.save(&stream, heap, false, nullptr);
    stream.flush();
    device->tryClose(&handle);

    if (rArg.mOutPath && rArg.mOutPath->cstr() != info.mPath.cstr()) {
        rArg.mOutPath->copy(info.mPath);
    }

    delete[] buffer;
    return true;
}

/**
 * Shows a host file dialog (unavailable in release builds).
 * @param pInfo receives the chosen file
 * @param rMode dialog mode
 * @param rId dialog id, or empty to use the filter
 * @param rFilter file extension filter
 * @param rFileName default file name
 * @return whether a file was chosen
 */
bool FileIOMgr::showDialog(sead::hostio::FileInfo* pInfo, const sead::SafeString& rMode,
                           const sead::SafeString& rId, const sead::SafeString& rFilter,
                           const sead::SafeString& rFileName) const {
    sead::SafeString id = rId == "" ? rFilter : rId;
    sead::FormatFixedSafeString<1024> arg(
        "FileName = %s, Mode = %s, Id = %s,Filter = %sファイル(*.%s)|*.%s|すべてのファイル(*.*)|*.*",
        rFileName.cstr(), rMode.cstr(), id.cstr(), rFilter.cstr(), rFilter.cstr(),
        rFilter.cstr());
    return false;
}

/**
 * Runs the checkout command on a file if checkout is enabled.
 * @param rPath file to check out
 */
void FileIOMgr::checkout_(const sead::SafeString& rPath) const {
    if (mFlags.isOn(1)) {
        if (mFlags.isOn(2)) {
            sead::FormatFixedSafeString<1024> arg(
                "File = %s, Arg = %s, WaitEnd = True, WindowStyle = Normal",
                mCheckoutCommandPath.cstr(), rPath.cstr());
        } else {
            sead::FormatFixedSafeString<1024> arg(
                "File = %s, Arg = %s, WaitEnd = True, WindowStyle = Hidden",
                mCheckoutCommandPath.cstr(), rPath.cstr());
        }
    }
}

/**
 * Reports that saving a file failed.
 * @param rPath file that could not be saved
 */
void FileIOMgr::showErrorDialog_(const sead::SafeString& rPath) const {
    sead::FormatFixedSafeString<1024> message(
        "%sの保存に失敗しました。\n保存先のアクセス権を確認して下さい。", rPath.cstr());
}

/**
 * Saves raw data to the path in the dialog argument (or one chosen via a dialog).
 * @param pData data to save
 * @param size size of the data
 * @param rArg dialog argument
 * @return whether the data was saved
 */
bool FileIOMgr::save(const void* pData, u32 size, const DialogArg& rArg) {
    sead::ScopedLock<sead::CriticalSection> lock(&mCS);

    sead::hostio::FileInfo info;
    if (rArg.mPath == sead::SafeString::cEmptyString) {
        const char* mode = mFlags.isOn(1) ? "SaveAcceptReadOnlyFile" : "Save";
        if (!showDialog(&info, mode, rArg.mId, rArg.mFilter, rArg.mFileName)) {
            return false;
        }
    } else {
        info.mPath = rArg.mPath;
    }

    if (!rArg.mSkipCheckout) {
        checkout_(info.mPath);
    }

    sead::FileHandle handle;
    sead::FileDevice* device =
        mDevice->tryOpen(&handle, info.mPath, sead::FileDevice::cFileOpenFlag_WriteOnly, 0);
    if (!device) {
        return false;
    }

    sead::Heap* heap = PrivateResource::instance()->getDebugHeap();
    u8* buffer = new (heap, 0x40) u8[(size + 0x1f) & ~0x1fu];
    sead::MemUtil::copy(buffer, pData, size);
    handle.write(buffer, size);
    device->tryClose(&handle);
    delete[] buffer;

    if (rArg.mOutPath) {
        rArg.mOutPath->copy(info.mPath);
    }

    return true;
}

/**
 * Loads a file into a free slot of the file table.
 * @param rArg dialog argument
 * @return handle of the loaded file, or -1 on failure
 */
s32 FileIOMgr::load(const DialogArg& rArg) {
    sead::ScopedLock<sead::CriticalSection> lock(&mCS);

    s32 handle = -1;
    File* file = nullptr;
    for (auto it = mFiles.begin(); it != mFiles.end(); ++it) {
        if (!it->mData) {
            file = &*it;
            handle = it.getIndex();
            break;
        }
    }

    sead::hostio::FileInfo info;
    if (rArg.mPath == sead::SafeString::cEmptyString) {
        if (!showDialog(&info, "Open", rArg.mId, rArg.mFilter, rArg.mFileName)) {
            return -1;
        }
    } else {
        info.mPath = rArg.mPath;
    }

    sead::Heap* heap = PrivateResource::instance()->getDebugHeap();
    sead::FileDevice::LoadArg loadArg;
    loadArg.path = info.mPath;
    loadArg.heap = heap;
    loadArg.alignment = rArg.mAlignment;
    file->mData = mDevice->tryLoad(loadArg);
    if (!file->mData) {
        return -1;
    }

    file->mSize = loadArg.read_size;

    if (rArg.mOutPath) {
        rArg.mOutPath->copy(info.mPath);
    }

    file->mUserData = rArg.mUserData;
    return handle;
}

/**
 * Frees a loaded file.
 * @param handle handle returned by load
 */
void FileIOMgr::close(s32 handle) {
    sead::ScopedLock<sead::CriticalSection> lock(&mCS);
    File& file = mFiles[handle];
    if (file.mData) {
        delete[] file.mData;
        file.mData = nullptr;
        file.mSize = 0;
        file._10 = nullptr;
    }
}

/**
 * Generates the host IO message (no-op in release builds).
 * @param pContext host IO context
 */
void FileIOMgr::genMessage(sead::hostio::Context* pContext) {}

/**
 * Handles a host IO property event (no-op in release builds).
 * @param pEvent property event
 */
void FileIOMgr::listenPropertyEvent(const sead::hostio::PropertyEvent* pEvent) {}

}  // namespace agl::detail
