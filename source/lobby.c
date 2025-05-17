#include "../include/lobby.h"
#include "../include/constants.h"
#include <SDL_ttf.h>
#include <string.h>

/* --------------------------------------------------------- */
struct lobby {
    SDL_Renderer *r;
    SDL_Window   *w;
    GameContext  *ctx;

    TTF_Font *big;
    TTF_Font *small;

    SDL_Rect startBtn;   bool hoverStart;
    SDL_Rect backBtn;    bool hoverBack;

    bool      ready;     /* host: start klickad  */
    bool      back;      /* alla: back klickad   */
    bool      isHost;
};

/* --------------------------------------------------------- */
static SDL_Texture *renderText(SDL_Renderer *r, TTF_Font *f,
                               const char *txt, SDL_Color col,
                               int *w, int *h)
{
    SDL_Surface *s = TTF_RenderUTF8_Blended(f, txt, col);
    if (!s) return NULL;
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    if (w) *w = s->w; if (h) *h = s->h;
    SDL_FreeSurface(s);
    return t;
}

/* --------------------------------------------------------- */
Lobby *lobbyCreate(SDL_Renderer *r, SDL_Window *w,
                   GameContext *ctx, bool isHost)
{
    Lobby *l = calloc(1, sizeof *l);
    l->r = r; l->w = w; l->ctx = ctx; l->isHost = isHost;

    l->big   = TTF_OpenFont("resources/font.ttf", 56);
    l->small = TTF_OpenFont("resources/font.ttf", 28);

    int ww, wh; SDL_GetWindowSize(w, &ww, &wh);
    l->startBtn = (SDL_Rect){ ww/2 - 150, wh/2 + 80, 300, 70 };
    l->backBtn  = (SDL_Rect){ 20, 20, 120, 50 };
    return l;
}

void lobbyDestroy(Lobby *l)
{
    if (l->big)   TTF_CloseFont(l->big);
    if (l->small) TTF_CloseFont(l->small);
    free(l);
}

/* --------------------------------------------------------- */
static bool in(const SDL_Rect *r, int x, int y)
{
    return SDL_PointInRect(&(SDL_Point){x,y}, r);
}

bool lobbyHandleEvent(Lobby *l, const SDL_Event *e)
{
    switch (e->type) {
    case SDL_QUIT:      l->ctx->isRunning = false;           return false;

    case SDL_MOUSEMOTION: {
        int mx=e->motion.x, my=e->motion.y;
        if (l->isHost)
            l->hoverStart = in(&l->startBtn, mx, my);
        l->hoverBack  = in(&l->backBtn , mx, my);
        break;
    }
    case SDL_MOUSEBUTTONDOWN: {
        int mx=e->button.x, my=e->button.y;
        if (l->isHost && in(&l->startBtn, mx, my)) l->ready = true;
        if (in(&l->backBtn, mx, my))               l->back  = true;
        break;
    }
    case SDL_KEYDOWN:
        if (e->key.keysym.sym == SDLK_ESCAPE)
            l->back = true;
        break;
    }
    return true;
}

bool lobbyIsReady    (const Lobby *l){ return l->ready; }
bool lobbyBackPressed(const Lobby *l){ return l->back ; }

/* --------------------------------------------------------- */
static void drawButton(SDL_Renderer *r, const SDL_Rect *rect,
                       bool hover, const char *txt,
                       TTF_Font *font)
{
    SDL_SetRenderDrawColor(r, hover?60:40, hover?90:60, 120, 255);
    SDL_RenderFillRect(r, rect);

    int tw, th;
    SDL_Texture *t =
        renderText(r, font, txt, (SDL_Color){255,255,255,255}, &tw, &th);
    SDL_Rect dst = { rect->x + (rect->w - tw)/2,
                     rect->y + (rect->h - th)/2, tw, th };
    SDL_RenderCopy(r, t, NULL, &dst);
    SDL_DestroyTexture(t);
}

/* --------------------------------------------------------- */
void lobbyRender(Lobby *l)
{
    SDL_SetRenderDrawColor(l->r, 20,20,30,255);
    SDL_RenderClear(l->r);

    int ww,wh; SDL_GetWindowSize(l->w,&ww,&wh);
    SDL_Color white={255,255,255,255};
    SDL_Color grey ={180,180,180,255};
    int tw,th;

    SDL_Texture *title =
        renderText(l->r, l->big, "LOBBY", white, &tw,&th);
    SDL_Rect dst = { ww/2 - tw/2, 40, tw, th };
    SDL_RenderCopy(l->r, title, NULL, &dst);
    SDL_DestroyTexture(title);

    /* spelarräknare */
    char buf[32];
    int connected = l->ctx->netMgr.peerCount + 1;
    snprintf(buf,sizeof buf,"%d / %d players", connected, MAX_PLAYERS);

    SDL_Texture *info =
        renderText(l->r, l->small, buf, white, &tw,&th);
    dst = (SDL_Rect){ ww/2 - tw/2, wh/2 - 40, tw, th };
    SDL_RenderCopy(l->r, info, NULL, &dst);
    SDL_DestroyTexture(info);

    /* status-text */
    const char *msg = l->isHost
        ? (connected<MAX_PLAYERS ? "Waiting for players..."
                                 : "All players connected!")
        : "Waiting for host to start...";
    SDL_Texture *wait =
        renderText(l->r, l->small, msg, grey, &tw,&th);
    dst = (SDL_Rect){ ww/2 - tw/2, wh/2, tw, th };
    SDL_RenderCopy(l->r, wait, NULL, &dst);
    SDL_DestroyTexture(wait);

    /* knappar */
    if (l->isHost)
        drawButton(l->r, &l->startBtn, l->hoverStart, "START", l->small);

    drawButton(l->r, &l->backBtn, l->hoverBack, "BACK", l->small);

    SDL_RenderPresent(l->r);
}