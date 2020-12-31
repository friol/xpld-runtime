# XPLD - a virtual computer :computer:

![XPLD environment](https://github.com/friol/xpld-runtime/raw/master/xpld0.3.png)

XPLD is a fantasy computer created from scratch. The CPU, video chip, disk interface, etc. are all invented.<br/>
Of course, XPLD is heavily inspired by home computers of my youth, like the Commodore family of 8 bit PCs, the Commodore Amiga and so on.

XPLD noteworthy components are:

- a 32 bit CPU called <b>Koobra</b>
- a videochip called <b>Koolibri</b>
- the disk interface, called <b>Frisbee</b>

This repository contains the XPLD runtime, which compiles with Visual Studio Community 2019. Before running XPLD, you probably have to edit the settings.json file, to point to the right paths for d0:, the system font and the kernal binary.<br/><br/>
The gui of XPLD is made with [imgui](https://github.com/ocornut/imgui "imgui").

## CPU (Koobra)

XPLD CPU is big endian. It has 16 general purpose registers, 32 bit wide: r0-r15.<br/><br/>
Also, you've got:<br/>
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
but we have terabytes of space, so who cares.<br/>
Down below you can find a list of the main opcodes. The syntax is similar to Intel assembly (the destination register comes before the source):

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
  
XPLD binaries (even the machine's kernal) are created with xpld-assembler, which you can find in [this project](https://github.com/friol/xpld-assembler), along with some examples.

## Memory map

This is XPLD's memory map:

```
0x00000000-0x0000ffff - kernal, execution starts from 0x0
0x00400000-0x0040ffff - stack
0x00500000-0x005fffff - RAM (1Mb)
0x00600000-0x00610fff - program load zone+data segment
0x10000000-0x100004af - mode 0, videoram, 8 bit chars
0x10001000-0x100014af - mode 0, videoram, 16bg+16fg colours
0x11000000-0x11012bff - mode 2, videoram, 8 bit palettized chars
0x20000000-0x20000fff - koolibri registers
0x20000000            - koolibri status register (8 bit) -> xxxxxxVL (vblank lineblank)
0x20000001            - set videomode register
0x20000002            - set palette entry videomode 2
0x20000003            - set palette value videomode 2 (ABGR)

0x20000100            - sprite 0 attributes: xxxxxxbe (b=0 monochrome, 1 32-bit) (e=enabled 0/1)
0x20000101            - sprite 0 dimx // must be multiple of 8, max 248
0x20000102            - sprite 0 dimy // max 248
0x20000103            - sprite 0 rotation/256*2PI
0x20000104            - sprite 0 posx (4 bytes)
0x20000108            - sprite 0 posy (4 bytes)
0x20000112            - sprite 0 data8 (for monochrome) (8 bits)
0x20000114            - sprite 0 data32 (for 32-bit)
0x20000118            - sprite 0 foreground color (background is transparent)
...

0x20000200            - sprite 1 attributes
...


0x20010000-0x2001ffff - misc registers (keyboard, etc.)
0x20010000            - clock register 32 bit (read only)
0x20010001            - read byte: get key pressed
0x20010002            - read byte: get key pressed modifier (shift, alt, control, etc.)
0x20010010            - mode 0 hw cursor x coordinate
0x20010011            - mode 0 hw cursor y coordinate

0x20020000            - d0: interface - write 8 bit
                        cmd=1: directory of current disk content
                        cmd=2: load program to memory - 0x20020001 is a string zero terminated with filename
0x20020001            - d0: interface - write 32 bit
```

## Video chip (Koolibri)

Koolibri has the following videomodes:

0. 40x30 rows text mode, commodore style, initial mode on boot. You have 16 colours for the foreground and 16 for the background, from a fixed palette
1. 320x240 graphics mode, linear framebuffer, 16M colours
2. 320x240, 256 colours with palette from 16M

Koolibri starts in mode 0, entire videoram is cleared with 0x00 char on reset, with 0x54 as fg/bg colours.
As you can see from the memory map, Koolibri has Vblank and hblank flags that you can read in code for sync and raster effects.

Koolibri has 16 sprites, with the following attributes:
- sprite attributes: monochrome/32bit, enabled/disabled (visibility)
- dimx (multiple of 8)
- dimy
- rotation degrees
- posx
- posy
- sprite data

## Disk interface (Frisbee)

Registers at 0x20020000 and 0x20020001 control the disk interface.
At the moment, register 0x20020000 has 2 commands: 

1. cmd=1: directory of current disk content
2. cmd=2: load program to memory - 0x20020001 is a string zero terminated with filename
