#pragma output nostreams
#pragma output nofileio

#include <spectrum.h>
#include <im2.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void di() __naked
{
#asm
    di
    ret
#endasm
}

void ei() __naked
{
#asm
    ei
    ret
#endasm
}

#define BANK(b) ((b) & 0x07)
#define SCREEN(s) (((s) & 0x01) << 3)
void bankm(char b) __z88dk_fastcall
{
#asm
    ld a, l
    and 0x0f
    add 0x10
    ld bc, 0x7ffd
    out (c), a
#endasm
}

char screen = 0;
M_BEGIN_ISR(isr)                    
{
    bankm(BANK(0) | SCREEN(screen = (screen + 1) % 2));
}
M_END_ISR

void initIM2() __z88dk_fastcall
{
    im2_Init(0xbc00);
    memset(0xbc00, 0xbd, 257);       // initialize 257-byte im2 vector table with all 0xb4 bytes
    bpoke(0xbdbd, 0xc3);              // POKE jump instruction at address 0xb4b4 (interrupt service routine entry)
    wpoke(0xbdbe, isr);  
}

void fillPalette() __z88dk_fastcall
{
    zx_border(INK_BLACK);
    bankm(BANK(5));
    memset(0xc000, 0, 0x1B00);
    for (int y = 0; y < 16; y++)
    {
        for (int pass = 0; pass < 2; pass++)
        {
            for (int x = 0; x < 8; x++)
            {
                * (char *)(0xd800 + ((y + 2) << 5) + (x << 1) + pass) = (pass << 6) | (x << 3);
            }
        }
    }
    bankm(BANK(7));
    memset(0xc000, 0, 0x1B00);
    for (int x = 0; x < 16; x++)
    {
        for (int pass = 0; pass < 2; pass++)
        {
            for (int y = 0; y < 8; y++)
            {
                * (char *)(0xd800 + (((y << 1) + 2 + pass) << 5) + x) = (pass << 6) | (y << 3);
            }
        }
    }
    bankm(BANK(0));
}

int main()
{
    di();
    fillPalette();
    initIM2();
    ei();

    while (1);

    return 0;
}
