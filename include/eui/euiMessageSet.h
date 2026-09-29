#pragma once

#include <eui/euiMessageString.h>
#include <message/seadMessageSet.h>

namespace eui {

class MessageSet : public sead::MessageSet<char16_t> {
public:
    MessageSet();
    /** @brief Destroys the message wrapper; resource finalization is explicit. */
    ~MessageSet() override = default;

    MessageString findMessage(const char* pLabel) const;
    MessageString tryFindMessage(const char* pLabel) const;
    MessageString findMessageByIndex(int index) const;
    MessageString tryFindMessageByIndex(int index) const;
    bool hasMessageByIndex(int index) const;
    bool hasMessage(const char* pLabel) const;
    int calcMessageLength(const char* pLabel) const;
    int calcMessageLengthByIndex(int index) const;
};

}  // namespace eui
