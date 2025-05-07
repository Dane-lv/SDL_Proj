#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <stdbool.h>

// Audio manager structure
typedef struct {
    Mix_Music *backgroundMusic;
    Mix_Chunk *gameOverSound;
    bool initialized;
} AudioManager;

// Initialize the audio system
bool audioInit(AudioManager *audio);

// Load and play background music
bool loadBackgroundMusic(AudioManager *audio, const char *musicPath);

// Play background music (loops indefinitely)
void playBackgroundMusic(AudioManager *audio);

// Stop background music
void stopBackgroundMusic(AudioManager *audio);

// Load sound effects
bool loadGameOverSound(AudioManager *audio, const char *soundPath);

// Play game over sound effect
void playGameOverSound(AudioManager *audio);

// Clean up audio resources
void audioCleanup(AudioManager *audio);

#endif // AUDIO_MANAGER_H 