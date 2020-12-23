#ifndef XPLD_MMU
#define XPLD_MMU

#include "koolibri.h"

#define biosMemorySize 0x10000
#define dataSegmentMaxSize 0x1000
#define ramSize 0x100000
#define stackSize 0x10000

class xpldMMU
{
private:

    unsigned char bios[biosMemorySize];
    unsigned char dataSegment[dataSegmentMaxSize];
    unsigned char ram[ramSize];
    unsigned char stack[stackSize];

    int loadSystemBios();
    xpldVideochip* theVDU;

public:

    xpldMMU(xpldVideochip* vdu);
    unsigned char read8(unsigned int address);
    unsigned int read32(unsigned int address);

    void write8(unsigned int address, unsigned char val);
    void write32(unsigned int address, unsigned int val);

    unsigned char* getBiosPtr();
};

#endif
