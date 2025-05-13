#include "../include/menu.h"
#include "../include/audio_manager.h"
#include <string.h>


struct button { SDL_Rect rect; const char *label; };
struct slider { SDL_Rect track; SDL_Rect handle; const char *label; int value; };

typedef enum { MODE_MAIN, MODE_JOIN, MODE_SETTINGS } MenuMode;

struct menu
{
    SDL_Renderer *r;
    SDL_Window   *w;
    GameContext  *gameContext;

    /* typsnitt -------------------------------------------------------- */
    TTF_Font    *fontButton;     /* 28 px  */
    TTF_Font    *fontTitle;      /* 64 px  */

    /* huvudknappar ---------------------------------------------------- */
    struct button btn[3];        /* Host / Join / Settings */
    int           hoveredMain;   /* index för mus‑hover i huvudmeny */

    /* join‑dialog ----------------------------------------------------- */
    char          ip[64];
    bool          hoverBack;     /* mus‑hover över BACK‑knapp */

    /* settings -------------------------------------------------------- */
    struct slider musicSlider;   /* Slider for music volume */
    struct slider sfxSlider;     /* Slider for sound effects volume */
    bool          draggingMusic; /* Currently dragging music slider */
    bool          draggingSfx;   /* Currently dragging SFX slider */
    struct button backBtn;       /* Back button for settings */
    bool          hoverSettings; /* Hover over back button in settings */

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
/* Create a slider with initial position and value */
static struct slider createSlider(int x, int y, int width, int height, const char *label, int initialValue)
{
    struct slider s;
    s.track = (SDL_Rect){ x, y, width, height };
    s.handle = (SDL_Rect){ x + (initialValue * width) / 128, y - 10, 20, height + 20 };
    s.label = label;
    s.value = initialValue;
    return s;
}

/* --------------------------------------------------------- */
/* Update slider handle position based on value */
static void updateSliderHandle(struct slider *s)
{
    s->handle.x = s->track.x + (s->value * s->track.w) / 128 - s->handle.w / 2;
    
    // Keep handle within track boundaries
    if (s->handle.x < s->track.x - s->handle.w / 2)
        s->handle.x = s->track.x - s->handle.w / 2;
    if (s->handle.x > s->track.x + s->track.w - s->handle.w / 2)
        s->handle.x = s->track.x + s->track.w - s->handle.w / 2;
}

/* --------------------------------------------------------- */
/* Update slider value based on mouse x position */
static void updateSliderValue(struct slider *s, int mouseX)
{
    int relativeX = mouseX - s->track.x;
    s->value = (relativeX * 128) / s->track.w;
    
    // Clamp value to 0-128 range
    if (s->value < 0) s->value = 0;
    if (s->value > 128) s->value = 128;
    
    updateSliderHandle(s);
}

/* --------------------------------------------------------- */
Menu *menuCreate(SDL_Renderer *r, SDL_Window *w, GameContext *gameContext)
{
    Menu *m = calloc(1, sizeof(Menu));
    m->r = r;
    m->w = w;
    m->gameContext = gameContext;

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

    /* placera settings knappar och sliders --------------------------- */
    int sliderWidth = 400, sliderHeight = 20;
    int sliderX = winW / 2 - sliderWidth / 2;
    int sliderY1 = winH / 2 - 60;
    int sliderY2 = winH / 2 + 30;
    
    // Initialize volume sliders with default values (75% of max)
    m->musicSlider = createSlider(sliderX, sliderY1, sliderWidth, sliderHeight, "Music Volume", 96);
    m->sfxSlider = createSlider(sliderX, sliderY2, sliderWidth, sliderHeight, "SFX Volume", 96);
    
    // Back button for settings
    m->backBtn.rect = (SDL_Rect){ winW / 2 - 100, winH / 2 + 120, 200, 50 };
    m->backBtn.label = "BACK";

    m->hoveredMain = -1;
    m->hoverBack = false;
    m->hoverSettings = false;
    m->draggingMusic = false;
    m->draggingSfx = false;
    m->ip[0] = '\0';
    m->mode = MODE_MAIN;
    
    // Set initial volumes
    if (m->gameContext && m->gameContext->audioManager) {
        setMusicVolume(m->musicSlider.value);
        setSoundVolume(m->sfxSlider.value);
    }
    
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

static void updateHoverSettings(Menu *m, int mx, int my)
{
    m->hoverSettings = SDL_PointInRect(&(SDL_Point){ mx, my }, &m->backBtn.rect);
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

        case 2:  m->mode = MODE_SETTINGS;                      break;
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

/* --------- KLICKA PÅ SETTINGS --------------------------- */
static void clickSettings(Menu *m)
{
    if (m->hoverSettings) {
        /* backa till huvudmeny */
        m->mode = MODE_MAIN;
        m->hoverSettings = false;
        return;
    }
}

/* --------- Start dragging sliders if clicked on them ----- */
static void checkSliderClick(Menu *m, int mx, int my)
{
    // Check music slider
    SDL_Rect hitbox = m->musicSlider.handle;
    hitbox.x -= 10; // Make click area a bit wider than visual handle
    hitbox.w += 20;
    if (SDL_PointInRect(&(SDL_Point){ mx, my }, &hitbox)) {
        m->draggingMusic = true;
        return;
    }
    
    // Check SFX slider
    hitbox = m->sfxSlider.handle;
    hitbox.x -= 10;
    hitbox.w += 20;
    if (SDL_PointInRect(&(SDL_Point){ mx, my }, &hitbox)) {
        m->draggingSfx = true;
        return;
    }
    
    // Check if clicked on track (not handle)
    if (SDL_PointInRect(&(SDL_Point){ mx, my }, &m->musicSlider.track)) {
        updateSliderValue(&m->musicSlider, mx);
        if (m->gameContext && m->gameContext->audioManager) {
            setMusicVolume(m->musicSlider.value);
        }
        m->draggingMusic = true;
        return;
    }
    
    if (SDL_PointInRect(&(SDL_Point){ mx, my }, &m->sfxSlider.track)) {
        updateSliderValue(&m->sfxSlider, mx);
        if (m->gameContext && m->gameContext->audioManager) {
            setSoundVolume(m->sfxSlider.value);
            
            // Play a sound effect sample when adjusting SFX volume
            if (m->gameContext->audioManager->deathSound) {
                playDeathSound(m->gameContext->audioManager);
            }
        }
        m->draggingSfx = true;
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
        if (m->draggingMusic) {
            updateSliderValue(&m->musicSlider, e->motion.x);
            if (m->gameContext && m->gameContext->audioManager) {
                setMusicVolume(m->musicSlider.value);
            }
        } else if (m->draggingSfx) {
            updateSliderValue(&m->sfxSlider, e->motion.x);
            if (m->gameContext && m->gameContext->audioManager) {
                setSoundVolume(m->sfxSlider.value);
            }
        } else if (m->mode == MODE_MAIN) {
            updateHoverMain(m, e->motion.x, e->motion.y);
        } else if (m->mode == MODE_JOIN) {
            updateHoverJoin(m, e->motion.x, e->motion.y);
        } else if (m->mode == MODE_SETTINGS) {
            updateHoverSettings(m, e->motion.x, e->motion.y);
        }
        break;

    case SDL_MOUSEBUTTONDOWN:
        if (m->mode == MODE_MAIN) {
            clickMain(m);
        } else if (m->mode == MODE_JOIN) {
            clickJoin(m);
        } else if (m->mode == MODE_SETTINGS) {
            checkSliderClick(m, e->button.x, e->button.y);
            if (!m->draggingMusic && !m->draggingSfx) {
                clickSettings(m);
            }
        }
        break;
        
    case SDL_MOUSEBUTTONUP:
        m->draggingMusic = false;
        m->draggingSfx = false;
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
        } else if (m->mode == MODE_SETTINGS) {
            SDL_Keycode k = e->key.keysym.sym;
            if (k == SDLK_ESCAPE) {
                m->mode = MODE_MAIN;
                m->hoverSettings = false;
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

    /* input field */
    SDL_SetRenderDrawColor(m->r, 20, 20, 30, 255);
    SDL_Rect in = { box.x + 20, box.y + 50, box.w - 40, 40 };
    SDL_RenderFillRect(m->r, &in);

    if (strlen(m->ip)) {
        SDL_Texture *ti = renderText(m->r, m->fontButton,
                                    m->ip, white, &tw, &th);
        SDL_Rect dt = { in.x + 10, in.y + (in.h - th) / 2, tw, th };
        SDL_RenderCopy(m->r, ti, NULL, &dt);
        SDL_DestroyTexture(ti);
    }

    /* back‑button */
    SDL_Rect back = backButtonRect(&box);
    bool hov = m->hoverBack;
    SDL_SetRenderDrawColor(m->r,
                          hov ? 60 : 40,
                          hov ? 60 : 40,
                          hov ? 80 : 60, 255);
    SDL_RenderFillRect(m->r, &back);

    SDL_Texture *tb = renderText(m->r, m->fontButton,
                                "BACK", white, &tw, &th);
    SDL_Rect db = { back.x + (back.w - tw) / 2,
                   back.y + (back.h - th) / 2,
                   tw, th };
    SDL_RenderCopy(m->r, tb, NULL, &db);
    SDL_DestroyTexture(tb);
}

static void renderSlider(Menu *m, struct slider *s, SDL_Color textColor)
{
    // Draw label
    int tw, th;
    SDL_Texture *t = renderText(m->r, m->fontButton, s->label, textColor, &tw, &th);
    SDL_Rect dst = { s->track.x, s->track.y - 40, tw, th };
    SDL_RenderCopy(m->r, t, NULL, &dst);
    SDL_DestroyTexture(t);
    
    // Draw track
    SDL_SetRenderDrawColor(m->r, 40, 40, 60, 255);
    SDL_RenderFillRect(m->r, &s->track);
    
    // Draw filled portion
    SDL_Rect filled = s->track;
    filled.w = ((s->value * s->track.w) / 128);
    SDL_SetRenderDrawColor(m->r, 60, 80, 140, 255);
    SDL_RenderFillRect(m->r, &filled);
    
    // Draw handle
    SDL_SetRenderDrawColor(m->r, 100, 120, 200, 255);
    SDL_RenderFillRect(m->r, &s->handle);
    
    // Draw value text - smaller percentage display
    char valueText[16];
    sprintf(valueText, "%d%%", (s->value * 100) / 128);
    
    // Create a smaller version of the percentage text
    SDL_Color grayColor = { 200, 200, 200, 255 };
    t = renderText(m->r, m->fontButton, valueText, grayColor, &tw, &th);
    
    // We'll scale the text to 70% of original size
    int smallerWidth = (int)(tw * 0.7);
    int smallerHeight = (int)(th * 0.7);
    
    // Position it right after the end of the track, centered vertically
    dst.x = s->track.x + s->track.w + 10;
    dst.y = s->track.y + (s->track.h - smallerHeight) / 2;
    dst.w = smallerWidth;
    dst.h = smallerHeight;
    
    SDL_RenderCopy(m->r, t, NULL, &dst);
    SDL_DestroyTexture(t);
}

static void renderSettings(Menu *m, SDL_Color white)
{
    // Draw panel background
    int winW, winH; 
    SDL_GetWindowSize(m->w, &winW, &winH);
    SDL_Rect panel = { winW / 2 - 250, winH / 2 - 200, 500, 400 };
    SDL_SetRenderDrawColor(m->r, 30, 30, 45, 255);
    SDL_RenderFillRect(m->r, &panel);
    
    // Draw title
    int tw, th;
    SDL_Texture *t = renderText(m->r, m->fontButton, "AUDIO SETTINGS", white, &tw, &th);
    SDL_Rect dst = { winW / 2 - tw / 2, panel.y + 20, tw, th };
    SDL_RenderCopy(m->r, t, NULL, &dst);
    SDL_DestroyTexture(t);
    
    // Draw sliders
    renderSlider(m, &m->musicSlider, white);
    renderSlider(m, &m->sfxSlider, white);
    
    // Draw back button
    bool hov = m->hoverSettings;
    SDL_SetRenderDrawColor(m->r,
                          hov ? 60 : 40,
                          hov ? 60 : 40,
                          hov ? 80 : 60, 255);
    SDL_RenderFillRect(m->r, &m->backBtn.rect);
    
    t = renderText(m->r, m->fontButton, m->backBtn.label, white, &tw, &th);
    dst = (SDL_Rect){ m->backBtn.rect.x + (m->backBtn.rect.w - tw) / 2,
                      m->backBtn.rect.y + (m->backBtn.rect.h - th) / 2,
                      tw, th };
    SDL_RenderCopy(m->r, t, NULL, &dst);
    SDL_DestroyTexture(t);
}

void menuRender(Menu *m)
{
    SDL_SetRenderDrawColor(m->r, 20, 20, 30, 255);
    SDL_RenderClear(m->r);

    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color grey  = { 170, 170, 170, 255 };

    renderTitle(m, white);

    if (m->mode == MODE_MAIN) {
        renderMain(m, white);
    } else if (m->mode == MODE_JOIN) {
        renderJoin(m, white, grey);
    } else if (m->mode == MODE_SETTINGS) {
        renderSettings(m, white);
    }

    SDL_RenderPresent(m->r);
}

MenuChoice menuGetChoice(const Menu *m)  { return m->choice; }
const char *menuGetJoinIP(const Menu *m) { return m->ip; }
