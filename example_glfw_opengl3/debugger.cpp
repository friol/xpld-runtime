
// xpld 2020

#include <sstream>
#include <iomanip>
#include "debugger.h"

debugger::debugger()
{
    // hello.
}

unsigned char debugger::read8(unsigned char* code, unsigned int codepos)
{
    return code[codepos];
}

unsigned int debugger::read32(unsigned char* code, unsigned int codepos)
{
    unsigned char a = code[codepos];
    unsigned char b = code[codepos + 1];
    unsigned char c = code[codepos + 2];
    unsigned char d = code[codepos + 3];

    return a | (b << 8) | (c << 16) | (d << 24);
}

std::string debugger::paddedHex(unsigned int n)
{
    std::string res = "";
    std::stringstream stream;
    stream << "0x" << std::setfill('0') << std::setw(4) << std::hex << n;
    res += stream.str();

    return res;
}

std::string debugger::addBytecode(unsigned char* code,std::string c, int ipointer, int bytes)
{
    std::string bc = "";

    for (unsigned int i = 0;i < (40-c.size());i++)
    {
        bc += " ";
    }

    for (int cl = 0;cl < bytes;cl++)
    {
        std::stringstream stream;
        stream << std::setfill('0') << std::setw(2) << std::hex << (int)code[ipointer + cl];
        bc+= stream.str()+" ";
    }

    return bc;
}

std::string debugger::disasm(unsigned char* code, unsigned int& ipointer)
{
    std::string res = "";

    std::stringstream stream;
    stream << "0x" << std::setfill('0') << std::setw(4) << std::hex << ipointer;
    res+=stream.str();
    res += "    ";

    unsigned char opcode = code[ipointer];

    switch (opcode)
    {
        case 0x00:
        {
            // HLT
            res+="hlt";
            res += addBytecode(code, res, ipointer, 1);
            ipointer++;
            break;
        }
        case 0x01:
        {
            // NOP
            res += "nop";
            res += addBytecode(code, res, ipointer, 1);
            ipointer++;
            break;
        }
        case 0x10:
        {
            // ld r0,immediate
            unsigned char regNum = read8(code,ipointer + 1);
            unsigned int n32 = read32(code,ipointer + 2);
            res += "ld r" + std::to_string(regNum) + "," + paddedHex(n32);
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0x11:
        {
            // ld r0,r1
            unsigned char regDest = read8(code, ipointer + 1);
            unsigned char regSrc = read8(code, ipointer + 2);
            res += "ld r" + std::to_string(regDest) + ",r" + std::to_string(regSrc);
            res += addBytecode(code, res, ipointer, 3);
            ipointer += 3;
            break;
        }
        case 0x12:
        {
            // ld r0,[hexaddress] (load 32 bit value into reg from memory)
            unsigned char regDest = read8(code,ipointer+1);
            unsigned int addr32 = read32(code,ipointer+2);
            res += "ld r" + std::to_string(regDest) + ",[" + paddedHex(addr32) + "]";
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0x13:
        {
            // ld32 [hexaddress],rx (load 32 bit value from reg into memory)
            unsigned int addr32 = read32(code, ipointer + 1);
            unsigned char regSrc = read8(code, ipointer + 5);
            res += "ld32 [" + paddedHex(addr32) + "],r"+std::to_string(regSrc);
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0x14:
        {
            // ld8 [memory address 32 bit],r0
            unsigned int addr32 = read32(code, ipointer + 1);
            unsigned char regSrc = read8(code, ipointer + 5);
            res += "ld8 [" + paddedHex(addr32) + "],r" + std::to_string(regSrc);
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0x15:
        {
            // ld32 [memory address 32 bit],imm
            unsigned int addr32 = read32(code, ipointer + 1);
            unsigned int imm = read32(code, ipointer + 5);
            res += "ld32 [" + paddedHex(addr32) + "]," + paddedHex(imm);
            res += addBytecode(code, res, ipointer, 9);
            ipointer += 9;
            break;
        }
        case 0x16:
        {
            // ld8 [memory address 32 bit],immediate
            unsigned int addr32 = read32(code,(ipointer) + 1);
            unsigned char srcVal = read8(code,(ipointer) + 5);
            res += "ld8 [" + paddedHex(addr32) + "]," + std::to_string(srcVal);
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0x17:
        {
            // ld8 [rx],ry
            unsigned char indirectReg = read8(code, (ipointer)+1);
            unsigned char srcReg = read8(code, (ipointer)+2);
            res += "ld8 [r" + std::to_string(indirectReg) + "],r" + std::to_string(srcReg);
            res += addBytecode(code, res, ipointer, 3);
            ipointer += 3;
            break;
        }
        case 0x18:
        {
            // ld8 rx,[memory address 32 bit]
            unsigned char destReg= read8(code, (ipointer)+1);
            unsigned int addr32 = read32(code, (ipointer)+2);
            res += "ld8 r" + std::to_string(destReg) + ",[" + paddedHex(addr32)+"]";
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0x19:
        {
            // ld rx,<address>
            unsigned char destReg = read8(code, (ipointer)+1);
            unsigned int addr32 = read32(code, (ipointer)+2);
            res += "ld r" + std::to_string(destReg) + ",<" + paddedHex(addr32) + ">";
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0x1a:
        {
            // ld8 rx,[ry]
            unsigned char destReg = read8(code, (ipointer)+1);
            unsigned char indReg = read8(code, (ipointer)+2);
            res += "ld8 r" + std::to_string(destReg) + ",[r" + std::to_string(indReg) + "]";
            res += addBytecode(code, res, ipointer, 3);
            ipointer += 3;
            break;
        }
        case 0x1b:
        {
            // ld32 [rx],ry
            unsigned char indirectReg = read8(code, (ipointer)+1);
            unsigned char srcReg = read8(code, (ipointer)+2);
            res += "ld32 [r" + std::to_string(indirectReg) + "],r" + std::to_string(srcReg);
            res += addBytecode(code, res, ipointer, 3);
            ipointer += 3;
            break;
        }
        case 0x1c:
        {
            // ld8 [rx],immediate
            unsigned char indirectReg = read8(code, (ipointer)+1);
            unsigned int imm8 = read8(code, (ipointer)+2);
            res += "ld8 [r" + std::to_string(indirectReg) + "]," + std::to_string(imm8);
            res += addBytecode(code, res, ipointer, 3);
            ipointer += 3;
            break;
        }
        case 0x1d:
        {
            // ld32 rx,[ry]
            unsigned char destReg = read8(code, (ipointer)+1);
            unsigned char indReg = read8(code, (ipointer)+2);
            res += "ld32 r" + std::to_string(destReg) + ",[r" + std::to_string(indReg) + "]";
            res += addBytecode(code, res, ipointer, 3);
            ipointer += 3;
            break;
        }
        case 0x20:
        {
            // and r0,immediate
            unsigned char regNum = read8(code, ipointer + 1);
            unsigned int n32 = read32(code, ipointer + 2);
            res += "and r" + std::to_string(regNum) + "," + std::to_string(n32);
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0x30:
        {
            // add r0,immediate
            unsigned char regNum = read8(code, ipointer + 1);
            unsigned int n32 = read32(code, ipointer + 2);
            res += "add r" + std::to_string(regNum) + "," + std::to_string(n32);
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0x31:
        {
            // add rx,ry
            unsigned char regDst = read8(code, ipointer + 1);
            unsigned char regSrc = read8(code, ipointer + 2);
            res += "add r" + std::to_string(regDst) + ",r" + std::to_string(regSrc);
            res += addBytecode(code, res, ipointer, 3);
            ipointer += 3;
            break;
        }
        case 0x32:
        {
            // mul rx,immediate
            unsigned char regNum = read8(code, ipointer + 1);
            unsigned int n32 = read32(code, ipointer + 2);
            res += "mul r" + std::to_string(regNum) + "," + std::to_string(n32);
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0x33:
        {
            // mul rx,ry
            unsigned char regDst = read8(code, ipointer + 1);
            unsigned char regSrc = read8(code, ipointer + 2);
            res += "mul r" + std::to_string(regDst) + ",r" + std::to_string(regSrc);
            res += addBytecode(code, res, ipointer, 3);
            ipointer += 3;
            break;
        }
        case 0x40:
        {
            // sub r0,immediate
            unsigned char regNum = read8(code, ipointer + 1);
            unsigned int n32 = read32(code, ipointer + 2);
            res += "sub r" + std::to_string(regNum) + "," + std::to_string(n32);
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0x41:
        {
            // sub rx,ry
            unsigned char regNum = read8(code, ipointer + 1);
            unsigned char regNum2 = read8(code, ipointer + 2);
            res += "sub r" + std::to_string(regNum) + ",r" + std::to_string(regNum2);
            res += addBytecode(code, res, ipointer, 3);
            ipointer += 3;
            break;
        }
        case 0x50:
        {
            // push rx
            unsigned char regNum = read8(code, ipointer + 1);
            res += "push r" + std::to_string(regNum);
            res += addBytecode(code, res, ipointer, 2);
            ipointer += 2;
            break;
        }
        case 0x51:
        {
            // pop rx
            unsigned char regNum = read8(code, ipointer + 1);
            res += "pop r" + std::to_string(regNum);
            res += addBytecode(code, res, ipointer, 2);
            ipointer += 2;
            break;
        }
        case 0x60:
        {
            // cmp r0,immediate
            unsigned char regNum = read8(code, ipointer + 1);
            unsigned int n32 = read32(code, ipointer + 2);
            res += "cmp r" + std::to_string(regNum) + "," + std::to_string(n32);
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0x70:
        {
            // mod r0,immediate
            unsigned char regNum = read8(code, ipointer + 1);
            unsigned int n32 = read32(code, ipointer + 2);
            res += "mod r" + std::to_string(regNum) + "," + std::to_string(n32);
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0x71:
        {
            // mod r0,r1
            unsigned char regNum = read8(code, ipointer + 1);
            unsigned char regNum2 = read8(code, ipointer + 2);
            res += "mod r" + std::to_string(regNum) + ",r" + std::to_string(regNum2);
            res += addBytecode(code, res, ipointer, 3);
            ipointer += 3;
            break;
        }
        case 0x80:
        {
            // jmp 32-bit-address
            unsigned int addr32 = read32(code, ipointer + 1);
            res += "jmp " + paddedHex(addr32);
            res += addBytecode(code, res, ipointer, 5);
            ipointer += 5;
            break;
        }
        case 0x81:
        {
            // jnz 32-bit-address
            unsigned int addr32 = read32(code, ipointer + 1);
            res += "jnz " + paddedHex(addr32);
            res += addBytecode(code, res, ipointer, 5);
            ipointer += 5;
            break;
        }
        case 0x83:
        {
            // jz 32-bit-address
            unsigned int addr32 = read32(code, ipointer + 1);
            res += "jz " + paddedHex(addr32);
            res += addBytecode(code, res, ipointer, 5);
            ipointer += 5;
            break;
        }
        case 0x90:
        {
            // jsr subroutine address
            unsigned int addr32 = read32(code, ipointer + 1);
            res += "jsr " + paddedHex(addr32);
            res += addBytecode(code, res, ipointer, 5);
            ipointer += 5;
            break;
        }
        case 0x91:
        {
            // rts
            res += "rts";
            res += addBytecode(code, res, ipointer, 1);
            ipointer += 1;
            break;
        }
        case 0xa0:
        {
            // shr rx,imm
            unsigned char regNum = read8(code, ipointer + 1);
            unsigned int n32 = read32(code, ipointer + 2);
            res += "shr r" + std::to_string(regNum) + "," + std::to_string(n32);
            res += addBytecode(code, res, ipointer, 6);
            ipointer += 6;
            break;
        }
        case 0xa1:
        {
            // shr rx,ry
            unsigned char srcReg = read8(code, (ipointer)+1);
            unsigned char opReg = read8(code, (ipointer)+2);
            res += "shr r" + std::to_string(srcReg) + ",r" + std::to_string(opReg);
            res += addBytecode(code, res, ipointer, 3);
            ipointer += 3;
            break;
        }
        default:
        {
            res += "unk";
            res += addBytecode(code, res, ipointer, 1);
            ipointer++;
            break;
        }
    }

    return res;
}

std::vector<std::string> debugger::disasmCode(unsigned char* code, unsigned int numInstructions)
{
    std::vector<std::string> disasmed;
    unsigned int ip = 0;

    for (unsigned int i = 0;i < numInstructions;i++)
    {
        disasmed.push_back(disasm(code,ip));
    }

    return disasmed;
}
