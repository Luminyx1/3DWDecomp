#include "Library/Debug/CommandOption.hpp"

#include <prim/seadStringUtil.h>

#include "Project/Base/StringUtil.hpp"

namespace al {

/**
 * Checks whether the first space separated token of a command line is the given command name.
 * @param pCommandLine Command line to check.
 * @param pName Command name to compare against.
 * @return True if the command line starts with the command name.
 */
bool isCommandName(const char* pCommandLine, const char* pName) {
    sead::SafeString commandLine(pCommandLine);
    sead::SafeString::token_iterator it = commandLine.tokenBegin(" ");
    StringTmp<128> command;
    it.get(&command);

    return command.isEqual(pName);
}

/**
 * Searches a command line for an option token of the form "name" or "name=value".
 * @param pCommandLine Command line to search, tokens separated by spaces.
 * @param pOptionName Name of the option.
 * @param pOutValue Receives the option's value (empty if it has none), may be null.
 * @return True if the option was found.
 */
bool isExistOption(const char* pCommandLine, const char* pOptionName,
                   sead::BufferedSafeString* pOutValue) {
    sead::SafeString commandLine(pCommandLine);
    sead::SafeString::token_iterator it = commandLine.tokenBegin(" ");
    const sead::SafeString::token_iterator end = commandLine.tokenEnd(" ");
    StringTmp<128> option;

    for (; end != it; ++it) {
        it.get(&option);

        sead::SafeString::token_iterator optionIt = option.tokenBegin("=");
        StringTmp<128> name;
        optionIt.get(&name);

        if (name.isEqual(pOptionName)) {
            if (pOutValue != nullptr) {
                ++optionIt;

                if (optionIt == option.tokenEnd("=")) {
                    pOutValue->format("");
                } else {
                    optionIt.get(pOutValue);
                }
            }

            return true;
        }
    }

    return false;
}

/**
 * Gets the integer value of a "name=value" option.
 * @param pOut Receives the value.
 * @param pCommandLine Command line to search.
 * @param pOptionName Name of the option.
 * @return True if the option exists and its value is an integer.
 */
bool tryGetIntOptionValue(s32* pOut, const char* pCommandLine, const char* pOptionName) {
    StringTmp<128> value;
    if (!isExistOption(pCommandLine, pOptionName, &value)) {
        return false;
    }

    return sead::StringUtil::tryParseNumber(pOut, value, sead::StringUtil::CardinalNumber::BaseAuto);
}

/**
 * Gets the floating point value of a "name=value" option.
 * @param pOut Receives the value.
 * @param pCommandLine Command line to search.
 * @param pOptionName Name of the option.
 * @return True if the option exists and its value is a number.
 */
bool tryGetFloatOptionValue(f32* pOut, const char* pCommandLine, const char* pOptionName) {
    StringTmp<128> value;
    if (!isExistOption(pCommandLine, pOptionName, &value)) {
        return false;
    }

    return sead::StringUtil::tryParseNumber(pOut, value, sead::StringUtil::CardinalNumber::BaseAuto);
}

/**
 * Gets the boolean value of a "name=value" option, given as an integer, "true" or "false".
 * @param pOut Receives the value.
 * @param pCommandLine Command line to search.
 * @param pOptionName Name of the option.
 * @return True if the option exists and its value is a boolean.
 */
bool tryGetBoolOptionValue(bool* pOut, const char* pCommandLine, const char* pOptionName) {
    StringTmp<128> value;
    if (!isExistOption(pCommandLine, pOptionName, &value)) {
        return false;
    }

    s32 number = 0;
    if (sead::StringUtil::tryParseNumber(&number, value,
                                         sead::StringUtil::CardinalNumber::BaseAuto)) {
        *pOut = number != 0;
        return true;
    }

    if (isEqualStringCase(value, "true")) {
        *pOut = true;
        return true;
    }

    if (isEqualStringCase(value, "false")) {
        *pOut = false;
        return true;
    }

    return false;
}

/**
 * Gets the string value of a "name=value" option.
 * @param pOut Receives the value.
 * @param pCommandLine Command line to search.
 * @param pOptionName Name of the option.
 * @return True if the option exists.
 */
bool tryGetStringOptionValue(sead::BufferedSafeString* pOut, const char* pCommandLine,
                             const char* pOptionName) {
    return isExistOption(pCommandLine, pOptionName, pOut);
}

}  // namespace al
