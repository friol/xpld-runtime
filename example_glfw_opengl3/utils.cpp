
#include <string>
#include <cstdio>
#include <cassert>
#include <fstream>
#include <vector>

#include "utils.h"

xpldUtils::xpldUtils()
{
}

int xpldUtils::loadXPLDbinary(std::string binaryFullPath,unsigned char* programArea,unsigned char* dataSegmentArea, unsigned int dsMaxSize)
{
    std::ifstream file(binaryFullPath.c_str(), std::ios::binary | std::ios::ate);

    if (file.fail())
    {
        // file not found
        return 1;
    }

    std::streamsize totalFileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // XPLD0002
    unsigned char header[8 + 1];
    file.read((char*)header, 8);
    header[8] = '\0';
    if (strcmp((const char*)header, "XPLD0002"))
    {
        throw("Invalid header in xpld binary file "+binaryFullPath);
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

    if (file.read((char*)programArea, codeSize))
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

        if (totalFileSize >= dsMaxSize)
        {
            throw("Data segment too big in binary file");
        }

        // DATASEGM
        unsigned char dataSegm[8 + 1];
        file.read((char*)dataSegm, 8);
        dataSegm[8] = '\0';
        totalFileSize -= 8;

        if (file.read((char*)dataSegmentArea, totalFileSize))
        {
            return 0;
        }

        return 1;
    }

    return 1;
}
