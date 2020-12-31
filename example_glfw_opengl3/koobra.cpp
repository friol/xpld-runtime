//
// xpld - CPU called Koobra
//

#include "koobra.h"

xpldCPU::xpldCPU(xpldMMU* mmu)
{
    reset();
    theMmu = mmu;
}

void xpldCPU::reset()
{
    pc = 0;
    sp = 0x00400000;
    freg = 0;

    for (int i = 0;i < numGeneralPurposeRegisters;i++)
    {
        r[i] = 0;
    }
}

void xpldCPU::setZeroFlag(bool v)
{
    if (v) freg |= 0x02;
    else freg &= 0xfd;
}

unsigned int xpldCPU::getPC()
{
    return pc;
}

unsigned int xpldCPU::getSP()
{
    return sp;
}

unsigned int xpldCPU::getF()
{
    return freg;
}

unsigned int xpldCPU::getRegister(int n)
{
    return r[n];
}

std::string xpldCPU::getFlagsRegister()
{
    std::string retval = "";
    for (int b = 0;b < 8;b++)
    {
        retval += std::to_string((freg >> (7-b)) & 0x1);
    }

    return retval;
}

void xpldCPU::push32(unsigned int v)
{
    theMmu->write32(sp, v);
    sp += 4;
}

unsigned int xpldCPU::pop32()
{
    sp -= 4;
    return theMmu->read32(sp);
}

void xpldCPU::stepOne()
{
    if (isHalted) return;

    unsigned char opcode = theMmu->read8(pc);

    switch (opcode)
    {
        case 0x00:
        {
            // HLT
            isHalted = true;
            break;
        }
        case 0x01:
        {
            // NOP
            // do nothing.
            pc += 1;
            break;
        }
        case 0x10:
        {
            // ld r0,immediate
            unsigned char regNum = theMmu->read8(pc + 1);
            unsigned int n32 = theMmu->read32(pc + 2);
            r[regNum] = n32;
            pc += 6;
            break;
        }
        case 0x11:
        {
            // ld r0,r1
            unsigned char regDest = theMmu->read8(pc + 1);
            unsigned char regSrc = theMmu->read8(pc + 2);
            r[regDest] = r[regSrc];
            pc += 3;
            break;
        }
        case 0x12:
        {
            // ld r0,[hexaddress] (load 32 bit value into reg from memory)
            unsigned char regDest = theMmu->read8(pc + 1);
            unsigned int addr32 = theMmu->read32(pc + 2);
            r[regDest] = theMmu->read32(addr32);
            pc += 6;
            break;
        }
        case 0x13:
        {
            // ld32 [memory address 32 bit],r0
            unsigned int addr32 = theMmu->read32(pc + 1);
            unsigned char regSrc = theMmu->read8(pc + 5);
            theMmu->write32(addr32,r[regSrc]);
            pc += 6;
            break;
        }
        case 0x14:
        {
            // ld8 [memory address 32 bit],r0
            unsigned int addr32 = theMmu->read32(pc + 1);
            unsigned char regSrc = theMmu->read8(pc + 5);
            theMmu->write8(addr32, r[regSrc]&0xff);
            pc += 6;
            break;
        }
        case 0x15:
        {
            // ld32 [memory address 32 bit],immediate
            unsigned int addr32 = theMmu->read32(pc + 1);
            unsigned int imm = theMmu->read32(pc + 5);
            theMmu->write32(addr32, imm);
            pc += 9;
            break;
        }
        case 0x16:
        {
            // ld8 [memory address 32 bit],immediate
            unsigned int addr32 = theMmu->read32(pc + 1);
            unsigned char srcVal = theMmu->read8(pc + 5);
            theMmu->write8(addr32, srcVal);
            pc += 6;
            break;
        }
        case 0x17:
        {
            // ld8 [rx],ry
            unsigned char srcReg = theMmu->read8(pc + 1);
            unsigned char dstReg = theMmu->read8(pc + 2);
            theMmu->write8(r[srcReg], r[dstReg]);
            pc += 3;
            break;
        }
        case 0x18:
        {
            // ld8 rx,[addr32]
            unsigned char dstReg = theMmu->read8(pc + 1);
            unsigned int addr32 = theMmu->read32(pc + 2);
            r[dstReg]= theMmu->read8(addr32);
            pc += 6;
            break;
        }
        case 0x19:
        {
            // ld rx,<addr32>
            unsigned char dstReg = theMmu->read8(pc + 1);
            unsigned int addr32 = theMmu->read32(pc + 2);
            r[dstReg] = addr32;
            pc += 6;
            break;
        }
        case 0x1a:
        {
            // ld8 rx,[ry]
            unsigned char dstReg = theMmu->read8(pc + 1);
            unsigned char indReg = theMmu->read8(pc + 2);
            unsigned char rrr= theMmu->read8(r[indReg]);
            r[dstReg] = theMmu->read8(r[indReg]);
            pc += 3;
            break;
        }
        case 0x1b:
        {
            // ld32 [rx],ry
            unsigned char srcReg = theMmu->read8(pc + 1);
            unsigned char dstReg = theMmu->read8(pc + 2);
            theMmu->write32(r[srcReg], r[dstReg]);
            pc += 3;
            break;
        }
        case 0x1c:
        {
            // ld8 [rx],imm
            unsigned char dstReg = theMmu->read8(pc + 1);
            unsigned char imm = theMmu->read8(pc + 2);
            theMmu->write8(r[dstReg], imm);
            pc += 3;
            break;
        }
        case 0x1d:
        {
            // ld32 rx,[ry]
            unsigned char dstReg = theMmu->read8(pc + 1);
            unsigned char srcReg = theMmu->read8(pc + 2);
            r[dstReg] = theMmu->read32(r[srcReg]);
            pc += 3;
            break;
        }
        case 0x20:
        {
            // and r0,immediate
            unsigned char regSrc = theMmu->read8(pc + 1);
            unsigned int num32 = theMmu->read32(pc + 2);
            r[regSrc] &= num32;
            pc += 6;
            break;
        }
        case 0x30:
        {
            // add r0,immediate
            unsigned char regSrc = theMmu->read8(pc + 1);
            unsigned int num32 = theMmu->read32(pc + 2);
            r[regSrc] += num32;
            pc += 6;
            break;
        }
        case 0x31:
        {
            // add rx,ry
            unsigned char regDst = theMmu->read8(pc + 1);
            unsigned char regSrc = theMmu->read8(pc + 2);
            r[regDst] += r[regSrc];
            pc += 3;
            break;
        }
        case 0x32:
        {
            // mul rx,immediate
            unsigned char regSrc = theMmu->read8(pc + 1);
            unsigned int num32 = theMmu->read32(pc + 2);
            r[regSrc] *= num32;
            pc += 6;
            break;
        }
        case 0x33:
        {
            // mul rx,ry
            unsigned char regDst = theMmu->read8(pc + 1);
            unsigned char regSrc = theMmu->read8(pc + 2);
            r[regDst] *= r[regSrc];
            pc += 3;
            break;
        }
        case 0x40:
        {
            // sub r0,immediate
            unsigned char regSrc = theMmu->read8(pc + 1);
            unsigned int num32 = theMmu->read32(pc + 2);
            r[regSrc] -= num32;
            pc += 6;
            break;
        }
        case 0x41:
        {
            // sub rx,ry
            unsigned char regSrc = theMmu->read8(pc + 1);
            unsigned char regSrc2 = theMmu->read8(pc + 2);
            r[regSrc] -= r[regSrc2];
            pc += 3;
            break;
        }
        case 0x50:
        {
            // push rx
            unsigned char regSrc = theMmu->read8(pc + 1);
            push32(r[regSrc]);
            pc += 2;
            break;
        }
        case 0x51:
        {
            // pop rx
            unsigned char regSrc = theMmu->read8(pc + 1);
            r[regSrc]=pop32();
            pc += 2;
            break;
        }
        case 0x60:
        {
            // cmp r0,immediate
            unsigned char regSrc = theMmu->read8(pc + 1);
            unsigned int num32 = theMmu->read32(pc + 2);

            if (r[regSrc] == num32) setZeroFlag(true);
            else setZeroFlag(false);

            pc += 6;
            break;
        }
        case 0x61:
        {
            // cmp rx,ry
            unsigned char regSrc = theMmu->read8(pc + 1);
            unsigned char regSrc2 = theMmu->read8(pc + 2);
            if (r[regSrc] == r[regSrc2]) setZeroFlag(true);
            else setZeroFlag(false);
            pc += 3;
            break;
        }
        case 0x70:
        {
            // mod r0,immediate
            unsigned char regSrc = theMmu->read8(pc + 1);
            unsigned int num32 = theMmu->read32(pc + 2);
            r[regSrc] %= num32;
            pc += 6;
            break;
        }
        case 0x71:
        {
            // mod r0,r1
            unsigned char regSrc = theMmu->read8(pc + 1);
            unsigned char regSrc2 = theMmu->read8(pc + 2);
            r[regSrc] %= r[regSrc2];
            pc += 3;
            break;
        }
        case 0x80:
        {
            // jmp addr32
            unsigned int addr32 = theMmu->read32(pc + 1);
            pc = addr32;
            break;
        }
        case 0x81:
        {
            // jnz addr32
            unsigned int addr32 = theMmu->read32(pc + 1);
            if (!(freg & 0x02)) pc = addr32;
            else pc += 5;
            break;
        }
        case 0x82:
        {
            // jmp addr32
            unsigned int addr32 = theMmu->read32(pc + 1);
            pc = addr32;
            break;
        }
        case 0x83:
        {
            // jz addr32
            unsigned int addr32 = theMmu->read32(pc + 1);
            if (freg & 0x02) pc = addr32;
            else pc += 5;
            break;
        }
        case 0x90:
        {
            // jsr addr32
            unsigned int addr32 = theMmu->read32(pc + 1);
            // push next PC on stack
            push32(pc+5);
            pc = addr32;
            break;
        }
        case 0x91:
        {
            // rts
            unsigned int newPC = pop32();
            pc = newPC;
            break;
        }
        case 0x92:
        {
            // jsr absolute addr32
            unsigned int addr32 = theMmu->read32(pc + 1);
            push32(pc + 5);
            pc = addr32;
            break;
        }
        case 0xa0:
        {
            // shr rx,immediate
            unsigned char regSrc = theMmu->read8(pc + 1);
            unsigned int num32 = theMmu->read32(pc + 2);
            r[regSrc] >>= num32;
            pc += 6;
            break;
        }
        case 0xa1:
        {
            // shr rx,ry
            unsigned char regDst = theMmu->read8(pc + 1);
            unsigned char regSrc = theMmu->read8(pc + 2);
            r[regDst] >>= r[regSrc];
            pc += 3;
            break;
        }
        case 0xb0:
        {
            // div rx,immediate
            unsigned char regSrc = theMmu->read8(pc + 1);
            unsigned int num32 = theMmu->read32(pc + 2);
            r[regSrc]/= num32;
            pc += 6;
            break;
        }
        case 0xb1:
        {
            // div rx,ry
            unsigned char regDst = theMmu->read8(pc + 1);
            unsigned char regSrc = theMmu->read8(pc + 2);
            r[regDst] /= r[regSrc];
            pc += 3;
            break;
        }
        default:
        {
            throw("Koobra::Unknown instuction");
            pc += 1;
            break;
        }
    }
}
