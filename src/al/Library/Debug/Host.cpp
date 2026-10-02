#include "Library/Debug/Host.hpp"

#include <nn/os.h>
#include <prim/seadEnvUtil.h>

#include "Project/Base/StringMatcher.hpp"

namespace al {

/**
 * Reads an environment variable into a string, clearing it first.
 * @param pOut Receives the variable's value.
 * @param pName Name of the environment variable.
 * @return True if the variable exists and is not empty.
 */
static inline bool tryGetEnvironmentVariable(sead::BufferedSafeString* pOut, const char* pName) {
    pOut->format("");
    pOut->format("");

    return sead::EnvUtil::getEnvironmentVariable(pOut, pName) > 0;
}

/**
 * Gets the name of the host computer from the COMPUTERNAME environment variable.
 * @param pComputerName Receives the computer name.
 * @return True if the computer name could be read.
 */
bool tryGetComputerName(sead::BufferedSafeString* pComputerName) {
    return tryGetEnvironmentVariable(pComputerName, "COMPUTERNAME");
}

/**
 * Gets the name of the host computer, leaving the string empty if it is unknown.
 * @param pComputerName Receives the computer name.
 */
void getComputerName(sead::BufferedSafeString* pComputerName) {
    tryGetComputerName(pComputerName);
}

/**
 * Gets the name of the user from the USERNAME environment variable.
 * @param pUserName Receives the user name, or "NO_USER" if it is unknown.
 */
void getUserName(sead::BufferedSafeString* pUserName) {
    if (!tryGetEnvironmentVariable(pUserName, "USERNAME")) {
        pUserName->format("NO_USER");
    }
}

/**
 * Builds a file name that is unique to this computer and moment in time.
 * @param pOut Receives the file name.
 * @param pFileName Suffix appended to the generated name.
 */
void makeUniqueTemporaryFilename(sead::BufferedSafeString* pOut, const char* pFileName) {
    StringTmp<128> computerName;
    getComputerName(&computerName);

    nn::os::Tick tick = nn::os::GetSystemTick();
    pOut->format("%s_%012lld%s", computerName.cstr(), tick, pFileName);
}

/**
 * Appends a string to the output, expanding the first "${NAME}" reference and recursing on the
 * remainder. If the variable does not exist, the rest of the string is appended unchanged.
 * @param pOut String the expanded text is appended to.
 * @param rStr String to expand.
 */
static void appendExpandedEnvironmentString(sead::BufferedSafeString* pOut,
                                            const sead::SafeString& rStr) {
    StringMatcher matcher("*${*}*");

    if (!matcher.tryMatch(rStr.cstr())) {
        pOut->append(rStr);
        return;
    }

    const StringMatcher::MatchInfo& prefixInfo = matcher.getMatchInfo(0);
    const StringMatcher::MatchInfo& suffixInfo = matcher.getMatchInfo(2);

    StringTmp<128> name;
    matcher.getMatchedString(&name, 1);

    StringTmp<128> value;
    if (!tryGetEnvironmentVariable(&value, name.cstr())) {
        pOut->append(rStr);
        return;
    }

    pOut->append(prefixInfo.mStart, prefixInfo.mEnd - prefixInfo.mStart);
    pOut->append(value);

    if (suffixInfo.mStart != nullptr) {
        appendExpandedEnvironmentString(pOut, rStr.getPart(suffixInfo.mStart - rStr.cstr()));
    }
}

/**
 * Expands every "${NAME}" environment variable reference in a string.
 * @param pOut Receives the expanded string.
 * @param rStr String to expand.
 */
void expandEnvironmentString(sead::BufferedSafeString* pOut, const sead::SafeString& rStr) {
    pOut->clear();
    appendExpandedEnvironmentString(pOut, rStr);
}

/**
 * Expands every "${NAME}" environment variable reference in a string.
 * @param rStr String to expand.
 * @return Temporary string holding the expanded string.
 */
StringTmp<128> makeTmpExpandEnvironmentString(const sead::SafeString& rStr) {
    StringTmp<128> str;
    expandEnvironmentString(&str, rStr);

    return str;
}

/**
 * Builds a unique path inside the TEMP directory.
 * @param pFileName Suffix appended to the generated name, may be null.
 * @return Temporary string holding the path.
 */
StringTmp<128> makeTmpFileFullPath(const char* pFileName) {
    return StringTmp<128>("${TEMP}/%012lld%s", nn::os::GetSystemTick(),
                          pFileName != nullptr ? pFileName : "");
}

/**
 * @return Path of the shared AL tool data directory.
 */
const char* getALCommon() {
    return "${AL_TOOL_ROOT}/ALCommon";
}

}  // namespace al
