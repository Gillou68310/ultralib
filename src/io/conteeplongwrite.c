#include "PR/os_internal.h"
#include "PRinternal/controller.h"

s32 osEepromLongWrite(OSMesgQueue* mq, u8 address, u8* buffer, int length) {
    s32 ret = 0;

#if BUILD_VERSION == VERSION_E
    if (address >= 0x41) {
        return -1;
    }
#endif
    while (length > 0) {
        ERRCK(osEepromWrite(mq, address, buffer));
        length -= EEPROM_BLOCK_SIZE;
        address++;
        buffer += EEPROM_BLOCK_SIZE;
#if BUILD_VERSION == VERSION_E
        osSetTimer(&__osEepromTimer, (12000*osClockRate)/1000000, 0, &__osEepromTimerQ, &__osEepromTimerMsg);
#else
        osSetTimer(&__osEepromTimer, OS_CYCLES_TO_NSEC(12000), 0, &__osEepromTimerQ, &__osEepromTimerMsg);
#endif
        osRecvMesg(&__osEepromTimerQ, NULL, OS_MESG_BLOCK);
    }

    return ret;
}
