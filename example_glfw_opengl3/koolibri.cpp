/* the koolibri floats. Float koolibri, float */

#include <string>
#include <cstdlib>
#include <math.h>

#include "koolibri.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


xpldVideochip::xpldVideochip()
{
    // init mode 0 videoram to zero
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

    // create mode2 palette

    float frequency = 3.141592f/256.0f;
    for (int i = 0; i < 256; ++i)
    {
        /*
        float red = sin((i*8.0*3.141592f)/256.0f) * 127 + 128;
        float green = sin(2.0*(i * 3.141592f) / 256.0f) * 127 - 128;
        float blue = sin(4.0*(i * 3.141592f) / 256.0f) * 127 + 64;

        int r = (int)red;
        int g = (int)green;
        int b = (int)blue;

        mode2palette[i] = r | (g << 8) | (b << 16) | (0xff << 24);
        */
        mode2palette[i] = 0xff000000;
    }

    //mode2palette[0] = 0;

    loadMode0Font();

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

void xpldVideochip::loadMode0Font()
{
    std::string fileName = "D:\\prova\\xpld\\visual\\8x8font.png";
    int width, height, numComponents;
    videomode0font = stbi_load(fileName.c_str(), &width, &height, &numComponents, 4);
    if (videomode0font == NULL)
    {
        throw("koolibri: cannot load system mode 0 font");
    }
}

void xpldVideochip::writeMode0Attr(unsigned long int addr, unsigned char c)
{
    videomode0attr[addr] = c;
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
