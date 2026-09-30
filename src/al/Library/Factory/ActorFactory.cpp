#include "Library/Factory/ActorFactory.hpp"

#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Constructs an actor factory.
 * @param pName Factory name.
 */
ActorFactory::ActorFactory(const char* pName) : Factory(pName) {}

/**
 * Sets the byml table used to convert object names to class names.
 * @param pData Byml data.
 */
void ActorFactory::setConvertNameData(const u8* pData) {
    mIter = ByamlIter(pData);
}

/**
 * Sets the byml table used to convert object names to class names.
 * @param rIter Byml iterator.
 */
void ActorFactory::setConvertNameData(const ByamlIter& rIter) {
    mIter = rIter;
}

/**
 * Converts an object name to its class name using the conversion table.
 * @param pName Object name.
 * @return The class name, or nullptr if there is no table or no entry.
 */
const char* ActorFactory::convertName(const char* pName) const {
    if (!mIter.isValid()) {
        return pName;
    }

    s32 size = mIter.getSize();
    for (s32 i = 0; i < size; i++) {
        ByamlIter iter;
        mIter.tryGetIterByIndex(&iter, i);

        const char* objectName = nullptr;
        if (!iter.tryGetStringByKey(&objectName, "ObjectName")) {
            continue;
        }
        if (!isEqualString(objectName, pName)) {
            continue;
        }

        const char* className = nullptr;
        if (iter.tryGetStringByKey(&className, "ClassName")) {
            return className;
        }
    }
    return nullptr;
}
}  // namespace al
