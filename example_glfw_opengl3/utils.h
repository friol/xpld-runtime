/* utility class */

#ifndef UTILS_H
#define UTILS_H

#include <string>

class xpldUtils
{
private:


public:

    xpldUtils();
    int loadXPLDbinary(std::string binaryFullPath, unsigned char* programArea, unsigned char* dataSegmentArea, unsigned int dsMaxSize);
};

#endif
