#include "g3d/aglShaderUtilG3D.h"

#include <heap/seadHeap.h>
#include <nn/g3d/g3d_ResShader.h>
#include <nn/gfx/gfx_Buffer.h>
#include <nvn/nvn_FuncPtrInline.h>

#include "common/aglDrawContext.h"
#include "common/aglShaderLocation.h"
#include "detail/aglPrivateResource.h"
#include "driver/aglNVNMgr.h"

namespace agl::g3d {

namespace {

constexpr s32 cPrintBufferSize = 0x100000;

/**
 * Replaces every tab of a string by a line feed.
 * @param pBuffer string
 * @param length length of the string
 */
void replaceTabs(char* pBuffer, s32 length)
{
    for (s32 i = 0; i < length; i++)
    {
        if (pBuffer[i] == '\t')
        {
            pBuffer[i] = '\n';
        }
    }
}

/**
 * Prints the keys and options of a shading model or shader selector.
 * @param rObj object to print
 * @param pBuffer output buffer
 * @return number of characters printed
 */
template <typename T>
s32 printAll(const T& rObj, char* pBuffer)
{
    s32 rawKeyLength = rObj.PrintRawKeyTo(pBuffer, cPrintBufferSize);
    s32 position = rawKeyLength;
    pBuffer[position++] = '\n';
    pBuffer[position++] = '\n';
    s32 keyLength = rObj.PrintKeyTo(pBuffer + position, cPrintBufferSize - position);
    position += keyLength;
    pBuffer[position++] = '\n';
    pBuffer[position++] = '\n';
    s32 optionLength = rObj.PrintOptionTo(pBuffer + position, cPrintBufferSize - position);
    position += optionLength;
    pBuffer[position++] = '\n';
    pBuffer[position++] = '\n';
    s32 rawOptionLength = rObj.PrintRawOptionTo(pBuffer + position, cPrintBufferSize - position);
    position += rawOptionLength;
    pBuffer[position++] = '\n';
    pBuffer[position++] = '\n';
    pBuffer[position] = '\0';
    return rawKeyLength + keyLength + optionLength + rawOptionLength + 8;
}

}  // namespace

/**
 * Binds a uniform buffer to every shader stage of a location.
 * @param pDrawContext draw context
 * @param rLocation uniform block location
 * @param rBuffer uniform buffer
 * @param size size of the buffer
 * @param unused unused
 */
void ShaderUtilG3D::load(DrawContext* pDrawContext, const ShaderLocation& rLocation,
                         const nn::gfx::Buffer& rBuffer, u64 size, s32 unused)
{
    if (size == 0)
    {
        return;
    }

    NVNbufferAddress address =
        nvnBufferGetAddress(static_cast<NVNbuffer*>(rBuffer.ToData()->pNvnBuffer.ptr));
    NVNcommandBuffer* pCommandBuffer = pDrawContext->getNvnCommandBuffer();

    for (s32 i = 0; i < cShaderType_Num; i++)
    {
        if (rLocation.getLocation(ShaderType(i)) != -1)
        {
            nvnCommandBufferBindUniformBuffer(
                pCommandBuffer, driver::NVNMgr::getNVNshaderStage(ShaderType(i)),
                rLocation.getLocation(ShaderType(i)), address, static_cast<u32>(size));
        }
    }
}

/**
 * Binds a texture and sampler to a location.
 * @param pDrawContext draw context
 * @param rLocation sampler location
 * @param rSamplerId sampler ID
 * @param rTextureId texture ID
 */
void ShaderUtilG3D::load(DrawContext* pDrawContext, const ShaderLocation& rLocation,
                         const u32& rSamplerId, const u32& rTextureId)
{
    NVNtextureHandle handle = nvnDeviceGetTextureHandle(
        driver::NVNMgr::instance()->getNvnDevice(), rTextureId, rSamplerId);
    driver::NVNMgr::instance()->nvnCommandBufferBindTexture(pDrawContext, handle, rLocation,
                                                            rTextureId);
}

/**
 * Searches a uniform block of a shading model program.
 * @param pLocation location output
 * @param pModel shading model
 * @param pProgram shader program
 * @param rName name of the uniform block
 */
void ShaderUtilG3D::search(UniformBlockLocation* pLocation,
                           const nn::g3d::ResShadingModel* pModel,
                           const nn::g3d::ResShaderProgram* pProgram,
                           const sead::SafeString& rName)
{
    ShaderLocation location;
    searchUniformBlock(&location, pModel, pProgram, rName);
    *pLocation = UniformBlockLocation(location, rName);
}

/**
 * Searches the per-stage locations of a uniform block of a shading model program.
 * @param pLocation location output
 * @param pModel shading model
 * @param pProgram shader program
 * @param rName name of the uniform block
 */
void ShaderUtilG3D::searchUniformBlock(ShaderLocation* pLocation,
                                       const nn::g3d::ResShadingModel* pModel,
                                       const nn::g3d::ResShaderProgram* pProgram,
                                       const sead::SafeString& rName)
{
    s32 index = pModel->FindUniformBlockIndex(rName.cstr());

    if (index == -1)
    {
        *pLocation = ShaderLocation();
        return;
    }

    s32 vertex = pProgram->GetUniformBlockLocation(index, nn::g3d::Stage_Vertex);
    s32 geometry = pProgram->GetUniformBlockLocation(index, nn::g3d::Stage_Geometry);
    s32 fragment = pProgram->GetUniformBlockLocation(index, nn::g3d::Stage_Pixel);
    pLocation->setLocation(cShaderType_Vertex, vertex);
    pLocation->setLocation(cShaderType_Geometry, geometry);
    pLocation->setLocation(cShaderType_Fragment, fragment);
}

/**
 * Searches a sampler of a shading model program.
 * @param pLocation location output
 * @param pModel shading model
 * @param pProgram shader program
 * @param rName name of the sampler
 */
void ShaderUtilG3D::search(SamplerLocation* pLocation, const nn::g3d::ResShadingModel* pModel,
                           const nn::g3d::ResShaderProgram* pProgram,
                           const sead::SafeString& rName)
{
    ShaderLocation location;
    searchSampler(&location, pModel, pProgram, rName);
    *pLocation = SamplerLocation(location, rName);
}

/**
 * Searches the per-stage locations of a sampler of a shading model program.
 * @param pLocation location output
 * @param pModel shading model
 * @param pProgram shader program
 * @param rName name of the sampler
 */
void ShaderUtilG3D::searchSampler(ShaderLocation* pLocation,
                                  const nn::g3d::ResShadingModel* pModel,
                                  const nn::g3d::ResShaderProgram* pProgram,
                                  const sead::SafeString& rName)
{
    s32 index = pModel->FindSamplerIndex(rName.cstr());

    if (index == -1)
    {
        *pLocation = ShaderLocation();
        return;
    }

    s32 vertex = pProgram->GetSamplerLocation(index, nn::g3d::Stage_Vertex);
    s32 geometry = pProgram->GetSamplerLocation(index, nn::g3d::Stage_Geometry);
    s32 fragment = pProgram->GetSamplerLocation(index, nn::g3d::Stage_Pixel);
    pLocation->setLocation(cShaderType_Vertex, vertex);
    pLocation->setLocation(cShaderType_Geometry, geometry);
    pLocation->setLocation(cShaderType_Fragment, fragment);
}

/**
 * Prints the keys and options of a shading model and a shader selector (debug heap only).
 * @param rModel shading model
 * @param rSelector shader selector
 */
void ShaderUtilG3D::print(const nn::g3d::ShadingModelObj& rModel,
                          const nn::g3d::ShaderSelector& rSelector)
{
    sead::Heap* pHeap = detail::PrivateResource::instance()->getDebugHeap();

    if (pHeap == nullptr)
    {
        return;
    }

    char* pBuffer = static_cast<char*>(pHeap->tryAlloc(cPrintBufferSize, 8));
    replaceTabs(pBuffer, printAll(rModel, pBuffer));
    replaceTabs(pBuffer, printAll(rSelector, pBuffer));
    pHeap->free(pBuffer);
}

}  // namespace agl::g3d
