#pragma once

#include <basis/seadTypes.h>

namespace sead
{
template <u32 TableSize, u32 MaxLength, u32 Tag, typename T>
class StringIdBase
{
public:
    StringIdBase() : mId(0) {}
    explicit StringIdBase(T id) : mId(id) {}

    T getId() const { return mId; }
    void setId(T id) { mId = id; }

    bool operator==(const StringIdBase& rhs) const { return mId == rhs.mId; }
    bool operator!=(const StringIdBase& rhs) const { return mId != rhs.mId; }

private:
    T mId;
};
}  // namespace sead
