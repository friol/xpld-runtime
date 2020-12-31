# XPLD - a virtual computer :computer:

![XPLD environment](https://github.com/friol/xpld-runtime/raw/master/xpld0.3.png)

XPLD is a fantasy computer created from scratch. The CPU, video chip, disk interface, etc. are all created by me.
Of course, XPLD is heavily inspired by home computers of my youth, like the Commodore family of 8 bit PCs, the Commodore Amiga and so on.

XPLD noteworthy components are:

- a 32 bit CPU called Koobra
- a videochip called Koolibri
- the disk interface, called Frisbee

## CPU (Koobra)

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
and rx,immediate | and rx register with immediate value | 0x20
and rx,ry | and rx register with ry | 0x21
add rx,immediate | add immediate to rx | 0x30
add rx,ry | add ry to rx and store result in rx | 0x31
mul rx,immediate | mul register rx by immediate value | 0x32
mul rx,ry | mul register rx by ry | 0x33
sub rx,immediate | sub immediate from register rx | 0x40
sub rx,ry | sub register ry from rx | 0x41
push rx | push rx on stack | 0x50
pop rx | pop into rx from stack | 0x51
cmp rx,immediate | compare rx with immediate and set flags accordingly | 0x60
mod rx,immediate | execute modulo division of rx by immediate | 0x70
jmp label | direct jump to label's address | 0x80
jnz label | jump if not zero flag set | 0x81
jmp <address 32 bit> | direct jump to absolute address | 0x82
jz label | jump if zero flag set | 0x83
jsr subName | jump to subroutine subName | 0x90
rts | return from subroutine | 0x91
jsr <absolute address> | jump to subroutine identified by absolute address | 0x92
shr rx,immediate | shift register rx by immediate bits | 0xa0
shr rx,ry | shift register rx by ry bits | 0xa1
div rx,immediate | divide rx by immediate value | 0xb0

## Video chip (Koolibri)

