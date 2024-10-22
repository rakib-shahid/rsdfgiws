#ifndef SPOTIFY_FUNCTIONS_H
#define SPOTIFY_FUNCTIONS_H

// #include <WiFi.h>
// #include <WebServer.h>
// #include <ESPmDNS.h>
// #include <SpotifyArduino.h>
// #include <SpotifyArduinoCert.h>

#include "TokenManager.h"
#include "TFTSetup.h"
#include <HTTPClient.h>

// global struct
struct SpotifyData
{
    String name;
    String artist;
    bool is_playing;
    int progress_ms;
    int total_ms;
    int volume;
    String album_art_url;
};
extern struct SpotifyData spotifyData;
extern HTTPClient http;

// bool getCurrentlyPlayingTrack(const String &accessToken);
// void refreshSpotifyTokens(const String &refreshToken);
// bool adjustVolume(const String &accessToken, int adjustment);
// bool togglePlay(const String &accessToken);
// bool togglePause(const String &accessToken);
// bool skipToNextTrack(const String &accessToken);
// bool skipToPreviousTrack(const String &accessToken);

#endif // SPOTIFY_FUNCTIONS_H
