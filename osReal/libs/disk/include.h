#pragma once
#include "../ports/include.h"

#define STATUS_BSY 0x80
#define STATUS_RDY 0x40
#define STATUS_DRQ 0x08
#define STATUS_DF 0x20
#define STATUS_ERR 0x01

#define ATA_WAIT_RDY() while (!(inb(0x1F7) & STATUS_RDY))
#define ATA_WAIT_BSY() while (inb(0x1F7) & STATUS_BSY)
#define ATA_WAIT_DRQ() while (!(inb(0x1F7) & STATUS_DRQ))

#define NOP_DELAY(s)            \
    for (int n = 0; n < s; n++) \
    asm("nop")

uint32_t LBA = 0;
char sector_count = 1;

void print_bin(char a)
{
    for (int i = 0; i < 8; i++)
        tty_putInt((int)(!!((a << i) & 0x80)));
}

void ata_send_identify()
{
    /*
    1. select a target drive by sending 0xA0 to the "drive select" IO port => 0x1F6.
    2. Set the Sectorcount, LBAlo, LBAmid, and LBAhi IO ports to 0 (port 0x1F2 to 0x1F5).
    3. Send the IDENTIFY command (0xEC) to the Command IO port (0x1F7).
    4. Read the Status port (0x1F7) again. If the value read is 0, the drive does not exist.
    5. Poll the Status port (0x1F7) until bit 7 (BSY, value = 0x80) clears.
    6. Check the LBAmid and LBAhi ports (0x1F4 and 0x1F5) to see if they are non-zero. If so, the
    drive is not ATA, and you should stop polling.
    7. Otherwise, continue polling one of the Status ports until bit 3 (DRQ, value = 8) sets, or until bit 0 (ERR, value = 1) sets.
    8. At that point, if ERR is clear, the data is ready to read from the Data port (0x1F0).
    Read 256 16-bit values, and store them.
    */
    // ATA_WAIT_RDY();

    outb(0x1F6, 0xA0);

    outb(0x1F2, 0);
    outb(0x1F3, 0);
    outb(0x1F4, 0);
    outb(0x1F5, 0);

    outb(0x1F7, 0xEC);

    tty_putString("... Checking disk\n-- Waiting BSY\n");

    if (inb(0x1F7) == 0)
        tty_putString("Drive does not exist");

    ATA_WAIT_BSY();

    tty_putString("-- Waiting DRQ\n");

    ATA_WAIT_DRQ();

    tty_putString("-- Reading Drive Format Status\n");

    if (inb(0x1F4) || inb(0x1F5))
    {
        tty_putString("\nUnsupported drive format");
    }

    tty_putString("-- Waiting DRQ\n");

    tty_putString("... IDENT succesful\n");
}

void ata_read_sector(uint16_t *ptr)
{
    outb(0x1F6, 0xE0 | ((LBA >> 24) & 0xF));
    outb(0x1F2, sector_count);
    outb(0x1F3, (char)LBA);
    outb(0x1F4, (char)(LBA >> 8));
    outb(0x1F5, (char)(LBA >> 16));
    outb(0x1F7, 0x20); // Read

    int i = 0;
    while (i++ < 256)
    {
        ATA_WAIT_BSY();
        ATA_WAIT_DRQ();
        *ptr = inw(0x1F0);
        tty_putChar(*(ptr++) + 48);
    }
}

void ata_write_sector(uint16_t *src)
{
    ATA_WAIT_BSY();
    outb(0x1F6, 0xE0 | ((LBA >> 24) & 0xF));
    outb(0x1F2, sector_count);
    outb(0x1F3, (uint8_t)LBA);
    outb(0x1F4, (uint8_t)(LBA >> 8));
    outb(0x1F5, (uint8_t)(LBA >> 16));
    outb(0x1F7, 0x30); // Send the write command
    outb(0x1F7, 0x30);
    
    int i = 0;
    while (i++ < 256)
    {
        ATA_WAIT_BSY();
        ATA_WAIT_DRQ();
        outl(0x1F0, src[i]);
        NOP_DELAY(100);
    }
}