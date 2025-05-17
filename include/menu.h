#ifndef MENU_H
#define MENU_H
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "game_core.h"

typedef enum {
    MENU_CHOICE_NONE,
    MENU_CHOICE_HOST,
    MENU_CHOICE_JOIN,
    MENU_CHOICE_QUIT
} MenuChoice;

typedef struct menu Menu;

Menu        *menuCreate (SDL_Renderer *,SDL_Window *,GameContext *);
void         menuDestroy(Menu *);
bool         menuHandleEvent(Menu *,const SDL_Event *);
void         menuRender(Menu *);
MenuChoice   menuGetChoice(const Menu *);
const char  *menuGetJoinIP(const Menu *);

#endif