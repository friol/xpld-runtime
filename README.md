# XPLD - a virtual computer

![XPLD environment](https://github.com/friol/xpld-runtime/raw/master/xpld0.3.png)

XPLD is a fantasy computer created from scratch. The CPU, video chip, disk interface, etc. are all created by me.
Of course, XPLD is heavily inspired by home computers of my youth, like the Commodore family of 8 bit PCs, the Commodore Amiga and so on.

XPLD noteworthy components are:

- a 32 bit CPU called Koobra
- a videochip called Koolibri
- the disk interface, called Frisbee

### CPU (Koobra)

XPLD CPU is big endian
16 general purpose registers, 32 bit: r0-r15
pc - program counter
sp - stack pointer
f - flags register

NVss DIZC
|||| ||||
|||| |||+- Carry
|||| ||+-- Zero
|||| |+--- Not Used
|||| +---- Not Used
||++------ Not Used
|+-------- Overflow
+--------- Negative

