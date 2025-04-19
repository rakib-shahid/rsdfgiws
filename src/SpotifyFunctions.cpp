#include "SpotifyFunctions.h"
#include "TokenManager.h"
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
// HTTPClient http;

// global struct for song data
SpotifyData spotifyData = {"", "", false, 0, 0, 0, ""};
SpotifyData lastSpotifyData = {"", "", false, 0, 0, 0, ""};
SemaphoreHandle_t spotifyMutex;
HTTPClient playingTrackHttpClient;

void spotifyTask(void *parameter)
{
    while (true)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            if (xSemaphoreTake(spotifyMutex, 1000 / portTICK_PERIOD_MS))
            {
                getCurrentlyPlayingTrack(accessToken);
                xSemaphoreGive(spotifyMutex);
            }
        }

        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
}

void startSpotifyTask()
{
    spotifyMutex = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(
        spotifyTask,    // function
        "Spotify Task", // name
        8192,           // stack size
        NULL,           // param
        1,              // priority
        NULL,           // handle
        1               // core ID (run on core 1)
    );
}

bool getCurrentlyPlayingTrack(const String &accessToken)
{
#ifdef ENABLE_CURRENTTRACK_TIMING
    unsigned long start = millis();
#endif
    // HTTPClient http;
    playingTrackHttpClient.begin("https://api.spotify.com/v1/me/player/currently-playing");
    playingTrackHttpClient.addHeader("Authorization", "Bearer " + accessToken);
    playingTrackHttpClient.addHeader("Connection", "keep-alive");
    int httpResponseCode = playingTrackHttpClient.GET();
    if (httpResponseCode == 200)
    {
        String response = playingTrackHttpClient.getString();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, response);

        if (error)
        {
            Serial.print("JSON deserialization failed: ");
            Serial.println(error.c_str());
#ifdef ENABLE_CURRENTTRACK_TIMING
            Serial.printf("[TIMER] getCurrentlyPlayingTrack runtime: %lu ms\n", millis() - start);
#endif
            return false;
        }

        // Update global variables
        spotifyData.name = doc["item"]["name"].as<String>();
        // NEED TO GET ALL ARTISTS INSTEAD OF JUST ONE
        // get list of artists by going through entire doc["item"]["artists"] array
        spotifyData.artist = doc["item"]["artists"][0]["name"].as<String>();
        spotifyData.is_playing = doc["is_playing"].as<bool>();
        spotifyData.progress_ms = doc["progress_ms"].as<int>();
        spotifyData.total_ms = doc["item"]["duration_ms"].as<int>();
        spotifyData.album_art_url = doc["item"]["album"]["images"][1]["url"].as<String>();
        // http.end();

#ifdef ENABLE_CURRENTTRACK_TIMING
        Serial.printf("[TIMER] getCurrentlyPlayingTrack runtime: %lu ms\n", millis() - start);
#endif
        return true;
    }
    else if (httpResponseCode == 401)
    {
        Serial.println("Access token expired. Refreshing token...");
        // http.end();
        refreshSpotifyTokens(refreshToken);
        // getCurrentlyPlayingTrack

#ifdef ENABLE_CURRENTTRACK_TIMING
        Serial.printf("[TIMER] getCurrentlyPlayingTrack runtime: %lu ms\n", millis() - start);
#endif
        return true;
    }
    else if (httpResponseCode == 204)
    {
        // http.end();

#ifdef ENABLE_CURRENTTRACK_TIMING
        Serial.printf("[TIMER] getCurrentlyPlayingTrack runtime: %lu ms\n", millis() - start);
#endif
        return true;
    }
    else
    {
        Serial.println("Failed to get currently playing track. HTTP response code: " + String(httpResponseCode));
        Serial.println("response: " + playingTrackHttpClient.getString());
        // http.end();

#ifdef ENABLE_CURRENTTRACK_TIMING
        Serial.printf("[TIMER] getCurrentlyPlayingTrack runtime: %lu ms\n", millis() - start);
#endif
        return false;
    }
}

void refreshSpotifyTokens(const String &oldRefreshToken)
{
    HTTPClient http;
    http.begin("https://accounts.spotify.com/api/token");
    const char *client_id = CLIENT_ID;
    const char *client_secret = CLIENT_SECRET;
    String body = "grant_type=refresh_token";
    body += "&refresh_token=" + oldRefreshToken;
    body += "&client_id=" + String(client_id);
    body += "&client_secret=" + String(client_secret);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Connection", "keep-alive");
    http.setReuse(true);
    int httpResponseCode = http.POST(body);
    if (httpResponseCode == 200)
    {
        String response = http.getString();
        JsonDocument doc;
        deserializeJson(doc, response);
        String newAccessToken = doc["access_token"].as<String>();
        accessToken = newAccessToken;
        saveToken(newAccessToken, accessTokenFileName);
    }
    else
    {
        String response = http.getString();
        Serial.println("Failed to refresh tokens. HTTP response code: " + String(httpResponseCode));
        Serial.println(response);
    }

    http.end();
}

bool adjustVolume(const String &accessToken, int adjustment)
{
    HTTPClient http;
    int newVolume = spotifyData.volume + adjustment * 10;
    if (newVolume > 100)
        newVolume = 100;
    if (newVolume < 0)
        newVolume = 0;

    if (newVolume == spotifyData.volume)
    {
        return true;
    }

    spotifyData.volume = newVolume;

    String url = "https://api.spotify.com/v1/me/player/volume?volume_percent=" + String(newVolume);
    http.begin(url);
    http.addHeader("Authorization", "Bearer " + accessToken);
    http.addHeader("Content-Length", "0");
    http.addHeader("Connection", "keep-alive");
    http.setReuse(true);

    int httpResponseCode = http.PUT("");
    http.end();

    if (httpResponseCode == 204)
    {
        return true;
    }
    else
    {
        Serial.println("Failed to adjust volume. HTTP response code: " + String(httpResponseCode));
        Serial.println("Response: " + http.getString());
        return false;
    }
}

bool togglePlay(const String &accessToken, HTTPClient &http)
{
// HTTPClient http;
#ifdef ENABLE_TIMING
    unsigned long start = millis();
#endif
    String endpoint = "https://api.spotify.com/v1/me/player/play";
    http.begin(endpoint);
    http.addHeader("Authorization", "Bearer " + accessToken);
    http.addHeader("Content-Length", "0");
    http.addHeader("Connection", "keep-alive");
    http.setReuse(true);
    int httpResponseCode = http.PUT("");
    if (httpResponseCode == 204 || httpResponseCode == 200)
    {
// http.end();
#ifdef ENABLE_TIMING
        Serial.printf("[TIMER] togglePlay runtime: %lu ms\n", millis() - start);
#endif
        return true;
    }
    else
    {
        Serial.println("Failed to toggle play. HTTP response code: " + String(httpResponseCode));
#ifdef ENABLE_TIMING
        Serial.printf("[TIMER] togglePlay runtime: %lu ms\n", millis() - start);
#endif
        // http.end();
        return false;
    }
}
bool togglePause(const String &accessToken, HTTPClient &http)
{
// HTTPClient http;
#ifdef ENABLE_TIMING
    unsigned long start = millis();
#endif
    String endpoint = "https://api.spotify.com/v1/me/player/pause";
    http.begin(endpoint);
    http.addHeader("Authorization", "Bearer " + accessToken);
    http.addHeader("Content-Length", "0");
    http.addHeader("Connection", "keep-alive");
    http.setReuse(true);
    int httpResponseCode = http.PUT("");
    if (httpResponseCode == 204 || httpResponseCode == 200)
    {
// http.end();
#ifdef ENABLE_TIMING
        Serial.printf("[TIMER] togglePause runtime: %lu ms\n", millis() - start);
#endif
        return true;
    }
    else
    {
        Serial.println("Failed to toggle Pause. HTTP response code: " + String(httpResponseCode));
#ifdef ENABLE_TIMING
        Serial.printf("[TIMER] togglePause runtime: %lu ms\n", millis() - start);
#endif
        // http.end();
        return false;
    }
}
bool skipToNextTrack(const String &accessToken, HTTPClient &http)
{
// HTTPClient http;
#ifdef ENABLE_TIMING
    unsigned long start = millis();
#endif
    String endpoint = "https://api.spotify.com/v1/me/player/next";
    http.begin(endpoint);
    http.addHeader("Authorization", "Bearer " + accessToken);
    http.addHeader("Content-Length", "0");
    http.addHeader("Connection", "keep-alive");
    http.setReuse(true);
    int httpResponseCode = http.POST("");
    if (httpResponseCode == 204 || httpResponseCode == 200)
    {
// http.end();
#ifdef ENABLE_TIMING
        Serial.printf("[TIMER] skipToNextTrack runtime: %lu ms\n", millis() - start);
#endif
        return true;
    }
    else
    {
        Serial.println("Failed to skip to the next track. HTTP response code: " + String(httpResponseCode));
// http.end();
#ifdef ENABLE_TIMING
        Serial.printf("[TIMER] skipToNextTrack runtime: %lu ms\n", millis() - start);
#endif
        return false;
    }
}
bool skipToPreviousTrack(const String &accessToken, HTTPClient &http)
{
// HTTPClient http;
#ifdef ENABLE_TIMING
    unsigned long start = millis();
#endif
    String endpoint = "https://api.spotify.com/v1/me/player/previous";
    http.begin(endpoint);
    http.addHeader("Authorization", "Bearer " + accessToken);
    http.addHeader("Content-Length", "0");
    http.addHeader("Connection", "keep-alive");
    http.setReuse(true);
    int httpResponseCode = http.POST("");
    if (httpResponseCode == 204 || httpResponseCode == 200)
    {
// http.end();
#ifdef ENABLE_TIMING
        Serial.printf("[TIMER] skipToPreviousTrack runtime: %lu ms\n", millis() - start);
#endif
        return true;
    }
    else
    {
        Serial.println("Failed to skip to the previous track. HTTP response code: " + String(httpResponseCode));
// http.end();
#ifdef ENABLE_TIMING
        Serial.printf("[TIMER] skipToPreviousTrack runtime: %lu ms\n", millis() - start);
#endif
        return false;
    }
}

bool seekTo(int x, HTTPClient &http)
{
#ifdef ENABLE_TIMING
    unsigned long start = millis();
#endif
    int newProgress = (x * spotifyData.total_ms) / 470;

    String endpoint = "https://api.spotify.com/v1/me/player/seek?position_ms=" + String(newProgress);
    http.begin(endpoint);
    http.addHeader("Authorization", "Bearer " + accessToken);
    http.addHeader("Content-Length", "0");
    http.addHeader("Connection", "keep-alive");
    http.setReuse(true);
    int httpResponseCode = http.PUT("");
    if (httpResponseCode == 204 || httpResponseCode == 200)
    {
#ifdef ENABLE_TIMING
        Serial.printf("[TIMER] seekTo runtime: %lu ms\n", millis() - start);
#endif
        return true;
    }
    else
    {
        Serial.println("Failed to seek to the specified position. HTTP response code: " + String(httpResponseCode));
#ifdef ENABLE_TIMING
        Serial.printf("[TIMER] seekTo runtime: %lu ms\n", millis() - start);
#endif
        return false;
    }
}

bool hasSongChanged(const SpotifyData &current, const SpotifyData &previous)
{
    return current.name != previous.name || current.artist != previous.artist || current.is_playing != previous.is_playing;
}
bool hasProgressChanged(const SpotifyData &current, const SpotifyData &previous)
{
    return current.progress_ms != previous.progress_ms;
}
