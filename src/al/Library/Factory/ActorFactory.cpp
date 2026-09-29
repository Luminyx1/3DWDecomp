#include "Library/Factory/ActorFactory.hpp"

#include "Project/Base/StringUtil.hpp"

namespace al {
    /**
     * @brief Constructs an actor factory.
     * @param pName The name of the factory.
     */
    ActorFactory::ActorFactory(const char* pName) : Factory(pName) {}

    /**
     * @brief Sets the byaml data used to convert object names to class names.
     * @param pData The raw byaml data.
     */
    void ActorFactory::setConvertNameData(const u8* pData) {
        mIter = ByamlIter(pData);
    }

    /**
     * @brief Sets the byaml iterator used to convert object names to class names.
     * @param rIter The byaml iterator.
     */
    void ActorFactory::setConvertNameData(const ByamlIter& rIter) {
        mIter = rIter;
    }

    /**
     * @brief Converts an object name to its class name using the convert name data.
     * @param pName The object name.
     * @return The class name, pName if no convert data is set, or nullptr if no entry matches.
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
            if (iter.tryGetStringByKey(&objectName, "ObjectName") && isEqualString(objectName, pName)) {
                const char* className = nullptr;
                if (iter.tryGetStringByKey(&className, "ClassName")) {
                    return className;
                }
            }
        }

        return nullptr;
    }
}  // namespace al
