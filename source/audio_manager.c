#include <stdio.h>
#include "../include/audio_manager.h"

bool audioInit(AudioManager *audio) {
    // Initialize audio manager fields
    audio->backgroundMusic = NULL;
    audio->gameOverSound = NULL;
    audio->initialized = false;
    
    // Initialize SDL_mixer with audio rate, format, channels, and chunk size
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }
    
    audio->initialized = true;
    return true;
}

bool loadBackgroundMusic(AudioManager *audio, const char *musicPath) {
    if (!audio->initialized) {
        printf("Audio system not initialized\n");
        return false;
    }
    
    // Free any existing music
    if (audio->backgroundMusic != NULL) {
        Mix_FreeMusic(audio->backgroundMusic);
        audio->backgroundMusic = NULL;
    }
    
    // Load the music
    audio->backgroundMusic = Mix_LoadMUS(musicPath);
    if (audio->backgroundMusic == NULL) {
        printf("Failed to load background music! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }
    
    return true;
}

void playBackgroundMusic(AudioManager *audio) {
    if (!audio->initialized || audio->backgroundMusic == NULL) {
        return;
    }
    
    // Play the music, -1 means loop indefinitely
    if (Mix_PlayingMusic() == 0) {
        Mix_PlayMusic(audio->backgroundMusic, -1);
    }
}

void stopBackgroundMusic(AudioManager *audio) {
    if (!audio->initialized) {
        return;
    }
    
    if (Mix_PlayingMusic() != 0) {
        Mix_HaltMusic();
    }
}

bool loadGameOverSound(AudioManager *audio, const char *soundPath) {
    if (!audio->initialized) {
        printf("Audio system not initialized\n");
        return false;
    }
    
    // Free any existing sound
    if (audio->gameOverSound != NULL) {
        Mix_FreeChunk(audio->gameOverSound);
        audio->gameOverSound = NULL;
    }
    
    // Load the sound effect
    audio->gameOverSound = Mix_LoadWAV(soundPath);
    if (audio->gameOverSound == NULL) {
        printf("Failed to load game over sound! SDL_mixer Error: %s\n", Mix_GetError());
        return false;
    }
    
    return true;
}

void playGameOverSound(AudioManager *audio) {
    if (!audio->initialized || audio->gameOverSound == NULL) {
        return;
    }
    
    // Play the sound effect once on any available channel
    Mix_PlayChannel(-1, audio->gameOverSound, 0);
}

void audioCleanup(AudioManager *audio) {
    if (!audio->initialized) {
        return;
    }
    
    // Free the music
    if (audio->backgroundMusic != NULL) {
        Mix_FreeMusic(audio->backgroundMusic);
        audio->backgroundMusic = NULL;
    }
    
    // Free the sound effects
    if (audio->gameOverSound != NULL) {
        Mix_FreeChunk(audio->gameOverSound);
        audio->gameOverSound = NULL;
    }
    
    // Quit SDL_mixer
    Mix_CloseAudio();
    audio->initialized = false;
} 