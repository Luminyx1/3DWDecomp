#include "Project/Yaml/YamlFormatter.hpp"

#include <cstdarg>
#include <prim/seadSafeString.h>
#include <stream/seadStream.h>

#include "Project/Base/StringUtil.hpp"
#include "Project/Base/StringMatcher.hpp"

namespace al {
/**
 * Starts a new document.
 */
void YamlFormatter::startDocument() {
    mIndent = 0;
    writeString("---\n");
}

/**
 * Writes a formatted string to the stream.
 * @param pFormat format string
 */
void YamlFormatter::writeString(const char* pFormat, ...) {
    std::va_list args;
    va_start(args, pFormat);
    StringTmp<256> str;
    str.formatV(pFormat, args);
    va_end(args);
    mStream->writeString(str, str.calcLength());
}

/**
 * Starts a hash.
 * @param pKey key of the hash
 */
void YamlFormatter::startHash(const char* pKey) {
    writeIndent();
    writeString("%s:\n", pKey);
    mIndent++;
}

/**
 * Writes the indentation of the current line.
 */
void YamlFormatter::writeIndent() {
    if (mIndent != 0) {
        writeString("%*c", mIndent * 2, ' ');
    }
}

/**
 * Ends a hash.
 */
void YamlFormatter::endHash() {
    mIndent--;

    if (mIndent < 0) {
        mIndent = 0;
    }
}

/**
 * Starts an array.
 */
void YamlFormatter::startArray() {
    writeIndent();
    writeString("-\n");
    mIndent++;
}

/**
 * Ends an array.
 */
void YamlFormatter::endArray() {
    mIndent--;

    if (mIndent < 0) {
        mIndent = 0;
    }
}

/**
 * Writes a bool hash entry.
 * @param pKey key
 * @param value value
 */
void YamlFormatter::writeHashBool(const char* pKey, bool value) {
    writeIndent();
    writeString("%s: %s\n", pKey, value ? "true" : "false");
}

/**
 * Writes an int hash entry.
 * @param pKey key
 * @param value value
 */
void YamlFormatter::writeHashInt(const char* pKey, s32 value) {
    writeIndent();
    writeString("%s: %d\n", pKey, value);
}

/**
 * Writes an unsigned int hash entry.
 * @param pKey key
 * @param value value
 */
void YamlFormatter::writeHashUInt(const char* pKey, u32 value) {
    writeIndent();
    writeString("%s: %u\n", pKey, value);
}

/**
 * Writes a 64 bit int hash entry.
 * @param pKey key
 * @param value value
 */
void YamlFormatter::writeHashInt64(const char* pKey, s64 value) {
    writeIndent();
    writeString("%s: %lld\n", pKey, value);
}

/**
 * Writes an unsigned 64 bit int hash entry.
 * @param pKey key
 * @param value value
 */
void YamlFormatter::writeHashUInt64(const char* pKey, u64 value) {
    writeIndent();
    writeString("%s: %llu\n", pKey, value);
}

/**
 * Writes a float hash entry.
 * @param pKey key
 * @param value value
 */
void YamlFormatter::writeHashFloat(const char* pKey, f32 value) {
    writeIndent();
    writeString("%s: %f\n", pKey, value);
}

/**
 * Writes a string hash entry.
 * @param pKey key
 * @param pValue value
 */
void YamlFormatter::writeHashString(const char* pKey, const char* pValue) {
    writeIndent();
    writeBlockString(StringTmp<256>("%s: ", pKey).cstr(), pValue);
}

/**
 * Writes a string, as a block if it spans multiple lines.
 * @param pPrefix text written before the string
 * @param pValue string
 */
void YamlFormatter::writeBlockString(const char* pPrefix, const char* pValue) {
    StringMatcher matcher("*\n*");

    if (!matcher.tryMatch(pValue)) {
        writeString("%s'%s'\n", pPrefix, pValue);
        return;
    }

    writeString("%s|\n", pPrefix);
    mIndent++;

    while (pValue != nullptr) {
        if (!matcher.tryMatch(pValue)) {
            writeIndent();
            writeString("%s\n", pValue);
            break;
        }

        const StringMatcher::MatchInfo& info = matcher.getMatchInfo(1);
        StringTmp<256> line;
        matcher.getMatchedString(&line, 0);
        writeIndent();
        writeString("%s\n", line.cstr());
        pValue = info.mStart;
    }

    mIndent--;
}

/**
 * Writes a Vector2f hash entry.
 * @param pKey key
 * @param rValue value
 */
void YamlFormatter::writeHashV2f(const char* pKey, const sead::Vector2f& rValue) {
    startHash(pKey);
    writeHashFloat("X", rValue.x);
    writeHashFloat("Y", rValue.y);
    endHash();
}

/**
 * Writes a Vector3f hash entry.
 * @param pKey key
 * @param rValue value
 */
void YamlFormatter::writeHashV3f(const char* pKey, const sead::Vector3f& rValue) {
    startHash(pKey);
    writeHashFloat("X", rValue.x);
    writeHashFloat("Y", rValue.y);
    writeHashFloat("Z", rValue.z);
    endHash();
}

/**
 * Writes a Vector4f hash entry.
 * @param pKey key
 * @param rValue value
 */
void YamlFormatter::writeHashV4f(const char* pKey, const sead::Vector4f& rValue) {
    startHash(pKey);
    writeHashFloat("X", rValue.x);
    writeHashFloat("Y", rValue.y);
    writeHashFloat("Z", rValue.z);
    writeHashFloat("W", rValue.w);
    endHash();
}

/**
 * Writes a color hash entry.
 * @param pKey key
 * @param rValue value
 */
void YamlFormatter::writeHashColor(const char* pKey, const sead::Color4f& rValue) {
    startHash(pKey);
    writeHashFloat("R", rValue.r);
    writeHashFloat("G", rValue.g);
    writeHashFloat("B", rValue.b);
    writeHashFloat("A", rValue.a);
    endHash();
}

/**
 * Writes an empty hash entry.
 * @param pKey key
 */
void YamlFormatter::writeHashNull(const char* pKey) {
    writeIndent();
    writeString("%s: \n", pKey);
}

/**
 * Writes a bool array entry.
 * @param value value
 */
void YamlFormatter::writeArrayBool(bool value) {
    writeIndent();
    writeString("- %s\n", value ? "true" : "false");
}

/**
 * Writes an int array entry.
 * @param value value
 */
void YamlFormatter::writeArrayInt(s32 value) {
    writeIndent();
    writeString("- %d\n", value);
}

/**
 * Writes a float array entry.
 * @param value value
 */
void YamlFormatter::writeArrayFloat(f32 value) {
    writeIndent();
    writeString("- %f\n", value);
}

/**
 * Writes a string array entry.
 * @param pValue value
 */
void YamlFormatter::writeArrayString(const char* pValue) {
    writeIndent();
    writeBlockString("- ", pValue);
}

/**
 * Writes a 64 bit int array entry.
 * @param value value
 */
void YamlFormatter::writeArrayInt64(s64 value) {
    writeIndent();
    writeString("- %lld\n", value);
}

/**
 * Writes an unsigned 64 bit int array entry.
 * @param value value
 */
void YamlFormatter::writeArrayUInt64(u64 value) {
    writeIndent();
    writeString("- %llu\n", value);
}

/**
 * Writes a double array entry.
 * @param value value
 */
void YamlFormatter::writeArrayDouble(f64 value) {
    writeIndent();
    writeString("- %8.8f\n", value);
}

/**
 * Sets the output stream.
 * @param pStream output stream
 */
void YamlFormatter::setStream(sead::WriteStream* pStream) {
    mStream = pStream;
}
}  // namespace al
