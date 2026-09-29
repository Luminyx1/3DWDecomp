#include <filedevice/seadPath.h>
#include <prim/seadSafeString.h>

namespace sead
{
bool Path::getDriveName(BufferedSafeString* pDriveName, const SafeString& rPath)
{
    SEAD_ASSERT_MSG(pDriveName, "destination buffer is null");

    pDriveName->trim(0);

    const s32 index = rPath.findIndex(":");
    if (index == -1)
    {
        return false;
    }

    pDriveName->copy(rPath, index);
    return true;
}

void Path::getPathExceptDrive(BufferedSafeString* pathNoDrive, const SafeString& rPath)
{
    SEAD_ASSERT_MSG(pathNoDrive, "destination buffer is null");

    pathNoDrive->trim(0);

    s32 index = rPath.findIndex("://");
    if (index == -1)
    {
        pathNoDrive->copyAt(0, rPath);
    }

    else
    {
        pathNoDrive->copyAt(0, rPath.getPart(index + 3));
    }
}

namespace
{
s32 rfindCharIndex(const SafeString& rPath, char c)
{
    const s32 length = rPath.calcLength();
    const char* cstr = rPath.cstr();
    for (s32 i = length; i >= 0; --i)
    {
        if (cstr[i] == c)
        {
            return i;
        }
    }
    return -1;
}

char getLastChar(const SafeString& rStr)
{
    return rStr.at(rStr.calcLength() - 1);
}
}  // namespace

/**
 * Gets the extension of a path (the part after the last dot, if it is not part of a directory).
 * @param pExt receives the extension (cleared on failure)
 * @param rPath path to examine
 * @return whether the path has an extension
 */
bool Path::getExt(BufferedSafeString* pExt, const SafeString& rPath)
{
    SEAD_ASSERT_MSG(pExt, "destination buffer is null");

    pExt->trim(0);

    const s32 dotIndex = rfindCharIndex(rPath, '.');
    if (!(dotIndex >= 0))
    {
        return false;
    }

    if (rPath.getPart(dotIndex).include('/') || rPath.getPart(dotIndex).include('\\'))
    {
        return false;
    }

    pExt->copy(rPath.getPart(dotIndex + 1));
    return true;
}

bool Path::getFileName(BufferedSafeString* pName, const SafeString& rPath)
{
    SEAD_ASSERT_MSG(pName, "destination buffer is null");
    pName->trim(0);

    const s32 slash_index = rfindCharIndex(rPath, '/');
    const s32 bslash_index = rfindCharIndex(rPath, '\\');
    const s32 idx = slash_index > bslash_index ? slash_index : bslash_index;
    pName->copy(rPath.getPart(idx + 1));
    return true;
}

bool Path::getBaseFileName(BufferedSafeString* pName, const SafeString& rPath)
{
    const s32 bslash_index = rfindCharIndex(rPath, '\\');
    const s32 slash_index = rfindCharIndex(rPath, '/');

    const s32 i = bslash_index > slash_index ? bslash_index : slash_index;
    const s32 part_idx = i < 0 ? 0 : i + 1;

    s32 dot_idx = rfindCharIndex(rPath, '.');
    if (dot_idx < 0)
    {
        dot_idx = rPath.calcLength();
    }

    pName->copy(rPath.getPart(part_idx), dot_idx - part_idx);
    return true;
}

bool Path::getDirectoryName(BufferedSafeString* pName, const SafeString& rPath)
{
    SEAD_ASSERT_MSG(pName, "destination buffer is null");

    if (pName == &rPath)
    {
        const s32 slash_index = rfindCharIndex(rPath, '/');
        const s32 bslash_index = rfindCharIndex(rPath, '\\');
        const s32 trim_index = slash_index > bslash_index ? slash_index : bslash_index;
        if (trim_index < 1)
        {
            return false;
        }

        pName->trim(trim_index);
    }
    else
    {
        pName->trim(0);

        const s32 slash_index = rfindCharIndex(rPath, '/');
        const s32 bslash_index = rfindCharIndex(rPath, '\\');
        const s32 trim_index = slash_index > bslash_index ? slash_index : bslash_index;
        if (trim_index < 1)
        {
            return false;
        }

        pName->copy(rPath, trim_index);
    }
    return true;
}

void Path::join(BufferedSafeString* pOut, const char* path1, const char* path2)
{
    // Trivial case 1: path1 is empty.
    if (!path1 || !path1[0])
    {
        pOut->copy(path2);
        return;
    }

    // Trivial case 2: path2 is empty.
    if (!path2 || !path2[0])
    {
        pOut->copy(path1);
        return;
    }

    if (path2[0] == '\\' || path2[0] == '/')
    {
        // If path1 also ends with a slash, skip the slash in path2 to avoid getting "//".
        const char last_char1 = getLastChar(path1);
        if (last_char1 == '\\' || last_char1 == '/')
        {
            ++path2;
            if (!path2[0])
            {
                pOut->copy(path1);
                return;
            }
        }
        pOut->format("%s%s", path1, path2);
    }
    else
    {
        // If path1 already ends with a slash, do not insert "/" in the middle to avoid "//".
        const char last_char1 = getLastChar(path1);
        if (last_char1 == '\\' || last_char1 == '/')
        {
            pOut->format("%s%s", path1, path2);
        }
        else
        {
            pOut->format("%s/%s", path1, path2);
        }
    }
}

void Path::changeDelimiter(BufferedSafeString* pOut, char delimiter)
{
    const s32 length = pOut->calcLength();
    char* buffer = pOut->getBuffer();
    for (s32 i = 0; i < length; ++i)
    {
        const char c = (*pOut)[i];
        if (c == '\\' || c == '/')
        {
            buffer[i] = delimiter;
        }
    }
}
}  // namespace sead
