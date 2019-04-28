#ifndef _DRAWING_H
#define _DRAWING_H
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "color.h"
#define CENTER 0
#define RIGHT 1
#define LEFT 2
#define UP 4
#define DOWN 8
extern char *fontPath;
extern char *fontPathb;
extern SDL_Window *sdlwin;
extern SDL_Renderer *sdlren;
extern Color renderColor;
int getScale(float val);
float getAntiScale(float val);
void startDisplay(bool fullscreen);
void display();
void quitDisplay();
void clear();
void setColor(Color color);
TTF_Font *openFont(char *text, float size);
void getFontSize(TTF_Font *font, const char *str, float *width, float *height);
void init_video();
void repaint();
#endif