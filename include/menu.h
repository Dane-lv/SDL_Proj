#ifndef MENU_H
#define MENU_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

typedef enum
{
    MENU_CHOICE_NONE,
    MENU_CHOICE_HOST,
    MENU_CHOICE_JOIN,
    MENU_CHOICE_QUIT
} MenuChoice;

typedef struct menu Menu;

Menu *menuCreate(SDL_Renderer *r, SDL_Window *w);
void menuDestroy(Menu *m);
bool menuHandleEvent(Menu *m, const SDL_Event *e);
void menuRender(Menu *m);
MenuChoice menuGetChoice(const Menu *m);
const char *menuGetJoinIP(const Menu *m);   

#endif 