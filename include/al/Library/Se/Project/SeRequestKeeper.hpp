#pragma once

namespace al {
class SeRequest;

class SeRequestKeeper {
  public:
    void addRequestDirect(SeRequest* pRequest);
};
} // namespace al
