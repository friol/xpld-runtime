/* the koolibri floats. Float koolibri, float */

#include <string>
#include <cstdlib>
#include <math.h>

#include "koolibri.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


xpldVideochip::xpldVideochip(std::string mode0FontPath)
{
    // init mode 0 videoram
    for (int b = 0;b < videomode0numCols * videomode0numRows;b++)
    {
        videomode0vram[b] = 0;
        videomode0attr[b] = 0x54;
    }

    // init mode 2 videoram to zero
    for (int b = 0;b < mode2dimx * mode2dimy;b++)
    {
        videomode2vram[b] = 0;
    }

    // init mode 0 palette
    mode0palette[0] = 0x0;
    mode0palette[1] = 0x626262;
    mode0palette[2] = 0x898989;
    mode0palette[3] = 0xadadad;
    mode0palette[4] = 0xffffff;
    mode0palette[5] = 0x9f4e44;
    mode0palette[6] = 0xcb7e75;
    mode0palette[7] = 0x6d5412;
    mode0palette[8] = 0xa1683c;
    mode0palette[9] = 0xc9d487;
    mode0palette[10] = 0x9ae29b;
    mode0palette[11] = 0x5cab5e;
    mode0palette[12] = 0x6abfc6;
    mode0palette[13] = 0x887ecb;
    mode0palette[14] = 0x50459b;
    mode0palette[15] = 0xa057a3;

    // mode2 palette is black
    for (int i = 0; i < 256; ++i)
    {
        mode2palette[i] = 0xff000000;
    }

    loadMode0Font(mode0FontPath);

    // allocate mode0 bitmap
    mode0bitmap = new unsigned char[videomode0numCols * videomode0numRows * mode0charDimx * mode0charDimy * 4];
    memset(mode0bitmap, 0, videomode0numCols * videomode0numRows * mode0charDimx * mode0charDimy * 4);

    // allocate mode2 bitmap
    mode2bitmap = new unsigned char[mode2dimx * mode2dimy * 4];
    memset(mode2bitmap, 0x0, mode2dimx * mode2dimy * 4);

    // allocate sprites
    for (int s = 0;s < numSprites;s++)
    {
        sprite snew;
        spriteList.push_back(snew);
    }
}

void xpldVideochip::reset()
{
    // init mode 0 videoram
    for (int b = 0;b < videomode0numCols * videomode0numRows;b++)
    {
        videomode0vram[b] = 0;
        videomode0attr[b] = 0x54;
    }

    // init mode 2 videoram to zero
    for (int b = 0;b < mode2dimx * mode2dimy;b++)
    {
        videomode2vram[b] = 0;
    }

    // mode2 palette is black
    for (int i = 0; i < 256; ++i)
    {
        mode2palette[i] = 0xff000000;
    }

    //

    internalClock = 0;
    demultiplier = 0;
    currentScanline = 0;

    currentPaletteEntry = -1;

    vblank = false;
    lineBlank = false;

    mode0hwcursorx = 0;
    mode0hwcursory = 0;
    hwcursorLuma = 0;
}

void xpldVideochip::loadMode0Font(std::string mode0FontPath)
{
    int width, height, numComponents;
    videomode0font = stbi_load(mode0FontPath.c_str(), &width, &height, &numComponents, 4);
    if (videomode0font == NULL)
    {
        throw("koolibri: cannot load system mode 0 font");
    }
}

void xpldVideochip::writeMode0Attr(unsigned long int addr, unsigned char c)
{
    videomode0attr[addr] = c;
}

unsigned char xpldVideochip::readMode0Char(unsigned int address)
{
    return videomode0vram[address- 0x10000000];
}

void xpldVideochip::renderMode0Char(int charnum, int row, int col)
{
    int bitmapPosx = col * mode0charDimx;
    int bitmapPosy = row * mode0charDimy;

    int fontPosx = (charnum % mode0fontNumCharx) * mode0charDimx;
    int fontPosy = floor(charnum / 16) * mode0charDimy;

    for (int py = 0;py < mode0charDimy;py++)
    {
        for (int px = 0;px < mode0charDimx;px++)
        {
            int realpx = px + bitmapPosx;
            int realpy = py + bitmapPosy;

            unsigned char* pdata = (unsigned char*)&mode0bitmap[(realpx + (realpy * videomode0numCols*mode0charDimx)) * 4];
            unsigned char* pfont = &videomode0font[(fontPosx+px+ ((fontPosy+py)* mode0fontNumCharx* mode0charDimx)) * 4];

            unsigned char attr = videomode0attr[col + (row * videomode0numCols)];
            unsigned char fgCol = attr & 0x0f;
            unsigned char bgCol = (attr & 0xf0)>>4;

            if ((*pfont) == 222)
            {
                pdata[0] = mode0palette[fgCol]&0xff;
                pdata[1] = (mode0palette[fgCol]>>8) & 0xff;
                pdata[2] = (mode0palette[fgCol] >> 16) & 0xff;
                pdata[3] = 0xff;
            }
            else
            {
                pdata[0] = mode0palette[bgCol] & 0xff;
                pdata[1] = (mode0palette[bgCol] >> 8) & 0xff;
                pdata[2] = (mode0palette[bgCol] >> 16) & 0xff;
                pdata[3] = 0xff;
            }
        }
    }
}

void xpldVideochip::renderHwCursor()
{
    int col = mode0hwcursorx;
    int row = mode0hwcursory;

    if ((col < 0) || (col >= videomode0numCols))
    {
        throw("Exception: hardware cursor column out of range");
    }

    if ((row < 0) || (row >= videomode0numRows))
    {
        throw("Exception: hardware cursor row out of range");
    }

    int bitmapPosx = col * mode0charDimx;
    int bitmapPosy = row * mode0charDimy;

    for (int py = 0;py < mode0charDimy;py++)
    {
        for (int px = 0;px < mode0charDimx;px++)
        {
            int realpx = px + bitmapPosx;
            int realpy = py + bitmapPosy;

            unsigned char* pdata = (unsigned char*)&mode0bitmap[(realpx + (realpy * videomode0numCols * mode0charDimx)) * 4];

            unsigned char attr = videomode0attr[col + (row * videomode0numCols)];
            unsigned char fgCol = attr & 0x0f;
            unsigned char bgCol = (attr & 0xf0) >> 4;

            int alpha = hwcursorLuma*2;
            if (hwcursorLuma >= 128) alpha = 256 - ((hwcursorLuma-128) * 2);

            int r0 = ((mode0palette[fgCol]&0xff)) ;
            int g0 = (((mode0palette[fgCol]&0xff00) >> 8)) ;
            int b0 = (((mode0palette[fgCol]&0xff0000) >> 16)) ;

            int r1 = ((mode0palette[bgCol] & 0xff));
            int g1 = (((mode0palette[bgCol] & 0xff00) >> 8));
            int b1 = (((mode0palette[bgCol] & 0xff0000) >> 16));

            int rf = ((r0 * alpha) + (r1 * (255 - alpha))) >> 8;
            int gf = ((g0 * alpha) + (g1 * (255 - alpha))) >> 8;
            int bf = ((b0 * alpha) + (b1 * (255 - alpha))) >> 8;

            pdata[0] = rf & 0xff;
            pdata[1] = gf & 0xff;
            pdata[2] = bf & 0xff;
            pdata[3] = 0xff;
        }
    }
}

unsigned char xpldVideochip::readMode2Char(unsigned int address)
{
    return videomode2vram[address- 0x11000000];
}

void xpldVideochip::writeMode0Char(unsigned long int addr, unsigned char c)
{
    videomode0vram[addr] = c;
}

void xpldVideochip::writeMode2Char(unsigned long int addr, unsigned char c)
{
    videomode2vram[addr] = c;
}

void xpldVideochip::setSpriteAttribute(int sprnum,std::string attribName, int attribVal)
{
    if (attribName == "spriteAttributes")
    {
        spriteList[sprnum].setAttributes(attribVal);
    }
    else if (attribName == "dimx")
    {
        spriteList[sprnum].setDimx(attribVal);
    }
    else if (attribName == "dimy")
    {
        spriteList[sprnum].setDimy(attribVal);
    }
    else if (attribName == "posx")
    {
        spriteList[sprnum].setPosx(attribVal);
    }
    else if (attribName == "posy")
    {
        spriteList[sprnum].setPosy(attribVal);
    }
    else if (attribName=="rotation")
    {
        spriteList[sprnum].setRotation(attribVal);
    }
    else if (attribName == "fgcolor")
    {
        spriteList[sprnum].setFgColor(attribVal);
    }

}

void xpldVideochip::feedData8(int sprnum, unsigned char val)
{
    spriteList[sprnum].feedData8(val);
}

unsigned int xpldVideochip::getMode0Palette(int e)
{
    return mode0palette[e];
}

int xpldVideochip::getCurModeBitmapWidth()
{
    if (currentVideomode==VIDEOMODE0_TEXT) return videomode0numCols * mode0charDimx;
    if (currentVideomode == VIDEOMODE2_320x240) return mode2dimx;
}

int xpldVideochip::getCurModeBitmapHeight()
{
    if (currentVideomode == VIDEOMODE0_TEXT) return videomode0numRows * mode0charDimy;
    if (currentVideomode == VIDEOMODE2_320x240) return mode2dimy;
}

unsigned char* xpldVideochip::getCurModeBitmap()
{
    if (currentVideomode == VIDEOMODE0_TEXT) return mode0bitmap;
    if (currentVideomode == VIDEOMODE2_320x240) return mode2bitmap;
}

unsigned char xpldVideochip::getStatusRegister()
{
    unsigned char sr = 0;

    if (lineBlank) sr |= 0x01;
    if (vblank) sr |= 0x02;

    return sr;
}

int xpldVideochip::getRasterLine()
{
    return currentScanline;
}

void xpldVideochip::setMode2PaletteEntry(unsigned char p)
{
    currentPaletteEntry = p;
}

void xpldVideochip::setMode2PaletteColor(unsigned int c)
{
    int r = c & 0xff;
    int g = (c >> 8) & 0xff;
    int b = (c >> 16) & 0xff;

    mode2palette[currentPaletteEntry] = r | (g << 8) | (b << 16) | (0xff << 24);
}

unsigned int xpldVideochip::getInternalClock()
{
    return internalClock;
}

void xpldVideochip::setMode0hwcursorX(unsigned char x)
{
    mode0hwcursorx = x;
}

void xpldVideochip::setMode0hwcursorY(unsigned char y)
{
    mode0hwcursory = y;
}

int xpldVideochip::getMode0hwcursorX()
{
    return mode0hwcursorx;
}

int xpldVideochip::getMode0hwcursorY()
{
    return mode0hwcursory;
}

void xpldVideochip::stepOne()
{
    demultiplier += 1;
    if (demultiplier == multFactor)
    {
        // real step
        demultiplier = 0;

        internalClock += 1;

        if ((internalClock >= lineBlankStart)&& (internalClock < lineBlankEnd)) lineBlank = true;
        if (internalClock == lineBlankEnd)
        {
            lineBlank = false;
            currentScanline += 1;

            if (currentScanline == numScanlines)
            {
                vblank = true;
                currentScanline = -1;
                hwcursorLuma += 8;
                hwcursorLuma %= 255;
            }
            else if (currentScanline == 0)
            {
                vblank = false;
            }

            internalClock = 0;
        }
    }
}

void xpldVideochip::renderFull()
{
    if (currentVideomode == VIDEOMODE0_TEXT)
    {
        for (int row = 0;row < videomode0numRows;row++)
        {
            for (int col = 0;col < videomode0numCols;col++)
            {
                renderMode0Char(videomode0vram[col+(row*videomode0numCols)], row, col);
            }
        }

        // draw hw cursor
        renderHwCursor();
    }
    else if (currentVideomode == VIDEOMODE2_320x240)
    {
        for (int y = 0;y < mode2dimy;y++)
        {
            for (int x = 0;x < mode2dimx;x++)
            {
                unsigned char* pdata = (unsigned char*)&mode2bitmap[(x + (y * mode2dimx)) * 4];
                int pal = mode2palette[videomode2vram[x + (y * mode2dimx)]];
                pdata[0] = pal&0xff;
                pdata[1] = (pal>>8)&0xff;
                pdata[2] = (pal>>16)&0xff;
                pdata[3] = 0xff;
            }
        }

    }

    // sprites are drawable in any videomode
    for (int spr = 0;spr < spriteList.size();spr++)
    {
        if (spriteList[spr].isDrawable())
        {
            if (currentVideomode == VIDEOMODE2_320x240) spriteList[spr].draw(mode2bitmap, mode2dimx, mode2dimy,mode0palette);
        }

    }
}

void xpldVideochip::setVideomode(unsigned char v)
{
    if (v == 0) currentVideomode = VIDEOMODE0_TEXT;
    else if (v == 1) currentVideomode = VIDEOMODE1_320x240;
    else if (v == 2) currentVideomode = VIDEOMODE2_320x240;
}

xpldVideochip::~xpldVideochip()
{
    delete(mode0bitmap);
    delete(mode2bitmap);
}
