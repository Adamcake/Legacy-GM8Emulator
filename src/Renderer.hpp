#pragma once

#include <vector>

struct GameSettings;
typedef unsigned int RImageIndex;

void RInit();
void RTerminate();

// Creates the main game window, should be called after all loading is done. Returns true on success, otherwise false.
bool RMakeGameWindow(GameSettings* settings, unsigned int w, unsigned int h);

// Resize game window
void RResizeGameWindow(unsigned int w, unsigned int h);

// Set window title/caption
void RSetGameWindowTitle(const char* title);

// Set colour for background rectangle
void RSetBGColour(unsigned int col);

// Get cursor pos
void RGetCursorPos(int* xpos, int* ypos);

// Our GL window will tell us when it's supposed to close, then we have to close it. This is for getting that value.
bool RShouldClose();


// Registers an image in the renderer. Assumes 32-bit pixels in RGBA format (which is how it is in the EXE.) 
RImageIndex RMakeImage(unsigned int w, unsigned int h, unsigned int originX, unsigned int originY, unsigned char* bytes);

// Draws a registered image at the given X and Y. Tries to imitate draw_sprite_ext() from GML.
void RDrawImage(RImageIndex ix, double x, double y, double xscale, double yscale, double rot, unsigned int blend, double alpha);

// Draws a given section of a registered image at the given X and Y.
void RDrawPartialImage(RImageIndex ix, double x, double y, double xscale, double yscale, double rot, unsigned int blend, double alpha, unsigned int partX, unsigned int partY, unsigned int partW, unsigned int partH);

// Clear the screen and prepare for drawing sprites
void RStartFrame();

// Render the current frame after drawing all images
void RRenderFrame();
