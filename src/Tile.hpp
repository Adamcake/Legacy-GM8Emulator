#ifndef _A_TILE_HPP_
#define _A_TILE_HPP_

struct Tile {
    double x;
    double y;
    int backgroundIndex;
    int tileX;
    int tileY;
    int width;
    int height;
    int depth;

    unsigned int id;
    double alpha = 1.0;
    unsigned int blend = 0xFFFFFF;
    double xscale = 1.0;
    double yscale = 1.0;
    bool visible = true;

    Tile(double pX, double pY, unsigned int pBackground, int pTX, int pTY, int pW, int pH, int pDepth, unsigned int pID)
        : x(pX), y(pY), backgroundIndex(pBackground), tileX(pTX), tileY(pTY), width(pW), height(pH), depth(pDepth), id(pID) {}
    Tile() {}
};

#endif