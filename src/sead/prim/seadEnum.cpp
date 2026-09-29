#include <basis/seadRawPrint.h>
#include <prim/seadEnum.h>
#include <thread/seadCriticalSection.h>

namespace
{
class EnumParseTextCriticalSection
{
public:
    EnumParseTextCriticalSection()
    {
        (void)getObject();  // force initialization of sObject
    }
    sead::CriticalSection* getObject()
    {
        static sead::CriticalSection sObject;
        return &sObject;
    }
};
static EnumParseTextCriticalSection sEnumParseTextCriticalSection;

class EnumInitValueArrayCriticalSection
{
public:
    EnumInitValueArrayCriticalSection()
    {
        (void)getObject();  // force initialization of sObject
    }
    sead::CriticalSection* getObject()
    {
        static sead::CriticalSection sObject;
        return &sObject;
    }
};
static EnumInitValueArrayCriticalSection sEnumInitValueArrayCriticalSection;
}  // namespace

namespace sead
{
CriticalSection* EnumUtil::getParseTextCS_()
{
    return sEnumParseTextCriticalSection.getObject();
}

CriticalSection* EnumUtil::getInitValueArrayCS_()
{
    return sEnumInitValueArrayCriticalSection.getObject();
}

void ParseFailed_([[maybe_unused]] char** pTextPtr, [[maybe_unused]] int v)
{
#ifdef SEAD_DEBUG
    system::Print("----------------------------------------\n");
    for (int i = 0; i < v; ++i)
    {
        system::Print("  text[%d] \"%s\"\n", i, pTextPtr[i]);
    }
    system::Print("----------------------------------------\n");
    SEAD_ASSERT_MSG(false, "SEAD_ENUM failed to parse text. Is number of comma correct?");
#endif
}

void EnumUtil::parseText_(char** pTextPtr, char* pTextAll, int size)
{
    int index = 0;
    while (*pTextAll)
    {
        skipToWordStart_(&pTextAll);
        if (*pTextAll == 0)
        {
            break;
        }

        pTextPtr[index] = pTextAll;
        ++index;

        char* next;
        skipToWordEnd_(&pTextAll, &next);
        const char next_char = *next;
        *pTextAll = 0;

        if (next_char == '=')
        {
            while (!(*++next == '\0' || *next == ',' || *next == '='))
                ;

            if (*next == '\0')
            {
                break;
            }
        }
        else if (next_char == '\0')
        {
            break;
        }

        // TODO: This is missing a call to skipToWordEnd_ and ParseFailed_ for the debug/develop
        // targets.
        if (index >= size)
        {
            break;
        }

        pTextAll = ++next;
    }

    if (index != size)
    {
        ParseFailed_(pTextPtr, index);
    }
}

// Example:
// AoCVerAtLastPlay   ,LatestAoCVerPlayed
// ^               ^  ^
// initial p       |  next (pNext)
//                 end (p_ptr)
void EnumUtil::skipToWordEnd_(char** p_ptr, char** pNext)
{
    char* p = *p_ptr;
    while (!(*p == '\0' || *p == ',' || *p == '='))
    {
        ++p;
    }

    *pNext = p;

    --p;
    while ((*p == '\t' || *p == '\n' || *p == ' ') && intptr_t(p) > intptr_t(*p_ptr))
    {
        --p;
    }

    *p_ptr = p + 1;
}

void EnumUtil::skipToWordStart_(char** pPtr)
{
    char* p = *pPtr;
    while (*p == '\t' || *p == '\n' || *p == ' ' || *p == ',')
    {
        ++p;
    }
    *pPtr = p;
}

}  // namespace sead
