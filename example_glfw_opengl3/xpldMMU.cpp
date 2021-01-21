
#include <string>
#include <cstdio>
#include <cassert>
#include <fstream>
#include <vector>

#include "xpldMMU.h"
#include "frisbee.h"
#include "utils.h"


xpldMMU::xpldMMU(xpldVideochip* vdu, xpldSoundChip* snd,std::string kernalPath)
{
    memset(bios, 0, biosMemorySize);
    if (loadSystemBios(kernalPath) != 0)
    {
        throw("Error loading system BIOS");
    }

    theVDU = vdu;
    theSoundChip = snd;
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
    else if ((address >= 0x00600000) && (address <= 0x0060ffff))
    {
        return programZone[address- 0x00600000];
    }
    else if ((address >= 0x00610000) && (address <= (0x00610000 + dataSegmentMaxSize)))
    {
        return programDataSegment[address - 0x00610000];
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
    if ((address >= 0x10000) && (address <= (0x10000+dataSegmentMaxSize)))
    {
        // RAM
        dataSegment[address - 0x10000] = val;
    }
    else if ((address >= 0x0500000) && (address <= 0x05fffff))
    {
        // RAM
        ram[address - 0x0500000] = val;
    }
    else if ((address >= 0x00600000) && (address <= 0x0060ffff))
    {
        programZone[address - 0x00600000]=val;
    }
    else if ((address >= 0x00610000) && (address <= (0x00610000 + dataSegmentMaxSize)))
    {
        programDataSegment[address - 0x00610000]=val;
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
    else if (address == 0x20020000)
    {
        // disk interface write
        theDisk->executeCommand(val);
    }
    else if ((address >= 0x30000000) && (address <= 0x3000002f))
    {
        int voiceNum = (address & 0x30) / 0x10;
        int baseAddr = address&0x0f;
        if (baseAddr==0)
        {
            // voice settings
            theSoundChip->setVoiceStatus(voiceNum, val);
        }
        else if ((baseAddr >= 0x1) && (baseAddr <= 0x4))
        {
            // ADSR
            if ((address % 5) == 1) theSoundChip->setVoiceAttack(voiceNum, val);
            else if ((address % 5) == 2) theSoundChip->setVoiceDecay(voiceNum, val);
            else if ((address % 5) == 3) theSoundChip->setVoiceSustain(voiceNum, val);
            else if ((address % 5) == 4) theSoundChip->setVoiceRelease(voiceNum, val);
        }
        else if (baseAddr == 0x6)
        {
            theSoundChip->setVoiceVolume(voiceNum, val);
        }
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
    else if (address == 0x20020001)
    {
        // disk interface set filename address for LOAD/SAVE
        theDisk->setDiskLoadSaveFilenameAddress(val);
    }
    else if (address == 0x20020002)
    {
        // low address of memory zone to save
        theDisk->setLoAddrSaveZone(val);
    }
    else if (address == 0x20020003)
    {
        // high address of memory zone to save
        theDisk->setHiAddrSaveZone(val);
    }
    else if ((address >= 0x30000000) && (address <= 0x3000002f))
    {
        int voiceNum = (address & 0x30) / 0x10;
        int baseAddr = address & 0x0f;
        if (baseAddr==5)
        {
            // voice frequency
            theSoundChip->setVoiceFrequency(voiceNum, val);
        }
        else if (baseAddr==7)
        {
            // voice duration
            theSoundChip->setVoiceDuration(voiceNum, val);
        }
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

void xpldMMU::setDisk(xpldDiskInterface* dsk)
{
    theDisk = dsk;
}

unsigned char* xpldMMU::getProgramArea()
{
    return programZone;
}

unsigned char* xpldMMU::getProgramDataSegment()
{
    return programDataSegment;
}

int xpldMMU::loadSystemBios(std::string kernalPath)
{
    xpldUtils xpldUtil;
    int retcode = xpldUtil.loadXPLDbinary(kernalPath, bios, dataSegment, dataSegmentMaxSize);
    return retcode;
}
