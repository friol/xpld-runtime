/* Koobra CPU - xpld 2020 */

#ifndef KOOBRA_H
#define KOOBRA_H

#include "xpldMMU.h"
#include <string>

#define numGeneralPurposeRegisters 16

class xpldCPU
{
private:

    unsigned int pc;
    unsigned int sp;
    unsigned int freg;
    unsigned int r[numGeneralPurposeRegisters];

    unsigned long int cycles = 0;

    bool isHalted = false;

    xpldMMU* theMmu;

    void setZeroFlag(bool v);
    void push32(unsigned int v);
    unsigned int pop32();

public:

    xpldCPU(xpldMMU* mmu);

    unsigned int xpldCPU::getPC();
    unsigned int xpldCPU::getSP();
    unsigned int xpldCPU::getF();
    unsigned int xpldCPU::getRegister(int n);
    std::string getFlagsRegister();

    void stepOne();
    void reset();
};

#endif
