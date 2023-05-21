/*
 * European Train Control System
 * Copyright (C) 2019-2023  César Benito <cesarbema2009@hotmail.com>
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#ifndef _DRAWING_H
#define _DRAWING_H
#include <SDL.h>
#include <SDL_ttf.h>
#include "color.h"
#include <string>
#define CENTER 0
#define RIGHT 1
#define LEFT 2
#define UP 4
#define DOWN 8
extern std::string fontPath;
extern std::string fontPathb;
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
TTF_Font *openFont(std::string text, float size);
void getFontSize(TTF_Font *font, const char *str, float *width, float *height);
void init_video();
void loop_video();
#endif
