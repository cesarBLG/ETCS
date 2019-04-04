#include "../drawing.h"
#include "../display.h"
using namespace std;
SDL_Window *sdlwin;
SDL_Renderer *sdlren;
char *fontPath = "fonts/swiss.ttf";
char *fontPathb = "fonts/swissb.ttf";
#define PI 3.14159265358979323846264338327950288419716939937510
float scale = 1;
float offset[2] = {0, 0};
void startDisplay(bool fullscreen)
{
    TTF_Init();
    sdlwin = SDL_CreateWindow("Driver Machine Interface", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);
    if(fullscreen) SDL_SetWindowFullscreen(sdlwin, SDL_WINDOW_FULLSCREEN_DESKTOP); 
    sdlren = SDL_CreateRenderer(sdlwin, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    int w,h;
    SDL_GetWindowSize(sdlwin, &w, &h);
    float scrsize[] = {w,h};
    float extra = 640/2*(scrsize[0]/(scrsize[1]*4/3)-1);
    offset[0] = extra;
    scale = scrsize[1]/480.0;
    prepareLayout();
    display();
}
void display()
{
    clear();
    displayETCS();
    SDL_RenderPresent(sdlren);
}
void quitDisplay()
{
    SDL_DestroyRenderer(sdlren);
    SDL_DestroyWindow(sdlwin);
    SDL_Quit();
}
void clear()
{
    setColor(DarkBlue);
    SDL_RenderClear(sdlren);
}
Color renderColor;
void setColor(Color color)
{
    renderColor = color;
    SDL_SetRenderDrawColor(sdlren, color.R,color.G,color.B,255);
    //glColor3ub(color.R, color.G, color.B);
}
inline int getScale(float val)
{
    return round(val*scale);
}
void getFontSize(TTF_Font *font, const char *str, float *width, float *height)
{
    int w;
    int h;
    TTF_SizeText(font, str, &w, &h);
    *width = w/scale;
    *height = h/scale;
}
TTF_Font *openFont(char *text, float size)
{
    TTF_Font *f = TTF_OpenFont(text, getScale(size)*1.4);
    /*float height = TTF_FontAscent(f)/scale;
    TTF_CloseFont(f);
    f = TTF_OpenFont(text, getScale(size)*size/height);*/
    return f;
}