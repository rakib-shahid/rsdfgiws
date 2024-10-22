#include "SpotifyFunctions.h"
#include "TokenManager.h"
#include "TFTSetup.h"
#include "Secrets.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>

// declare all global variables
// String songName;
// String artistName;
// bool isPlaying;
// int progressMs;
// int durationMs;
// int volume = 50;
// String albumArtUrl;
HTTPClient http;

// global struct for song data
SpotifyData spotifyData = {"", "", false, 0, 0, 0, ""};
