#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include "../include/audio_manager.h"

#define DEATH_SOUND_PATH "resources/sfx/death.wav"
#define BACKGROUND_MUSIC_PATH "resources/backgroundMusic/background_music.mp3"
#define AUDIO_FREQUENCY 44100
#define AUDIO_FORMAT MIX_DEFAULT_FORMAT
#define AUDIO_CHANNELS 2
#define AUDIO_CHUNK_SIZE 2048

AudioManager* createAudioManager(void) {
    AudioManager *audio = (AudioManager*)malloc(sizeof(AudioManager));
    if (!audio) {
        printf("Failed to allocate memory for AudioManager\n");
        return NULL;
    }
    
    // Initialize to NULL
    audio->deathSound = NULL;
    audio->backgroundMusic = NULL;
    audio->audioInitialized = false;
    
    // Initialize SDL_mixer
    if (Mix_OpenAudio(AUDIO_FREQUENCY, AUDIO_FORMAT, AUDIO_CHANNELS, AUDIO_CHUNK_SIZE) < 0) {
        printf("SDL_mixer initialization failed: %s\n", Mix_GetError());
        free(audio);
        return NULL;
    }
    
    // Load sound effects
    audio->deathSound = Mix_LoadWAV(DEATH_SOUND_PATH);
    if (!audio->deathSound) {
        printf("Failed to load death sound: %s\n", Mix_GetError());
        // Continue anyway, just without this sound
    }
    
    // Load background music
    audio->backgroundMusic = Mix_LoadMUS(BACKGROUND_MUSIC_PATH);
    if (!audio->backgroundMusic) {
        printf("Failed to load background music: %s\n", Mix_GetError());
        // Continue anyway, just without music
    }
    
    // Set audio system as initialized
    audio->audioInitialized = true;
    
    return audio;
}

void destroyAudioManager(AudioManager *audio) {
    if (!audio) return;
    
    // Free sound effects
    if (audio->deathSound) {
        Mix_FreeChunk(audio->deathSound);
    }
    
    // Free music
    if (audio->backgroundMusic) {
        Mix_FreeMusic(audio->backgroundMusic);
    }
    
    // Close audio system
    Mix_CloseAudio();
    
    // Free memory
    free(audio);
}

void playDeathSound(AudioManager *audio) {
    if (!audio || !audio->audioInitialized || !audio->deathSound) return;
    
    // Play sound on the first available channel, no loops
    Mix_PlayChannel(-1, audio->deathSound, 0);
}

void playBackgroundMusic(AudioManager *audio) {
    if (!audio || !audio->audioInitialized || !audio->backgroundMusic) return;
    
    // If music is already playing, don't restart it
    if (Mix_PlayingMusic()) return;
    
    // Start playing music, loop indefinitely (-1)
    Mix_PlayMusic(audio->backgroundMusic, -1);
}

void stopBackgroundMusic(void) {
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }
}

void setSoundVolume(int volume) {
    // Ensure volume is in valid range (0-128)
    if (volume < 0) volume = 0;
    if (volume > 128) volume = 128;
    
    Mix_Volume(-1, volume); // -1 means all channels
}

void setMusicVolume(int volume) {
    // Ensure volume is in valid range (0-128)
    if (volume < 0) volume = 0;
    if (volume > 128) volume = 128;
    
    Mix_VolumeMusic(volume);
} 