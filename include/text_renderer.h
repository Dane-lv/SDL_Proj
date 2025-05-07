#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>

typedef struct TextRenderer TextRenderer;

// Initialize the text renderer
TextRenderer* createTextRenderer(SDL_Renderer* renderer);

// Load fonts for the text renderer
bool loadFonts(TextRenderer* renderer);

// Render "GAME OVER" text with outline
void renderGameOverText(TextRenderer* renderer, int screenWidth, int screenHeight);

// Add other text rendering functions as needed
// void renderScoreText(TextRenderer* renderer, int x, int y, int score);
// void renderStatusText(TextRenderer* renderer, int x, int y, const char* text);

// Clean up resources
void destroyTextRenderer(TextRenderer* renderer);

#endif // TEXT_RENDERER_H 