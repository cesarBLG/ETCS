/*
 * European Train Control System
 * Copyright (C) 2019-2020  César Benito <cesarbema2009@hotmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
void repaint();
#endif
