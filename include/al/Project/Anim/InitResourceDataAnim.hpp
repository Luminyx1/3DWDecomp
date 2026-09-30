#pragma once

namespace al {
class Resource;

class InitResourceDataAnim {
public:
    static InitResourceDataAnim* tryCreate(Resource* pModelRes, Resource* pAnimRes,
                                           Resource* pOtherRes);
};
}  // namespace al
