/*

    frisbee is xpld disk interface
    inspired by Disk ][
    (not)

*/

#ifndef FRISBEE_H
#define FRISBEE_H

#include <string>
#include "xpldMMU.h"

class xpldDiskInterface
{
private:

    xpldMMU* theMMU;

    std::string d0path;

    unsigned int diskLoadSaveFilenameAddress = 0;
    unsigned int memoryZoneSaveLowAddress = 0;
    unsigned int memoryZoneSaveHighAddress = 0;

public:

    xpldDiskInterface(xpldMMU* mmu,std::string disk0path);

    void executeCommand(unsigned char cmd);

    void setDiskLoadSaveFilenameAddress(unsigned int a);
    void setLoAddrSaveZone(unsigned int a);
    void setHiAddrSaveZone(unsigned int a);
};

#endif
