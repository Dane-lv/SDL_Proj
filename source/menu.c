#include "../include/menu.h"
#include "../include/audio_manager.h"
#include <string.h>

struct button
{
    SDL_Rect rect;
    const char *label;
};
struct slider
{
    SDL_Rect track, handle;
    const char *label;
    int value;
};

typedef enum
{
    MODE_MAIN,
    MODE_JOIN,
    MODE_SETTINGS
} MenuMode;

struct menu
{
    SDL_Renderer *r;
    SDL_Window *w;
    GameContext *ctx;

    TTF_Font *fontButton;
    TTF_Font *fontTitle;
    struct button btn[3];
    int hovered;

    char ip[64];
    bool hoverBack;

    struct slider musicSlider, sfxSlider;
    bool draggingMusic, draggingSfx;
    struct button backBtn;
    bool hoverSettings;

    MenuMode mode;
    MenuChoice choice;
};
static SDL_Texture *
txt(SDL_Renderer *r, TTF_Font *f, const char *s, SDL_Color col, int *w, int *h)
{
    SDL_Surface *surf = TTF_RenderUTF8_Blended(f, s, col);
    SDL_Texture *t = SDL_CreateTextureFromSurface(r, surf);
    if (w)
        *w = surf->w;
    if (h)
        *h = surf->h;
    SDL_FreeSurface(surf);
    return t;
}
/* ---------------- slider-helpers ------------------------- */
static struct slider mkSlider(int x,int y,int w,int h,
                              const char *lbl,int v)
{
    struct slider s;
    s.track = (SDL_Rect){x, y, w, h};
    s.handle = (SDL_Rect){x + (v * w) / 128 - 10, y - 10, 20, h + 20};
    s.label = lbl;
    s.value = v;
    return s;
}
static void updateHandle(struct slider *s)
{
    s->handle.x = s->track.x + (s->value * s->track.w) / 128 - s->handle.w / 2;
    if (s->handle.x < s->track.x - s->handle.w / 2)
        s->handle.x = s->track.x - s->handle.w / 2;
    if (s->handle.x > s->track.x + s->track.w - s->handle.w / 2)
        s->handle.x = s->track.x + s->track.w - s->handle.w / 2;
}
static void updateValue(struct slider *s, int mx)
{
    int rel = mx - s->track.x;
    s->value = (rel * 128) / s->track.w;
    if (s->value < 0)
        s->value = 0;
    if (s->value > 128)
        s->value = 128;
    updateHandle(s);
}
Menu *menuCreate(SDL_Renderer *r, SDL_Window *w, GameContext *ctx)
{
    Menu *m = calloc(1, sizeof *m);
    m->r = r;
    m->w = w;
    m->ctx = ctx;

    m->fontButton = TTF_OpenFont("resources/font.ttf", 28);
    m->fontTitle = TTF_OpenFont("resources/font.ttf", 64);

    int winW, winH;
    SDL_GetWindowSize(w, &winW, &winH);
    int bw = 240, bh = 60, gap = 20;
    int y0 = winH / 2 - (bh * 3 + gap * 2) / 2;
    for (int i = 0; i < 3; ++i)
        m->btn[i].rect = (SDL_Rect){winW / 2 - bw / 2, y0 + i * (bh + gap), bw, bh};
    m->btn[0].label = "HOST GAME";
    m->btn[1].label = "JOIN GAME";
    m->btn[2].label = "SETTINGS";

    int sw=400,sh=20, sx=winW/2-sw/2, sy1=winH/2-60, sy2=winH/2+30;
    m->musicSlider=mkSlider(sx,sy1,sw,sh,"Music Volume",96);
    m->sfxSlider  =mkSlider(sx,sy2,sw,sh,"SFX Volume",96);

    m->backBtn.rect = (SDL_Rect){winW / 2 - 100, winH / 2 + 120, 200, 50};
    m->backBtn.label = "BACK";

    m->mode = MODE_MAIN;
    m->choice = MENU_CHOICE_NONE;
    if (ctx && ctx->audioManager)
    {
        setMusicVolume(m->musicSlider.value);
        setSoundVolume(m->sfxSlider.value);
    }
    return m;
}
void menuDestroy(Menu *m)
{
    if (m->fontButton)
        TTF_CloseFont(m->fontButton);
    if (m->fontTitle)
        TTF_CloseFont(m->fontTitle);
    free(m);
}
static void hovMain(Menu *m, int mx, int my)
{
    m->hovered = -1;
    for (int i = 0; i < 3; ++i)
        if (SDL_PointInRect(&(SDL_Point){mx, my}, &m->btn[i].rect))
        {
            m->hovered = i;
            break;
        }
}
static SDL_Rect joinBox(Menu *m)
{
    int w, h;
    SDL_GetWindowSize(m->w, &w, &h);
    return (SDL_Rect){w / 2 - 210, h / 2 - 70, 420, 140};
}
static void hovJoin(Menu *m, int mx, int my)
{
    SDL_Rect back = {joinBox(m).x + 280, joinBox(m).y + 90, 120, 40};
    m->hoverBack = SDL_PointInRect(&(SDL_Point){mx, my}, &back);
}
static void hovSettings(Menu *m, int mx, int my)
{
    m->hoverSettings =
        SDL_PointInRect(&(SDL_Point){mx, my}, &m->backBtn.rect);
}
static void clickMain(Menu *m)
{
    switch (m->hovered)
    {
    case 0:
        m->choice = MENU_CHOICE_HOST;
        break;
    case 1:
        m->mode = MODE_JOIN;
        SDL_StartTextInput();
        break;
    case 2:
        m->mode = MODE_SETTINGS;
        break;
    default:
        break;
    }
}
static void clickJoin(Menu *m)
{
    if (m->hoverBack)
    {
        m->mode = MODE_MAIN;
        m->hoverBack = false;
        SDL_StopTextInput();
    }
}
static void clickSettings(Menu *m)
{
    if (m->hoverSettings)
    {
        m->mode = MODE_MAIN;
        m->hoverSettings = false;
    }
}
static void sliderClick(Menu *m, int mx, int my)
{
    SDL_Rect hit = m->musicSlider.handle;
    hit.x -= 10;
    hit.w += 20;
    if (SDL_PointInRect(&(SDL_Point){mx, my}, &hit))
    {
        m->draggingMusic = true;
        return;
    }
    hit = m->sfxSlider.handle;
    hit.x -= 10;
    hit.w += 20;
    if (SDL_PointInRect(&(SDL_Point){mx, my}, &hit))
    {
        m->draggingSfx = true;
        return;
    }
    if (SDL_PointInRect(&(SDL_Point){mx, my}, &m->musicSlider.track))
    {
        updateValue(&m->musicSlider, mx);
        m->draggingMusic = true;
        if (m->ctx && m->ctx->audioManager)
            setMusicVolume(m->musicSlider.value);
    }
    if (SDL_PointInRect(&(SDL_Point){mx, my}, &m->sfxSlider.track))
    {
        updateValue(&m->sfxSlider, mx);
        m->draggingSfx = true;
        if (m->ctx && m->ctx->audioManager)
        {
            setSoundVolume(m->sfxSlider.value);
            if (m->ctx->audioManager->deathSound)
                playDeathSound(m->ctx->audioManager);
        }
    }
}
bool menuHandleEvent(Menu *m, const SDL_Event *e)
{
    if (m->choice != MENU_CHOICE_NONE)
        return false;

    switch (e->type)
    {
    case SDL_QUIT:
        m->choice = MENU_CHOICE_QUIT;
        return false;

    case SDL_MOUSEMOTION:
        if (m->draggingMusic)
        {
            updateValue(&m->musicSlider, e->motion.x);
            if (m->ctx && m->ctx->audioManager)
                setMusicVolume(m->musicSlider.value);
        }
        else if (m->draggingSfx)
        {
            updateValue(&m->sfxSlider, e->motion.x);
            if (m->ctx && m->ctx->audioManager)
                setSoundVolume(m->sfxSlider.value);
        }
        else if (m->mode == MODE_MAIN)
            hovMain(m, e->motion.x, e->motion.y);
        else if (m->mode == MODE_JOIN)
            hovJoin(m, e->motion.x, e->motion.y);
        else if (m->mode == MODE_SETTINGS)
            hovSettings(m, e->motion.x, e->motion.y);
        break;

    case SDL_MOUSEBUTTONDOWN:
        if (m->mode == MODE_MAIN)
            clickMain(m);
        else if (m->mode == MODE_JOIN)
            clickJoin(m);
        else if (m->mode == MODE_SETTINGS)
        {
            sliderClick(m, e->button.x, e->button.y);
            if (!m->draggingMusic && !m->draggingSfx)
                clickSettings(m);
        }
        break;
    case SDL_MOUSEBUTTONUP:
        m->draggingMusic = m->draggingSfx = false;
        break;

    case SDL_TEXTINPUT:
        if (m->mode == MODE_JOIN && strlen(m->ip) < sizeof(m->ip) - 1)
            strcat(m->ip, e->text.text);
        break;

    case SDL_KEYDOWN:
        if (m->mode == MODE_JOIN)
        {
            if (e->key.keysym.sym == SDLK_BACKSPACE && strlen(m->ip))
                m->ip[strlen(m->ip) - 1] = '\0';
            else if (e->key.keysym.sym == SDLK_RETURN && strlen(m->ip))
            {
                SDL_StopTextInput();
                m->choice = MENU_CHOICE_JOIN;
            }
            else if (e->key.keysym.sym == SDLK_ESCAPE)
            {
                SDL_StopTextInput();
                m->mode = MODE_MAIN;
                m->hoverBack = false;
            }
        }
        else if (m->mode == MODE_SETTINGS &&
                 e->key.keysym.sym == SDLK_ESCAPE)
            m->mode = MODE_MAIN, m->hoverSettings = false;
        break;
    }
    return true;
}
static void renderTitle(Menu *m, SDL_Color col)
{
    int winW;
    SDL_GetWindowSize(m->w, &winW, NULL);
    int tw, th;
    SDL_Texture *t = txt(m->r, m->fontTitle, "MAZE MAYHEM", col, &tw, &th);
    SDL_Rect dst = {winW / 2 - tw / 2, 40, tw, th};
    SDL_RenderCopy(m->r, t, NULL, &dst);
    SDL_DestroyTexture(t);
}
static void renderMain(Menu *m, SDL_Color white)
{
    for (int i = 0; i < 3; ++i)
    {
        bool hov = i == m->hovered;
        SDL_SetRenderDrawColor(m->r, hov ? 60 : 40, hov ? 60 : 40, hov ? 80 : 60, 255);
        SDL_RenderFillRect(m->r, &m->btn[i].rect);
        int tw, th;
        SDL_Texture *t = txt(m->r, m->fontButton, m->btn[i].label, white, &tw, &th);
        SDL_Rect dst = {m->btn[i].rect.x + (m->btn[i].rect.w - tw) / 2,
                        m->btn[i].rect.y + (m->btn[i].rect.h - th) / 2, tw, th};
        SDL_RenderCopy(m->r, t, NULL, &dst);
        SDL_DestroyTexture(t);
    }
}
static void renderJoin(Menu *m, SDL_Color white, SDL_Color grey)
{
    SDL_Rect box = joinBox(m);
    SDL_SetRenderDrawColor(m->r, 40, 40, 55, 255);
    SDL_RenderFillRect(m->r, &box);

    int tw, th;
    SDL_Texture *t = txt(m->r, m->fontButton,
                         "Enter IP-address and press enter", grey, &tw, &th);
    SDL_Rect dst = {box.x + (box.w - tw) / 2, box.y + 15, tw, th};
    SDL_RenderCopy(m->r, t, NULL, &dst);
    SDL_DestroyTexture(t);

    SDL_SetRenderDrawColor(m->r, 20, 20, 30, 255);
    SDL_Rect in = {box.x + 20, box.y + 50, box.w - 40, 40};
    SDL_RenderFillRect(m->r, &in);

    if (strlen(m->ip))
    {
        t = txt(m->r, m->fontButton, m->ip, white, &tw, &th);
        SDL_Rect di = {in.x + 10, in.y + (in.h - th) / 2, tw, th};
        SDL_RenderCopy(m->r, t, NULL, &di);
        SDL_DestroyTexture(t);
    }

    SDL_Rect back = {box.x + box.w - 140, box.y + box.h - 50, 120, 40};
    SDL_SetRenderDrawColor(m->r, m->hoverBack ? 60 : 40, m->hoverBack ? 60 : 40,
                           m->hoverBack ? 80 : 60, 255);
    SDL_RenderFillRect(m->r, &back);
    t = txt(m->r, m->fontButton, "BACK", white, &tw, &th);
    dst = (SDL_Rect){back.x + (back.w - tw) / 2, back.y + (back.h - th) / 2, tw, th};
    SDL_RenderCopy(m->r, t, NULL, &dst);
    SDL_DestroyTexture(t);
}
static void drawSlider(Menu *m, struct slider *s, SDL_Color white)
{
    int tw, th;
    SDL_Texture *t = txt(m->r, m->fontButton, s->label, white, &tw, &th);
    SDL_Rect dst = {s->track.x, s->track.y - 40, tw, th};
    SDL_RenderCopy(m->r, t, NULL, &dst);
    SDL_DestroyTexture(t);

    SDL_SetRenderDrawColor(m->r, 40, 40, 60, 255);
    SDL_RenderFillRect(m->r, &s->track);

    SDL_Rect fill = s->track;
    fill.w = (s->value * s->track.w) / 128;
    SDL_SetRenderDrawColor(m->r, 60, 80, 140, 255);
    SDL_RenderFillRect(m->r, &fill);

    SDL_SetRenderDrawColor(m->r, 100, 120, 200, 255);
    SDL_RenderFillRect(m->r, &s->handle);

    char perc[16];
    sprintf(perc, "%d%%", (s->value * 100) / 128);
    t = txt(m->r, m->fontButton, perc, white, &tw, &th);
    dst = (SDL_Rect){s->track.x + s->track.w + 10,
                     s->track.y + (s->track.h - th * 0.7) / 2,
                     (int)(tw * 0.7), (int)(th * 0.7)};
    SDL_RenderCopy(m->r, t, NULL, &dst);
    SDL_DestroyTexture(t);
}
static void renderSettings(Menu *m, SDL_Color white)
{
    int w, h;
    SDL_GetWindowSize(m->w, &w, &h);
    SDL_Rect panel = {w / 2 - 250, h / 2 - 200, 500, 400};
    SDL_SetRenderDrawColor(m->r, 30, 30, 45, 255);
    SDL_RenderFillRect(m->r, &panel);

    int tw, th;
    SDL_Texture *t = txt(m->r, m->fontButton, "AUDIO SETTINGS", white, &tw, &th);
    SDL_Rect dst = {w / 2 - tw / 2, panel.y + 20, tw, th};
    SDL_RenderCopy(m->r, t, NULL, &dst);
    SDL_DestroyTexture(t);

    drawSlider(m, &m->musicSlider, white);
    drawSlider(m, &m->sfxSlider, white);

    SDL_SetRenderDrawColor(m->r, m->hoverSettings ? 60 : 40,
                           m->hoverSettings ? 60 : 40,
                           m->hoverSettings ? 80 : 60, 255);
    SDL_RenderFillRect(m->r, &m->backBtn.rect);
    t = txt(m->r, m->fontButton, m->backBtn.label, white, &tw, &th);
    dst = (SDL_Rect){m->backBtn.rect.x + (m->backBtn.rect.w - tw) / 2,
                     m->backBtn.rect.y + (m->backBtn.rect.h - th) / 2, tw, th};
    SDL_RenderCopy(m->r, t, NULL, &dst);
    SDL_DestroyTexture(t);
}
void menuRender(Menu *m)
{
    SDL_SetRenderDrawColor(m->r, 20, 20, 30, 255);
    SDL_RenderClear(m->r);

    SDL_Color w = {255, 255, 255, 255}, g = {170, 170, 170, 255};
    renderTitle(m, w);

    if (m->mode == MODE_MAIN)
        renderMain(m, w);
    else if (m->mode == MODE_JOIN)
        renderJoin(m, w, g);
    else if (m->mode == MODE_SETTINGS)
        renderSettings(m, w);

    SDL_RenderPresent(m->r);
}
MenuChoice menuGetChoice(const Menu *m) { return m->choice; }
const char *menuGetJoinIP(const Menu *m) { return m->ip; }