/* Koolibri - the Video chip - xpld 2020 */

#ifndef KOOLIBRI_H
#define KOOLIBRI_H

#include <string>
#include <vector>
#include "sprite.h"


enum koolibriVideomode { VIDEOMODE0_TEXT, VIDEOMODE1_320x240, VIDEOMODE2_320x240 };

#define videomode0numCols 40
#define videomode0numRows 30

#define mode0charDimx 20
#define mode0charDimy 20

#define mode0fontNumCharx 16
#define mode0fontNumChary 6

#define mode2dimx 320
#define mode2dimy 240

#define numSprites 16

class xpldVideochip
{
private:

    //int currentVideomode = VIDEOMODE0_TEXT;
    int currentVideomode = VIDEOMODE2_320x240;

    unsigned char videomode0vram[videomode0numCols * videomode0numRows];
    unsigned char videomode0attr[videomode0numCols * videomode0numRows];

    unsigned char videomode2vram[mode2dimx * mode2dimy];

    unsigned char* mode0bitmap;
    unsigned char* videomode0font;
    unsigned char* mode2bitmap;

    unsigned int mode0palette[16];
    unsigned int mode2palette[256];

    unsigned int internalClock = 0;
    unsigned int demultiplier = 0;
    const int multFactor = 4;
    int currentScanline = 0;

    const int lineBlankStart = 320;
    const int lineBlankEnd = lineBlankStart + 16;
    const int numScanlines = 240;

    int currentPaletteEntry = -1;

    bool vblank = false;
    bool lineBlank = false;

    unsigned char mode0hwcursorx = 0;
    unsigned char mode0hwcursory = 0;
    int hwcursorLuma = 0;

    std::vector<sprite> spriteList;

    void renderMode0Char(int charnum, int row, int col);

    void renderHwCursor();

public:

    xpldVideochip(std::string mode0FontPath);
    void loadMode0Font(std::string mode0FontPath);

    int getCurModeBitmapWidth();
    int getCurModeBitmapHeight();
    unsigned char* getCurModeBitmap();

    void writeMode0Char(unsigned long int addr, unsigned char c);
    void writeMode0Attr(unsigned long int addr, unsigned char c);

    unsigned char readMode0Char(unsigned int address);

    unsigned char readMode2Char(unsigned int address);
    void writeMode2Char(unsigned long int addr, unsigned char c);

    int getRasterLine();
    unsigned char getStatusRegister(); // 0x20000000
    void setVideomode(unsigned char v);

    void setSpriteAttribute(int sprnum,std::string attribName, int attribVal);
    void feedData8(int sprnum, unsigned char val);

    unsigned int getMode0Palette(int e);

    void setMode2PaletteEntry(unsigned char p);
    void setMode2PaletteColor(unsigned int c);

    unsigned int getInternalClock();

    void setMode0hwcursorX(unsigned char x);
    void setMode0hwcursorY(unsigned char y);

    int getMode0hwcursorX();
    int getMode0hwcursorY();

    void reset();

    void stepOne();
    void renderFull();

    ~xpldVideochip();
};

#endif
