#include "PR/os_internal.h"
#include "PR/rcp.h"
#include "PR/rdb.h"


extern OSThread *__osRunningThread;
extern u32 __osRdb_IP6_Empty;

#if BUILD_VERSION == VERSION_E
static u8 buffer[256];
OSThread __osThreadSave;
static s32 D_80007050 = 0;
static s32 numChars = 0;
static s32 D_80007058 = 0;

void func_80005D10(u32 arg0, u8* arg1) {
    arg1[0] = ((arg0 >> 0x18) & 0xFF);
    arg1[1] = ((arg0 >> 0x10) & 0xFF);
    arg1[2] = ((arg0 >> 8) & 0xFF);
    arg1[3] = (arg0 & 0xFF);
}

static u32 string_to_u32(u8* s) {
    u32 k;
    k = ((s[0] & 0xFF) << 0x18);
    k |= ((s[1] & 0xFF) << 0x10);
    k |= ((s[2] & 0xFF) << 0x8);
    k |= (s[3] & 0xFF);
    return k;
}

static void send_packet(u8 *s, s32 n)
{
    rdbPacket packet;
    s32 i;

    packet.type = 0x2;
    packet.length = n;

    for (i = 0; i < n; i++)
    {
        packet.buf[i] = s[i];
    }
    *(vu32 *)RDB_BASE_REG = *(u32 *)&packet;
    while (!(__osGetCause() & CAUSE_IP6))
    {
        ;
    }
    *(vu32 *)RDB_READ_INTR_REG = 0;
}

static void send(u8 *addr, s32 length)
{
    s32 sp24;
    s32 sp20;
    s32 sp1C;

    if (__osRdbWriteOK == 0)
    {
        while (!(__osGetCause() & CAUSE_IP6))
        {
            ;
        }
        *(vu32 *)RDB_READ_INTR_REG = 0;
        __osRdbWriteOK = 1;
    }

    sp1C = length % 3;
    sp20 = length - sp1C;
    for (sp24 = 0; sp24 < sp20; sp24 = sp24 + 3)
    {
        send_packet(&addr[sp24], 3);
    }
    if (sp1C > 0)
    {
        send_packet(&addr[sp20], sp1C);
    }
}

static void func_80005F30(void)
{
    u8 *addr;
    u32 length;

    addr = string_to_u32(&buffer[1]);
    length = string_to_u32(&buffer[5]);
    send(addr, length);
}

static void func_80005F7C(void)
{
    send((u8 *)&__osThreadSave.context, sizeof(__OSThreadContext));
}

void kdebugserver(s32 p)
{
    u32 i;
    rdbPacket packet;

    *(s32 *)&packet = p;

    for (i = 0; i < packet.length; i++)
    {
        buffer[numChars++] = packet.buf[i];
    }
    D_80007058 -= packet.length;
    switch (D_80007050)
    {
    case 0:
        switch (packet.buf[0])
        {
        case 1:
            D_80007050 = 1;
            D_80007058 = 9 - packet.length;
            return;
        case 2:
            func_80005F7C();
            D_80007050 = 0;
            numChars = 0;
            D_80007058 = 0;
            return;
        default:
            D_80007050 = 0;
            numChars = 0;
            D_80007058 = 0;
            return;
        }
        break;
    case 1:
        if (D_80007058 <= 0)
        {
            if (buffer[0] == 1)
            {
                func_80005F30();
                D_80007050 = 0;
                numChars = 0;
                D_80007058 = 0;
            }
            else
            {
                D_80007050 = 0;
                numChars = 0;
                D_80007058 = 0;
            }
        }
        break;
    default:
        D_80007050 = 0;
        numChars = 0;
        D_80007058 = 0;
        break;
    }
}

#else
#ifndef _FINALROM
OSThread __osThreadSave;
static u8 buffer[12];
static u32 numChars = 0;

static u32 string_to_u32(u8* s) {
    u32 k;

    k = ((s[0] & 0xFF) << 0x18);
    k |= ((s[1] & 0xFF) << 0x10);
    k |= ((s[2] & 0xFF) << 0x8);
    k |= (s[3] & 0xFF);

    return k;
}

static void send_packet(u8* s, u32 n) {
    rdbPacket packet;
    u32 i;

    packet.type = 0xC;
    packet.length = n;

    for (i = 0; i < n; i++) {
        packet.buf[i] = s[i];
    }
    *(vu32*)RDB_BASE_REG = *(u32*)&packet;
}

static void clear_IP6(void) {
    while (!(__osGetCause() & CAUSE_IP6)) {
        ;
    }
    *(vu32*)RDB_READ_INTR_REG = 0;

    while (__osGetCause() & CAUSE_IP6) {
        ;
    }
}

static void send(u8* s, u32 n) {
    u32 ct;
    u32 i = 0;
    u32 getLastIP6;

    if (!__osRdb_IP6_Empty) {
        clear_IP6();
        getLastIP6 = FALSE;
    } else {
        getLastIP6 = TRUE;
    }
    while (n != 0) {
        ct = (n < 3) ? n : 3;
        send_packet(s + i, ct);
        n -= ct;
        i += ct;
        if (n != 0) {
            clear_IP6();
        }
    }
    if (getLastIP6) {
        clear_IP6();
    }
}

void kdebugserver(rdbPacket packet) {
    u32 i;
    u32 length;
    u8* addr;

    for (i = 0; i < 3; i++) {
        buffer[numChars++] = packet.buf[i];
    }

    if (buffer[0] == 2) {
        send((char*)&__osRunningThread->context, sizeof(__OSThreadContext));
        numChars = 0;
    } else if (numChars >= 9 && buffer[0] == 1) {
        addr = string_to_u32(&buffer[1]);
        length = string_to_u32(&buffer[5]);
        send(addr, length);
        numChars = 0;
    }
}

#endif
#endif
