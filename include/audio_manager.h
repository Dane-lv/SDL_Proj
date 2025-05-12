#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <stdbool.h>

typedef struct {
    Mix_Chunk *deathSound;
    // Add more sound effects as needed
    
    Mix_Music *backgroundMusic;
    // Add more music tracks as needed
    
    bool audioInitialized;
} AudioManager;

// Initialize the audio system and load resources
AudioManager* createAudioManager(void);

// Free all audio resources
void destroyAudioManager(AudioManager *audio);

// Play the death sound
void playDeathSound(AudioManager *audio);

// Play background music (looped)
void playBackgroundMusic(AudioManager *audio);

// Stop background music
void stopBackgroundMusic(void);

// Set volume for sound effects (0-128)
void setSoundVolume(int volume);

// Set volume for music (0-128)
void setMusicVolume(int volume);

#endif // AUDIO_MANAGER_H 