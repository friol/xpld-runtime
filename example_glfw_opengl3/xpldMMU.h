#ifndef XPLD_MMU
#define XPLD_MMU

#include <queue>
#include "koolibri.h"
#include "goorilla.h"

class xpldDiskInterface;

#define biosMemorySize 0x10000
#define dataSegmentMaxSize 0x1000
#define ramSize 0x100000
#define stackSize 0x10000

#define programZoneSize 0x10000

class xpldMMU
{
private:

    unsigned char bios[biosMemorySize];
    unsigned char dataSegment[dataSegmentMaxSize];
    unsigned char ram[ramSize];
    unsigned char stack[stackSize];
    unsigned char programZone[programZoneSize];
    unsigned char programDataSegment[dataSegmentMaxSize];

    xpldVideochip* theVDU;
    xpldDiskInterface* theDisk;
    xpldSoundChip* theSoundChip;

    int loadSystemBios(std::string kernalPath);

    std::queue<int> keyPressArray;

public:

    xpldMMU(xpldVideochip* vdu,xpldSoundChip* snd,std::string kernalPath);
    unsigned char read8(unsigned int address);
    unsigned int read32(unsigned int address);

    void write8(unsigned int address, unsigned char val);
    void write32(unsigned int address, unsigned int val);

    unsigned char* getBiosPtr();

    void setDisk(xpldDiskInterface* dsk);

    unsigned char* getProgramArea();
    unsigned char* getProgramDataSegment();

    void setKeyPressed(int k);
};

#endif
