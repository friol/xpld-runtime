#ifndef SPRITE_H
#define SPRITE_H

class sprite
{
private:

    bool enabled = false;
    unsigned int type; // 0 monochrome, 1 color 32 bit

    unsigned int dimx=0;
    unsigned int dimy=0;

    unsigned int posx;
    unsigned int posy;

    unsigned int rotation=0;
    unsigned char fgColor;

    unsigned char* spriteData=0L;

    unsigned int data8pos = 0;

public:

    sprite();
    ~sprite();

    void setAttributes(int bitstring);
    void setDimx(int dx);
    void setDimy(int dy);
    void setPosx(int px);
    void setPosy(int py);
    void setRotation(int r);
    void setFgColor(int fgcol);

    void feedData8(unsigned char v);

    bool isDrawable();

    void draw(unsigned char* bitmap, int bitmapdimx, int bitmapdimy, unsigned int* palette);

};

#endif
