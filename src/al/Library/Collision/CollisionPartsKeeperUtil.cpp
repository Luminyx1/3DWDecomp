#include "Library/Collision/CollisionPartsKeeperUtil.hpp"

#include "Library/Yaml/ByamlIter.hpp"
#include "Library/Yaml/ByamlUtil.hpp"
#include "Project/Base/StringUtil.hpp"
#include "Project/Collision/CollisionPartsTriangle.hpp"

namespace al {
/**
 * Gets the material code of a triangle.
 * @param rTriangle triangle
 * @return the material code, or nullptr
 */
const char* getMaterialCodeName(const Triangle& rTriangle) {
    return getCollisionCodeName(rTriangle, "MaterialCode");
}

/**
 * Gets a collision code of a triangle.
 * @param rTriangle triangle
 * @param pCategory code category
 * @return the collision code, or nullptr
 */
const char* getCollisionCodeName(const Triangle& rTriangle, const char* pCategory) {
    ByamlIter attributes;
    rTriangle.getAttributes(&attributes);
    if (!attributes.isValid()) {
        return nullptr;
    }
    if (isTypeStringByKey(attributes, pCategory)) {
        const char* code = nullptr;
        if (attributes.tryGetStringByKey(&code, pCategory)) {
            return code;
        }
    }
    ByamlIter codeIter;
    if (attributes.tryGetIterByKey(&codeIter, pCategory)) {
        const char* code = nullptr;
        if (codeIter.tryGetStringByIndex(&code, 0)) {
            return code;
        }
    }
    return nullptr;
}

/**
 * Gets the floor code of a triangle.
 * @param rTriangle triangle
 * @return the floor code, or nullptr
 */
const char* getFloorCodeName(const Triangle& rTriangle) {
    return getCollisionCodeName(rTriangle, "FloorCode");
}

/**
 * Gets the wall code of a triangle.
 * @param rTriangle triangle
 * @return the wall code, or nullptr
 */
const char* getWallCodeName(const Triangle& rTriangle) {
    return getCollisionCodeName(rTriangle, "WallCode");
}

/**
 * Gets the camera code of a triangle.
 * @param rTriangle triangle
 * @return the camera code, or nullptr
 */
const char* getCameraCodeName(const Triangle& rTriangle) {
    return getCollisionCodeName(rTriangle, "CameraCode");
}

/**
 * Checks the floor code of a triangle.
 * @param pCode code to compare
 * @param rTriangle triangle
 * @return true if the triangle has the floor code
 */
bool isFloorCode(const char* pCode, const Triangle& rTriangle) {
    return isEqualString(pCode, getFloorCodeName(rTriangle));
}

/**
 * Checks the wall code of a triangle.
 * @param pCode code to compare
 * @param rTriangle triangle
 * @return true if the triangle has the wall code
 */
bool isWallCode(const char* pCode, const Triangle& rTriangle) {
    return isEqualString(pCode, getWallCodeName(rTriangle));
}

/**
 * Checks the material code of a triangle.
 * @param pCode code to compare
 * @param rTriangle triangle
 * @return true if the triangle has the material code
 */
bool isMaterialCode(const char* pCode, const Triangle& rTriangle) {
    return isEqualString(pCode, getMaterialCodeName(rTriangle));
}

/**
 * Checks the camera code of a triangle.
 * @param pCode code to compare
 * @param rTriangle triangle
 * @return true if the triangle has the camera code
 */
bool isCameraCode(const char* pCode, const Triangle& rTriangle) {
    return isEqualString(pCode, getCameraCodeName(rTriangle));
}
}  // namespace al
