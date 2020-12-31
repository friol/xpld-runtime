# XPLD - a virtual computer :computer:

![XPLD environment](https://github.com/friol/xpld-runtime/raw/master/xpld0.3.png)

XPLD is a fantasy computer created from scratch. The CPU, video chip, disk interface, etc. are all created by me.
Of course, XPLD is heavily inspired by home computers of my youth, like the Commodore family of 8 bit PCs, the Commodore Amiga and so on.

XPLD noteworthy components are:

- a 32 bit CPU called Koobra
- a videochip called Koolibri
- the disk interface, called Frisbee

### CPU (Koobra)

XPLD CPU is big endian<br/>
It has 16 general purpose registers, 32 bit: r0-r15<br/>
pc - program counter<br/>
sp - stack pointer<br/>
f - flags register<br/>
```
NVss DIZC
|||| ||||
|||| |||+- Carry
|||| ||+-- Zero
|||| |+--- Not Used
|||| +---- Not Used
||++------ Not Used
|+-------- Overflow
+--------- Negative
```
Instruction encoding for the CPU is very simple and unoptimized:<br/>
```
<opcode> [param1, param2, ...]
```
But we have terabytes of space, so who cares. Here is a list of the main opcodes. The syntax is similar to Intel assembly (the destination register comes before the source):

Instruction | Meaning | Opcode
------------ | ------------- | -------------
hlt | Halt CPU | 0x00
nop | No operation | 0x01
ld rx,immediate | Load immediate into register rx | 0x10
ld rx,ry | Load register ry into register rx | 0x11
ld rx,[memory address 32 bit] | load content of memory address into rx, 32 bit | 0x12
ld32 [memory address 32 bit],rx | load content of register rx into memory addres, 32 bit | 0x13
ld8 [memory address 32 bit],rx | load content of register rx into memory address, 8 bit| 0x14
ld32 [memory address 32 bit],immediate | load immediate into memory address, 32 bit| 0x15
ld8 [memory address 32 bit],immediate |load immediate into memory address, 8 bit| 0x16
ld8 [rx],ry |load content of register ry into memory address pointed by rx| 0x17
ld8 rx,[memory address 32 bit] | load content of memory address into rx, 8 bit | 0x18
ld rx,<address of data in datalabel> |load address of datalabel into rx | 0x19
ld8 rx,[ry] | load contents of memory address pointed by ry into rx, 8 bit | 0x1a
ld32 [rx],ry | load contents of register ry into rx, 32 bit | 0x1b
ld8 [rx],immediate | load immediate into address pointed by rx, 8 bit | 0x1c
ld32 rx,[ry] | load content of address pointed by ry into rx, 32 bit | 0x1d

