
#include <string>
#include <cstdio>
#include <cassert>
#include <fstream>
#include <vector>
#include "xpldMMU.h"


xpldMMU::xpldMMU(xpldVideochip* vdu)
{
    memset(bios, 0, biosMemorySize);
    if (loadSystemBios() != 0)
    {
        throw("Error loading system BIOS");
    }

    theVDU = vdu;
}

unsigned char xpldMMU::read8(unsigned int address)
{
    if ((address >= 0x0) && (address <= 0xffff))
    {
        return bios[address];
    }
    else if ((address >= 0x10000) && (address <= (0x10000+ dataSegmentMaxSize)))
    {
        return dataSegment[address- 0x10000];
    }
    else if ((address >= 0x10000000) && (address <= 0x100004af))
    {
        return theVDU->readMode0Char(address);
    }
    else if ((address >= 0x11000000) && (address <= 0x11012bff))
    {
        return theVDU->readMode2Char(address);
    }
    else if ((address >= 0x00400000) && (address <= 0x0040ffff))
    {
        return stack[address - 0x00400000];
    }
    else if ((address >= 0x0500000) && (address <= 0x05fffff))
    {
        // RAM
        return ram[address - 0x0500000];
    }
    else if (address == 0x20000000)
    {
        // VDU status register
        return theVDU->getStatusRegister();
    }
    else if (address == 0x20010001)
    {
        // keyboard keypress read
        if (keyPressArray.size() == 0) return 0;
        int res = keyPressArray.front();
        keyPressArray.pop();
        return res;
    }
    else if (address == 0x20010010)
    {
        // VDU mode0 hw cursor x position
        return theVDU->getMode0hwcursorX();
    }
    else if (address == 0x20010011)
    {
        // VDU mode0 hw cursor y position
        return theVDU->getMode0hwcursorY();
    }
    else
    {
        //printf("Error: unhandled read8 from address %d", address);
    }

    return 0;
}

unsigned int xpldMMU::read32(unsigned int address)
{
    if (address == 0x20010000)
    {
        // clock register
        return theVDU->getInternalClock();
    }
    else
    {
        unsigned int a = read8(address);
        unsigned int b = read8(address + 1);
        unsigned int c = read8(address + 2);
        unsigned int d = read8(address + 3);

        return (a) | (b << 8) | (c << 16) | (d << 24);
    }
}

void xpldMMU::write8(unsigned int address, unsigned char val)
{
    if ((address >= 0x0500000) && (address <= 0x05fffff))
    {
        // RAM
        ram[address - 0x0500000] = val;
    }
    else if ((address >= 0x00400000) && (address <= 0x0040ffff))
    {
        stack[address - 0x00400000]=val;
    }
    else if ((address >= 0x10000000) && (address <= 0x100004af))
    {
        // mode0 videoram chars
        theVDU->writeMode0Char(address - 0x10000000, val);
    }
    else if ((address >= 0x10001000) && (address <= 0x100014af))
    {
        // mode0 videoram attributes
        theVDU->writeMode0Attr(address - 0x10001000, val);
    }
    else if ((address >= 0x11000000) && (address <= 0x11012bff))
    {
        // mode2 videoram write
        theVDU->writeMode2Char(address - 0x11000000, val);
    }
    else if (address == 0x20000001)
    {
        // set videomode register
        theVDU->setVideomode(val);
    }
    else if (address == 0x20000002)
    {
        // set palette entry
        theVDU->setMode2PaletteEntry(val);
    }
    else if (address == 0x20010010)
    {
        theVDU->setMode0hwcursorX(val);
    }
    else if (address == 0x20010011)
    {
        theVDU->setMode0hwcursorY(val);
    }
    else if ((address >= 0x20000100) && (address <= 0x20001018))
    {
        // sprite control - 16 sprites
        int sprnum = ((address & 0x1f00) >> 8)-1;
        assert((sprnum >= 0) && (sprnum < numSprites));
        if ((address & 0xff) == 0) theVDU->setSpriteAttribute(sprnum, "spriteAttributes", val);
        if ((address & 0xff) == 1) theVDU->setSpriteAttribute(sprnum, "dimx", val);
        if ((address & 0xff) == 2) theVDU->setSpriteAttribute(sprnum, "dimy", val);
        if ((address & 0xff) == 3) theVDU->setSpriteAttribute(sprnum, "rotation", val);
        if ((address & 0xff) == 0x12) theVDU->feedData8(sprnum,val);
        if ((address & 0xff) == 0x18) theVDU->setSpriteAttribute(sprnum, "fgcolor", val);
    }
}

void xpldMMU::write32(unsigned int address, unsigned int val)
{
    if ((address >= 0x00400000) && (address <= 0x0040ffff))
    {
        write8(address + 0, val & 0xff);
        write8(address + 1, (val>>8) & 0xff);
        write8(address + 2, (val>>16) & 0xff);
        write8(address + 3, (val>>24) & 0xff);
    }
    else if ((address >= 0x00500000) && (address <= 0x005fffff))
    {
        write8(address + 0, val & 0xff);
        write8(address + 1, (val >> 8) & 0xff);
        write8(address + 2, (val >> 16) & 0xff);
        write8(address + 3, (val >> 24) & 0xff);
    }
    else if ((address >= 0x20000100) && (address <= 0x20001018))
    {
        // sprite control - 16 sprites
        int sprnum = ((address & 0x1f00) >> 8)-1;
        assert((sprnum >= 0) && (sprnum < numSprites));
        if ((address & 0xff) == 4) theVDU->setSpriteAttribute(sprnum, "posx", val);
        if ((address & 0xff) == 8) theVDU->setSpriteAttribute(sprnum, "posy", val);
    }
    else if (address == 0x20000003)
    {
        // set palette color
        theVDU->setMode2PaletteColor(val);
    }
}

unsigned char* xpldMMU::getBiosPtr()
{
    return bios;
}

void xpldMMU::setKeyPressed(int k)
{
    keyPressArray.push(k);
}

int xpldMMU::loadSystemBios()
{
    std::string biosFilename = "D:\\prova\\xpld\\xpld.assembler\\bios.bin";

    std::ifstream file(biosFilename.c_str(), std::ios::binary | std::ios::ate);
    std::streamsize totalFileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // XPLD0002
    unsigned char header[8+1];
    file.read((char*)header, 8);
    header[8] = '\0';
    if (strcmp((const char*)header, "XPLD0002"))
    {
        throw("Invalid header in xpld bios.bin");
    }
    totalFileSize -= 8;

    // CODESIZE
    unsigned char codesizeHeader[8 + 1];
    file.read((char*)codesizeHeader, 8);
    codesizeHeader[8] = '\0';
    totalFileSize -= 8;

    uint32_t codeSize;
    file.read(reinterpret_cast<char*>(&codeSize), sizeof(codeSize));
    totalFileSize -= 4;

    // CODESEGM
    unsigned char codeSegm[8 + 1];
    file.read((char*)codeSegm, 8);
    codeSegm[8] = '\0';
    totalFileSize -= 8;

    if (file.read((char*)bios, codeSize))
    {
        // everything will be ok.

        totalFileSize -= codeSize;

        // DSBASEDR
        unsigned char dataSegmentBaseAddress[8 + 1];
        file.read((char*)dataSegmentBaseAddress, 8);
        dataSegmentBaseAddress[8] = '\0';
        totalFileSize -= 8;

        uint32_t dsBaseAddr;
        file.read(reinterpret_cast<char*>(&dsBaseAddr), sizeof(dsBaseAddr));
        totalFileSize -= 4;

        if (totalFileSize >= dataSegmentMaxSize)
        {
            throw("Data segment too big");
        }

        // DATASEGM
        unsigned char dataSegm[8 + 1];
        file.read((char*)dataSegm, 8);
        dataSegm[8] = '\0';
        totalFileSize -= 8;

        if (file.read((char*)dataSegment, totalFileSize))
        {
            return 0;
        }

        return 1;
    }

    return 1;
}
