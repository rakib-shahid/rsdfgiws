#include "TokenManager.h"
#include <FS.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include "Secrets.h"
#include <HTTPClient.h>
#include <Base64.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "SpotifyFunctions.h"
#include "WifiSetup.h"

String authCode;
String accessToken;
String refreshToken;
const char *authTokenFileName = "/authtoken.txt";
const char *tokenFileName = "/token.txt";
const char *accessTokenFileName = "/accesstoken.txt";
const char *refreshTokenFileName = "/refreshtoken.txt";

// Initialize file system
void initializeFileSystem()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("Failed to mount file system");
    }
    else
    {
        Serial.println("File system mounted");
    }
}

void readAccessToken(const char *filename)
{
    String tokenjson = getToken(tokenFileName);
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, tokenjson);

    if (error)
    {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        return;
    }

    for (JsonPair kv : doc.as<JsonObject>())
    {
        if (kv.key() == "access_token")
        {
            accessToken = kv.value().as<String>();
            saveToken(accessToken, accessTokenFileName);
        }
        else if (kv.key() == "refresh_token")
        {
            refreshToken = kv.value().as<String>();
            saveToken(refreshToken, refreshTokenFileName);
        }
    }
}

bool validateAuthCode()
{
    File file = SPIFFS.open(authTokenFileName, "r");
    if (!file)
    {
        return false;
    }

    String authCode = file.readString();
    file.close();

    if (authCode == "dummy_access_token" || authCode.length() == 0)
    {
        return false;
    }

    String accessResponse = exchangeCodeForToken(authCode);
    if (sizeof(accessResponse) == 0)
    {
        return false;
    }

    return true;
}

void saveToken(const String &token, const String fileName)
{
    File file = SPIFFS.open(fileName, "w");
    if (!file)
    {
        Serial.println("Failed to open token file for writing");
        return;
    }
    file.print(token);
    file.close();
}

String getToken(const String fileName)
{
    File file = SPIFFS.open(fileName, "r");
    if (!file)
    {
        Serial.println("Failed to open token file for reading");
        return "";
    }

    String token = file.readString();
    file.close();
    return token;
}

String exchangeCodeForToken(const String &code)
{

    String authHeader = "Basic " + base64::encode(String(CLIENT_ID) + ":" + String(CLIENT_SECRET));

    http.begin("https://accounts.spotify.com/api/token");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Authorization", authHeader);
    http.addHeader("Connection", "keep-alive");

    String postData = "grant_type=authorization_code&code=" + code + "&redirect_uri=" + String(REDIRECT_URI);

    int httpResponseCode = http.POST(postData);

    if (httpResponseCode != 400)
    {
        String response = http.getString();
        // Serial.println("Response Code: " + String(httpResponseCode));
        // Serial.println("Response: " + response);
        http.end();
        saveToken(response, tokenFileName);
        return response;
    }
    else
    {
        // Serial.println("Error Response Code: " + String(httpResponseCode));
        // String response = http.getString();
        // Serial.println("Response: " + response);
        http.end();
        String empty = "";
        return empty;
    }
}

void getAuthorizationCode()
{
    if (!MDNS.begin("esp32-callback-url"))
    {
        Serial.println("Error setting up MDNS responder!");
        while (1)
        {
            delay(1000);
        }
    }
    Serial.println("mDNS responder started");
    bool loggedIn = false;
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    String scopes = "user-read-playback-state%20user-modify-playback-state";
    String html = "<html><body><h1>Login to Spotify</h1>";
    html += "<a href=\"https://accounts.spotify.com/authorize?client_id=" + String(CLIENT_ID) + "&response_type=code&redirect_uri=" + String(REDIRECT_URI) + "&scope=" + scopes + "\">Login</a>";
    html += "</body></html>";
    // Serial.println("html page: " + html);
    request->send(200, "text/html", html); });

    server.on("/callback", HTTP_GET, [&loggedIn](AsyncWebServerRequest *request)
              {
            if (request->hasParam("code")) {
                String code = request->getParam("code")->value();
                // Serial.println("Authorization Code: " + code);
                // saveToken(code);
                saveToken(code, authTokenFileName);
                loggedIn = true;
                request->send(200, "text/plain", "Login successful! You can now close this window.");
                tft.fillScreen(TFT_BLACK);
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                tft.setCursor(0, 10);
                tft.println("Login successful!\nToken saved.");
                // return;
            } else {
                request->send(400, "text/plain", "Authorization code not found!");
            } });

    // Start the server
    server.begin();
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 30);
    tft.print("Go to ");
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print(WiFi.localIP().toString());
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print("\non another device\nto login to Spotify");
    while (!loggedIn)
    {
        delay(100);
    }
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

bool isAccessTokenValid(const String &accessToken)
{
    if (accessToken.length() == 0)
    {
        return false;
    }
    http.begin("https://api.spotify.com/v1/me/player/currently-playing");

    // Set the Authorization header with the access token
    http.addHeader("Authorization", "Bearer " + accessToken);
    http.addHeader("Connection", "keep-alive");
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200)
    {
        // Token is valid
        http.end();
        return true;
    }
    else if (httpResponseCode == 401)
    {
        // Token is invalid or expired (401 Unauthorized)
        http.end();
        return false;
    }
    else
    {
        http.end();
        // Serial.println("Unexpected response code: " + String(httpResponseCode));
        ;
    }
    return false;
}

bool isRefreshTokenValid()
{
    // HTTPClient http;
    http.begin("https://accounts.spotify.com/api/token");
    // headers
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("Connection", "keep-alive");
    // body
    String postBody = "grant_type=refresh_token";
    postBody += "&refresh_token=" + refreshToken;
    postBody += "&client_id=" + String(CLIENT_ID);
    postBody += "&client_secret=" + String(CLIENT_SECRET);

    int httpResponseCode = http.POST(postBody);

    if (httpResponseCode == 200)
    {
        http.end();
        return true;
    }
    else if (httpResponseCode == 400)
    {
        http.end();
        return false;
    }
    else
    {
        // Serial.println("Unexpected response code: " + String(httpResponseCode));
        ;
    }

    http.end();
    return false;
}