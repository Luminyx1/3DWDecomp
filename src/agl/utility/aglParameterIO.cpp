#include "utility/aglParameterIO.h"
#include <basis/seadRawPrint.h>

namespace agl::utl
{

IParameterIO::IParameterIO()
{
    mType = "xml";
    mVersion = 0;
    setParameterListName_("param_root");
}

IParameterIO::IParameterIO(const sead::SafeString& name, u32 version)
{
    mType = name;
    mVersion = version;
    setParameterListName_("param_root");
}

/**
 * Applies a parameter archive to this IO, reporting version mismatches.
 * @param arc archive to apply
 */
void IParameterIO::applyResParameterArchive(ResParameterArchive arc)
{
    SEAD_ASSERT(arc.isValid());
    mResFileSize = arc.ptr()->file_size;

    if (mVersion != arc.ptr()->pio_version)
    {
        callbackInvalidVersion_(arc);
    }

    applyResParameterList(arc.getRootList());
}

/**
 * Applies the interpolation of two parameter archives to this IO.
 * @param arc_a archive used at t = 0
 * @param arc_b archive used at t = 1
 * @param t interpolation factor
 */
void IParameterIO::applyResParameterArchiveLerp(ResParameterArchive arc_a,
                                                ResParameterArchive arc_b, f32 t)
{
    SEAD_ASSERT(arc_a.isValid());
    SEAD_ASSERT(arc_b.isValid());

    if (mVersion != arc_a.ptr()->pio_version)
    {
        callbackInvalidVersion_(arc_a);
    }

    if (mVersion != arc_b.ptr()->pio_version)
    {
        callbackInvalidVersion_(arc_b);
    }

    applyResParameterList(arc_a.getRootList(), arc_b.getRootList(), t);
}

void IParameterIO::genMessageIO(sead::hostio::Context* pContext, u32 flags)
{
    if (flags & 1)
    {
        sead::FormatFixedSafeString<1024> meta("Save (*.%s)", mType.cstr());
    }

    if (flags & 2)
    {
        sead::FormatFixedSafeString<1024> meta("Load (*.%s)", mType.cstr());
    }

    const char* is_enable = "false";
    if (mPath != sead::SafeString::cEmptyString)
    {
        sead::FormatFixedSafeString<1024> meta("%s", mPath.cstr());
        is_enable = "true";
    }

    {
        sead::FormatFixedSafeString<1024> meta("Mode = Small, IsEnable=%s", is_enable);
    }
    {
        sead::FormatFixedSafeString<1024> meta("Size of last binary loaded:%d[byte]", mResFileSize);
    }
}

/**
 * Checks whether an archive contains every list and object of this IO.
 * @param archive archive to check
 * @param checkValues forwarded to isComplete
 * @return whether the archive is complete
 */
bool IParameterIO::isCompleteArchive(ResParameterArchive archive, bool checkValues) const
{
    return isComplete(archive.getRootList(), checkValues);
}

/**
 * Destroys the parameter IO.
 */
IParameterIO::~IParameterIO() = default;

}  // namespace agl::utl
