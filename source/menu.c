#include "../include/menu.h"

/* --------------------------------------------------------- */
/* interna strukturer                                         */
/* --------------------------------------------------------- */
struct button { SDL_Rect rect; const char *label; };

typedef enum { MODE_MAIN, MODE_JOIN } MenuMode;

struct menu
{
    SDL_Renderer *r;
    SDL_Window   *w;

    /* typsnitt -------------------------------------------------------- */
    TTF_Font    *fontButton;     /* 28 px  */
    TTF_Font    *fontTitle;      /* 64 px  */

    /* huvudknappar ---------------------------------------------------- */
    struct button btn[3];        /* Host / Join / Settings */
    int           hoveredMain;   /* index för mus‑hover i huvudmeny */

    /* join‑dialog ----------------------------------------------------- */
    char          ip[64];
    bool          hoverBack;     /* mus‑hover över BACK‑knapp */

    /* state ----------------------------------------------------------- */
    MenuMode      mode;
    MenuChoice    choice;
};

/* --------------------------------------------------------- */
/* helper: text -> texture                                    */
/* --------------------------------------------------------- */
static SDL_Texture *renderText(SDL_Renderer *r, TTF_Font *font,
                               const char *txt, SDL_Color col,
                               int *w, int *h)
{
    SDL_Surface *s = TTF_RenderUTF8_Blended(font, txt, col);
    if (!s) return NULL;

    SDL_Texture *t = SDL_CreateTextureFromSurface(r, s);
    if (w) { *w = s->w; }
    if (h) { *h = s->h; }
    SDL_FreeSurface(s);
    return t;
}

/* --------------------------------------------------------- */
Menu *menuCreate(SDL_Renderer *r, SDL_Window *w)
{
    Menu *m = calloc(1, sizeof(Menu));
    m->r = r;
    m->w = w;

    m->fontButton = TTF_OpenFont("resources/font.ttf", 28);
    m->fontTitle  = TTF_OpenFont("resources/font.ttf", 64);
    if (!m->fontButton || !m->fontTitle)
        SDL_Log("TTF_OpenFont: %s", TTF_GetError());

    /* placera huvudknappar ----------------------------------------- */
    int winW, winH; SDL_GetWindowSize(w, &winW, &winH);
    int bw = 240, bh = 60, gap = 20;
    int y0 = winH / 2 - (bh * 3 + gap * 2) / 2;

    for (int i = 0; i < 3; ++i) {
        m->btn[i].rect = (SDL_Rect){ winW / 2 - bw / 2,
                                     y0 + i * (bh + gap),
                                     bw, bh };
    }
    m->btn[0].label = "HOST GAME";
    m->btn[1].label = "JOIN GAME";
    m->btn[2].label = "SETTINGS";

    m->hoveredMain = -1;
    m->hoverBack   = false;
    m->ip[0]       = '\0';
    m->mode        = MODE_MAIN;
    return m;
}

/* --------------------------------------------------------- */
void menuDestroy(Menu *m)
{
    if (m->fontButton) TTF_CloseFont(m->fontButton);
    if (m->fontTitle ) TTF_CloseFont(m->fontTitle );
    free(m);
}

/* --------------------------------------------------------- */
/* --------- HOVERS ---------------------------------------- */
static void updateHoverMain(Menu *m, int mx, int my)
{
    m->hoveredMain = -1;
    for (int i = 0; i < 3; ++i) {
        if (SDL_PointInRect(&(SDL_Point){ mx, my }, &m->btn[i].rect)) {
            m->hoveredMain = i;
            break;
        }
    }
}

static SDL_Rect joinBoxRect(Menu *m)
{
    int winW, winH; SDL_GetWindowSize(m->w, &winW, &winH);
    return (SDL_Rect){ winW / 2 - 210, winH / 2 - 70, 420, 140 };
}

static SDL_Rect backButtonRect(const SDL_Rect *box)
{
    return (SDL_Rect){ box->x + box->w - 140,   /* högra nedre hörnet */
                       box->y + box->h - 50,
                       120, 40 };
}

static void updateHoverJoin(Menu *m, int mx, int my)
{
    SDL_Rect box  = joinBoxRect(m);
    SDL_Rect back = backButtonRect(&box);
    m->hoverBack = SDL_PointInRect(&(SDL_Point){ mx, my }, &back);
}

/* --------------------------------------------------------- */
/* --------- KLICKA PÅ HUVUDMENY --------------------------- */
static void clickMain(Menu *m)
{
    switch (m->hoveredMain) {
        case 0:  m->choice = MENU_CHOICE_HOST;                 break;

        case 1:  m->mode = MODE_JOIN;
                 SDL_StartTextInput();
                 break;

        case 2:  /* SETTINGS – ej implementerat */             break;
        default: break;
    }
}

/* --------- KLICKA PÅ JOIN‑DIALOG ------------------------- */
static void clickJoin(Menu *m)
{
    if (m->hoverBack) {
        /* backa till huvudmeny */
        SDL_StopTextInput();
        m->mode      = MODE_MAIN;
        m->hoverBack = false;
        return;
    }
}

/* --------------------------------------------------------- */
bool menuHandleEvent(Menu *m, const SDL_Event *e)
{
    if (m->choice != MENU_CHOICE_NONE) return false;

    switch (e->type)
    {
    case SDL_QUIT:
        m->choice = MENU_CHOICE_QUIT;
        return false;

/* -------- MUS ------------------------------------------- */
    case SDL_MOUSEMOTION:
        if (m->mode == MODE_MAIN)
            updateHoverMain(m, e->motion.x, e->motion.y);
        else
            updateHoverJoin(m, e->motion.x, e->motion.y);
        break;

    case SDL_MOUSEBUTTONDOWN:
        if (m->mode == MODE_MAIN)
            clickMain(m);
        else
            clickJoin(m);
        break;

/* -------- TEXTINPUT / TANGENTBORD I JOIN‑LÄGE ----------- */
    case SDL_TEXTINPUT:
        if (m->mode == MODE_JOIN &&
            strlen(m->ip) < sizeof(m->ip) - 1)
            strcat(m->ip, e->text.text);
        break;

    case SDL_KEYDOWN:
        if (m->mode == MODE_JOIN) {
            SDL_Keycode k = e->key.keysym.sym;
            if (k == SDLK_BACKSPACE && strlen(m->ip))
                m->ip[strlen(m->ip) - 1] = '\0';
            else if (k == SDLK_RETURN && strlen(m->ip)) {
                SDL_StopTextInput();
                m->choice = MENU_CHOICE_JOIN;
            }
            else if (k == SDLK_ESCAPE) {
                SDL_StopTextInput();
                m->mode      = MODE_MAIN;
                m->hoverBack = false;
            }
        }
        break;
    }
    return true;
}

/* --------------------------------------------------------- */
/* --------- RENDERING ------------------------------------ */
static void renderTitle(Menu *m, SDL_Color col)
{
    int winW; SDL_GetWindowSize(m->w, &winW, NULL);

    int tw, th;
    SDL_Texture *t = renderText(m->r, m->fontTitle,
                                "MAZE MAYHEM", col, &tw, &th);
    SDL_Rect dst = { winW / 2 - tw / 2, 40, tw, th };
    SDL_RenderCopy(m->r, t, NULL, &dst);
    SDL_DestroyTexture(t);
}

static void renderMain(Menu *m, SDL_Color white)
{
    for (int i = 0; i < 3; ++i) {
        bool hov = (i == m->hoveredMain);
        SDL_SetRenderDrawColor(m->r,
                               hov ? 60 : 40,
                               hov ? 60 : 40,
                               hov ? 80 : 60, 255);
        SDL_RenderFillRect(m->r, &m->btn[i].rect);

        int tw, th;
        SDL_Texture *t = renderText(m->r, m->fontButton,
                                    m->btn[i].label, white, &tw, &th);
        SDL_Rect dst = { m->btn[i].rect.x + (m->btn[i].rect.w - tw) / 2,
                         m->btn[i].rect.y + (m->btn[i].rect.h - th) / 2,
                         tw, th };
        SDL_RenderCopy(m->r, t, NULL, &dst);
        SDL_DestroyTexture(t);
    }
}

static void renderJoin(Menu *m, SDL_Color white, SDL_Color grey)
{
    SDL_Rect box = joinBoxRect(m);
    SDL_SetRenderDrawColor(m->r, 40, 40, 55, 255);
    SDL_RenderFillRect(m->r, &box);

    /* prompt */
    int tw, th;
    SDL_Texture *tp = renderText(m->r, m->fontButton,
                                 "Enter IP-address and press enter",
                                 grey, &tw, &th);
    SDL_Rect dp = { box.x + (box.w - tw) / 2, box.y + 15, tw, th };
    SDL_RenderCopy(m->r, tp, NULL, &dp);
    SDL_DestroyTexture(tp);

    /* textfält */
    char buf[96]; snprintf(buf, sizeof(buf), "%s|", m->ip);
    SDL_Texture *ti = renderText(m->r, m->fontButton,
                                 buf, white, &tw, &th);
    SDL_Rect di = { box.x + 20, box.y + box.h / 2 - th / 2, tw, th };
    SDL_RenderCopy(m->r, ti, NULL, &di);
    SDL_DestroyTexture(ti);

    /* BACK‑knapp */
    SDL_Rect back = backButtonRect(&box);
    SDL_SetRenderDrawColor(m->r,
                           m->hoverBack ? 70 : 50,
                           m->hoverBack ? 70 : 50,
                           m->hoverBack ? 90 : 70, 255);
    SDL_RenderFillRect(m->r, &back);

    SDL_Texture *tb = renderText(m->r, m->fontButton,
                                 "BACK", white, &tw, &th);
    SDL_Rect db = { back.x + (back.w - tw) / 2,
                    back.y + (back.h - th) / 2,
                    tw, th };
    SDL_RenderCopy(m->r, tb, NULL, &db);
    SDL_DestroyTexture(tb);
}

/* --------------------------------------------------------- */
void menuRender(Menu *m)
{
    SDL_SetRenderDrawColor(m->r, 15, 15, 20, 255);
    SDL_RenderClear(m->r);

    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color grey  = { 130, 130, 130, 255 };

    renderTitle(m, white);

    if (m->mode == MODE_MAIN)
        renderMain(m, white);
    else
        renderJoin(m, white, grey);

    SDL_RenderPresent(m->r);
}

/* --------------------------------------------------------- */
MenuChoice menuGetChoice(const Menu *m)  { return m->choice; }
const char *menuGetJoinIP(const Menu *m) { return m->ip;     }
