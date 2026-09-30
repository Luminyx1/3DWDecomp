#include <filedevice/seadFileDeviceMgr.h>
#include <heap/seadHeap.h>
#include <heap/seadHeapMgr.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadBitUtil.h>
#include <prim/seadEndian.h>
#include <prim/seadPtrUtil.h>
#include <prim/seadSafeString.h>
#include <resource/seadSZSDecompressor.h>

namespace
{
#ifdef cafe
__attribute__((aligned(0x20))) s32 decodeSZSCafeAsm_(void* dst, const void* src)
{
    asm("lwz r5, 0x4(r4)\n");
    asm("li r11, 0x20\n");
    asm("li r6, 0\n");
    asm("mr r0, r5\n");
    asm("addi r4, r4, 0xf\n");
    asm("subi r3, r3, 1\n");
    asm("cmpwi r5, 0x132\n");
    asm("ble _final_decloop0\n");
    asm("subi r5, r5, 0x132\n");
    asm("nop\n");
    asm("nop\n");
    asm("nop\n");
    asm("nop\n");
    asm("nop\n");
    asm("nop\n");
    asm("nop\n");

    asm("_decloop0: rlwinm. r6, r6, 0x1f, 1, 0x1f\n");
    asm("bne _decloop1\n");
    asm("lbzu r7, 1(r4)\n");
    asm("li r6, 0x80\n");

    asm("_decloop1: and. r8, r6, r7\n");
    asm("lbzu r8, 1(r4)\n");
    asm("beq _decloop2\n");
    asm("andi. r9, r3, 0x1f\n");
    asm("bne _decloop1x\n");
    asm("dcbz r11, r3\n");

    asm("_decloop1x: subic. r5, r5, 1\n");
    asm("stbu r8, 1(r3)\n");
    asm("bne _decloop0\n");
    asm("b _decloop8\n");

    asm("_decloop2: lbzu r9, 1(r4)\n");
    asm("rlwinm. r10, r8, 0x1c, 4, 0x1f\n");
    asm("bne _decloop3\n");
    asm("lbzu r10, 1(r4)\n");
    asm("addi r10, r10, 0x10");

    asm("_decloop3: addi r10, r10, 2\n");
    asm("rlwimi r9, r8, 8, 0x14, 0x17\n");
    asm("subf r5, r10, r5\n");
    asm("subf r8, r9, r4\n");
    asm("mtspr CTR, r10\n");
    asm("addi r8, r8, 1\n");

    asm("_decloop4: andi. r9, r3, 0x1f\n");
    asm("lbz r9, -1(r8)\n");
    asm("addi r8, r8, 1\n");
    asm("bne _decloop5\n");
    asm("dcbz r11, r3\n");

    asm("_decloop5: stbu r9, 1(r3)\n");
    asm("bdnz _decloop4\n");
    asm("cmpwi r5, 0\n");
    asm("bgt _decloop0\n");

    asm("_decloop8: addi r5, r5, 0x132\n");
    asm("cmpwi r5, 0\n");
    asm("ble _final_decloop8\n");

    asm("_final_decloop0: rlwinm. r6, r6, 0x1f, 1, 0x1f\n");
    asm("bne _final_decloop1\n");
    asm("lbzu r7, 1(r4)\n");
    asm("li r6, 0x80\n");

    asm("_final_decloop1: and. r8, r6, r7\n");
    asm("lbzu r8, 1(r4)\n");
    asm("beq _final_decloop2\n");
    asm("subic. r5, r5, 1\n");
    asm("stbu r8, 1(r3)\n");
    asm("bne _final_decloop0\n");
    asm("b _final_decloop8\n");

    asm("_final_decloop2: lbzu r9, 1(r4)\n");
    asm("rlwinm. r10, r8, 0x1c, 4, 0x1f\n");
    asm("bne _final_decloop3\n");
    asm("lbzu r10, 1(r4)\n");
    asm("addi r10, r10, 0x10\n");

    asm("_final_decloop3: addi r10, r10, 2\n");
    asm("rlwimi r9, r8, 8, 0x14, 0x17\n");
    asm("subf. r5, r10, r5\n");
    asm("blt _final_decloop8\n");
    asm("subf r8, r9, r3\n");
    asm("mtspr CTR, r10\n");
    asm("addi r8, r8, 1\n");

    asm("_final_decloop4: lbz r9, -1(r8)\n");
    asm("addi r8, r8, 1\n");
    asm("stbu r9, 1(r3)\n");
    asm("bdnz _final_decloop4\n");
    asm("cmpwi r5, 0\n");
    asm("bgt _final_decloop0\n");

    s32 register error asm("r3");
    asm("_final_decloop8: mr %0, r0\n" : "=r"(error));
    asm("blr");

    return error;
}
#endif  // cafe
}  // namespace

#ifdef SWITCH
__attribute__((noinline)) s32 decodeSZSNxAsm64_(void* pDst, const void* pSrc)
{
    register s32 error asm("w2");
    asm volatile(
        "ldr w5, [%[pSrc],#4]\n"
        "rev w4, w5\n"
        "mov %w[error], w4\n"
        "add %[pSrc], %[pSrc], #0x10\n"
        "mov w5, #0\n"
        "mov w14, #0x1000\n"
        "sub w14, w14, #0x1\n"

        "1: lsr w5, w5, #1\n"
        "cbnz w5, 2f\n"
        "ldrb w6, [%[pSrc]],#1\n"
        "mov w5, #0x80\n"

        "2: tst w6, w5\n"
        "b.ne 5f\n"
        "ldrb w3, [%[pSrc]],#1\n"
        "ldrb w7, [%[pSrc]],#1\n"
        "add w3, w7, w3,lsl#8\n"
        "lsr w9, w3, #0xc\n"
        "and w3, w3, w14\n"
        "add w3, w3, #1\n"
        "add w7, w9, #2\n"
        "cbnz w9, 3f\n"
        "ldrb w7, [%[pSrc]],#1\n"
        "add w7, w7, #0x12\n"

        "3: sub w4, w4, w7\n"
        "neg w10, w3\n"

        "4: ldrb w12, [%[pDst],w10,sxtw]\n"
        "subs w7, w7, #1\n"
        "strb w12, [%[pDst]],#1\n"
        "b.ne 4b\n"
        "cbnz w4, 1b\n"
        "b 6f\n"

        "5: ldrb w12, [%[pSrc]],#1\n"
        "subs w4, w4, #1\n"
        "strb w12, [%[pDst]],#1\n"
        "b.ne 1b\n"

        "6:"
        : [error] "=r"(error)
        : [pDst] "r"(pDst), [pSrc] "r"(pSrc));
    return error;
}
#endif

namespace sead
{
// NON_MATCHING: the target stores the fields with a memset-style sequence
SZSDecompressor::DecompContext::DecompContext()
{
    initialize(NULL);
}

SZSDecompressor::DecompContext::DecompContext(void* pDst)
{
    initialize(pDst);
}

void SZSDecompressor::DecompContext::initialize(void* pDst)
{
    destp = static_cast<u8*>(pDst);
    destCount = 0;
    forceDestCount = 0;
    headerSize = 0x10;
    step = SZSDecompressor::cStepNormal;
    lzOffset = 0;
    packHigh = 0;
    flagMask = 0;
    flags = 0;
}

SZSDecompressor::SZSDecompressor(u32 workSize, u8* pWorkBuffer) : Decompressor("szs")
{
    if (pWorkBuffer == NULL)
    {
        mWorkSize = Mathu::roundUpPow2(workSize, FileDevice::cBufferMinAlignment);
        mWorkBuffer = NULL;
    }

    else
    {
        mWorkSize = workSize;
        mWorkBuffer = pWorkBuffer;
    }
}

/**
 * Reads an SZS file from its device and decompresses it, streaming when no whole-file buffer is
 * used.
 * @param rLoadArg Load parameters (path, device, heap, destination buffer and alignments).
 * @param pResource Resource the data is loaded for, used to query the required alignment.
 * @param pOutSize Receives the decompressed size; may be null.
 * @param pOutAllocSize Receives the size of the destination buffer; may be null.
 * @param pOutAllocated Receives whether the destination buffer was allocated here; may be null.
 * @return Decompressed data, or nullptr on failure.
 */
u8* SZSDecompressor::tryDecompFromDevice(const ResourceMgr::LoadArg& rLoadArg, Resource* pResource,
                                         u32* pOutSize, u32* pOutAllocSize, bool* pOutAllocated)
{
    Heap* heap = rLoadArg.load_data_heap;

    if (heap == nullptr)
    {
        heap = HeapMgr::sInstancePtr->getCurrentHeap();
    }

    if ((rLoadArg.load_data_buffer_alignment & 0x1f) != 0)
    {
        return nullptr;
    }

    FileHandle handle;
    FileDevice* device;
    u8* src;

    if (rLoadArg.device != nullptr)
    {
        device = rLoadArg.device->tryOpen(&handle, rLoadArg.path,
                                          FileDevice::cFileOpenFlag_ReadOnly, rLoadArg.div_size);
    }
    else
    {
        device = FileDeviceMgr::instance()->tryOpen(
            &handle, rLoadArg.path, FileDevice::cFileOpenFlag_ReadOnly, rLoadArg.div_size);
    }

    if (device == nullptr)
    {
        return nullptr;
    }

    src = mWorkBuffer;

    if (src == nullptr)
    {
        src = new (heap, -FileDevice::cBufferMinAlignment, std::nothrow) u8[mWorkSize];
    }

    if (src == nullptr)
    {
        return nullptr;
    }

    u32 bytesRead = 0;

    if (!handle.tryRead(&bytesRead, src, mWorkSize))
    {
        if (mWorkBuffer == nullptr)
        {
            delete[] src;
        }

        return nullptr;
    }

    if (bytesRead < 0x10)
    {
        if (mWorkBuffer == nullptr)
        {
            delete[] src;
        }

        return nullptr;
    }

    u32 decompSize = getDecompSize(src);
    s32 decompAlignment = getDecompAlignment(src);

    u32 bufferSize = rLoadArg.load_data_buffer_size;

    if (!(decompSize <= bufferSize || bufferSize == 0))
    {
        decompSize = bufferSize;
    }

    u32 allocSize;
    s32 bufferAlignment = rLoadArg.load_data_buffer_alignment;

    if (bufferAlignment != 0)
    {
        allocSize = (decompSize + bufferAlignment - 1) / bufferAlignment * bufferAlignment;
    }
    else
    {
        allocSize = Mathu::roundUpPow2(decompSize, 0x20);
    }

    u8* dst = rLoadArg.load_data_buffer;
    bool allocated = false;

    if (dst == nullptr)
    {
        s32 alignment;
        DirectResource* directResource = DynamicCast<DirectResource>(pResource);

        if (directResource != nullptr)
        {
            if (rLoadArg.load_data_alignment != 0)
            {
                alignment = Mathi::max(rLoadArg.load_data_alignment, 0x20);
            }
            else
            {
                if (decompAlignment == 0)
                {
                    decompAlignment = directResource->getLoadDataAlignment();
                }

                alignment =
                    (rLoadArg.instance_alignment >= 0 ? 1 : -1) * Mathi::max(decompAlignment, 0x20);
            }
        }
        else
        {
            alignment = (rLoadArg.instance_alignment >= 0 ? 1 : -1) * -0x20;
        }

        dst = new (heap, alignment, std::nothrow) u8[allocSize];

        if (dst == nullptr)
        {
            if (mWorkBuffer == nullptr)
            {
                delete[] src;
            }

            return nullptr;
        }

        allocated = true;
    }

    if (bytesRead < mWorkSize)
    {
        if (decomp(dst, allocSize, src, mWorkSize) < 0)
        {
            if (allocated)
            {
                delete[] dst;
            }

            if (mWorkBuffer == nullptr)
            {
                delete[] src;
            }

            return nullptr;
        }
    }
    else
    {
        DecompContext context;
        context.destp = dst;
        context.destCount = 0;
        context.forceDestCount = decompSize;
        context.headerSize = 0x10;
        context.step = cStepNormal;
        context.lzOffset = 0;
        context.packHigh = 0;
        context.flagMask = 0;
        context.flags = 0;

        while (bytesRead != 0)
        {
            s32 error = streamDecomp(&context, src, bytesRead);

            if (error == 0)
            {
                break;
            }

            if (error < 0 || !handle.tryRead(&bytesRead, src, mWorkSize))
            {
                if (allocated)
                {
                    delete[] dst;
                }

                if (mWorkBuffer == nullptr)
                {
                    delete[] src;
                }

                return nullptr;
            }
        }
    }

    if (mWorkBuffer == nullptr)
    {
        delete[] src;
    }

    if (pOutSize != nullptr)
    {
        *pOutSize = decompSize;
    }

    if (pOutAllocSize != nullptr)
    {
        *pOutAllocSize = allocSize;
    }

    if (pOutAllocated != nullptr)
    {
        *pOutAllocated = allocated;
    }

    return dst;
}

/**
 * Sets the size of each streamed read; ignored when an external work buffer is used.
 * @param workSize Read size, rounded up to 0x20 bytes.
 */
void SZSDecompressor::setWorkSize(u32 workSize)
{
    if (mWorkBuffer == nullptr)
    {
        mWorkSize = Mathu::roundUpPow2(workSize, FileDevice::cBufferMinAlignment);
    }
}

u32 SZSDecompressor::getDecompAlignment(const void* pSrc)
{
    return Endian::toHostU32(Endian::cBig, BitUtil::bitCastPtr<u32>(pSrc, 8));
}

u32 SZSDecompressor::getDecompSize(const void* pSrc)
{
    return Endian::toHostU32(Endian::cBig, BitUtil::bitCastPtr<u32>(pSrc, 4));
}

// NON_MATCHING: the jump table is carved into .rodata.str1.1 in the target
s32 SZSDecompressor::readHeader_(DecompContext* pContext, const u8* pSrc, u32 srcSize)
{
    s32 len = 0;

    while (pContext->headerSize != 0)
    {
        pContext->headerSize -= 1;

        if (pContext->headerSize == 0xF)
        {
            if (*pSrc != 0x59)
            {
                return -1;
            }
        }

        else if (pContext->headerSize == 0xE)
        {
            if (*pSrc != 0x61)
            {
                return -1;
            }
        }

        else if (pContext->headerSize == 0xD)
        {
            if (*pSrc != 0x7A)
            {
                return -1;
            }
        }

        else if (pContext->headerSize == 0xC)
        {
            if (*pSrc != 0x30)
            {
                return -1;
            }
        }

        else if (7 < pContext->headerSize)
        {
            pContext->destCount |= static_cast<u32>(*pSrc) << (pContext->headerSize - 8) * 8;
        }

        pSrc++;
        len += 1;

        if (--srcSize == 0 && pContext->headerSize != 0)
        {
            return len;
        }
    }

    if (pContext->forceDestCount < 1)
    {
        return len;
    }

    if (pContext->forceDestCount < pContext->destCount)
    {
        pContext->destCount = pContext->forceDestCount;
    }

    return len;
}

// NON_MATCHING: the jump table is carved into .rodata.str1.1 in the target
s32 SZSDecompressor::streamDecomp(DecompContext* pContext, const void* pSrc, u32 srcSize)
{
    const u8* src = static_cast<const u8*>(pSrc);

    if (pContext->headerSize != 0)
    {
        s32 len = readHeader_(pContext, src, srcSize);

        if (len < 0)
        {
            return len;
        }

        srcSize -= len;

        if (srcSize == 0)
        {
            if (pContext->headerSize == 0)
            {
                return pContext->destCount;
            }

            return -1;
        }

        src += len;
    }

    s32 destCount = pContext->destCount;
    Step step = pContext->step;
    u8* destp = pContext->destp;
    u32 lzOffset = pContext->lzOffset;
    u8 flagMask = pContext->flagMask;
    u8 flags = pContext->flags;
    u8 packHigh = pContext->packHigh;

    while (destCount > 0)
    {
        if (step == cStepLong)
        {
            u32 n = *src + 0x12;

            if (n > u32(destCount))
            {
                if (pContext->forceDestCount == 0)
                {
                    return -2;
                }

                n = destCount & 0xFFFF;
            }

            destCount -= n;
            do
            {
                *destp = *(destp - lzOffset);
                destp++;
            } while (--n != 0);
            step = cStepNormal;
        }
        else if (step == cStepShort)
        {
            lzOffset = (((packHigh << 8) & 0xf00) | *src) + 1;
            u32 n = packHigh >> 4;

            if (n != 0)
            {
                n += 2;

                if (n > u32(destCount))
                {
                    if (pContext->forceDestCount == 0)
                    {
                        return -2;
                    }

                    n = destCount & 0xFFFF;
                }

                destCount -= n;
                do
                {
                    *destp = *(destp - lzOffset);
                    destp++;
                } while (--n != 0);
                step = cStepNormal;
            }
            else
            {
                step = cStepLong;
            }
        }
        else
        {
            if (flagMask == 0)
            {
                flags = *src++;
                flagMask = 0x80;

                if (--srcSize == 0)
                {
                    break;
                }
            }

            if ((flags & flagMask) == 0)
            {
                packHigh = *src;
                step = cStepShort;
            }
            else
            {
                *destp++ = *src;
                destCount--;
            }

            flagMask >>= 1;
        }

        src++;

        if (--srcSize == 0)
        {
            break;
        }
    }

    pContext->destCount = destCount;
    pContext->step = step;
    pContext->destp = destp;
    pContext->lzOffset = lzOffset;
    pContext->flagMask = flagMask;
    pContext->flags = flags;
    pContext->packHigh = packHigh;

    if (destCount == 0 && pContext->forceDestCount == 0 && 0x20 < srcSize)
    {
        return -1;
    }

    return destCount;
}

/**
 * Decompresses a complete SZS buffer in one pass.
 * @param pDst Destination buffer.
 * @param dstSize Size of the destination buffer.
 * @param pSrc Compressed data starting with the Yaz0 header.
 * @return Decompressed size, -1 for a bad magic or -2 when the destination is too small.
 */
s32 SZSDecompressor::decomp(void* pDst, u32 dstSize, const void* pSrc, u32)
{
    u32 magic = Endian::toHostU32(Endian::cBig, BitUtil::bitCastPtr<u32>(pSrc));

    if (magic != 0x59617A30)
    {
        return -1;
    }

    u32 decompSize = getDecompSize(pSrc);

    if (dstSize < decompSize)
    {
        return -2;
    }

    decodeSZSNxAsm64_(pDst, pSrc);
    return decompSize;
}

}  // namespace sead
