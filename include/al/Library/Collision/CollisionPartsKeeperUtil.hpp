#pragma once

namespace al {
class Triangle;

const char* getMaterialCodeName(const Triangle& rTriangle);
const char* getCollisionCodeName(const Triangle& rTriangle, const char* pCategory);
const char* getFloorCodeName(const Triangle& rTriangle);
const char* getWallCodeName(const Triangle& rTriangle);
const char* getCameraCodeName(const Triangle& rTriangle);
bool isFloorCode(const char* pCode, const Triangle& rTriangle);
bool isWallCode(const char* pCode, const Triangle& rTriangle);
bool isMaterialCode(const char* pCode, const Triangle& rTriangle);
bool isCameraCode(const char* pCode, const Triangle& rTriangle);

}  // namespace al
