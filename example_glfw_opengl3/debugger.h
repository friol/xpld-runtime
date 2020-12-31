#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <vector>
#include <string>

// koobra CPU debugger/disasm

class debugger
{
private:

    std::string disasm(unsigned char* code, unsigned int& ipointer,unsigned int baseIP);

    unsigned char read8(unsigned char* code,unsigned int codepos);
    unsigned int read32(unsigned char* code,unsigned int codepos);

    std::string paddedHex(unsigned int n);

    std::string addBytecode(unsigned char* code, std::string c, int ipointer, int bytes);

public:

    debugger();

    std::vector<std::string> disasmCode(unsigned char* code, unsigned int numInstructions,unsigned int baseIP);


};

#endif
