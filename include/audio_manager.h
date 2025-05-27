#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <SDL.h>
#include <SDL_mixer.h>
#include <stdbool.h>

typedef struct
{
    Mix_Chunk *deathSound;

    Mix_Music *backgroundMusic;

    bool audioInitialized;
} AudioManager;

AudioManager *createAudioManager(void);

void destroyAudioManager(AudioManager *audio);

void playDeathSound(AudioManager *audio);

void playBackgroundMusic(AudioManager *audio);

void stopBackgroundMusic(void);

void setSoundVolume(int volume);

void setMusicVolume(int volume);

#endif