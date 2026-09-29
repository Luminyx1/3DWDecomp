#include "nw/lms/lms.h"

void *LMS_CloseMessage(LmsMessage *msg) {
    if (msg->buffer) {
        LMSi_Free(msg->buffer);
    }
    return LMSi_Free(msg);
}
