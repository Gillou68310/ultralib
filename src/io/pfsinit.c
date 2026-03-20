#include "PR/os_internal.h"
#include "PRinternal/controller.h"
#include "PRinternal/siint.h"

s32 osPfsInit(OSMesgQueue* queue, OSPfs* pfs, int channel) {
    s32 ret = 0;

    __osSiGetAccess();
    ret = __osPfsGetStatus(queue, channel);
#if BUILD_VERSION == VERSION_E
    __osContLastCmd = CONT_CMD_RESET;
#endif
    __osSiRelAccess();

    if (ret != 0) {
        return ret;
    }

    pfs->queue = queue;
    pfs->channel = channel;
    pfs->status = 0;
#if BUILD_VERSION > VERSION_E
    pfs->activebank = -1;
#endif
    ERRCK(__osGetId(pfs));

    ret = osPfsChecker(pfs);
    pfs->status |= PFS_INITIALIZED;
    return ret;
}

#if BUILD_VERSION == VERSION_E
s32 __osPfsGetStatus(OSMesgQueue* queue, int channel) {
    s32 ret;
    OSMesg dummy;
    u8 bitpattern;
    OSContStatus contData[MAXCONTROLLERS];

    ret = 0;
    __osPfsRequestData(0xFF);
    ret = __osSiRawStartDma(OS_WRITE, &__osPfsPifRam);
    osRecvMesg(queue, &dummy, OS_MESG_BLOCK);
    ret = __osSiRawStartDma(OS_READ, &__osPfsPifRam);
    osRecvMesg(queue, &dummy, OS_MESG_BLOCK);
    __osPfsGetInitData(&bitpattern, &contData[0]);
    if (((contData[channel].status & 1) != 0) && (contData[channel].status & 2)) {
        return PFS_ERR_NEW_PACK;
    }
    if ((contData[channel].errno != 0) ||((contData[channel].status & 1) == 0)) {
        return PFS_ERR_NOPACK;
    }
    else if ((contData[channel].status & CONT_ADDR_CRC_ER) != 0) {
        return PFS_ERR_CONTRFAIL;
    }
    return ret;
}
#endif
