#include <resource/seadResource.h>

#include <filedevice/seadFileDeviceMgr.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadPtrUtil.h>
#include <stream/seadFileDeviceStream.h>
#include <stream/seadRamStream.h>

namespace sead
{
Resource::Resource() = default;

Resource::~Resource() = default;

DirectResource::DirectResource() = default;

DirectResource::~DirectResource()
{
    if (mSettingFlag.isOnBit(0))
    {
        delete[] mRawData;
    }
}

void DirectResource::create(u8* pBuffer, u32 bufferSize, u32 allocSize, bool allocated, Heap* pHeap)
{
    if (mRawData)
    {
        SEAD_ASSERT_MSG(false, "read twice");
        return;
    }

    mRawData = pBuffer;
    mRawSize = bufferSize;
    mBufferSize = allocSize;

    mSettingFlag.changeBit(0, allocated);

    doCreate_(pBuffer, bufferSize, pHeap);
}

IndirectResource::IndirectResource() = default;

IndirectResource::~IndirectResource() = default;

void IndirectResource::create(sead::ReadStream* pStream, u32 size, sead::Heap* pHeap)
{
    doCreate_(pStream, size, pHeap);
}

ResourceFactory::~ResourceFactory()
{
    auto* mgr = ResourceMgr::instance();

    if (mgr == nullptr)
    {
        return;
    }

    mgr->unregisterFactory(this);

    if (mgr->getDefaultFactory() == this)
    {
        mgr->setDefaultFactory(nullptr);
    }
}

Resource* DirectResourceFactoryBase::create(const ResourceMgr::CreateArg& rCreateArg)
{
    DirectResource* resource = newResource_(rCreateArg.heap, rCreateArg.alignment);

    if (resource == nullptr)
    {
        SEAD_ASSERT_MSG(false, "resource new failed.");
        return nullptr;
    }

    if (!PtrUtil::isAligned(rCreateArg.buffer, resource->getLoadDataAlignment()))
    {
        SEAD_ASSERT_MSG(false, "buffer alignment invalid: %p, %d", rCreateArg.buffer,
                        resource->getLoadDataAlignment());
        delete resource;
        return nullptr;
    }

    resource->create(rCreateArg.buffer, rCreateArg.file_size, rCreateArg.buffer_size,
                     rCreateArg.need_unload, rCreateArg.heap);
    return resource;
}

Resource* DirectResourceFactoryBase::tryCreate(const ResourceMgr::LoadArg& rLoadArg)
{
    DirectResource* resource = newResource_(rLoadArg.instance_heap, rLoadArg.instance_alignment);

    if (resource == nullptr)
    {
        return nullptr;
    }

    FileDevice::LoadArg fileLoadArg;
    u8* data;

    fileLoadArg.path = rLoadArg.path;
    fileLoadArg.buffer = rLoadArg.load_data_buffer;
    fileLoadArg.buffer_size = rLoadArg.load_data_buffer_size;
    fileLoadArg.buffer_size_alignment = rLoadArg.load_data_buffer_alignment;
    fileLoadArg.heap = rLoadArg.load_data_heap;
    fileLoadArg.div_size = rLoadArg.div_size;
    fileLoadArg.assert_on_alloc_fail = rLoadArg.assert_on_alloc_fail;

    if (rLoadArg.load_data_alignment != 0)
    {
        fileLoadArg.alignment = rLoadArg.load_data_alignment;
    }
    else
    {
        fileLoadArg.alignment =
            Mathi::sign(rLoadArg.instance_alignment) * resource->getLoadDataAlignment();
    }

    if (rLoadArg.device != nullptr)
    {
        data = rLoadArg.device->tryLoad(fileLoadArg);
    }
    else
    {
        data = FileDeviceMgr::instance()->tryLoad(fileLoadArg);
    }

    if (data == nullptr)
    {
        delete resource;
        return nullptr;
    }

    resource->create(data, fileLoadArg.read_size, fileLoadArg.roundup_size, fileLoadArg.need_unload,
                     rLoadArg.instance_heap);
    return resource;
}

Resource* DirectResourceFactoryBase::tryCreateWithDecomp(const ResourceMgr::LoadArg& rLoadArg,
                                                         Decompressor* pDecompressor)
{
    DirectResource* resource = newResource_(rLoadArg.instance_heap, rLoadArg.instance_alignment);

    if (resource == nullptr)
    {
        return nullptr;
    }

    u32 outSize = 0;
    u32 outAllocSize = 0;
    bool outAllocated = false;

    u8* data = pDecompressor->tryDecompFromDevice(rLoadArg, resource, &outSize, &outAllocSize,
                                                  &outAllocated);

    if (!data)
    {
        delete resource;
        return nullptr;
    }

    resource->create(data, outSize, outAllocSize, outAllocated, rLoadArg.instance_heap);
    return resource;
}

Resource* IndirectResourceFactoryBase::create(const ResourceMgr::CreateArg& rCreateArg)
{
    IndirectResource* resource = newResource_(rCreateArg.heap, rCreateArg.alignment);

    if (resource == nullptr)
    {
        return nullptr;
    }

    RamReadStream stream(rCreateArg.buffer, rCreateArg.file_size, Stream::Modes::Binary);
    resource->create(&stream, rCreateArg.file_size, rCreateArg.heap);

    return resource;
}

Resource* IndirectResourceFactoryBase::tryCreate(const ResourceMgr::LoadArg& rLoadArg)
{
    IndirectResource* resource = newResource_(rLoadArg.instance_heap, rLoadArg.instance_alignment);

    if (resource == nullptr)
    {
        return nullptr;
    }

    FileHandle handle;

    bool isOpen;

    if (rLoadArg.device)
    {
        isOpen =
            rLoadArg.device->tryOpen(&handle, rLoadArg.path, FileDevice::cFileOpenFlag_ReadOnly, 0);
    }
    else
    {
        isOpen = FileDeviceMgr::instance()->tryOpen(&handle, rLoadArg.path,
                                                    FileDevice::cFileOpenFlag_ReadOnly, 0);
    }

    if (!isOpen)
    {
        delete resource;
        return nullptr;
    }

    BufferFileDeviceReadStream stream(&handle, Stream::Modes::Binary);
    resource->create(&stream, handle.getFileSize(), rLoadArg.instance_heap);

    if (!handle.tryClose())
    {
        delete resource;
        return nullptr;
    }

    return resource;
}

Resource* IndirectResourceFactoryBase::tryCreateWithDecomp(const ResourceMgr::LoadArg& rLoadArg,
                                                           Decompressor* pDecompressor)
{
    IndirectResource* resource = newResource_(rLoadArg.instance_heap, rLoadArg.instance_alignment);

    if (resource == nullptr)
    {
        return nullptr;
    }

    u32 outSize = 0;
    u32 outAllocSize = 0;
    bool outAllocated = false;

    u8* data = pDecompressor->tryDecompFromDevice(rLoadArg, resource, &outSize, &outAllocSize,
                                                  &outAllocated);

    if (!data)
    {
        delete resource;
        return nullptr;
    }

    RamReadStream stream(data, outSize, Stream::Modes::Binary);
    resource->create(&stream, outSize, rLoadArg.instance_heap);
    delete[] data;

    return resource;
}

}  // namespace sead
