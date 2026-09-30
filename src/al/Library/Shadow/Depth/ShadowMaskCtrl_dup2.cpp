#include "Library/Shadow/ShadowKeeper.hpp"

#include "Library/Shadow/ShadowMaskBase.hpp"
#include "Library/Yaml/MacroUtil.hpp"

#define AL_YAML_PARAM_GROUP(NAME)                                                                  \
    alYamlMacroUtil::YamlParamGroup NAME =                                                         \
        (alYamlMacroUtil::YamlParamGroup::sCurrent = &NAME, alYamlMacroUtil::YamlParamGroup());
#define AL_YAML_PARAM(TYPE, NAME) alYamlMacroUtil::YamlParam_##TYPE instance_##NAME(#NAME);

namespace np_ShadowMaskCommon {
AL_YAML_PARAM_GROUP(ShadowMaskCommon)
AL_YAML_PARAM(YamlString, Name)
AL_YAML_PARAM(YamlString, ShadowMaskType)
AL_YAML_PARAM(YamlString, ActorJointName)
AL_YAML_PARAM(V3f, Offset)
AL_YAML_PARAM(YamlColor, Color)
AL_YAML_PARAM(bool, IsApplyShadowIntensityUser)
AL_YAML_PARAM(u8, ShadowIntensityUser)
AL_YAML_PARAM(bool, IsIgnoreHide)
AL_YAML_PARAM(bool, IsFollowHostScale)
AL_YAML_PARAM(YamlString, DrawCategory)
AL_YAML_PARAM(bool, IsShadowFixed)
}  // namespace np_ShadowMaskCommon

namespace np_ShadowMaskSphereParam {
AL_YAML_PARAM_GROUP(ShadowMaskSphereParam)
AL_YAML_PARAM(f32, Scale)
AL_YAML_PARAM(f32, Exp)
AL_YAML_PARAM(bool, IsEnableCollisionCheck)
AL_YAML_PARAM(f32, CollisionCheckLength)
}  // namespace np_ShadowMaskSphereParam

namespace np_ShadowMaskCylinderParam {
AL_YAML_PARAM_GROUP(ShadowMaskCylinderParam)
AL_YAML_PARAM(f32, Radius)
AL_YAML_PARAM(f32, ExpXZ)
AL_YAML_PARAM(f32, ExpY)
AL_YAML_PARAM(f32, DistYBase)
}  // namespace np_ShadowMaskCylinderParam

namespace np_ShadowMaskCubeParam {
AL_YAML_PARAM_GROUP(ShadowMaskCubeParam)
AL_YAML_PARAM(V3f, Scale)
AL_YAML_PARAM(V3f, Exp)
AL_YAML_PARAM(f32, DistYBase)
AL_YAML_PARAM(YamlString, TextureBaseName)
AL_YAML_PARAM(f32, TextureFixedScale)
}  // namespace np_ShadowMaskCubeParam

namespace np_ShadowMaskCastOvalCylinderParam {
AL_YAML_PARAM_GROUP(ShadowMaskCastOvalCylinderParam)
AL_YAML_PARAM(V3f, Scale)
AL_YAML_PARAM(f32, ExpXZ)
AL_YAML_PARAM(f32, ExpY)
AL_YAML_PARAM(f32, DistYBase)
}  // namespace np_ShadowMaskCastOvalCylinderParam

namespace al {

/**
 * Checks whether a draw category uses the shadow intensity.
 * @param category Draw category.
 * @return Whether the category is a shadow category.
 */
bool isShadowIntensity(s32 category) {
    return category < ShadowMaskDrawCategory::LightScale;
}

/**
 * Checks whether a draw category is a light scale category.
 * @param category Draw category.
 * @return Whether the category is a light scale category.
 */
bool isShadowLightScale(s32 category) {
    return category == ShadowMaskDrawCategory::LightScale ||
           category == ShadowMaskDrawCategory::LightScaleLight;
}

/**
 * Checks whether a draw category is drawn to multiple render targets.
 * @param category Draw category.
 * @return Whether the category uses multiple render targets.
 */
bool isShadowMrt(s32 category) {
    switch (category) {
    case ShadowMaskDrawCategory::LightScaleLight:
    case ShadowMaskDrawCategory::MapObjAOSO:
    case ShadowMaskDrawCategory::MapObjAndWaterAOSO:
    case ShadowMaskDrawCategory::EnemyAOSO:
    case ShadowMaskDrawCategory::PlayerAOSO:
    case ShadowMaskDrawCategory::AllAOSO:
        return true;
    default:
        return false;
    }
}

}  // namespace al
