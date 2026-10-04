#pragma once
#include <basis/seadTypes.h>
class GameDataHolder;
namespace sead {
class ReadStream;
class WriteStream;
} // namespace sead
class GameDataPlayReportCommon {
  public:
    explicit GameDataPlayReportCommon(GameDataHolder* pHolder);
    void initializeData();
    bool checkValid();
    bool readFromStream(sead::ReadStream* pStream);
    void writeToStream(sead::WriteStream* pStream) const;

  private:
    u32 mValues[23];
};
static_assert(sizeof(GameDataPlayReportCommon) == 0x5c);
