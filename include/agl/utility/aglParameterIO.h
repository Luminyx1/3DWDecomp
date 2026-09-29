#pragma once

#include "utility/aglParameterList.h"

namespace sead {
class XmlDocument;
}

namespace agl::utl {

class IParameterIO : public IParameterList {
public:
    IParameterIO();
    IParameterIO(const sead::SafeString& name, u32 version);
    ~IParameterIO() override;

    virtual bool save(const sead::SafeString& path, u32) const;
    virtual void applyResParameterArchive(ResParameterArchive arc);
    virtual void applyResParameterArchiveLerp(ResParameterArchive arc_a, ResParameterArchive arc_b,
                                              f32 t);

    bool load(const sead::SafeString& rPath, bool dump);
    s32 loadText(const void* pData, u32 size, bool x);

    bool isCompleteArchive(ResParameterArchive archive, bool checkValues) const;

    void genMessageIO(sead::hostio::Context* pContext, u32 flags);
    s32 listenPropertyEventIO(sead::hostio::Reflexible* pReflexible,
                              const sead::hostio::PropertyEvent* pEvent);

protected:
    virtual void callbackInvalidVersion_(ResParameterArchive) {}

    void writeHeader_(sead::XmlElement* pElement, sead::Heap* pHeap) const;
    bool save_(const sead::SafeString& rPath, const sead::XmlDocument* pDocument) const;

    sead::FixedSafeString<64> mType;
    u32 mVersion;
    void* _f8 = nullptr;
    sead::FixedSafeString<256> mPath = sead::SafeString::cEmptyString;
    u32 mResFileSize = 0;
    u32 _21c = 0;
};

}  // namespace agl::utl
