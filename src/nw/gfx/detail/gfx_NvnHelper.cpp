#include <nn/gfx/detail/gfx_NvnHelper.h>

#include <nn/gfx/gfx_BufferInfo.h>
#include <nn/gfx/gfx_MemoryPoolInfo.h>
#include <nn/gfx/gfx_QueueInfo.h>
#include <nn/gfx/gfx_SamplerInfo.h>
#include <nn/gfx/gfx_SwapChainInfo.h>
#include <nn/gfx/gfx_TextureInfo.h>
#include <nn/util.h>
#include <nvn/nvn_FuncPtrBase.h>
#include <nvn/nvn_FuncPtrInline.h>

#include <algorithm>
#include <iterator>

// Entry points loaded by nvnLoadCProcs that nvn_FuncPtrBase.h does not declare (no prototype
// typedefs are available for them yet).
extern "C" {

extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnDeviceBuilderGetFlags;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnQueueBuilderSetQueuePriority;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnQueueBuilderGetQueuePriority;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnQueueBuilderGetDevice;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnQueueBuilderGetFlags;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnQueueBuilderGetCommandMemorySize;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnQueueBuilderGetComputeMemorySize;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnQueueBuilderGetControlMemorySize;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnQueueBuilderGetCommandFlushThreshold;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnQueueBuilderGetMemorySize;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnQueueBuilderGetMemory;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnWindowBuilderSetNumActiveTextures;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnWindowBuilderGetDevice;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnWindowBuilderGetNumTextures;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnWindowBuilderGetTexture;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnWindowBuilderGetNumActiveTextures;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnWindowSetNumActiveTextures;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnWindowGetNumActiveTextures;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnWindowGetNumTextures;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnProgramSetShadersExt;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnProgramSetSampleShading;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnMemoryPoolBuilderGetDevice;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnBufferBuilderGetDevice;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnTextureBuilderGetDevice;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnTextureBuilderGetPackagedTextureLayout;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnTextureBuilderGetRawStorageClass;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnTextureGetRawStorageClass;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnSamplerBuilderGetDevice;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnCommandBufferSetCommandMemoryCallbackEnabled;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnCommandBufferSetColorReductionEnable;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnCommandBufferSetColorReductionThresholds;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnSyncInitializeFromFencedGLSync;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnSyncCreateGLSync;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnEventBuilderGetStorage;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnEventBuilderGetMemoryPool;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnEventBuilderGetMemoryOffset;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnEventGetMemoryPool;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnEventGetMemoryOffset;
extern PFNNVNGENERICFUNCPTRPROC pfnc_nvnCommandBufferSetStencilCullCriteria;

}  // extern "C"

/**
 * Stores the address of the named NVN entry point into its pfnc_ function pointer.
 */
#define NVN_LOAD_PROC(name)                                                                        \
    pfnc_##name = reinterpret_cast<decltype(pfnc_##name)>(deviceGetProcAddress(device, #name))

/**
 * Loads every NVN C entry point through the device's procedure address query.
 *
 * @param device Device the entry points are queried from.
 * @param deviceGetProcAddress Bootstrap nvnDeviceGetProcAddress function.
 */
void nvnLoadCProcs(const NVNdevice* device, PFNNVNDEVICEGETPROCADDRESSPROC deviceGetProcAddress) {
    NVN_LOAD_PROC(nvnDeviceBuilderSetDefaults);
    NVN_LOAD_PROC(nvnDeviceBuilderSetFlags);
    NVN_LOAD_PROC(nvnDeviceBuilderGetFlags);
    NVN_LOAD_PROC(nvnDeviceInitialize);
    NVN_LOAD_PROC(nvnDeviceFinalize);
    NVN_LOAD_PROC(nvnDeviceSetDebugLabel);
    NVN_LOAD_PROC(nvnDeviceGetProcAddress);
    NVN_LOAD_PROC(nvnDeviceGetInteger);
    NVN_LOAD_PROC(nvnDeviceGetCurrentTimestampInNanoseconds);
    NVN_LOAD_PROC(nvnDeviceSetIntermediateShaderCache);
    NVN_LOAD_PROC(nvnDeviceGetTextureHandle);
    NVN_LOAD_PROC(nvnDeviceGetTexelFetchHandle);
    NVN_LOAD_PROC(nvnDeviceGetImageHandle);
    NVN_LOAD_PROC(nvnDeviceInstallDebugCallback);
    NVN_LOAD_PROC(nvnDeviceGenerateDebugDomainId);
    NVN_LOAD_PROC(nvnDeviceSetWindowOriginMode);
    NVN_LOAD_PROC(nvnDeviceSetDepthMode);
    NVN_LOAD_PROC(nvnDeviceRegisterFastClearColor);
    NVN_LOAD_PROC(nvnDeviceRegisterFastClearColori);
    NVN_LOAD_PROC(nvnDeviceRegisterFastClearColorui);
    NVN_LOAD_PROC(nvnDeviceRegisterFastClearDepth);
    NVN_LOAD_PROC(nvnDeviceGetWindowOriginMode);
    NVN_LOAD_PROC(nvnDeviceGetDepthMode);
    NVN_LOAD_PROC(nvnDeviceGetTimestampInNanoseconds);
    NVN_LOAD_PROC(nvnDeviceApplyDeferredFinalizes);
    NVN_LOAD_PROC(nvnDeviceFinalizeCommandHandle);
    NVN_LOAD_PROC(nvnDeviceWalkDebugDatabase);
    NVN_LOAD_PROC(nvnDeviceGetSeparateTextureHandle);
    NVN_LOAD_PROC(nvnDeviceGetSeparateSamplerHandle);
    NVN_LOAD_PROC(nvnDeviceIsExternalDebuggerAttached);
    NVN_LOAD_PROC(nvnQueueGetError);
    NVN_LOAD_PROC(nvnQueueGetTotalCommandMemoryUsed);
    NVN_LOAD_PROC(nvnQueueGetTotalControlMemoryUsed);
    NVN_LOAD_PROC(nvnQueueGetTotalComputeMemoryUsed);
    NVN_LOAD_PROC(nvnQueueResetMemoryUsageCounts);
    NVN_LOAD_PROC(nvnQueueBuilderSetDevice);
    NVN_LOAD_PROC(nvnQueueBuilderSetDefaults);
    NVN_LOAD_PROC(nvnQueueBuilderSetFlags);
    NVN_LOAD_PROC(nvnQueueBuilderSetCommandMemorySize);
    NVN_LOAD_PROC(nvnQueueBuilderSetComputeMemorySize);
    NVN_LOAD_PROC(nvnQueueBuilderSetControlMemorySize);
    NVN_LOAD_PROC(nvnQueueBuilderGetQueueMemorySize);
    NVN_LOAD_PROC(nvnQueueBuilderSetQueueMemory);
    NVN_LOAD_PROC(nvnQueueBuilderSetCommandFlushThreshold);
    NVN_LOAD_PROC(nvnQueueBuilderSetQueuePriority);
    NVN_LOAD_PROC(nvnQueueBuilderGetQueuePriority);
    NVN_LOAD_PROC(nvnQueueBuilderGetDevice);
    NVN_LOAD_PROC(nvnQueueBuilderGetFlags);
    NVN_LOAD_PROC(nvnQueueBuilderGetCommandMemorySize);
    NVN_LOAD_PROC(nvnQueueBuilderGetComputeMemorySize);
    NVN_LOAD_PROC(nvnQueueBuilderGetControlMemorySize);
    NVN_LOAD_PROC(nvnQueueBuilderGetCommandFlushThreshold);
    NVN_LOAD_PROC(nvnQueueBuilderGetMemorySize);
    NVN_LOAD_PROC(nvnQueueBuilderGetMemory);
    NVN_LOAD_PROC(nvnQueueInitialize);
    NVN_LOAD_PROC(nvnQueueFinalize);
    NVN_LOAD_PROC(nvnQueueSetDebugLabel);
    NVN_LOAD_PROC(nvnQueueSubmitCommands);
    NVN_LOAD_PROC(nvnQueueFlush);
    NVN_LOAD_PROC(nvnQueueFinish);
    NVN_LOAD_PROC(nvnQueuePresentTexture);
    NVN_LOAD_PROC(nvnQueueAcquireTexture);
    NVN_LOAD_PROC(nvnWindowBuilderSetDevice);
    NVN_LOAD_PROC(nvnWindowBuilderSetDefaults);
    NVN_LOAD_PROC(nvnWindowBuilderSetNativeWindow);
    NVN_LOAD_PROC(nvnWindowBuilderSetTextures);
    NVN_LOAD_PROC(nvnWindowBuilderSetPresentInterval);
    NVN_LOAD_PROC(nvnWindowBuilderSetNumActiveTextures);
    NVN_LOAD_PROC(nvnWindowBuilderGetDevice);
    NVN_LOAD_PROC(nvnWindowBuilderGetNumTextures);
    NVN_LOAD_PROC(nvnWindowBuilderGetTexture);
    NVN_LOAD_PROC(nvnWindowBuilderGetNativeWindow);
    NVN_LOAD_PROC(nvnWindowBuilderGetPresentInterval);
    NVN_LOAD_PROC(nvnWindowBuilderGetNumActiveTextures);
    NVN_LOAD_PROC(nvnWindowInitialize);
    NVN_LOAD_PROC(nvnWindowFinalize);
    NVN_LOAD_PROC(nvnWindowSetDebugLabel);
    NVN_LOAD_PROC(nvnWindowAcquireTexture);
    NVN_LOAD_PROC(nvnWindowGetNativeWindow);
    NVN_LOAD_PROC(nvnWindowGetPresentInterval);
    NVN_LOAD_PROC(nvnWindowSetPresentInterval);
    NVN_LOAD_PROC(nvnWindowSetCrop);
    NVN_LOAD_PROC(nvnWindowGetCrop);
    NVN_LOAD_PROC(nvnWindowSetNumActiveTextures);
    NVN_LOAD_PROC(nvnWindowGetNumActiveTextures);
    NVN_LOAD_PROC(nvnWindowGetNumTextures);
    NVN_LOAD_PROC(nvnProgramInitialize);
    NVN_LOAD_PROC(nvnProgramFinalize);
    NVN_LOAD_PROC(nvnProgramSetDebugLabel);
    NVN_LOAD_PROC(nvnProgramSetShaders);
    NVN_LOAD_PROC(nvnProgramSetShadersExt);
    NVN_LOAD_PROC(nvnProgramSetSampleShading);
    NVN_LOAD_PROC(nvnMemoryPoolBuilderSetDevice);
    NVN_LOAD_PROC(nvnMemoryPoolBuilderSetDefaults);
    NVN_LOAD_PROC(nvnMemoryPoolBuilderSetStorage);
    NVN_LOAD_PROC(nvnMemoryPoolBuilderSetFlags);
    NVN_LOAD_PROC(nvnMemoryPoolBuilderGetDevice);
    NVN_LOAD_PROC(nvnMemoryPoolBuilderGetMemory);
    NVN_LOAD_PROC(nvnMemoryPoolBuilderGetSize);
    NVN_LOAD_PROC(nvnMemoryPoolBuilderGetFlags);
    NVN_LOAD_PROC(nvnMemoryPoolInitialize);
    NVN_LOAD_PROC(nvnMemoryPoolSetDebugLabel);
    NVN_LOAD_PROC(nvnMemoryPoolFinalize);
    NVN_LOAD_PROC(nvnMemoryPoolMap);
    NVN_LOAD_PROC(nvnMemoryPoolFlushMappedRange);
    NVN_LOAD_PROC(nvnMemoryPoolInvalidateMappedRange);
    NVN_LOAD_PROC(nvnMemoryPoolGetBufferAddress);
    NVN_LOAD_PROC(nvnMemoryPoolMapVirtual);
    NVN_LOAD_PROC(nvnMemoryPoolGetSize);
    NVN_LOAD_PROC(nvnMemoryPoolGetFlags);
    NVN_LOAD_PROC(nvnTexturePoolInitialize);
    NVN_LOAD_PROC(nvnTexturePoolSetDebugLabel);
    NVN_LOAD_PROC(nvnTexturePoolFinalize);
    NVN_LOAD_PROC(nvnTexturePoolRegisterTexture);
    NVN_LOAD_PROC(nvnTexturePoolRegisterImage);
    NVN_LOAD_PROC(nvnTexturePoolGetMemoryPool);
    NVN_LOAD_PROC(nvnTexturePoolGetMemoryOffset);
    NVN_LOAD_PROC(nvnTexturePoolGetSize);
    NVN_LOAD_PROC(nvnSamplerPoolInitialize);
    NVN_LOAD_PROC(nvnSamplerPoolSetDebugLabel);
    NVN_LOAD_PROC(nvnSamplerPoolFinalize);
    NVN_LOAD_PROC(nvnSamplerPoolRegisterSampler);
    NVN_LOAD_PROC(nvnSamplerPoolRegisterSamplerBuilder);
    NVN_LOAD_PROC(nvnSamplerPoolGetMemoryPool);
    NVN_LOAD_PROC(nvnSamplerPoolGetMemoryOffset);
    NVN_LOAD_PROC(nvnSamplerPoolGetSize);
    NVN_LOAD_PROC(nvnBufferBuilderSetDevice);
    NVN_LOAD_PROC(nvnBufferBuilderSetDefaults);
    NVN_LOAD_PROC(nvnBufferBuilderSetStorage);
    NVN_LOAD_PROC(nvnBufferBuilderGetDevice);
    NVN_LOAD_PROC(nvnBufferBuilderGetMemoryPool);
    NVN_LOAD_PROC(nvnBufferBuilderGetMemoryOffset);
    NVN_LOAD_PROC(nvnBufferBuilderGetSize);
    NVN_LOAD_PROC(nvnBufferInitialize);
    NVN_LOAD_PROC(nvnBufferSetDebugLabel);
    NVN_LOAD_PROC(nvnBufferFinalize);
    NVN_LOAD_PROC(nvnBufferMap);
    NVN_LOAD_PROC(nvnBufferGetAddress);
    NVN_LOAD_PROC(nvnBufferFlushMappedRange);
    NVN_LOAD_PROC(nvnBufferInvalidateMappedRange);
    NVN_LOAD_PROC(nvnBufferGetMemoryPool);
    NVN_LOAD_PROC(nvnBufferGetMemoryOffset);
    NVN_LOAD_PROC(nvnBufferGetSize);
    NVN_LOAD_PROC(nvnBufferGetDebugID);
    NVN_LOAD_PROC(nvnTextureBuilderSetDevice);
    NVN_LOAD_PROC(nvnTextureBuilderSetDefaults);
    NVN_LOAD_PROC(nvnTextureBuilderSetFlags);
    NVN_LOAD_PROC(nvnTextureBuilderSetTarget);
    NVN_LOAD_PROC(nvnTextureBuilderSetWidth);
    NVN_LOAD_PROC(nvnTextureBuilderSetHeight);
    NVN_LOAD_PROC(nvnTextureBuilderSetDepth);
    NVN_LOAD_PROC(nvnTextureBuilderSetSize1D);
    NVN_LOAD_PROC(nvnTextureBuilderSetSize2D);
    NVN_LOAD_PROC(nvnTextureBuilderSetSize3D);
    NVN_LOAD_PROC(nvnTextureBuilderSetLevels);
    NVN_LOAD_PROC(nvnTextureBuilderSetFormat);
    NVN_LOAD_PROC(nvnTextureBuilderSetSamples);
    NVN_LOAD_PROC(nvnTextureBuilderSetSwizzle);
    NVN_LOAD_PROC(nvnTextureBuilderSetDepthStencilMode);
    NVN_LOAD_PROC(nvnTextureBuilderGetStorageSize);
    NVN_LOAD_PROC(nvnTextureBuilderGetStorageAlignment);
    NVN_LOAD_PROC(nvnTextureBuilderSetStorage);
    NVN_LOAD_PROC(nvnTextureBuilderSetPackagedTextureData);
    NVN_LOAD_PROC(nvnTextureBuilderSetPackagedTextureLayout);
    NVN_LOAD_PROC(nvnTextureBuilderSetStride);
    NVN_LOAD_PROC(nvnTextureBuilderSetGLTextureName);
    NVN_LOAD_PROC(nvnTextureBuilderGetStorageClass);
    NVN_LOAD_PROC(nvnTextureBuilderGetDevice);
    NVN_LOAD_PROC(nvnTextureBuilderGetFlags);
    NVN_LOAD_PROC(nvnTextureBuilderGetTarget);
    NVN_LOAD_PROC(nvnTextureBuilderGetWidth);
    NVN_LOAD_PROC(nvnTextureBuilderGetHeight);
    NVN_LOAD_PROC(nvnTextureBuilderGetDepth);
    NVN_LOAD_PROC(nvnTextureBuilderGetLevels);
    NVN_LOAD_PROC(nvnTextureBuilderGetFormat);
    NVN_LOAD_PROC(nvnTextureBuilderGetSamples);
    NVN_LOAD_PROC(nvnTextureBuilderGetSwizzle);
    NVN_LOAD_PROC(nvnTextureBuilderGetDepthStencilMode);
    NVN_LOAD_PROC(nvnTextureBuilderGetPackagedTextureData);
    NVN_LOAD_PROC(nvnTextureBuilderGetPackagedTextureLayout);
    NVN_LOAD_PROC(nvnTextureBuilderGetStride);
    NVN_LOAD_PROC(nvnTextureBuilderGetSparseTileLayout);
    NVN_LOAD_PROC(nvnTextureBuilderGetGLTextureName);
    NVN_LOAD_PROC(nvnTextureBuilderGetZCullStorageSize);
    NVN_LOAD_PROC(nvnTextureBuilderGetMemoryPool);
    NVN_LOAD_PROC(nvnTextureBuilderGetMemoryOffset);
    NVN_LOAD_PROC(nvnTextureBuilderGetRawStorageClass);
    NVN_LOAD_PROC(nvnTextureViewSetDefaults);
    NVN_LOAD_PROC(nvnTextureViewSetLevels);
    NVN_LOAD_PROC(nvnTextureViewSetLayers);
    NVN_LOAD_PROC(nvnTextureViewSetFormat);
    NVN_LOAD_PROC(nvnTextureViewSetSwizzle);
    NVN_LOAD_PROC(nvnTextureViewSetDepthStencilMode);
    NVN_LOAD_PROC(nvnTextureViewSetTarget);
    NVN_LOAD_PROC(nvnTextureViewGetLevels);
    NVN_LOAD_PROC(nvnTextureViewGetLayers);
    NVN_LOAD_PROC(nvnTextureViewGetFormat);
    NVN_LOAD_PROC(nvnTextureViewGetSwizzle);
    NVN_LOAD_PROC(nvnTextureViewGetDepthStencilMode);
    NVN_LOAD_PROC(nvnTextureViewGetTarget);
    NVN_LOAD_PROC(nvnTextureViewCompare);
    NVN_LOAD_PROC(nvnTextureInitialize);
    NVN_LOAD_PROC(nvnTextureGetZCullStorageSize);
    NVN_LOAD_PROC(nvnTextureFinalize);
    NVN_LOAD_PROC(nvnTextureSetDebugLabel);
    NVN_LOAD_PROC(nvnTextureGetStorageClass);
    NVN_LOAD_PROC(nvnTextureGetViewOffset);
    NVN_LOAD_PROC(nvnTextureGetFlags);
    NVN_LOAD_PROC(nvnTextureGetTarget);
    NVN_LOAD_PROC(nvnTextureGetWidth);
    NVN_LOAD_PROC(nvnTextureGetHeight);
    NVN_LOAD_PROC(nvnTextureGetDepth);
    NVN_LOAD_PROC(nvnTextureGetLevels);
    NVN_LOAD_PROC(nvnTextureGetFormat);
    NVN_LOAD_PROC(nvnTextureGetSamples);
    NVN_LOAD_PROC(nvnTextureGetSwizzle);
    NVN_LOAD_PROC(nvnTextureGetDepthStencilMode);
    NVN_LOAD_PROC(nvnTextureGetStride);
    NVN_LOAD_PROC(nvnTextureGetTextureAddress);
    NVN_LOAD_PROC(nvnTextureGetSparseTileLayout);
    NVN_LOAD_PROC(nvnTextureWriteTexels);
    NVN_LOAD_PROC(nvnTextureWriteTexelsStrided);
    NVN_LOAD_PROC(nvnTextureReadTexels);
    NVN_LOAD_PROC(nvnTextureReadTexelsStrided);
    NVN_LOAD_PROC(nvnTextureFlushTexels);
    NVN_LOAD_PROC(nvnTextureInvalidateTexels);
    NVN_LOAD_PROC(nvnTextureGetMemoryPool);
    NVN_LOAD_PROC(nvnTextureGetMemoryOffset);
    NVN_LOAD_PROC(nvnTextureGetStorageSize);
    NVN_LOAD_PROC(nvnTextureCompare);
    NVN_LOAD_PROC(nvnTextureGetDebugID);
    NVN_LOAD_PROC(nvnTextureGetRawStorageClass);
    NVN_LOAD_PROC(nvnSamplerBuilderSetDevice);
    NVN_LOAD_PROC(nvnSamplerBuilderSetDefaults);
    NVN_LOAD_PROC(nvnSamplerBuilderSetMinMagFilter);
    NVN_LOAD_PROC(nvnSamplerBuilderSetWrapMode);
    NVN_LOAD_PROC(nvnSamplerBuilderSetLodClamp);
    NVN_LOAD_PROC(nvnSamplerBuilderSetLodBias);
    NVN_LOAD_PROC(nvnSamplerBuilderSetCompare);
    NVN_LOAD_PROC(nvnSamplerBuilderSetBorderColor);
    NVN_LOAD_PROC(nvnSamplerBuilderSetBorderColori);
    NVN_LOAD_PROC(nvnSamplerBuilderSetBorderColorui);
    NVN_LOAD_PROC(nvnSamplerBuilderSetMaxAnisotropy);
    NVN_LOAD_PROC(nvnSamplerBuilderSetReductionFilter);
    NVN_LOAD_PROC(nvnSamplerBuilderSetLodSnap);
    NVN_LOAD_PROC(nvnSamplerBuilderGetDevice);
    NVN_LOAD_PROC(nvnSamplerBuilderGetMinMagFilter);
    NVN_LOAD_PROC(nvnSamplerBuilderGetWrapMode);
    NVN_LOAD_PROC(nvnSamplerBuilderGetLodClamp);
    NVN_LOAD_PROC(nvnSamplerBuilderGetLodBias);
    NVN_LOAD_PROC(nvnSamplerBuilderGetCompare);
    NVN_LOAD_PROC(nvnSamplerBuilderGetBorderColor);
    NVN_LOAD_PROC(nvnSamplerBuilderGetBorderColori);
    NVN_LOAD_PROC(nvnSamplerBuilderGetBorderColorui);
    NVN_LOAD_PROC(nvnSamplerBuilderGetMaxAnisotropy);
    NVN_LOAD_PROC(nvnSamplerBuilderGetReductionFilter);
    NVN_LOAD_PROC(nvnSamplerBuilderGetLodSnap);
    NVN_LOAD_PROC(nvnSamplerInitialize);
    NVN_LOAD_PROC(nvnSamplerFinalize);
    NVN_LOAD_PROC(nvnSamplerSetDebugLabel);
    NVN_LOAD_PROC(nvnSamplerGetMinMagFilter);
    NVN_LOAD_PROC(nvnSamplerGetWrapMode);
    NVN_LOAD_PROC(nvnSamplerGetLodClamp);
    NVN_LOAD_PROC(nvnSamplerGetLodBias);
    NVN_LOAD_PROC(nvnSamplerGetCompare);
    NVN_LOAD_PROC(nvnSamplerGetBorderColor);
    NVN_LOAD_PROC(nvnSamplerGetBorderColori);
    NVN_LOAD_PROC(nvnSamplerGetBorderColorui);
    NVN_LOAD_PROC(nvnSamplerGetMaxAnisotropy);
    NVN_LOAD_PROC(nvnSamplerGetReductionFilter);
    NVN_LOAD_PROC(nvnSamplerCompare);
    NVN_LOAD_PROC(nvnSamplerGetDebugID);
    NVN_LOAD_PROC(nvnBlendStateSetDefaults);
    NVN_LOAD_PROC(nvnBlendStateSetBlendTarget);
    NVN_LOAD_PROC(nvnBlendStateSetBlendFunc);
    NVN_LOAD_PROC(nvnBlendStateSetBlendEquation);
    NVN_LOAD_PROC(nvnBlendStateSetAdvancedMode);
    NVN_LOAD_PROC(nvnBlendStateSetAdvancedOverlap);
    NVN_LOAD_PROC(nvnBlendStateSetAdvancedPremultipliedSrc);
    NVN_LOAD_PROC(nvnBlendStateSetAdvancedNormalizedDst);
    NVN_LOAD_PROC(nvnBlendStateGetBlendTarget);
    NVN_LOAD_PROC(nvnBlendStateGetBlendFunc);
    NVN_LOAD_PROC(nvnBlendStateGetBlendEquation);
    NVN_LOAD_PROC(nvnBlendStateGetAdvancedMode);
    NVN_LOAD_PROC(nvnBlendStateGetAdvancedOverlap);
    NVN_LOAD_PROC(nvnBlendStateGetAdvancedPremultipliedSrc);
    NVN_LOAD_PROC(nvnBlendStateGetAdvancedNormalizedDst);
    NVN_LOAD_PROC(nvnColorStateSetDefaults);
    NVN_LOAD_PROC(nvnColorStateSetBlendEnable);
    NVN_LOAD_PROC(nvnColorStateSetLogicOp);
    NVN_LOAD_PROC(nvnColorStateSetAlphaTest);
    NVN_LOAD_PROC(nvnColorStateGetBlendEnable);
    NVN_LOAD_PROC(nvnColorStateGetLogicOp);
    NVN_LOAD_PROC(nvnColorStateGetAlphaTest);
    NVN_LOAD_PROC(nvnChannelMaskStateSetDefaults);
    NVN_LOAD_PROC(nvnChannelMaskStateSetChannelMask);
    NVN_LOAD_PROC(nvnChannelMaskStateGetChannelMask);
    NVN_LOAD_PROC(nvnMultisampleStateSetDefaults);
    NVN_LOAD_PROC(nvnMultisampleStateSetMultisampleEnable);
    NVN_LOAD_PROC(nvnMultisampleStateSetSamples);
    NVN_LOAD_PROC(nvnMultisampleStateSetAlphaToCoverageEnable);
    NVN_LOAD_PROC(nvnMultisampleStateSetAlphaToCoverageDither);
    NVN_LOAD_PROC(nvnMultisampleStateGetMultisampleEnable);
    NVN_LOAD_PROC(nvnMultisampleStateGetSamples);
    NVN_LOAD_PROC(nvnMultisampleStateGetAlphaToCoverageEnable);
    NVN_LOAD_PROC(nvnMultisampleStateGetAlphaToCoverageDither);
    NVN_LOAD_PROC(nvnMultisampleStateSetRasterSamples);
    NVN_LOAD_PROC(nvnMultisampleStateGetRasterSamples);
    NVN_LOAD_PROC(nvnMultisampleStateSetCoverageModulationMode);
    NVN_LOAD_PROC(nvnMultisampleStateGetCoverageModulationMode);
    NVN_LOAD_PROC(nvnMultisampleStateSetCoverageToColorEnable);
    NVN_LOAD_PROC(nvnMultisampleStateGetCoverageToColorEnable);
    NVN_LOAD_PROC(nvnMultisampleStateSetCoverageToColorOutput);
    NVN_LOAD_PROC(nvnMultisampleStateGetCoverageToColorOutput);
    NVN_LOAD_PROC(nvnMultisampleStateSetSampleLocationsEnable);
    NVN_LOAD_PROC(nvnMultisampleStateGetSampleLocationsEnable);
    NVN_LOAD_PROC(nvnMultisampleStateGetSampleLocationsGrid);
    NVN_LOAD_PROC(nvnMultisampleStateSetSampleLocationsGridEnable);
    NVN_LOAD_PROC(nvnMultisampleStateGetSampleLocationsGridEnable);
    NVN_LOAD_PROC(nvnMultisampleStateSetSampleLocations);
    NVN_LOAD_PROC(nvnPolygonStateSetDefaults);
    NVN_LOAD_PROC(nvnPolygonStateSetCullFace);
    NVN_LOAD_PROC(nvnPolygonStateSetFrontFace);
    NVN_LOAD_PROC(nvnPolygonStateSetPolygonMode);
    NVN_LOAD_PROC(nvnPolygonStateSetPolygonOffsetEnables);
    NVN_LOAD_PROC(nvnPolygonStateGetCullFace);
    NVN_LOAD_PROC(nvnPolygonStateGetFrontFace);
    NVN_LOAD_PROC(nvnPolygonStateGetPolygonMode);
    NVN_LOAD_PROC(nvnPolygonStateGetPolygonOffsetEnables);
    NVN_LOAD_PROC(nvnDepthStencilStateSetDefaults);
    NVN_LOAD_PROC(nvnDepthStencilStateSetDepthTestEnable);
    NVN_LOAD_PROC(nvnDepthStencilStateSetDepthWriteEnable);
    NVN_LOAD_PROC(nvnDepthStencilStateSetDepthFunc);
    NVN_LOAD_PROC(nvnDepthStencilStateSetStencilTestEnable);
    NVN_LOAD_PROC(nvnDepthStencilStateSetStencilFunc);
    NVN_LOAD_PROC(nvnDepthStencilStateSetStencilOp);
    NVN_LOAD_PROC(nvnDepthStencilStateGetDepthTestEnable);
    NVN_LOAD_PROC(nvnDepthStencilStateGetDepthWriteEnable);
    NVN_LOAD_PROC(nvnDepthStencilStateGetDepthFunc);
    NVN_LOAD_PROC(nvnDepthStencilStateGetStencilTestEnable);
    NVN_LOAD_PROC(nvnDepthStencilStateGetStencilFunc);
    NVN_LOAD_PROC(nvnDepthStencilStateGetStencilOp);
    NVN_LOAD_PROC(nvnVertexAttribStateSetDefaults);
    NVN_LOAD_PROC(nvnVertexAttribStateSetFormat);
    NVN_LOAD_PROC(nvnVertexAttribStateSetStreamIndex);
    NVN_LOAD_PROC(nvnVertexAttribStateGetFormat);
    NVN_LOAD_PROC(nvnVertexAttribStateGetStreamIndex);
    NVN_LOAD_PROC(nvnVertexStreamStateSetDefaults);
    NVN_LOAD_PROC(nvnVertexStreamStateSetStride);
    NVN_LOAD_PROC(nvnVertexStreamStateSetDivisor);
    NVN_LOAD_PROC(nvnVertexStreamStateGetStride);
    NVN_LOAD_PROC(nvnVertexStreamStateGetDivisor);
    NVN_LOAD_PROC(nvnCommandBufferInitialize);
    NVN_LOAD_PROC(nvnCommandBufferFinalize);
    NVN_LOAD_PROC(nvnCommandBufferSetDebugLabel);
    NVN_LOAD_PROC(nvnCommandBufferSetMemoryCallback);
    NVN_LOAD_PROC(nvnCommandBufferSetMemoryCallbackData);
    NVN_LOAD_PROC(nvnCommandBufferSetCommandMemoryCallbackEnabled);
    NVN_LOAD_PROC(nvnCommandBufferAddCommandMemory);
    NVN_LOAD_PROC(nvnCommandBufferAddControlMemory);
    NVN_LOAD_PROC(nvnCommandBufferGetCommandMemorySize);
    NVN_LOAD_PROC(nvnCommandBufferGetCommandMemoryUsed);
    NVN_LOAD_PROC(nvnCommandBufferGetCommandMemoryFree);
    NVN_LOAD_PROC(nvnCommandBufferGetControlMemorySize);
    NVN_LOAD_PROC(nvnCommandBufferGetControlMemoryUsed);
    NVN_LOAD_PROC(nvnCommandBufferGetControlMemoryFree);
    NVN_LOAD_PROC(nvnCommandBufferBeginRecording);
    NVN_LOAD_PROC(nvnCommandBufferEndRecording);
    NVN_LOAD_PROC(nvnCommandBufferCallCommands);
    NVN_LOAD_PROC(nvnCommandBufferCopyCommands);
    NVN_LOAD_PROC(nvnCommandBufferBindBlendState);
    NVN_LOAD_PROC(nvnCommandBufferBindChannelMaskState);
    NVN_LOAD_PROC(nvnCommandBufferBindColorState);
    NVN_LOAD_PROC(nvnCommandBufferBindMultisampleState);
    NVN_LOAD_PROC(nvnCommandBufferBindPolygonState);
    NVN_LOAD_PROC(nvnCommandBufferBindDepthStencilState);
    NVN_LOAD_PROC(nvnCommandBufferBindVertexAttribState);
    NVN_LOAD_PROC(nvnCommandBufferBindVertexStreamState);
    NVN_LOAD_PROC(nvnCommandBufferBindProgram);
    NVN_LOAD_PROC(nvnCommandBufferBindVertexBuffer);
    NVN_LOAD_PROC(nvnCommandBufferBindVertexBuffers);
    NVN_LOAD_PROC(nvnCommandBufferBindUniformBuffer);
    NVN_LOAD_PROC(nvnCommandBufferBindUniformBuffers);
    NVN_LOAD_PROC(nvnCommandBufferBindTransformFeedbackBuffer);
    NVN_LOAD_PROC(nvnCommandBufferBindTransformFeedbackBuffers);
    NVN_LOAD_PROC(nvnCommandBufferBindStorageBuffer);
    NVN_LOAD_PROC(nvnCommandBufferBindStorageBuffers);
    NVN_LOAD_PROC(nvnCommandBufferBindTexture);
    NVN_LOAD_PROC(nvnCommandBufferBindTextures);
    NVN_LOAD_PROC(nvnCommandBufferBindImage);
    NVN_LOAD_PROC(nvnCommandBufferBindImages);
    NVN_LOAD_PROC(nvnCommandBufferSetPatchSize);
    NVN_LOAD_PROC(nvnCommandBufferSetInnerTessellationLevels);
    NVN_LOAD_PROC(nvnCommandBufferSetOuterTessellationLevels);
    NVN_LOAD_PROC(nvnCommandBufferSetPrimitiveRestart);
    NVN_LOAD_PROC(nvnCommandBufferBeginTransformFeedback);
    NVN_LOAD_PROC(nvnCommandBufferEndTransformFeedback);
    NVN_LOAD_PROC(nvnCommandBufferPauseTransformFeedback);
    NVN_LOAD_PROC(nvnCommandBufferResumeTransformFeedback);
    NVN_LOAD_PROC(nvnCommandBufferDrawTransformFeedback);
    NVN_LOAD_PROC(nvnCommandBufferDrawArrays);
    NVN_LOAD_PROC(nvnCommandBufferDrawElements);
    NVN_LOAD_PROC(nvnCommandBufferDrawElementsBaseVertex);
    NVN_LOAD_PROC(nvnCommandBufferDrawArraysInstanced);
    NVN_LOAD_PROC(nvnCommandBufferDrawElementsInstanced);
    NVN_LOAD_PROC(nvnCommandBufferDrawArraysIndirect);
    NVN_LOAD_PROC(nvnCommandBufferDrawElementsIndirect);
    NVN_LOAD_PROC(nvnCommandBufferMultiDrawArraysIndirectCount);
    NVN_LOAD_PROC(nvnCommandBufferMultiDrawElementsIndirectCount);
    NVN_LOAD_PROC(nvnCommandBufferClearColor);
    NVN_LOAD_PROC(nvnCommandBufferClearColori);
    NVN_LOAD_PROC(nvnCommandBufferClearColorui);
    NVN_LOAD_PROC(nvnCommandBufferClearDepthStencil);
    NVN_LOAD_PROC(nvnCommandBufferDispatchCompute);
    NVN_LOAD_PROC(nvnCommandBufferDispatchComputeIndirect);
    NVN_LOAD_PROC(nvnCommandBufferSetViewport);
    NVN_LOAD_PROC(nvnCommandBufferSetViewports);
    NVN_LOAD_PROC(nvnCommandBufferSetViewportSwizzles);
    NVN_LOAD_PROC(nvnCommandBufferSetScissor);
    NVN_LOAD_PROC(nvnCommandBufferSetScissors);
    NVN_LOAD_PROC(nvnCommandBufferSetDepthRange);
    NVN_LOAD_PROC(nvnCommandBufferSetDepthBounds);
    NVN_LOAD_PROC(nvnCommandBufferSetDepthRanges);
    NVN_LOAD_PROC(nvnCommandBufferSetTiledCacheAction);
    NVN_LOAD_PROC(nvnCommandBufferSetTiledCacheTileSize);
    NVN_LOAD_PROC(nvnCommandBufferBindSeparateTexture);
    NVN_LOAD_PROC(nvnCommandBufferBindSeparateSampler);
    NVN_LOAD_PROC(nvnCommandBufferBindSeparateTextures);
    NVN_LOAD_PROC(nvnCommandBufferBindSeparateSamplers);
    NVN_LOAD_PROC(nvnCommandBufferSetStencilValueMask);
    NVN_LOAD_PROC(nvnCommandBufferSetStencilMask);
    NVN_LOAD_PROC(nvnCommandBufferSetStencilRef);
    NVN_LOAD_PROC(nvnCommandBufferSetBlendColor);
    NVN_LOAD_PROC(nvnCommandBufferSetPointSize);
    NVN_LOAD_PROC(nvnCommandBufferSetLineWidth);
    NVN_LOAD_PROC(nvnCommandBufferSetPolygonOffsetClamp);
    NVN_LOAD_PROC(nvnCommandBufferSetAlphaRef);
    NVN_LOAD_PROC(nvnCommandBufferSetSampleMask);
    NVN_LOAD_PROC(nvnCommandBufferSetRasterizerDiscard);
    NVN_LOAD_PROC(nvnCommandBufferSetDepthClamp);
    NVN_LOAD_PROC(nvnCommandBufferSetConservativeRasterEnable);
    NVN_LOAD_PROC(nvnCommandBufferSetConservativeRasterDilate);
    NVN_LOAD_PROC(nvnCommandBufferSetSubpixelPrecisionBias);
    NVN_LOAD_PROC(nvnCommandBufferCopyBufferToTexture);
    NVN_LOAD_PROC(nvnCommandBufferCopyTextureToBuffer);
    NVN_LOAD_PROC(nvnCommandBufferCopyTextureToTexture);
    NVN_LOAD_PROC(nvnCommandBufferCopyBufferToBuffer);
    NVN_LOAD_PROC(nvnCommandBufferClearBuffer);
    NVN_LOAD_PROC(nvnCommandBufferClearTexture);
    NVN_LOAD_PROC(nvnCommandBufferClearTexturei);
    NVN_LOAD_PROC(nvnCommandBufferClearTextureui);
    NVN_LOAD_PROC(nvnCommandBufferUpdateUniformBuffer);
    NVN_LOAD_PROC(nvnCommandBufferReportCounter);
    NVN_LOAD_PROC(nvnCommandBufferResetCounter);
    NVN_LOAD_PROC(nvnCommandBufferReportValue);
    NVN_LOAD_PROC(nvnCommandBufferSetRenderEnable);
    NVN_LOAD_PROC(nvnCommandBufferSetRenderEnableConditional);
    NVN_LOAD_PROC(nvnCommandBufferSetRenderTargets);
    NVN_LOAD_PROC(nvnCommandBufferDiscardColor);
    NVN_LOAD_PROC(nvnCommandBufferDiscardDepthStencil);
    NVN_LOAD_PROC(nvnCommandBufferDownsample);
    NVN_LOAD_PROC(nvnCommandBufferTiledDownsample);
    NVN_LOAD_PROC(nvnCommandBufferDownsampleTextureView);
    NVN_LOAD_PROC(nvnCommandBufferTiledDownsampleTextureView);
    NVN_LOAD_PROC(nvnCommandBufferBarrier);
    NVN_LOAD_PROC(nvnCommandBufferWaitSync);
    NVN_LOAD_PROC(nvnCommandBufferFenceSync);
    NVN_LOAD_PROC(nvnCommandBufferSetTexturePool);
    NVN_LOAD_PROC(nvnCommandBufferSetSamplerPool);
    NVN_LOAD_PROC(nvnCommandBufferSetShaderScratchMemory);
    NVN_LOAD_PROC(nvnCommandBufferSaveZCullData);
    NVN_LOAD_PROC(nvnCommandBufferRestoreZCullData);
    NVN_LOAD_PROC(nvnCommandBufferSetCopyRowStride);
    NVN_LOAD_PROC(nvnCommandBufferSetCopyImageStride);
    NVN_LOAD_PROC(nvnCommandBufferGetCopyRowStride);
    NVN_LOAD_PROC(nvnCommandBufferGetCopyImageStride);
    NVN_LOAD_PROC(nvnCommandBufferDrawTexture);
    NVN_LOAD_PROC(nvnProgramSetSubroutineLinkage);
    NVN_LOAD_PROC(nvnCommandBufferSetProgramSubroutines);
    NVN_LOAD_PROC(nvnCommandBufferBindCoverageModulationTable);
    NVN_LOAD_PROC(nvnCommandBufferResolveDepthBuffer);
    NVN_LOAD_PROC(nvnCommandBufferSetColorReductionEnable);
    NVN_LOAD_PROC(nvnCommandBufferSetColorReductionThresholds);
    NVN_LOAD_PROC(nvnCommandBufferPushDebugGroupStatic);
    NVN_LOAD_PROC(nvnCommandBufferPushDebugGroupDynamic);
    NVN_LOAD_PROC(nvnCommandBufferPushDebugGroup);
    NVN_LOAD_PROC(nvnCommandBufferPopDebugGroup);
    NVN_LOAD_PROC(nvnCommandBufferPopDebugGroupId);
    NVN_LOAD_PROC(nvnCommandBufferInsertDebugMarkerStatic);
    NVN_LOAD_PROC(nvnCommandBufferInsertDebugMarkerDynamic);
    NVN_LOAD_PROC(nvnCommandBufferInsertDebugMarker);
    NVN_LOAD_PROC(nvnCommandBufferGetMemoryCallback);
    NVN_LOAD_PROC(nvnCommandBufferGetMemoryCallbackData);
    NVN_LOAD_PROC(nvnCommandBufferIsRecording);
    NVN_LOAD_PROC(nvnSyncInitialize);
    NVN_LOAD_PROC(nvnSyncFinalize);
    NVN_LOAD_PROC(nvnSyncSetDebugLabel);
    NVN_LOAD_PROC(nvnQueueFenceSync);
    NVN_LOAD_PROC(nvnSyncWait);
    NVN_LOAD_PROC(nvnQueueWaitSync);
    NVN_LOAD_PROC(nvnSyncInitializeFromFencedGLSync);
    NVN_LOAD_PROC(nvnSyncCreateGLSync);
    NVN_LOAD_PROC(nvnEventBuilderSetDefaults);
    NVN_LOAD_PROC(nvnEventBuilderSetStorage);
    NVN_LOAD_PROC(nvnEventBuilderGetStorage);
    NVN_LOAD_PROC(nvnEventBuilderGetMemoryPool);
    NVN_LOAD_PROC(nvnEventBuilderGetMemoryOffset);
    NVN_LOAD_PROC(nvnEventInitialize);
    NVN_LOAD_PROC(nvnEventFinalize);
    NVN_LOAD_PROC(nvnEventGetValue);
    NVN_LOAD_PROC(nvnEventSignal);
    NVN_LOAD_PROC(nvnEventGetMemoryPool);
    NVN_LOAD_PROC(nvnEventGetMemoryOffset);
    NVN_LOAD_PROC(nvnCommandBufferWaitEvent);
    NVN_LOAD_PROC(nvnCommandBufferSignalEvent);
    NVN_LOAD_PROC(nvnCommandBufferSetStencilCullCriteria);
}

#undef NVN_LOAD_PROC

namespace nn::gfx::detail {

// Defined in gfx_CommonHelper.
ImageDimension GetImageDimension(ImageStorageDimension dimension, bool isArray,
                                 bool isMultisample);
size_t CalculateRowSize(uint32_t width, ChannelFormat format);

namespace {

struct ImageFormatAndNvnFormat {
    ImageFormat imageFormat;
    NVNformat nvnFormat;
};

struct AttributeFormatAndNvnFormat {
    AttributeFormat attributeFormat;
    NVNformat nvnFormat;
};

struct ImageFormatAndProperty {
    ImageFormat format;
    ImageFormatProperty property;
};

ImageFormatAndNvnFormat s_NvnTextureFormatList[] = {
    {ImageFormat_R8_Unorm, NVN_FORMAT_R8},
    {ImageFormat_R8_Snorm, NVN_FORMAT_R8SN},
    {ImageFormat_R8_Uint, NVN_FORMAT_R8UI},
    {ImageFormat_R8_Sint, NVN_FORMAT_R8I},
    {ImageFormat_R4_G4_B4_A4_Unorm, NVN_FORMAT_RGBA4},
    {ImageFormat_R5_G5_B5_A1_Unorm, NVN_FORMAT_RGB5A1},
    {ImageFormat_A1_B5_G5_R5_Unorm, NVN_FORMAT_A1BGR5},
    {ImageFormat_R5_G6_B5_Unorm, NVN_FORMAT_RGB565},
    {ImageFormat_B5_G6_R5_Unorm, NVN_FORMAT_BGR565},
    {ImageFormat_R8_G8_Unorm, NVN_FORMAT_RG8},
    {ImageFormat_R8_G8_Snorm, NVN_FORMAT_RG8SN},
    {ImageFormat_R8_G8_Uint, NVN_FORMAT_RG8UI},
    {ImageFormat_R8_G8_Sint, NVN_FORMAT_RG8I},
    {ImageFormat_R16_Unorm, NVN_FORMAT_R16},
    {ImageFormat_R16_Snorm, NVN_FORMAT_R16SN},
    {ImageFormat_R16_Uint, NVN_FORMAT_R16UI},
    {ImageFormat_R16_Sint, NVN_FORMAT_R16I},
    {ImageFormat_R16_Float, NVN_FORMAT_R16F},
    {ImageFormat_D16_Unorm, NVN_FORMAT_DEPTH16},
    {ImageFormat_R8_G8_B8_A8_Unorm, NVN_FORMAT_RGBA8},
    {ImageFormat_R8_G8_B8_A8_Snorm, NVN_FORMAT_RGBA8SN},
    {ImageFormat_R8_G8_B8_A8_Uint, NVN_FORMAT_RGBA8UI},
    {ImageFormat_R8_G8_B8_A8_Sint, NVN_FORMAT_RGBA8I},
    {ImageFormat_R8_G8_B8_A8_UnormSrgb, NVN_FORMAT_RGBA8_SRGB},
    {ImageFormat_B8_G8_R8_A8_Unorm, NVN_FORMAT_BGRA8},
    {ImageFormat_B8_G8_R8_A8_UnormSrgb, NVN_FORMAT_BGRA8_SRGB},
    {ImageFormat_R9_G9_B9_E5_SharedExp, NVN_FORMAT_RGB9E5F},
    {ImageFormat_R10_G10_B10_A2_Unorm, NVN_FORMAT_RGB10A2},
    {ImageFormat_R10_G10_B10_A2_Uint, NVN_FORMAT_RGB10A2UI},
    {ImageFormat_R11_G11_B10_Float, NVN_FORMAT_R11G11B10F},
    {ImageFormat_R16_G16_Unorm, NVN_FORMAT_RG16},
    {ImageFormat_R16_G16_Snorm, NVN_FORMAT_RG16SN},
    {ImageFormat_R16_G16_Uint, NVN_FORMAT_RG16UI},
    {ImageFormat_R16_G16_Sint, NVN_FORMAT_RG16I},
    {ImageFormat_R16_G16_Float, NVN_FORMAT_RG16F},
    {ImageFormat_D24_Unorm_S8_Uint, NVN_FORMAT_DEPTH24_STENCIL8},
    {ImageFormat_R32_Uint, NVN_FORMAT_R32UI},
    {ImageFormat_R32_Sint, NVN_FORMAT_R32I},
    {ImageFormat_R32_Float, NVN_FORMAT_R32F},
    {ImageFormat_D32_Float, NVN_FORMAT_DEPTH32F},
    {ImageFormat_R16_G16_B16_A16_Unorm, NVN_FORMAT_RGBA16},
    {ImageFormat_R16_G16_B16_A16_Snorm, NVN_FORMAT_RGBA16SN},
    {ImageFormat_R16_G16_B16_A16_Uint, NVN_FORMAT_RGBA16UI},
    {ImageFormat_R16_G16_B16_A16_Sint, NVN_FORMAT_RGBA16I},
    {ImageFormat_R16_G16_B16_A16_Float, NVN_FORMAT_RGBA16F},
    {ImageFormat_D32_Float_S8_Uint_X24, NVN_FORMAT_DEPTH32F_STENCIL8},
    {ImageFormat_R32_G32_Uint, NVN_FORMAT_RG32UI},
    {ImageFormat_R32_G32_Sint, NVN_FORMAT_RG32I},
    {ImageFormat_R32_G32_Float, NVN_FORMAT_RG32F},
    {ImageFormat_R32_G32_B32_Uint, NVN_FORMAT_RGB32UI},
    {ImageFormat_R32_G32_B32_Sint, NVN_FORMAT_RGB32I},
    {ImageFormat_R32_G32_B32_Float, NVN_FORMAT_RGB32F},
    {ImageFormat_R32_G32_B32_A32_Uint, NVN_FORMAT_RGBA32UI},
    {ImageFormat_R32_G32_B32_A32_Sint, NVN_FORMAT_RGBA32I},
    {ImageFormat_R32_G32_B32_A32_Float, NVN_FORMAT_RGBA32F},
    {ImageFormat_Bc1_Unorm, NVN_FORMAT_RGBA_DXT1},
    {ImageFormat_Bc1_UnormSrgb, NVN_FORMAT_RGBA_DXT1_SRGB},
    {ImageFormat_Bc2_Unorm, NVN_FORMAT_RGBA_DXT3},
    {ImageFormat_Bc2_UnormSrgb, NVN_FORMAT_RGBA_DXT3_SRGB},
    {ImageFormat_Bc3_Unorm, NVN_FORMAT_RGBA_DXT5},
    {ImageFormat_Bc3_UnormSrgb, NVN_FORMAT_RGBA_DXT5_SRGB},
    {ImageFormat_Bc4_Unorm, NVN_FORMAT_RGTC1_UNORM},
    {ImageFormat_Bc4_Snorm, NVN_FORMAT_RGTC1_SNORM},
    {ImageFormat_Bc5_Unorm, NVN_FORMAT_RGTC2_UNORM},
    {ImageFormat_Bc5_Snorm, NVN_FORMAT_RGTC2_SNORM},
    {ImageFormat_Bc6_Float, NVN_FORMAT_BPTC_SFLOAT},
    {ImageFormat_Bc6_Ufloat, NVN_FORMAT_BPTC_UFLOAT},
    {ImageFormat_Bc7_Unorm, NVN_FORMAT_BPTC_UNORM},
    {ImageFormat_Bc7_UnormSrgb, NVN_FORMAT_BPTC_UNORM_SRGB},
    {ImageFormat_Astc_4x4_Unorm, NVN_FORMAT_RGBA_ASTC_4x4},
    {ImageFormat_Astc_4x4_UnormSrgb, NVN_FORMAT_RGBA_ASTC_4x4_SRGB},
    {ImageFormat_Astc_5x4_Unorm, NVN_FORMAT_RGBA_ASTC_5x4},
    {ImageFormat_Astc_5x4_UnormSrgb, NVN_FORMAT_RGBA_ASTC_5x4_SRGB},
    {ImageFormat_Astc_5x5_Unorm, NVN_FORMAT_RGBA_ASTC_5x5},
    {ImageFormat_Astc_5x5_UnormSrgb, NVN_FORMAT_RGBA_ASTC_5x5_SRGB},
    {ImageFormat_Astc_6x5_Unorm, NVN_FORMAT_RGBA_ASTC_6x5},
    {ImageFormat_Astc_6x5_UnormSrgb, NVN_FORMAT_RGBA_ASTC_6x5_SRGB},
    {ImageFormat_Astc_6x6_Unorm, NVN_FORMAT_RGBA_ASTC_6x6},
    {ImageFormat_Astc_6x6_UnormSrgb, NVN_FORMAT_RGBA_ASTC_6x6_SRGB},
    {ImageFormat_Astc_8x5_Unorm, NVN_FORMAT_RGBA_ASTC_8x5},
    {ImageFormat_Astc_8x5_UnormSrgb, NVN_FORMAT_RGBA_ASTC_8x5_SRGB},
    {ImageFormat_Astc_8x6_Unorm, NVN_FORMAT_RGBA_ASTC_8x6},
    {ImageFormat_Astc_8x6_UnormSrgb, NVN_FORMAT_RGBA_ASTC_8x6_SRGB},
    {ImageFormat_Astc_8x8_Unorm, NVN_FORMAT_RGBA_ASTC_8x8},
    {ImageFormat_Astc_8x8_UnormSrgb, NVN_FORMAT_RGBA_ASTC_8x8_SRGB},
    {ImageFormat_Astc_10x5_Unorm, NVN_FORMAT_RGBA_ASTC_10x5},
    {ImageFormat_Astc_10x5_UnormSrgb, NVN_FORMAT_RGBA_ASTC_10x5_SRGB},
    {ImageFormat_Astc_10x6_Unorm, NVN_FORMAT_RGBA_ASTC_10x6},
    {ImageFormat_Astc_10x6_UnormSrgb, NVN_FORMAT_RGBA_ASTC_10x6_SRGB},
    {ImageFormat_Astc_10x8_Unorm, NVN_FORMAT_RGBA_ASTC_10x8},
    {ImageFormat_Astc_10x8_UnormSrgb, NVN_FORMAT_RGBA_ASTC_10x8_SRGB},
    {ImageFormat_Astc_10x10_Unorm, NVN_FORMAT_RGBA_ASTC_10x10},
    {ImageFormat_Astc_10x10_UnormSrgb, NVN_FORMAT_RGBA_ASTC_10x10_SRGB},
    {ImageFormat_Astc_12x10_Unorm, NVN_FORMAT_RGBA_ASTC_12x10},
    {ImageFormat_Astc_12x10_UnormSrgb, NVN_FORMAT_RGBA_ASTC_12x10_SRGB},
    {ImageFormat_Astc_12x12_Unorm, NVN_FORMAT_RGBA_ASTC_12x12},
    {ImageFormat_Astc_12x12_UnormSrgb, NVN_FORMAT_RGBA_ASTC_12x12_SRGB},
    {ImageFormat_B5_G5_R5_A1_Unorm, NVN_FORMAT_BGR5A1},
};

const AttributeFormatAndNvnFormat s_NvnAttributeFormatList[] = {
    {AttributeFormat_4_4_Unorm, NVN_FORMAT_NONE},
    {AttributeFormat_8_Unorm, NVN_FORMAT_R8},
    {AttributeFormat_8_Snorm, NVN_FORMAT_R8SN},
    {AttributeFormat_8_Uint, NVN_FORMAT_R8UI},
    {AttributeFormat_8_Sint, NVN_FORMAT_R8I},
    {AttributeFormat_8_UintToFloat, NVN_FORMAT_R8_UI2F},
    {AttributeFormat_8_SintToFloat, NVN_FORMAT_R8_I2F},
    {AttributeFormat_8_8_Unorm, NVN_FORMAT_RG8},
    {AttributeFormat_8_8_Snorm, NVN_FORMAT_RG8SN},
    {AttributeFormat_8_8_Uint, NVN_FORMAT_RG8UI},
    {AttributeFormat_8_8_Sint, NVN_FORMAT_RG8I},
    {AttributeFormat_8_8_UintToFloat, NVN_FORMAT_RG8_UI2F},
    {AttributeFormat_8_8_SintToFloat, NVN_FORMAT_RG8_I2F},
    {AttributeFormat_16_Unorm, NVN_FORMAT_R16},
    {AttributeFormat_16_Snorm, NVN_FORMAT_R16SN},
    {AttributeFormat_16_Uint, NVN_FORMAT_R16UI},
    {AttributeFormat_16_Sint, NVN_FORMAT_R16I},
    {AttributeFormat_16_Float, NVN_FORMAT_R16F},
    {AttributeFormat_16_UintToFloat, NVN_FORMAT_R16_UI2F},
    {AttributeFormat_16_SintToFloat, NVN_FORMAT_R16_I2F},
    {AttributeFormat_8_8_8_8_Unorm, NVN_FORMAT_RGBA8},
    {AttributeFormat_8_8_8_8_Snorm, NVN_FORMAT_RGBA8SN},
    {AttributeFormat_8_8_8_8_Uint, NVN_FORMAT_RGBA8UI},
    {AttributeFormat_8_8_8_8_Sint, NVN_FORMAT_RGBA8I},
    {AttributeFormat_8_8_8_8_UintToFloat, NVN_FORMAT_RGBA8_UI2F},
    {AttributeFormat_8_8_8_8_SintToFloat, NVN_FORMAT_RGBA8_I2F},
    {AttributeFormat_10_10_10_2_Unorm, NVN_FORMAT_RGB10A2},
    {AttributeFormat_10_10_10_2_Snorm, NVN_FORMAT_RGB10A2SN},
    {AttributeFormat_10_10_10_2_Uint, NVN_FORMAT_RGB10A2UI},
    {AttributeFormat_10_10_10_2_Sint, NVN_FORMAT_RGB10A2I},
    {AttributeFormat_16_16_Unorm, NVN_FORMAT_RG16},
    {AttributeFormat_16_16_Snorm, NVN_FORMAT_RG16SN},
    {AttributeFormat_16_16_Uint, NVN_FORMAT_RG16UI},
    {AttributeFormat_16_16_Sint, NVN_FORMAT_RG16I},
    {AttributeFormat_16_16_Float, NVN_FORMAT_RG16F},
    {AttributeFormat_16_16_UintToFloat, NVN_FORMAT_RG16_UI2F},
    {AttributeFormat_16_16_SintToFloat, NVN_FORMAT_RG16_I2F},
    {AttributeFormat_32_Uint, NVN_FORMAT_R32UI},
    {AttributeFormat_32_Sint, NVN_FORMAT_R32I},
    {AttributeFormat_32_Float, NVN_FORMAT_R32F},
    {AttributeFormat_16_16_16_16_Unorm, NVN_FORMAT_RGBA16},
    {AttributeFormat_16_16_16_16_Snorm, NVN_FORMAT_RGBA16SN},
    {AttributeFormat_16_16_16_16_Uint, NVN_FORMAT_RGBA16UI},
    {AttributeFormat_16_16_16_16_Sint, NVN_FORMAT_RGBA16I},
    {AttributeFormat_16_16_16_16_Float, NVN_FORMAT_RGBA16F},
    {AttributeFormat_16_16_16_16_UintToFloat, NVN_FORMAT_RGBA16_UI2F},
    {AttributeFormat_16_16_16_16_SintToFloat, NVN_FORMAT_RGBA16_I2F},
    {AttributeFormat_32_32_Uint, NVN_FORMAT_RG32UI},
    {AttributeFormat_32_32_Sint, NVN_FORMAT_RG32I},
    {AttributeFormat_32_32_Float, NVN_FORMAT_RG32F},
    {AttributeFormat_32_32_32_Uint, NVN_FORMAT_RGB32UI},
    {AttributeFormat_32_32_32_Sint, NVN_FORMAT_RGB32I},
    {AttributeFormat_32_32_32_Float, NVN_FORMAT_RGB32F},
    {AttributeFormat_32_32_32_32_Uint, NVN_FORMAT_RGBA32UI},
    {AttributeFormat_32_32_32_32_Sint, NVN_FORMAT_RGBA32I},
    {AttributeFormat_32_32_32_32_Float, NVN_FORMAT_RGBA32F},
};

const ImageFormatAndProperty g_ImageFormatAndPropertyTable[] = {
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_R8_Unorm, {7, 7}},
    {ImageFormat_R8_Snorm, {7, 7}},
    {ImageFormat_R8_Uint, {7, 7}},
    {ImageFormat_R8_Sint, {7, 7}},
    {ImageFormat_R16_Float, {7, 7}},
    {ImageFormat_R16_Unorm, {7, 7}},
    {ImageFormat_R16_Snorm, {7, 7}},
    {ImageFormat_R16_Uint, {7, 7}},
    {ImageFormat_R16_Sint, {7, 7}},
    {ImageFormat_R32_Float, {7, 7}},
    {ImageFormat_R32_Uint, {7, 7}},
    {ImageFormat_R32_Sint, {7, 7}},
    {ImageFormat_R8_G8_Unorm, {7, 7}},
    {ImageFormat_R8_G8_Snorm, {7, 7}},
    {ImageFormat_R8_G8_Uint, {7, 7}},
    {ImageFormat_R8_G8_Sint, {7, 7}},
    {ImageFormat_R16_G16_Float, {7, 7}},
    {ImageFormat_R16_G16_Unorm, {7, 7}},
    {ImageFormat_R16_G16_Snorm, {7, 7}},
    {ImageFormat_R16_G16_Uint, {7, 7}},
    {ImageFormat_R16_G16_Sint, {7, 7}},
    {ImageFormat_R32_G32_Float, {7, 7}},
    {ImageFormat_R32_G32_Uint, {7, 7}},
    {ImageFormat_R32_G32_Sint, {7, 7}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_R32_G32_B32_Float, {1, 1}},
    {ImageFormat_R32_G32_B32_Uint, {1, 1}},
    {ImageFormat_R32_G32_B32_Sint, {1, 1}},
    {ImageFormat_R8_G8_B8_A8_Unorm, {7, 7}},
    {ImageFormat_R8_G8_B8_A8_Snorm, {7, 7}},
    {ImageFormat_R8_G8_B8_A8_Uint, {7, 7}},
    {ImageFormat_R8_G8_B8_A8_Sint, {7, 7}},
    {ImageFormat_R16_G16_B16_A16_Float, {7, 7}},
    {ImageFormat_R16_G16_B16_A16_Unorm, {7, 7}},
    {ImageFormat_R16_G16_B16_A16_Snorm, {7, 7}},
    {ImageFormat_R16_G16_B16_A16_Uint, {7, 7}},
    {ImageFormat_R16_G16_B16_A16_Sint, {7, 7}},
    {ImageFormat_R32_G32_B32_A32_Float, {7, 7}},
    {ImageFormat_R32_G32_B32_A32_Uint, {7, 7}},
    {ImageFormat_R32_G32_B32_A32_Sint, {7, 7}},
    {ImageFormat_Undefined, {9, 9}},
    {ImageFormat_D16_Unorm, {9, 9}},
    {ImageFormat_Undefined, {9, 9}},
    {ImageFormat_D32_Float, {9, 9}},
    {ImageFormat_D24_Unorm_S8_Uint, {9, 9}},
    {ImageFormat_D32_Float_S8_Uint_X24, {9, 9}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_R8_G8_B8_A8_UnormSrgb, {3, 3}},
    {ImageFormat_R4_G4_B4_A4_Unorm, {1, 1}},
    {ImageFormat_Undefined, {1, 1}},
    {ImageFormat_R5_G5_B5_A1_Unorm, {1, 1}},
    {ImageFormat_R5_G6_B5_Unorm, {1, 1}},
    {ImageFormat_R10_G10_B10_A2_Unorm, {7, 7}},
    {ImageFormat_R10_G10_B10_A2_Uint, {7, 7}},
    {ImageFormat_R11_G11_B10_Float, {7, 7}},
    {ImageFormat_R9_G9_B9_E5_SharedExp, {1, 1}},
    {ImageFormat_Undefined, {1, 1}},
    {ImageFormat_Bc1_Unorm, {1, 1}},
    {ImageFormat_Bc2_Unorm, {1, 1}},
    {ImageFormat_Bc3_Unorm, {1, 1}},
    {ImageFormat_Undefined, {1, 1}},
    {ImageFormat_Bc1_UnormSrgb, {1, 1}},
    {ImageFormat_Bc2_UnormSrgb, {1, 1}},
    {ImageFormat_Bc3_UnormSrgb, {1, 1}},
    {ImageFormat_Bc4_Unorm, {1, 1}},
    {ImageFormat_Bc4_Snorm, {1, 1}},
    {ImageFormat_Bc5_Unorm, {1, 1}},
    {ImageFormat_Bc5_Snorm, {1, 1}},
    {ImageFormat_Bc7_Unorm, {1, 1}},
    {ImageFormat_Bc7_UnormSrgb, {1, 1}},
    {ImageFormat_Bc6_Float, {1, 1}},
    {ImageFormat_Bc6_Ufloat, {1, 1}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {0, 0}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_Astc_4x4_Unorm, {1, 1}},
    {ImageFormat_Astc_5x4_Unorm, {1, 1}},
    {ImageFormat_Astc_5x5_Unorm, {1, 1}},
    {ImageFormat_Astc_6x5_Unorm, {1, 1}},
    {ImageFormat_Astc_6x6_Unorm, {1, 1}},
    {ImageFormat_Astc_8x5_Unorm, {1, 1}},
    {ImageFormat_Astc_8x6_Unorm, {1, 1}},
    {ImageFormat_Astc_8x8_Unorm, {1, 1}},
    {ImageFormat_Astc_10x5_Unorm, {1, 1}},
    {ImageFormat_Astc_10x6_Unorm, {1, 1}},
    {ImageFormat_Astc_10x8_Unorm, {1, 1}},
    {ImageFormat_Astc_10x10_Unorm, {1, 1}},
    {ImageFormat_Astc_12x10_Unorm, {1, 1}},
    {ImageFormat_Astc_12x12_Unorm, {1, 1}},
    {ImageFormat_Astc_4x4_UnormSrgb, {1, 1}},
    {ImageFormat_Astc_5x4_UnormSrgb, {1, 1}},
    {ImageFormat_Astc_5x5_UnormSrgb, {1, 1}},
    {ImageFormat_Astc_6x5_UnormSrgb, {1, 1}},
    {ImageFormat_Astc_6x6_UnormSrgb, {1, 1}},
    {ImageFormat_Astc_8x5_UnormSrgb, {1, 1}},
    {ImageFormat_Astc_8x6_UnormSrgb, {1, 1}},
    {ImageFormat_Astc_8x8_UnormSrgb, {1, 1}},
    {ImageFormat_Astc_10x5_UnormSrgb, {1, 1}},
    {ImageFormat_Astc_10x6_UnormSrgb, {1, 1}},
    {ImageFormat_Astc_10x8_UnormSrgb, {1, 1}},
    {ImageFormat_Astc_10x10_UnormSrgb, {1, 1}},
    {ImageFormat_Astc_12x10_UnormSrgb, {1, 1}},
    {ImageFormat_Astc_12x12_UnormSrgb, {1, 1}},
    {ImageFormat_B5_G6_R5_Unorm, {3, 3}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_B5_G5_R5_A1_Unorm, {3, 3}},
    {ImageFormat_A1_B5_G5_R5_Unorm, {1, 1}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_B8_G8_R8_A8_Unorm, {7, 7}},
    {ImageFormat_Undefined, {3, 3}},
    {ImageFormat_B8_G8_R8_A8_UnormSrgb, {3, 3}},
};

const float s_BorderColorTable[][4] = {
    {1.0f, 1.0f, 1.0f, 1.0f},
    {0.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

}  // namespace

/**
 * Checks the NVN API version against the one required by nn::gfx (no-op in release builds).
 *
 * @param majorVersion Required major version.
 * @param minorVersion Required minor version.
 */
void Nvn::CheckRequiredVersion(int majorVersion, int minorVersion) {}

/**
 * Converts an image format to the matching NVN format.
 *
 * @param format Image format.
 * @return NVN format, or NVN_FORMAT_NONE if the format has no NVN equivalent.
 */
NVNformat Nvn::GetImageFormat(ImageFormat format) {
    struct Comp {
        bool operator()(const ImageFormatAndNvnFormat& rEntry, ImageFormat value) {
            return rEntry.imageFormat < value;
        }
    };

    ImageFormatAndNvnFormat* pEnd = std::end(s_NvnTextureFormatList);
    ImageFormatAndNvnFormat* pFound =
        std::lower_bound(s_NvnTextureFormatList, pEnd, format, Comp());
    if (pFound != pEnd && format >= pFound->imageFormat) {
        return pFound->nvnFormat;
    }

    return NVN_FORMAT_NONE;
}

/**
 * Converts a vertex attribute format to the matching NVN format.
 *
 * @param format Attribute format.
 * @return NVN format, or NVN_FORMAT_NONE if the format has no NVN equivalent.
 */
NVNformat Nvn::GetAttributeFormat(AttributeFormat format) {
    struct Comp {
        bool operator()(const AttributeFormatAndNvnFormat& rEntry, AttributeFormat value) {
            return rEntry.attributeFormat < value;
        }
    };

    const AttributeFormatAndNvnFormat* pEnd = std::end(s_NvnAttributeFormatList);
    const AttributeFormatAndNvnFormat* pFound =
        std::lower_bound(s_NvnAttributeFormatList, pEnd, format, Comp());
    if (pFound != pEnd && format >= pFound->attributeFormat) {
        return pFound->nvnFormat;
    }

    return NVN_FORMAT_NONE;
}

/**
 * Converts an image dimension to the matching NVN texture target.
 *
 * @param dimension Image dimension.
 * @return NVN texture target.
 */
NVNtextureTarget Nvn::GetImageTarget(ImageDimension dimension) {
    const NVNtextureTarget s_TargetTable[] = {
        NVN_TEXTURE_TARGET_1D,
        NVN_TEXTURE_TARGET_2D,
        NVN_TEXTURE_TARGET_3D,
        NVN_TEXTURE_TARGET_CUBEMAP,
        NVN_TEXTURE_TARGET_1D_ARRAY,
        NVN_TEXTURE_TARGET_2D_ARRAY,
        NVN_TEXTURE_TARGET_2D_MULTISAMPLE,
        NVN_TEXTURE_TARGET_2D_MULTISAMPLE_ARRAY,
        NVN_TEXTURE_TARGET_CUBEMAP_ARRAY,
        NVN_TEXTURE_TARGET_RECTANGLE,
    };

    return s_TargetTable[dimension];
}

/**
 * Converts a comparison function to the matching NVN depth function.
 *
 * @param compare Comparison function.
 * @return NVN depth function.
 */
NVNdepthFunc Nvn::GetDepthFunction(ComparisonFunction compare) {
    const NVNdepthFunc s_DepthFunction[] = {
        NVN_DEPTH_FUNC_NEVER,  NVN_DEPTH_FUNC_LESS,    NVN_DEPTH_FUNC_EQUAL,
        NVN_DEPTH_FUNC_LEQUAL, NVN_DEPTH_FUNC_GREATER, NVN_DEPTH_FUNC_NOTEQUAL,
        NVN_DEPTH_FUNC_GEQUAL, NVN_DEPTH_FUNC_ALWAYS,
    };

    return s_DepthFunction[compare];
}

/**
 * Converts a stencil operation to the matching NVN stencil operation.
 *
 * @param operation Stencil operation.
 * @return NVN stencil operation.
 */
NVNstencilOp Nvn::GetStencilOperation(StencilOperation operation) {
    const NVNstencilOp s_StencilOperation[] = {
        NVN_STENCIL_OP_KEEP,      NVN_STENCIL_OP_ZERO,      NVN_STENCIL_OP_REPLACE,
        NVN_STENCIL_OP_INCR,      NVN_STENCIL_OP_DECR,      NVN_STENCIL_OP_INVERT,
        NVN_STENCIL_OP_INCR_WRAP, NVN_STENCIL_OP_DECR_WRAP,
    };

    return s_StencilOperation[operation];
}

/**
 * Converts a comparison function to the matching NVN stencil function.
 *
 * @param compare Comparison function.
 * @return NVN stencil function.
 */
NVNstencilFunc Nvn::GetStencilFunction(ComparisonFunction compare) {
    const NVNstencilFunc s_StencilFunction[] = {
        NVN_STENCIL_FUNC_NEVER,  NVN_STENCIL_FUNC_LESS,    NVN_STENCIL_FUNC_EQUAL,
        NVN_STENCIL_FUNC_LEQUAL, NVN_STENCIL_FUNC_GREATER, NVN_STENCIL_FUNC_NOTEQUAL,
        NVN_STENCIL_FUNC_GEQUAL, NVN_STENCIL_FUNC_ALWAYS,
    };

    return s_StencilFunction[compare];
}

/**
 * Converts a blend function to the matching NVN blend equation.
 *
 * @param function Blend function.
 * @return NVN blend equation.
 */
NVNblendEquation Nvn::GetBlendEquation(BlendFunction function) {
    const NVNblendEquation s_BlendEquation[] = {
        NVN_BLEND_EQUATION_ADD, NVN_BLEND_EQUATION_SUB, NVN_BLEND_EQUATION_REVERSE_SUB,
        NVN_BLEND_EQUATION_MIN, NVN_BLEND_EQUATION_MAX,
    };

    return s_BlendEquation[function];
}

/**
 * Converts a blend factor to the matching NVN blend function.
 *
 * @param factor Blend factor.
 * @return NVN blend function.
 */
NVNblendFunc Nvn::GetBlendFunction(BlendFactor factor) {
    const NVNblendFunc s_BlendFunction[] = {
        NVN_BLEND_FUNC_ZERO,
        NVN_BLEND_FUNC_ONE,
        NVN_BLEND_FUNC_SRC_COLOR,
        NVN_BLEND_FUNC_ONE_MINUS_SRC_COLOR,
        NVN_BLEND_FUNC_DST_COLOR,
        NVN_BLEND_FUNC_ONE_MINUS_DST_COLOR,
        NVN_BLEND_FUNC_SRC_ALPHA,
        NVN_BLEND_FUNC_ONE_MINUS_SRC_ALPHA,
        NVN_BLEND_FUNC_DST_ALPHA,
        NVN_BLEND_FUNC_ONE_MINUS_DST_ALPHA,
        NVN_BLEND_FUNC_CONSTANT_COLOR,
        NVN_BLEND_FUNC_ONE_MINUS_CONSTANT_COLOR,
        NVN_BLEND_FUNC_CONSTANT_ALPHA,
        NVN_BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA,
        NVN_BLEND_FUNC_SRC_ALPHA_SATURATE,
        NVN_BLEND_FUNC_SRC1_COLOR,
        NVN_BLEND_FUNC_ONE_MINUS_SRC1_COLOR,
        NVN_BLEND_FUNC_SRC1_ALPHA,
        NVN_BLEND_FUNC_ONE_MINUS_SRC1_ALPHA,
    };

    return s_BlendFunction[factor];
}

/**
 * Converts a logic operation to the matching NVN logic operation.
 *
 * @param operation Logic operation.
 * @return NVN logic operation.
 */
NVNlogicOp Nvn::GetLogicOperation(LogicOperation operation) {
    const NVNlogicOp s_LogicOperation[] = {
        NVN_LOGIC_OP_CLEAR,         NVN_LOGIC_OP_AND,          NVN_LOGIC_OP_AND_REVERSE,
        NVN_LOGIC_OP_COPY,          NVN_LOGIC_OP_AND_INVERTED, NVN_LOGIC_OP_NOOP,
        NVN_LOGIC_OP_XOR,           NVN_LOGIC_OP_OR,           NVN_LOGIC_OP_NOR,
        NVN_LOGIC_OP_EQUIV,         NVN_LOGIC_OP_INVERT,       NVN_LOGIC_OP_OR_REVERSE,
        NVN_LOGIC_OP_COPY_INVERTED, NVN_LOGIC_OP_OR_INVERTED,  NVN_LOGIC_OP_NAND,
        NVN_LOGIC_OP_SET,
    };

    return s_LogicOperation[operation];
}

/**
 * Converts a front face winding to the matching NVN front face.
 *
 * @param frontFace Front face winding.
 * @return NVN front face.
 */
NVNfrontFace Nvn::GetFrontFace(FrontFace frontFace) {
    const NVNfrontFace s_FrontFace[] = {
        NVN_FRONT_FACE_CCW,
        NVN_FRONT_FACE_CW,
    };

    return s_FrontFace[frontFace];
}

/**
 * Converts a fill mode to the matching NVN polygon mode.
 *
 * @param fillMode Fill mode.
 * @return NVN polygon mode.
 */
NVNpolygonMode Nvn::GetFillMode(FillMode fillMode) {
    const NVNpolygonMode s_FillMode[] = {
        NVN_POLYGON_MODE_POINT,
        NVN_POLYGON_MODE_LINE,
        NVN_POLYGON_MODE_FILL,
    };

    return s_FillMode[fillMode];
}

/**
 * Converts a primitive topology to the matching NVN draw primitive.
 *
 * @param topology Primitive topology.
 * @return NVN draw primitive.
 */
NVNdrawPrimitive Nvn::GetDrawPrimitive(PrimitiveTopology topology) {
    const NVNdrawPrimitive s_DrawPrimitive[] = {
        NVN_DRAW_PRIMITIVE_POINTS,
        NVN_DRAW_PRIMITIVE_LINES,
        NVN_DRAW_PRIMITIVE_LINE_STRIP,
        NVN_DRAW_PRIMITIVE_TRIANGLES,
        NVN_DRAW_PRIMITIVE_TRIANGLE_STRIP,
        NVN_DRAW_PRIMITIVE_LINES_ADJACENCY,
        NVN_DRAW_PRIMITIVE_LINE_STRIP_ADJACENCY,
        NVN_DRAW_PRIMITIVE_TRIANGLES_ADJACENCY,
        NVN_DRAW_PRIMITIVE_TRIANGLE_STRIP_ADJACENCY,
        NVN_DRAW_PRIMITIVE_PATCHES,
    };

    return s_DrawPrimitive[topology];
}

/**
 * Converts the minification and mip filter bits of a filter mode to an NVN min filter.
 *
 * @param filterMode Filter mode.
 * @return NVN min filter.
 */
NVNminFilter Nvn::GetMinFilter(FilterMode filterMode) {
    switch ((filterMode & FilterModeBit_MinFilterMask) >> FilterModeBit_MinFilterShift) {
    case FilterModeBit_Point:
        switch ((filterMode & FilterModeBit_MipFilterMask) >> FilterModeBit_MipFilterShift) {
        case FilterModeBit_Point:
            return NVN_MIN_FILTER_NEAREST_MIPMAP_NEAREST;

        case FilterModeBit_Linear:
            return NVN_MIN_FILTER_NEAREST_MIPMAP_LINEAR;

        default:
            return NVN_MIN_FILTER_NEAREST;
        }

    case FilterModeBit_Linear:
        switch ((filterMode & FilterModeBit_MipFilterMask) >> FilterModeBit_MipFilterShift) {
        case FilterModeBit_Point:
            return NVN_MIN_FILTER_LINEAR_MIPMAP_NEAREST;

        case FilterModeBit_Linear:
            return NVN_MIN_FILTER_LINEAR_MIPMAP_LINEAR;

        default:
            return NVN_MIN_FILTER_LINEAR;
        }

    default:
        NN_UNEXPECTED_DEFAULT;
    }
}

/**
 * Converts the magnification filter bits of a filter mode to an NVN mag filter.
 *
 * @param filterMode Filter mode.
 * @return NVN mag filter.
 */
NVNmagFilter Nvn::GetMagFilter(FilterMode filterMode) {
    switch ((filterMode & FilterModeBit_MagFilterMask) >> FilterModeBit_MagFilterShift) {
    case FilterModeBit_Point:
        return NVN_MAG_FILTER_NEAREST;

    case FilterModeBit_Linear:
        return NVN_MAG_FILTER_LINEAR;

    default:
        NN_UNEXPECTED_DEFAULT;
    }
}

/**
 * Converts a texture address mode to the matching NVN wrap mode.
 *
 * @param addressMode Texture address mode.
 * @return NVN wrap mode.
 */
NVNwrapMode Nvn::GetWrapMode(TextureAddressMode addressMode) {
    const NVNwrapMode s_WrapMode[] = {
        NVN_WRAP_MODE_REPEAT,
        NVN_WRAP_MODE_MIRRORED_REPEAT,
        NVN_WRAP_MODE_CLAMP_TO_EDGE,
        NVN_WRAP_MODE_CLAMP_TO_BORDER,
        NVN_WRAP_MODE_MIRROR_CLAMP_TO_EDGE,
    };

    return s_WrapMode[addressMode];
}

/**
 * Converts a shader stage to the matching NVN shader stage.
 *
 * @param stage Shader stage.
 * @return NVN shader stage.
 */
NVNshaderStage Nvn::GetShaderStage(ShaderStage stage) {
    const NVNshaderStage s_ShaderStage[] = {
        NVN_SHADER_STAGE_VERTEX,   NVN_SHADER_STAGE_TESS_CONTROL, NVN_SHADER_STAGE_TESS_EVALUATION,
        NVN_SHADER_STAGE_GEOMETRY, NVN_SHADER_STAGE_FRAGMENT,     NVN_SHADER_STAGE_COMPUTE,
    };

    return s_ShaderStage[stage];
}

/**
 * Converts a combination of shader stage bits to the matching NVN shader stage bits.
 *
 * @param shaderStageBits Combination of ShaderStageBit values.
 * @return Combination of NVNshaderStageBits values.
 */
int Nvn::GetShaderStageBits(int shaderStageBits) {
    const NVNshaderStageBits s_ShaderStageBitTable[] = {
        NVN_SHADER_STAGE_VERTEX_BIT,          NVN_SHADER_STAGE_TESS_CONTROL_BIT,
        NVN_SHADER_STAGE_TESS_EVALUATION_BIT, NVN_SHADER_STAGE_GEOMETRY_BIT,
        NVN_SHADER_STAGE_FRAGMENT_BIT,        NVN_SHADER_STAGE_COMPUTE_BIT,
    };

    int result = 0;
    for (int idxStage = 0, mask = 1; idxStage < ShaderStage_End; ++idxStage, mask <<= 1) {
        if (shaderStageBits & mask) {
            result |= s_ShaderStageBitTable[idxStage];
        }
    }

    return result;
}

/**
 * Converts a comparison function to the matching NVN compare function.
 *
 * @param compare Comparison function.
 * @return NVN compare function.
 */
NVNcompareFunc Nvn::GetRComparisonFunction(ComparisonFunction compare) {
    const NVNcompareFunc s_ComparisonFunction[] = {
        NVN_COMPARE_FUNC_NEVER,  NVN_COMPARE_FUNC_LESS,    NVN_COMPARE_FUNC_EQUAL,
        NVN_COMPARE_FUNC_LEQUAL, NVN_COMPARE_FUNC_GREATER, NVN_COMPARE_FUNC_NOTEQUAL,
        NVN_COMPARE_FUNC_GEQUAL, NVN_COMPARE_FUNC_ALWAYS,
    };

    return s_ComparisonFunction[compare];
}

/**
 * Converts an index format to the matching NVN index type.
 *
 * @param format Index format.
 * @return NVN index type.
 */
NVNindexType Nvn::GetIndexFormat(IndexFormat format) {
    const NVNindexType s_IndexFormat[] = {
        NVN_INDEX_TYPE_UNSIGNED_BYTE,
        NVN_INDEX_TYPE_UNSIGNED_SHORT,
        NVN_INDEX_TYPE_UNSIGNED_INT,
    };

    return s_IndexFormat[format];
}

/**
 * Converts a cull mode to the matching NVN face.
 *
 * @param cullMode Cull mode.
 * @return NVN face to cull.
 */
NVNface Nvn::GetCullMode(CullMode cullMode) {
    const NVNface s_CullMode[] = {
        NVN_FACE_NONE,
        NVN_FACE_FRONT,
        NVN_FACE_BACK,
    };

    return s_CullMode[cullMode];
}

/**
 * Converts memory pool properties to NVN memory pool flags.
 *
 * @param memoryPoolProperty Combination of MemoryPoolProperty values.
 * @return NVN memory pool flags (the bit layouts are identical).
 */
int Nvn::GetMemoryPoolFlags(int memoryPoolProperty) {
    return memoryPoolProperty;
}

/**
 * Converts a GPU address to an NVN buffer address.
 *
 * @param gpuAddress GPU address.
 * @return NVN buffer address.
 */
NVNbufferAddress Nvn::GetBufferAddress(GpuAddress gpuAddress) {
    return gpuAddress.ToData()->value;
}

/**
 * Fills in the texture info describing a scan buffer of a swap chain.
 *
 * @param pOutInfo Texture info to fill in.
 * @param rInfo Swap chain description.
 */
void Nvn::SetupScanBufferTextureInfo(TextureInfo* pOutInfo, const SwapChainInfo& rInfo) {
    pOutInfo->SetImageStorageDimension(ImageStorageDimension_2d);
    pOutInfo->SetTileMode(TileMode_Optimal);
    pOutInfo->SetSwizzle(0);
    pOutInfo->SetMipCount(1);
    pOutInfo->SetMultiSampleCount(1);
    pOutInfo->SetImageFormat(rInfo.GetFormat());
    pOutInfo->SetGpuAccessFlags(GpuAccess_ScanBuffer | GpuAccess_ColorBuffer);
    pOutInfo->SetWidth(rInfo.GetWidth());
    pOutInfo->SetHeight(rInfo.GetHeight());
    pOutInfo->SetDepth(1);
    pOutInfo->SetArrayLength(1);
}

/**
 * Converts a query target to the matching NVN counter type.
 *
 * @param target Query target.
 * @return NVN counter type.
 */
NVNcounterType Nvn::GetCounterType(QueryTarget target) {
    const NVNcounterType s_CounterTypeTable[] = {
        NVN_COUNTER_TYPE_TIMESTAMP,
        NVN_COUNTER_TYPE_SAMPLES_PASSED,
        NVN_COUNTER_TYPE_INPUT_VERTICES,
        NVN_COUNTER_TYPE_INPUT_PRIMITIVES,
        NVN_COUNTER_TYPE_VERTEX_SHADER_INVOCATIONS,
        NVN_COUNTER_TYPE_GEOMETRY_SHADER_INVOCATIONS,
        NVN_COUNTER_TYPE_GEOMETRY_SHADER_PRIMITIVES,
        NVN_COUNTER_TYPE_CLIPPER_INPUT_PRIMITIVES,
        NVN_COUNTER_TYPE_CLIPPER_OUTPUT_PRIMITIVES,
        NVN_COUNTER_TYPE_FRAGMENT_SHADER_INVOCATIONS,
        NVN_COUNTER_TYPE_TESS_CONTROL_SHADER_INVOCATIONS,
        NVN_COUNTER_TYPE_TESS_EVALUATION_SHADER_INVOCATIONS,
        static_cast<NVNcounterType>(-1),
    };

    return s_CounterTypeTable[target];
}

/**
 * Converts the reduction bits of a filter mode to an NVN sampler reduction.
 *
 * @param filterMode Filter mode.
 * @return NVN sampler reduction.
 */
NVNsamplerReduction Nvn::GetSamplerReduction(FilterMode filterMode) {
    const NVNsamplerReduction s_SamplerReduction[] = {
        NVN_SAMPLER_REDUCTION_AVERAGE,
        NVN_SAMPLER_REDUCTION_MIN,
        NVN_SAMPLER_REDUCTION_MAX,
    };

    return s_SamplerReduction[(filterMode >> 8) & 3];
}

/**
 * Returns the RGBA color of a border color type.
 *
 * @param borderColorType Border color type.
 * @return Pointer to four floats.
 */
const float* Nvn::GetBorderColor(TextureBorderColorType borderColorType) {
    return s_BorderColorTable[borderColorType];
}

/**
 * Queries the optional features supported by an NVN device.
 *
 * @param pDevice Device to query.
 * @return Bit set of NvnDeviceFeature values.
 */
nn::util::BitPack32 Nvn::GetDeviceFeature(const NVNdevice* pDevice) {
    nn::util::BitPack32 feature;
    feature.Clear();

    int supported;

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_MIN_MAX_FILTERING, &supported);
    feature.SetBit(NvnDeviceFeature_SupportMinMaxFiltering, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_STENCIL8_FORMAT, &supported);
    feature.SetBit(NvnDeviceFeature_SupportStencil8Format, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_ASTC_FORMATS, &supported);
    feature.SetBit(NvnDeviceFeature_SupportAstcFormat, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_CONSERVATIVE_RASTER, &supported);
    feature.SetBit(NvnDeviceFeature_SupportConservativeRaster, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_ZERO_FROM_UNMAPPED_VIRTUAL_POOL_PAGES,
                        &supported);
    feature.SetBit(NvnDeviceFeature_SupportZeroFromUnmappedVirtualPoolPage, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_PASSTHROUGH_GEOMETRY_SHADERS,
                        &supported);
    feature.SetBit(NvnDeviceFeature_SupportPassthroughGeometryShader, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_VIEWPORT_SWIZZLE, &supported);
    feature.SetBit(NvnDeviceFeature_SupportViewportSwizzle, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_SPARSE_TILED_PACKAGED_TEXTURES,
                        &supported);
    feature.SetBit(NvnDeviceFeature_SupportSparseTiledPackagedTexture, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_ADVANCED_BLEND_MODES, &supported);
    feature.SetBit(NvnDeviceFeature_AdvancedBlendModes, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_DRAW_TEXTURE, &supported);
    feature.SetBit(NvnDeviceFeature_DrawTexture, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_TARGET_INDEPENDENT_RASTERIZATION,
                        &supported);
    feature.SetBit(NvnDeviceFeature_TargetIndependentRasterization, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_FRAGMENT_COVERAGE_TO_COLOR, &supported);
    feature.SetBit(NvnDeviceFeature_FragmentCoverageToColor, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_POST_DEPTH_COVERAGE, &supported);
    feature.SetBit(NvnDeviceFeature_PostDepthCoverage, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_IMAGES_USING_TEXTURE_HANDLES,
                        &supported);
    feature.SetBit(NvnDeviceFeature_ImagesUsingTextureHandles, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_SAMPLE_LOCATIONS, &supported);
    feature.SetBit(NvnDeviceFeature_SampleLocations, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_FRAGMENT_SHADER_INTERLOCK, &supported);
    feature.SetBit(NvnDeviceFeature_SupportFragmentShaderInterlock, supported);

    nvnDeviceGetInteger(pDevice, NVN_DEVICE_INFO_SUPPORTS_DEBUG_LAYER, &supported);
    feature.SetBit(NvnDeviceFeature_SupportsDebugLayer, supported);

    return feature;
}

/**
 * Returns the format properties of an NVN format.
 *
 * @param pOutProperty Receives the properties.
 * @param nvnFormat NVN format.
 */
void Nvn::GetImageFormatProperty(ImageFormatProperty* pOutProperty, NVNformat nvnFormat) {
    *pOutProperty = g_ImageFormatAndPropertyTable[nvnFormat].property;
}

/**
 * Converts an NVN format back to the matching image format.
 *
 * @param nvnFormat NVN format.
 * @return Image format, or ImageFormat_Undefined.
 */
ImageFormat Nvn::GetGfxImageFormat(NVNformat nvnFormat) {
    return g_ImageFormatAndPropertyTable[nvnFormat].format;
}

/**
 * Initializes an NVN queue builder from a queue description.
 *
 * @param pBuilder Builder to initialize.
 * @param rInfo Queue description.
 */
void Nvn::ConvertToNvnQueueBuilder(NVNqueueBuilder* pBuilder, const QueueInfo& rInfo) {
    nvnQueueBuilderSetDefaults(pBuilder);

    if ((rInfo.ToData()->capability & QueueCapability_Compute) == 0) {
        nvnQueueBuilderSetComputeMemorySize(pBuilder, 0);
    }
}

/**
 * Initializes an NVN memory pool builder from a memory pool description.
 *
 * @param pBuilder Builder to initialize.
 * @param rInfo Memory pool description.
 */
void Nvn::ConvertToNvnMemoryPoolBuilder(NVNmemoryPoolBuilder* pBuilder,
                                        const MemoryPoolInfo& rInfo) {
    nvnMemoryPoolBuilderSetDefaults(pBuilder);
    nvnMemoryPoolBuilderSetFlags(pBuilder, GetMemoryPoolFlags(rInfo.GetMemoryPoolProperty()));
    nvnMemoryPoolBuilderSetStorage(pBuilder, rInfo.GetPoolMemory(), rInfo.GetPoolMemorySize());
}

/**
 * Initializes an NVN buffer builder from a buffer description.
 *
 * @param pBuilder Builder to initialize.
 * @param rInfo Buffer description (unused; storage is set by the caller).
 */
void Nvn::ConvertToNvnBufferBuilder(NVNbufferBuilder* pBuilder, const BufferInfo& rInfo) {
    nvnBufferBuilderSetDefaults(pBuilder);
}

/**
 * Initializes an NVN texture builder from a texture description.
 *
 * @param pBuilder Builder to initialize.
 * @param rInfo Texture description.
 */
void Nvn::ConvertToNvnTextureBuilder(NVNtextureBuilder* pBuilder, const TextureInfo& rInfo) {
    int arrayLength = rInfo.GetArrayLength();
    ImageDimension dimension = GetImageDimension(rInfo.GetImageStorageDimension(),
                                                 arrayLength > 1, rInfo.GetMultisampleCount() > 1);
    NVNtextureTarget target = GetImageTarget(dimension);

    nvnTextureBuilderSetDefaults(pBuilder);
    nvnTextureBuilderSetWidth(pBuilder, rInfo.GetWidth());
    nvnTextureBuilderSetHeight(pBuilder,
                               dimension == ImageDimension_1dArray ? arrayLength
                                                                   : rInfo.GetHeight());
    nvnTextureBuilderSetDepth(pBuilder, target == NVN_TEXTURE_TARGET_2D_ARRAY ||
                                                target == NVN_TEXTURE_TARGET_2D_MULTISAMPLE_ARRAY ?
                                            arrayLength :
                                            rInfo.GetDepth());
    nvnTextureBuilderSetLevels(pBuilder, rInfo.GetMipCount());

    if (rInfo.GetMultisampleCount() > 1) {
        nvnTextureBuilderSetSamples(pBuilder, rInfo.GetMultisampleCount());
    }

    nvnTextureBuilderSetTarget(pBuilder, target);
    nvnTextureBuilderSetFormat(pBuilder, GetImageFormat(rInfo.GetImageFormat()));

    int gpuAccessFlags = rInfo.GetGpuAccessFlags();
    int renderTargetAccess = gpuAccessFlags & (GpuAccess_ColorBuffer | GpuAccess_DepthStencil);
    int flags = renderTargetAccess == 0 || rInfo.GetTileMode() == TileMode_Linear ?
                    NVN_TEXTURE_FLAGS_IMAGE :
                    NVN_TEXTURE_FLAGS_IMAGE | NVN_TEXTURE_FLAGS_COMPRESSIBLE;

    if (gpuAccessFlags & GpuAccess_ScanBuffer) {
        flags |= NVN_TEXTURE_FLAGS_DISPLAY;
    }

    if (rInfo.ToData()->flags.GetBit(TextureInfoData::Flag_SparseResidency)) {
        flags |= NVN_TEXTURE_FLAGS_SPARSE;
    }

    if (rInfo.GetTileMode() == TileMode_Linear) {
        size_t strideAlignment;
        if (renderTargetAccess == 0) {
            flags |= NVN_TEXTURE_FLAGS_LINEAR;
            strideAlignment = 32;
        } else {
            flags |= NVN_TEXTURE_FLAGS_LINEAR_RENDER_TARGET;
            strideAlignment = 128;
        }

        size_t rowSize = CalculateRowSize(
            rInfo.GetWidth(), static_cast<ChannelFormat>(rInfo.GetImageFormat() >> 8));
        nvnTextureBuilderSetStride(pBuilder, (rowSize + strideAlignment - 1) & -strideAlignment);
    }

    nvnTextureBuilderSetFlags(pBuilder, flags);

    if (rInfo.ToData()->flags.GetBit(TextureInfoData::Flag_SpecifyTextureLayout)) {
        nvnTextureBuilderSetPackagedTextureLayout(
            pBuilder,
            reinterpret_cast<const NVNpackagedTextureLayout*>(rInfo.ToData()->textureLayout));
    }
}

/**
 * Initializes an NVN sampler builder from a sampler description.
 *
 * @param pBuilder Builder to initialize.
 * @param rInfo Sampler description.
 */
void Nvn::ConvertToNvnSamplerBuilder(NVNsamplerBuilder* pBuilder, const SamplerInfo& rInfo) {
    nvnSamplerBuilderSetDefaults(pBuilder);
    nvnSamplerBuilderSetMinMagFilter(pBuilder, GetMinFilter(rInfo.GetFilterMode()),
                                     GetMagFilter(rInfo.GetFilterMode()));
    nvnSamplerBuilderSetWrapMode(pBuilder, GetWrapMode(rInfo.GetAddressU()),
                                 GetWrapMode(rInfo.GetAddressV()),
                                 GetWrapMode(rInfo.GetAddressW()));
    nvnSamplerBuilderSetLodClamp(pBuilder, rInfo.GetMinLod(), rInfo.GetMaxLod());
    nvnSamplerBuilderSetLodBias(pBuilder, rInfo.GetLodBias());
    nvnSamplerBuilderSetCompare(
        pBuilder,
        (rInfo.GetFilterMode() & FilterModeBit_Comparison) != 0 ? NVN_COMPARE_MODE_COMPARE_R_TO_TEXTURE
                                                                 : NVN_COMPARE_MODE_NONE,
        GetRComparisonFunction(rInfo.GetComparisonFunction()));
    nvnSamplerBuilderSetBorderColor(pBuilder, GetBorderColor(rInfo.GetBorderColorType()));

    if (rInfo.GetFilterMode() & FilterModeBit_Anisotropic) {
        nvnSamplerBuilderSetMaxAnisotropy(pBuilder, rInfo.GetMaxAnisotropy());
    }

    nvnSamplerBuilderSetReductionFilter(pBuilder, GetSamplerReduction(rInfo.GetFilterMode()));
}

/**
 * NVN debug layer callback.
 *
 * @param source Message source.
 * @param type Message type.
 * @param id Message id.
 * @param severity Message severity.
 * @param pMessage Message text.
 * @param pUserParam User parameter.
 */
void Nvn::DebugCallback(NVNdebugCallbackSource source, NVNdebugCallbackType type, int id,
                        NVNdebugCallbackSeverity severity, const char* pMessage,
                        void* pUserParam) {
    if (static_cast<uint32_t>(type) >= 4) {
        NN_UNEXPECTED_DEFAULT;
    }
}

/**
 * Returns the process-wide GLSLC library instance.
 *
 * @return GLSLC library instance.
 */
GlslcDll& GlslcDll::GetInstance() {
    static GlslcDll s_GlslcDll;

    return s_GlslcDll;
}

/**
 * Constructs a GLSLC library wrapper with no entry points loaded.
 */
GlslcDll::GlslcDll() {
    GlslcCompilePreSpecialized = nullptr;
    GlslcCompileSpecialized = nullptr;
    GlslcInitialize = nullptr;
    GlslcFinalize = nullptr;
    GlslcCompile = nullptr;
    GlslcGetVersion = nullptr;
    GlslcSetAllocator = nullptr;
    GlslcGetDefaultOptions = nullptr;
    m_hDll = nullptr;
}

}  // namespace nn::gfx::detail
