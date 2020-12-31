# XPLD - a virtual computer

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


