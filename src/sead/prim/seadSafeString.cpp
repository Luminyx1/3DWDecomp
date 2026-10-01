#include <prim/seadSafeString.h>
#include <prim/seadStringUtil.h>

namespace
{
static const char16 cEmptyStringChar16[1] = u"";

template <typename T>
inline bool isEqualCharIgnoreCase(T a, T b)
{
    return sead::StringUtil::toLowerCapital(a) == sead::StringUtil::toLowerCapital(b);
}

}  // namespace

namespace sead
{
template <>
const char SafeString::cNullChar = '\0';

template <>
const char SafeString::cLineBreakChar = '\n';

template <>
const SafeString SafeString::cEmptyString("");

template <>
const char16 WSafeString::cNullChar = 0;

template <>
const char16 WSafeString::cLineBreakChar = static_cast<char16>('\n');

template <>
const WSafeString WSafeString::cEmptyString(cEmptyStringChar16);

// NON_MATCHING: in-place branch computes the new-string copy address from the updated dst_i
// (target sign-extends the old index and subtracts new_str_len separately)
template <typename T>
s32 replaceStringImpl_(T* pDst, s32* pLength, s32 dstSize, const T* pSrc, s32 srcSize,
                       const SafeStringBase<T>& rOldStr, const SafeStringBase<T>& rNewStr,
                       bool* pIsBufferOverflow)
{
    s32 ret = 0;
    *pIsBufferOverflow = false;
    const s32 dst_max_idx = dstSize - 1;

    const T* old_cstr = rOldStr.cstr();
    const s32 old_str_len = rOldStr.calcLength();

    if (old_str_len == 0)
    {
        if (pDst == pSrc)
        {
            return 0;
        }

        *pIsBufferOverflow = srcSize >= dstSize;
        if (srcSize >= dstSize)
        {
            MemUtil::copy(pDst, pSrc, dst_max_idx * sizeof(T));
            pDst[dst_max_idx] = SafeStringBase<T>::cNullChar;

            if (pLength)
            {
                *pLength = dst_max_idx;
            }
        }
        else
        {
            MemUtil::copy(pDst, pSrc, (srcSize + 1) * sizeof(T));

            if (pLength)
            {
                *pLength = srcSize;
            }
        }

        return 0;
    }

    const T* new_cstr = rNewStr.cstr();
    const s32 new_str_len = rNewStr.calcLength();

    // Replace in-place.
    if (pDst == pSrc && old_str_len < new_str_len)
    {
        s32 src_final_size = 0;
        s32 dst_final_size = 0;
        // First, terminate the string and check for buffer overflow.
        while (src_final_size < srcSize)
        {
            const s32 cmp = MemUtil::compare(&pDst[src_final_size], old_cstr, old_str_len * sizeof(T));
            const s32 dst_step = cmp == 0 ? new_str_len : 1;
            const s32 src_step = cmp == 0 ? old_str_len : 1;
            dst_final_size += dst_step;
            src_final_size += src_step;

            if (dst_final_size >= dstSize)
            {
                *pIsBufferOverflow = true;
                break;
            }
        }

        if (*pIsBufferOverflow)
        {
            pDst[dst_max_idx] = SafeStringBase<T>::cNullChar;

            if (pLength)
            {
                *pLength = dst_max_idx;
            }
        }
        else
        {
            pDst[dst_final_size] = SafeStringBase<T>::cNullChar;

            if (pLength)
            {
                *pLength = dst_final_size;
            }
        }

        s32 dst_i = dst_final_size - 1;
        s32 src_i = src_final_size - 1;

        while (src_i >= 0)
        {
            if (MemUtil::compare(pDst + src_i - old_str_len + 1, old_cstr, old_str_len * sizeof(T)) == 0)
            {
                dst_i -= new_str_len;
                const s32 copy_size = std::min(dst_max_idx - dst_i - 1, new_str_len);

                if (copy_size > 0)
                {
                    MemUtil::copy(pDst + dst_i + 1, new_cstr, copy_size * sizeof(T));
                    ret += 1;
                }

                src_i -= old_str_len;
            }
            else
            {
                if (dst_i < dst_max_idx)
                {
                    pDst[dst_i] = pDst[src_i];
                }

                --dst_i;
                --src_i;
            }
        }
    }
    else
    {
        s32 buffer_i = 0;
        s32 target_i = 0;

        while (target_i < srcSize)
        {
            s32 dst_step;
            s32 src_step;

            if (MemUtil::compare(&pSrc[target_i], old_cstr, old_str_len * sizeof(T)) == 0)
            {
                const s32 rest = dst_max_idx - buffer_i;
                const s32 copy_size = std::min(rest, new_str_len);

                if (copy_size >= 1)
                {
                    MemUtil::copy(&pDst[buffer_i], new_cstr, copy_size * sizeof(T));
                }

                ret += new_str_len == 0 || copy_size > 0;

                if (new_str_len > rest)
                {
                    *pIsBufferOverflow = true;
                    pDst[dst_max_idx] = SafeStringBase<T>::cNullChar;

                    if (pLength)
                    {
                        *pLength = dst_max_idx;
                    }

                    return ret;
                }

                dst_step = new_str_len;
                src_step = old_str_len;
            }
            else
            {
                if (buffer_i >= dst_max_idx)
                {
                    *pIsBufferOverflow = true;
                    pDst[dst_max_idx] = SafeStringBase<T>::cNullChar;

                    if (pLength)
                    {
                        *pLength = dst_max_idx;
                    }

                    return ret;
                }

                pDst[buffer_i] = pSrc[target_i];
                dst_step = 1;
                src_step = 1;
            }

            buffer_i += dst_step;
            target_i += src_step;
        }

        pDst[buffer_i] = SafeStringBase<T>::cNullChar;

        if (pLength)
        {
            *pLength = buffer_i;
        }
    }

    return ret;
}

template s32 replaceStringImpl_<char>(char* buffer, s32* pLength, s32 buffer_size,
                                      const char* target_buf, s32 target_len,
                                      const SafeString& rOldStr,
                                      const SafeString& rNewStr, bool* pIsBufferOverflow);

template s32 replaceStringImpl_<char16>(char16* buffer, s32* pLength, s32 buffer_size,
                                        const char16* target_buf, s32 target_len,
                                        const WSafeString& rOldStr,
                                        const WSafeString& rNewStr,
                                        bool* pIsBufferOverflow);

/**
 * Checks whether the string contains a character, ignoring ASCII case.
 * @param c character to look for
 * @return whether c occurs in the string
 */
template <typename T>
bool SafeStringBase<T>::includeIgnoreCase(const T& c) const
{
    assureTerminationImpl_();

    for (s32 i = 0; i <= cMaximumLength; ++i)
    {
        if (unsafeAt_(i) == cNullChar)
        {
            break;
        }

        if (isEqualCharIgnoreCase(unsafeAt_(i), c))
        {
            return true;
        }
    }

    return false;
}

template bool SafeString::includeIgnoreCase(const char& c) const;
template bool WSafeString::includeIgnoreCase(const char16& c) const;

/**
 * Checks whether the string contains a substring, ignoring ASCII case.
 * @param str substring to look for (an empty substring is never found)
 * @return whether str occurs in the string
 */
template <typename T>
bool SafeStringBase<T>::includeIgnoreCase(const SafeStringBase<T>& str) const
{
    assureTerminationImpl_();
    const s32 len = calcLength();
    const s32 subStrLen = str.calcLength();

    for (s32 i = 0; i <= len - subStrLen; ++i)
    {
        for (s32 j = 0; j < subStrLen; ++j)
        {
            if (!isEqualCharIgnoreCase(unsafeAt_(i + j), str.unsafeAt_(j)))
            {
                break;
            }

            if (j == subStrLen - 1)
            {
                return true;
            }
        }
    }

    return false;
}

template bool SafeString::includeIgnoreCase(const SafeString& str) const;
template bool WSafeString::includeIgnoreCase(const WSafeString& str) const;

/**
 * Compares two strings for equality, ignoring ASCII case.
 * @param str string to compare with
 * @return whether both strings are equal
 */
template <typename T>
bool SafeStringBase<T>::isEqualIgnoreCase(const SafeStringBase<T>& str) const
{
    assureTerminationImpl_();

    if (cstr() == str.cstr())
    {
        return true;
    }

    for (s32 i = 0; i <= cMaximumLength; i++)
    {
        if (!isEqualCharIgnoreCase(unsafeAt_(i), str.unsafeAt_(i)))
        {
            return false;
        }

        if (unsafeAt_(i) == cNullChar)
        {
            return true;
        }
    }

    return false;
}

template bool SafeString::isEqualIgnoreCase(const SafeString& str) const;
template bool WSafeString::isEqualIgnoreCase(const WSafeString& str) const;

/**
 * Checks whether the string starts with a prefix, ignoring ASCII case.
 * @param prefix prefix to check
 * @return whether the string starts with prefix
 */
template <typename T>
bool SafeStringBase<T>::startsWithIgnoreCase(const SafeStringBase<T>& prefix) const
{
    const T* strc = mStringTop;
    const T* prefixc = prefix.mStringTop;
    s32 i = 0;

    while (prefixc[i] != cNullChar)
    {
        if (!isEqualCharIgnoreCase(strc[i], prefixc[i]))
        {
            return false;
        }

        ++i;
    }

    return true;
}

template bool SafeString::startsWithIgnoreCase(const SafeString& prefix) const;
template bool
WSafeString::startsWithIgnoreCase(const WSafeString& prefix) const;

/**
 * Checks whether the string ends with a suffix, ignoring ASCII case.
 * @param suffix suffix to check
 * @return whether the string ends with suffix
 */
template <typename T>
bool SafeStringBase<T>::endsWithIgnoreCase(const SafeStringBase<T>& suffix) const
{
    const s32 subStrLen = suffix.calcLength();

    if (subStrLen == 0)
    {
        return true;
    }

    const T* strc = mStringTop;
    const T* suffixc = suffix.mStringTop;

    const s32 len = calcLength();

    if (len < subStrLen)
    {
        return false;
    }

    for (s32 i = 0; i < subStrLen; ++i)
    {
        if (!isEqualCharIgnoreCase(strc[len - subStrLen + i], suffixc[i]))
        {
            return false;
        }
    }

    return true;
}

template bool SafeString::endsWithIgnoreCase(const SafeString& suffix) const;
template bool WSafeString::endsWithIgnoreCase(const WSafeString& suffix) const;

template <>
SafeString& SafeString::operator=(const SafeString& other) = default;

template <>
WSafeString&
WSafeString::operator=(const WSafeString& other) = default;

template <>
BufferedSafeString&
BufferedSafeString::operator=(const SafeString& other)
{
    copy(other);
    return *this;
}

template <>
WBufferedSafeString&
WBufferedSafeString::operator=(const WSafeString& other)
{
    copy(other);
    return *this;
}

template <>
HeapSafeString& HeapSafeString::operator=(const SafeString& other)
{
    this->copy(other);
    return *this;
}

template <>
HeapSafeStringBase<char16>&
HeapSafeStringBase<char16>::operator=(const WSafeString& other)
{
    this->copy(other);
    return *this;
}

template <>
void BufferedSafeString::assureTerminationImpl_() const
{
    auto* mutableSafeString = const_cast<BufferedSafeString*>(this);
    mutableSafeString->getMutableStringTop_()[mBufferSize - 1] = cNullChar;
}

template <>
void WBufferedSafeString::assureTerminationImpl_() const
{
    auto* mutableSafeString = const_cast<WBufferedSafeString*>(this);
    mutableSafeString->getMutableStringTop_()[mBufferSize - 1] = cNullChar;
}

template <>
s32 BufferedSafeString::formatImpl_(char* pS, s32 n, const char* pFormatStr, va_list args)
{
    const s32 ret = StringUtil::vsnprintf(pS, n, pFormatStr, args);
    return ret < 0 ? n - 1 : ret;
}

template <>
s32 WBufferedSafeString::formatImpl_(char16* pS, s32 n, const char16* pFormatStr,
                                                va_list args)
{
    const s32 ret = StringUtil::vsw16printf(pS, n, pFormatStr, args);

    if (ret >= 0 && ret < n)
    {
        return ret;
    }

    pS[n - 1] = WSafeString::cNullChar;
    return n - 1;
}

template <>
s32 BufferedSafeString::formatV(const char* pFormatStr, va_list args)
{
    char* mutableString = getMutableStringTop_();
    return formatImpl_(mutableString, mBufferSize, pFormatStr, args);
}

template <>
s32 WBufferedSafeString::formatV(const char16* pFormatStr, va_list args)
{
    char16* mutableString = getMutableStringTop_();
    return formatImpl_(mutableString, mBufferSize, pFormatStr, args);
}

template <>
s32 BufferedSafeString::format(const char* pFormatStr, ...)
{
    va_list args;
    va_start(args, pFormatStr);
    s32 ret = formatV(pFormatStr, args);
    va_end(args);

    return ret;
}

template <>
s32 WBufferedSafeString::format(const char16* pFormatStr, ...)
{
    va_list args;
    va_start(args, pFormatStr);
    s32 ret = formatV(pFormatStr, args);
    va_end(args);

    return ret;
}

template <>
s32 BufferedSafeString::appendWithFormatV(const char* pFormat, std::va_list args)
{
    char* mutableString = getMutableStringTop_();
    const s32 len = calcLength();
    return formatImpl_(mutableString + len, mBufferSize - len, pFormat, args) + len;
}

template <>
s32 WBufferedSafeString::appendWithFormatV(const char16* pFormat, std::va_list args)
{
    char16* mutableString = getMutableStringTop_();
    const s32 len = calcLength();
    return formatImpl_(mutableString + len, mBufferSize - len, pFormat, args) + len;
}

template <>
s32 BufferedSafeString::appendWithFormat(const char* pFormat, ...)
{
    std::va_list args;
    va_start(args, pFormat);
    const s32 ret = appendWithFormatV(pFormat, args);
    va_end(args);
    return ret;
}

template <>
s32 WBufferedSafeString::appendWithFormat(const char16* pFormat, ...)
{
    std::va_list args;
    va_start(args, pFormat);
    const s32 ret = appendWithFormatV(pFormat, args);
    va_end(args);
    return ret;
}

}  // namespace sead
