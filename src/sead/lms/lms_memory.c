#include "lms/lms.h"

static LMSMallocPtr sMalloc;
static LMSFreePtr sFree;

/**
 * Sets the allocator LibMessageStudio uses for all of its allocations.
 * @param pMallocFunc allocation function
 * @param pFreeFunc deallocation function
 */
void LMS_SetMemFuncs(LMSMallocPtr pMallocFunc, LMSFreePtr pFreeFunc) {
    sMalloc = pMallocFunc;
    sFree = pFreeFunc;
}

/**
 * Allocates memory through the allocator set by LMS_SetMemFuncs.
 * @param size number of bytes
 */
void* LMSi_Malloc(size_t size) {
    return sMalloc(size);
}

/**
 * Frees memory through the deallocator set by LMS_SetMemFuncs.
 * @param pPtr memory to free
 */
void LMSi_Free(void* pPtr) {
    sFree(pPtr);
}

/**
 * Compares two byte ranges.
 * @param pA first byte range
 * @param pB second byte range
 * @param size number of bytes
 * @return 1 if the first size bytes are equal, 0 otherwise (not memcmp semantics).
 */
int LMSi_MemCmp(const char* pA, const char* pB, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (pA[i] != pB[i]) {
            return 0;
        }
    }
    return 1;
}

/**
 * Copies size bytes from pSrc to pDst, one byte at a time.
 * @param pDst destination
 * @param pSrc source
 * @param size number of bytes
 */
void LMSi_MemCopy(void* pDst, const void* pSrc, int size) {
    int i;
    for (i = 0; i < size; i++) {
        ((char*)pDst)[i] = ((const char*)pSrc)[i];
    }
}
