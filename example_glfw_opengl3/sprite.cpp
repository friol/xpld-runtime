
/* back to commodore */

#include <cstring>
#include "sprite.h"

sprite::sprite()
{
    posx = 0;
    posy = 0;
}

sprite::~sprite()
{
    delete(spriteData);
}

void sprite::setAttributes(int bitstring)
{
    if (bitstring & 0x01) enabled = true;
    else enabled = false;

    if (bitstring & 0x02) type = 1;
    else type = 0;
}

void sprite::setDimx(int dx)
{
    dimx = dx;
    if (dimy != 0)
    {
        delete(spriteData);
        if (type == 0)
        {
            spriteData = new unsigned char[dimx * dimy];
            memset(spriteData, 0, dimx * dimy);
        }
        else if (type == 1)
        {
            spriteData = new unsigned char[dimx * dimy * 4];
            memset(spriteData, 0, dimx * dimy*4);
        }
    }
}

void sprite::setDimy(int dy)
{
    dimy = dy;
    if (dimx != 0)
    {
        delete(spriteData);
        if (type == 0)
        {
            spriteData = new unsigned char[dimx * dimy];
            memset(spriteData, 0, dimx * dimy);
        }
        else if (type == 1)
        {
            spriteData = new unsigned char[dimx * dimy * 4];
            memset(spriteData, 0, dimx * dimy * 4);
        }
    }
}

void sprite::setPosx(int px)
{
    posx = px;
}

void sprite::setPosy(int py)
{
    posy = py;
}

void sprite::setRotation(int r)
{
    rotation = r;
}

void sprite::setFgColor(int fgcol)
{
    fgColor = fgcol;
}

void sprite::feedData8(unsigned char v)
{
    if (spriteData == 0)
    {
        throw("Code is trying to write on sprite not initializeddd!");
    }

    if (type == 1)
    {
        throw("Code is trying to feed 8bit data to a multicolour sprite");
    }

    for (int b = 0;b < 8;b++)
    {
        spriteData[data8pos] = (v >> (7 - b)) & 0x01;
        data8pos += 1;
    }

    if (data8pos >= (dimx * dimy))
    {
        data8pos = 0; // pointer wraps
    }
}

bool sprite::isDrawable()
{
    return ((enabled)&&(spriteData!=0));
}

void sprite::draw(unsigned char* bitmap,int bitmapdimx,int bitmapdimy,unsigned int* palette)
{
    for (int y = 0;y < dimy;y++)
    {
        for (int x = 0;x < dimx;x++)
        {
            int bitmapPosx = x + posx;
            int bitmapPosy = y + posy;

            if ((bitmapPosx >= 0) && (bitmapPosx < bitmapdimx))
            {
                if ((bitmapPosy >= 0) && (bitmapPosy < bitmapdimy))
                {
                    if (type == 0)
                    {
                        if (spriteData != 0)
                        {
                            unsigned char pix = spriteData[x + (y * dimx)];
                            if (pix == 1)
                            {
                                int pos = (bitmapPosx + (bitmapPosy * bitmapdimx)) * 4;
                                bitmap[pos + 0] = palette[fgColor] & 0xff;
                                bitmap[pos + 1] = (palette[fgColor] >> 8) & 0xff;
                                bitmap[pos + 2] = (palette[fgColor] >> 16) & 0xff;
                                bitmap[pos + 3] = 0xff;
                            }
                        }
                    }
                }
            }
        }
    }
}
