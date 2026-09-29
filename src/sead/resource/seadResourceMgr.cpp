#include <basis/seadRawPrint.h>
#include <filedevice/seadPath.h>
#include <heap/seadHeapMgr.h>
#include <resource/seadResource.h>
#include <resource/seadResourceMgr.h>

namespace sead
{
SEAD_SINGLETON_DISPOSER_IMPL(ResourceMgr)

ResourceMgr::ResourceMgr()
{
    if (HeapMgr::sInstancePtr == NULL)
    {
        SEAD_ASSERT_MSG(false, "ResourceMgr need HeapMgr");
        return;
    }

    mNullResourceFactory =
        new (HeapMgr::sInstancePtr->findContainHeap(this)) DirectResourceFactory<DirectResource>();
    mDefaultResourceFactory = mNullResourceFactory;
    registerFactory(mNullResourceFactory, "");
}

ResourceMgr::~ResourceMgr()
{
    if (mNullResourceFactory == NULL)
    {
        return;
    }

    delete mNullResourceFactory;
    mNullResourceFactory = NULL;
}

void ResourceMgr::registerFactory(ResourceFactory* pFactory, const SafeString& rName)
{
    pFactory->setExt(rName);

    mFactoryList.pushBack(pFactory);
}

ResourceFactory* ResourceMgr::setDefaultFactory(ResourceFactory* pFactory)
{
    ResourceFactory* const previous_default = mDefaultResourceFactory;

    if (pFactory)
    {
        mDefaultResourceFactory = pFactory;
    }
    else
    {
        mDefaultResourceFactory = mNullResourceFactory;
    }
    registerFactory(mDefaultResourceFactory, "");

    return previous_default;
}

ResourceFactory* ResourceMgr::findFactory(const SafeString& rName)
{
    for (auto& factory : mFactoryList)
    {
        if (factory->getExt() == rName)
        {
            return factory;
        }
    }

    return mDefaultResourceFactory;
}

void ResourceMgr::registerDecompressor(Decompressor* pDecompressor, const SafeString& rName)
{
    if (!rName.isEqual(SafeString::cEmptyString))
    {
        pDecompressor->setName(rName);
    }

    mDecompList.pushBack(pDecompressor);
}

void ResourceMgr::unregisterFactory(ResourceFactory* pFactory)
{
    mFactoryList.erase(pFactory);
}

void ResourceMgr::unregisterDecompressor(Decompressor* pDecompressor)
{
    mDecompList.erase(pDecompressor);
}

Decompressor* ResourceMgr::findDecompressor(const SafeString& rName)
{
    for (auto& decompressor : mDecompList)
    {
        if (decompressor->getName() == rName)
        {
            return decompressor;
        }
    }

    return nullptr;
}

#if not SEAD_RESOURCEMGR_TRYCREATE_NO_FACTORY_NAME
Resource* ResourceMgr::tryLoad(const ResourceMgr::LoadArg& rArg, const SafeString& rFactoryName,
                               Decompressor* pDecompressor)
{
    SafeString actual_factory_name;
    FixedSafeString<32> ext;

    if (!pDecompressor)
    {
        if (!Path::getExt(&ext, rArg.path))
        {
            SEAD_ASSERT_MSG(false, "no file extension");
            return nullptr;
        }

        pDecompressor = findDecompressor(ext);
    }

    if (pDecompressor)
    {
        actual_factory_name = rFactoryName;
    }
    else
    {
        actual_factory_name = ext;
    }

    auto* factory = rArg.factory;
    if (!factory)
    {
        factory = findFactory(actual_factory_name);
        SEAD_ASSERT(factory);
    }

    if (rArg.has_tried_create_with_decomp)
    {
        *rArg.has_tried_create_with_decomp = pDecompressor;
    }

    if (pDecompressor)
    {
        return factory->tryCreateWithDecomp(rArg, pDecompressor);
    }
    return factory->tryCreate(rArg);
}
#endif

/**
 * Loads a resource without decompression, picking the factory from the argument or the path
 * extension.
 * @param rArg Load parameters.
 * @return Created resource, or nullptr on failure.
 */
Resource* ResourceMgr::tryLoadWithoutDecomp(const ResourceMgr::LoadArg& rArg)
{
    auto* factory = rArg.factory;
    if (!factory)
    {
        FixedSafeString<32> ext;
        if (!Path::getExt(&ext, rArg.path))
        {
            factory = mDefaultResourceFactory;
        }
        else
        {
            factory = findFactory(ext);
        }
    }
    return factory->tryCreate(rArg);
}

void ResourceMgr::unload(Resource* pRes)
{
    if (pRes)
    {
        delete pRes;
    }
}
// NON_MATCHING: tail call for factory->create
Resource* ResourceMgr::create(const ResourceMgr::CreateArg& rArg)
{
    if (!rArg.buffer)
    {
        SEAD_ASSERT_MSG(false, "buffer null");
        return nullptr;
    }
    if (rArg.file_size == 0)
    {
        SEAD_ASSERT_MSG(false, "file_size is 0");
        return nullptr;
    }
    if (rArg.buffer_size == 0)
    {
        SEAD_ASSERT_MSG(false, "buffer_size is 0");
        return nullptr;
    }

    if (rArg.factory)
    {
        return rArg.factory->create(rArg);
    }

    auto* factory = findFactory(rArg.ext);
    if (factory)
    {
        return factory->create(rArg);
    }

    SEAD_ASSERT_MSG(false, "factory not found: %s", rArg.ext.cstr());
    return nullptr;
}

}  // namespace sead
