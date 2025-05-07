#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "../include/text_renderer.h"

struct TextRenderer {
    SDL_Renderer* renderer;
    TTF_Font* gameFont;      // Main game font (blocky/bold for GAME OVER)
    // Add more fonts as needed for different text styles
};

TextRenderer* createTextRenderer(SDL_Renderer* renderer) {
    // Initialize SDL_ttf if not already done
    if (!TTF_WasInit() && TTF_Init() < 0) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        return NULL;
    }
    
    TextRenderer* textRenderer = (TextRenderer*)malloc(sizeof(TextRenderer));
    if (!textRenderer) {
        printf("Memory allocation error for TextRenderer\n");
        return NULL;
    }
    
    textRenderer->renderer = renderer;
    textRenderer->gameFont = NULL;
    
    // Load the fonts
    if (!loadFonts(textRenderer)) {
        free(textRenderer);
        return NULL;
    }
    
    return textRenderer;
}

bool loadFonts(TextRenderer* renderer) {
    // Try to load Impact font (blocky, bold font perfect for GAME OVER)
    renderer->gameFont = TTF_OpenFont("C:/Windows/Fonts/impact.ttf", 96);
    
    if (!renderer->gameFont) {
        printf("Impact Font Error: %s\n", TTF_GetError());
        
        // Try Consolas Bold as fallback (also has rectangular appearance)
        renderer->gameFont = TTF_OpenFont("C:/Windows/Fonts/consolab.ttf", 96);
        
        if (!renderer->gameFont) {
            printf("Consolas Bold Font Error: %s\n", TTF_GetError());
            
            // Last resort - Arial
            renderer->gameFont = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 96);
            
            if (!renderer->gameFont) {
                printf("Cannot load any system fonts: %s\n", TTF_GetError());
                return false;
            }
        }
    }
    
    return true;
}

void renderGameOverText(TextRenderer* renderer, int screenWidth, int screenHeight) {
    if (!renderer || !renderer->gameFont) {
        return;
    }
    
    // Draw text with black outline effect for better visibility
    SDL_Color outlineColor = {0, 0, 0, 255};    // Black for outline
    SDL_Color textColor = {255, 0, 0, 255};     // Bright red for main text
    
    // Create the main text surface
    SDL_Surface* textSurface = TTF_RenderText_Blended(renderer->gameFont, "GAME OVER", textColor);
    
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer->renderer, textSurface);
        
        if (textTexture) {
            SDL_Rect textRect;
            textRect.w = textSurface->w;
            textRect.h = textSurface->h;
            textRect.x = (screenWidth - textRect.w) / 2;    // Center horizontally
            textRect.y = (screenHeight - textRect.h) / 2;   // Center vertically
            
            // Draw black outline using 4 slightly offset copies
            const int outlineSize = 4; // Thickness of outline
            
            // Create outline text
            SDL_Surface* outlineSurface = TTF_RenderText_Blended(renderer->gameFont, "GAME OVER", outlineColor);
            if (outlineSurface) {
                SDL_Texture* outlineTexture = SDL_CreateTextureFromSurface(renderer->renderer, outlineSurface);
                if (outlineTexture) {
                    SDL_Rect outlineRect = textRect;
                    
                    // Draw outline in multiple positions
                    // Top
                    outlineRect.y = textRect.y - outlineSize;
                    SDL_RenderCopy(renderer->renderer, outlineTexture, NULL, &outlineRect);
                    
                    // Bottom
                    outlineRect.y = textRect.y + outlineSize;
                    SDL_RenderCopy(renderer->renderer, outlineTexture, NULL, &outlineRect);
                    
                    // Left
                    outlineRect.y = textRect.y;
                    outlineRect.x = textRect.x - outlineSize;
                    SDL_RenderCopy(renderer->renderer, outlineTexture, NULL, &outlineRect);
                    
                    // Right
                    outlineRect.x = textRect.x + outlineSize;
                    SDL_RenderCopy(renderer->renderer, outlineTexture, NULL, &outlineRect);
                    
                    SDL_DestroyTexture(outlineTexture);
                }
                SDL_FreeSurface(outlineSurface);
            }
            
            // Draw the main red text on top
            SDL_RenderCopy(renderer->renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }
}

void destroyTextRenderer(TextRenderer* renderer) {
    if (renderer) {
        if (renderer->gameFont) {
            TTF_CloseFont(renderer->gameFont);
        }
        
        // Don't quit TTF here since other parts of the game might be using it
        // Let the main shutdown function handle TTF_Quit
        
        free(renderer);
    }
} 