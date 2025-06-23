#pragma once

#include "Spotify/TokenManager.h"
#include "TFTSetup.h"
#include <HTTPClient.h>

struct SpotifyData {
  String name;
  String artist;
  bool is_playing;
  int progress_ms;
  int total_ms;
  int volume;
  String album_art_url;
};
extern struct SpotifyData spotifyData;
extern struct SpotifyData lastSpotifyData;

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
bool hasSongChanged(const SpotifyData &current, const SpotifyData &previous);
bool hasProgressChanged(const SpotifyData &current,
                        const SpotifyData &previous);

// for second core (to check current song in background)
#include <Arduino.h>

extern SemaphoreHandle_t spotifyMutex;
void startSpotifyTask(TokenInfo &tokenInfo);
