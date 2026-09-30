#include "Project/AreaObj/AreaShapeFactory.hpp"

#include "Project/AreaObj/AreaShapeCube.hpp"
#include "Project/AreaObj/AreaShapeRound.hpp"

namespace al {
static const NameToCreator<AreaShapeCreatorFunction> sAreaShapeFactoryEntries[] = {
    {"AreaCubeBase", createAreaShapeFunction<AreaShapeCubeBase>},
    {"AreaCubeCenter", createAreaShapeFunction<AreaShapeCubeCenter>},
    {"AreaSphere", createAreaShapeFunction<AreaShapeOvalBase>},
    {"AreaCylinder", createAreaShapeFunction<AreaShapeCylinderBase>},
};

/**
 * Constructs the area shape factory.
 * @param pName name of the factory
 */
AreaShapeFactory::AreaShapeFactory(const char* pName) : Factory(pName) {
    initFactory(sAreaShapeFactoryEntries);
}
}  // namespace al
