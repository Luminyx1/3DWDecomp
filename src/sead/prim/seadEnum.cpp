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
/**
 * Returns the critical section guarding enum text parsing.
 * @return the parse text critical section
 */
CriticalSection* EnumUtil::getParseTextCS_()
{
    return sEnumParseTextCriticalSection.getObject();
}

/**
 * Returns the critical section guarding enum value array initialization.
 * @return the value array initialization critical section
 */
CriticalSection* EnumUtil::getInitValueArrayCS_()
{
    return sEnumInitValueArrayCriticalSection.getObject();
}

/**
 * Reports a failed enum text parse, printing the parsed words in debug builds.
 * @param pTextPtr the array of parsed word pointers
 * @param v the number of words parsed
 */
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

/**
 * Splits a comma-separated enum text into null-terminated words, skipping any value initializers.
 * @param pTextPtr the array that receives the word pointers
 * @param pTextAll the full enum text, modified in place
 * @param size the expected number of words
 */
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
/**
 * Advances to the end of the current word, trimming trailing whitespace.
 * @param p_ptr in: the word start; out: one past the last non-whitespace character
 * @param pNext receives the position of the terminating separator
 */
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

/**
 * Advances past whitespace and commas to the start of the next word.
 * @param pPtr the text position to advance
 */
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
