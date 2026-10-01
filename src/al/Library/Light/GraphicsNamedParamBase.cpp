#include "Library/Light/GraphicsNamedParamBase.hpp"

#include "Project/Base/StringUtil.hpp"

namespace al {
/**
 * Creates the name parameter.
 * @param pDefaultName Initial name.
 */
GraphicsNamedParamBase::GraphicsNamedParamBase(const char* pDefaultName) {
    mName = new agl::utl::Parameter<sead::FixedSafeString<64>>(
        sead::FixedSafeString<64>(pDefaultName), "name", "パラメータ名", this);
}

/**
 * Interpolates the parameters between two named parameters.
 * @param rA Start parameter.
 * @param rB End parameter.
 * @param rate Interpolation rate.
 */
void GraphicsNamedParamBase::interp(const GraphicsNamedParamBase& rA,
                                    const GraphicsNamedParamBase& rB, f32 rate) {
    copyLerp(rA, rB, rate);
}

/**
 * Compares the names of two named parameters.
 * @param rOther Parameter to compare with.
 * @return Whether the names are equal.
 */
bool GraphicsNamedParamBase::operator==(const GraphicsNamedParamBase& rOther) const {
    return isEqualString(getName(), rOther.getName());
}

}  // namespace al
