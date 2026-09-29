#ifndef NW_LMS_LMS_H
#define NW_LMS_LMS_H

/* LibMessageStudio (LMS) - NintendoWare message-resource runtime.
 * C API; symbols are unmangled in the binary. */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct LmsMessage {
    char _pad0[0x18];
    void *buffer;
} LmsMessage;

void *LMSi_Free(void *ptr);

void *LMS_CloseMessage(LmsMessage *msg);

#ifdef __cplusplus
}
#endif

#endif /* NW_LMS_LMS_H */
