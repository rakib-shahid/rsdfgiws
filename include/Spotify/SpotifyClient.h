#pragma once

#include "Core/AppContext.h"
#include "Spotify/SpotifyTypes.h"
#include "Spotify/TokenManager.h"
#include "TFTSetup.h"
#include <HTTPClient.h>

extern struct PlayerData playerData;
extern struct PlayerData lastPlayerData;

// token hoopla
bool getCurrentlyPlayingTrack(TokenInfo &tokenInfo);
void refreshSpotifyTokens(TokenInfo &tokenInfo);

// playback functions
bool adjustVolume(const String &accessToken, int adjustment);
bool togglePlay(const String &accessToken, HTTPClient &http);
bool togglePause(const String &accessToken, HTTPClient &http);
bool skipToNextTrack(const String &accessToken, HTTPClient &http);
bool skipToPreviousTrack(const String &accessToken, HTTPClient &http);
bool seekTo(TokenInfo &tokenInfo, int x, HTTPClient &http);

// checking functions
bool hasSongChanged(const PlayerData &current, const PlayerData &previous);
bool hasProgressChanged(const PlayerData &current, const PlayerData &previous);

// for second core (to check current song in background)
#include <Arduino.h>

extern SemaphoreHandle_t spotifyMutex;
void startSpotifyTask(AppContext &appContext);
