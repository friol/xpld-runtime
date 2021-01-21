
/* xpld - disk interface */

#include <iostream>
#include <filesystem>
#include "frisbee.h"
#include "utils.h"

xpldDiskInterface::xpldDiskInterface(xpldMMU* mmu, std::string disk0path)
{
    theMMU = mmu;
    d0path = disk0path;
}

void xpldDiskInterface::executeCommand(unsigned char cmd)
{
    if (cmd == 1)
    {
        // directory

        typedef struct fsEntry
        {
            std::string name;
            int size;
        };

        std::vector<fsEntry> listOfFiles;

        const std::filesystem::path pth(d0path);
        for (const auto& entry : std::filesystem::directory_iterator(pth))
        {
            //std::cout << entry.path() << std::endl;
            std::filesystem::path fname = entry.path().filename();
            std::string fonly=fname.u8string();

            if (fonly.find(".bin")!=std::string::npos)
            {
                fsEntry e;
                e.name = fonly;
                e.size = entry.file_size();
                listOfFiles.push_back(e);
            }
        }

        // write directory data to XPLD working RAM

        int baseAddr = 0x00500000;
        theMMU->write8(baseAddr, listOfFiles.size()&0xff);
        baseAddr++;

        for (int f = 0;f < listOfFiles.size();f++)
        {
            int blocks = listOfFiles[f].size / 256;
            if (blocks == 0) blocks = 1;
            if (blocks > 255) blocks = 255;

            theMMU->write8(baseAddr, blocks);
            baseAddr++;

            for (int c = 0;c < listOfFiles[f].name.size();c++)
            {
                theMMU->write8(baseAddr, listOfFiles[f].name[c]);
                baseAddr++;
            }
            theMMU->write8(baseAddr, 0);
            baseAddr++;
        }
    }
    else if (cmd == 2)
    {
        if (diskLoadSaveFilenameAddress == 0)
        {
            throw("Error: trying to load a file without setting the filename address");
        }

        // load file to internal memory
        // retrieve file name

        unsigned char fname[256];
        memset(fname, 0, 256);

        int memPos = 0;
        bool zeroFound = false;
        unsigned int memPtr = diskLoadSaveFilenameAddress;
        while ((!zeroFound)&&(memPos<40))
        {
            unsigned char curchar = theMMU->read8(memPtr);
            fname[memPos] = curchar+32;

            if (curchar == '\0') zeroFound = true;
            else
            {
                memPos += 1;
                memPtr += 1;
            }
        }

        if (!zeroFound)
        {
            throw("Exception: disk load filename address doesn't point to a zero terminated string");
        }

        std::string strFilename((char*)fname);
        std::string fullFileName = d0path + "\\"+ strFilename;

        xpldUtils xpldUtil;
        unsigned char* programPtr = theMMU->getProgramArea();
        unsigned char* programDS = theMMU->getProgramDataSegment();
        int retcode = xpldUtil.loadXPLDbinary(fullFileName, programPtr, programDS, dataSegmentMaxSize);
        if (retcode != 0)
        {
            throw("Error loading binary file");
        }
    }
    else if (cmd == 3)
    {
        // save memory zone to disk
        // retrieve file name
        unsigned char fname[256];
        memset(fname, 0, 256);

        int memPos = 0;
        bool zeroFound = false;
        unsigned int memPtr = diskLoadSaveFilenameAddress;
        while ((!zeroFound) && (memPos < 40))
        {
            unsigned char curchar = theMMU->read8(memPtr);
            fname[memPos] = curchar + 32;

            if (curchar == '\0') zeroFound = true;
            else
            {
                memPos += 1;
                memPtr += 1;
            }
        }

        if (!zeroFound)
        {
            throw("Exception: disk load filename address doesn't point to a zero terminated string");
        }

        std::string strFilename((char*)fname);
        std::string fullFileName = d0path + "\\" + strFilename;

        //

        FILE* fout = fopen(fullFileName.c_str(), "wb");
        unsigned int workingAddr = memoryZoneSaveLowAddress;
        for (int c = 0;c < (memoryZoneSaveHighAddress - memoryZoneSaveLowAddress);c++)
        {
            unsigned char b = theMMU->read8(workingAddr);
            fwrite(&b, 1, 1, fout);
            workingAddr++;
        }
        fclose(fout);
    }
    else if (cmd == 4)
    {
        // load memory zone from disk
        // retrieve file name
        unsigned char fname[256];
        memset(fname, 0, 256);

        int memPos = 0;
        bool zeroFound = false;
        unsigned int memPtr = diskLoadSaveFilenameAddress;
        while ((!zeroFound) && (memPos < 40))
        {
            unsigned char curchar = theMMU->read8(memPtr);
            fname[memPos] = curchar + 32;

            if (curchar == '\0') zeroFound = true;
            else
            {
                memPos += 1;
                memPtr += 1;
            }
        }

        if (!zeroFound)
        {
            throw("Exception: disk load filename address doesn't point to a zero terminated string");
        }

        std::string strFilename((char*)fname);
        std::string fullFileName = d0path + "\\" + strFilename;

        //

        FILE* fin = fopen(fullFileName.c_str(), "rb");
        unsigned int workingAddr = memoryZoneSaveLowAddress;
        for (int c = 0;c < (memoryZoneSaveHighAddress - memoryZoneSaveLowAddress);c++)
        {
            unsigned char b;
            fread(&b, 1, 1, fin);
            theMMU->write8(workingAddr, b);
            workingAddr++;
        }
        fclose(fin);
    }
}

void xpldDiskInterface::setDiskLoadSaveFilenameAddress(unsigned int a)
{
    diskLoadSaveFilenameAddress = a;
}

void xpldDiskInterface::setLoAddrSaveZone(unsigned int a)
{
    memoryZoneSaveLowAddress = a;
}

void xpldDiskInterface::setHiAddrSaveZone(unsigned int a)
{
    memoryZoneSaveHighAddress = a;
}
