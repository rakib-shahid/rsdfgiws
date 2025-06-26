#include "Spotify/TokenManager.h"
#include "Secrets.h"
#include "Spotify/SpotifyClient.h"
#include "WifiSetup.h"
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <TFT_eSPI.h>
#include <base64.h>

String authCode;
// String accessToken;
// String refreshToken;
// const char *authTokenFileName = "/authtoken.txt";
// const char *tokenFileName = "/token.txt";
// const char *accessTokenFileName = "/accesstoken.txt";
// const char *refreshTokenFileName = "/refreshtoken.txt";

// Initialize file system
void initializeFileSystem() {
  if (!SPIFFS.begin(true)) {
    Serial.println("[SPIFFS] Failed to mount file system");
  } else {
    Serial.println("[SPIFFS] File system mounted");
  }
}

void readAccessToken(TokenInfo &tokenInfo) {
  String tokenjson = getToken(tokenInfo.tokenFileName);
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, tokenjson);

  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  for (JsonPair kv : doc.as<JsonObject>()) {
    if (kv.key() == "access_token") {
      tokenInfo.accessToken = kv.value().as<String>();
      saveToken(tokenInfo.accessToken, tokenInfo.accessTokenFileName);
    } else if (kv.key() == "refresh_token") {
      tokenInfo.refreshToken = kv.value().as<String>();
      saveToken(tokenInfo.refreshToken, tokenInfo.refreshTokenFileName);
    }
  }
}

bool validateAuthCode(TokenInfo &tokenInfo) {
  File file = SPIFFS.open(tokenInfo.authTokenFileName, "r");
  if (!file) {
    return false;
  }

  String authCode = file.readString();
  file.close();

  if (authCode == "dummy_access_token" || authCode.length() == 0) {
    return false;
  }

  String accessResponse = exchangeCodeForToken(tokenInfo, authCode);
  if (sizeof(accessResponse) == 0) {
    return false;
  }

  return true;
}

void saveToken(const String &token, const String fileName) {
  File file = SPIFFS.open(fileName, "w");
  if (!file) {
    Serial.println("Failed to open token file for writing");
    return;
  }
  file.print(token);
  file.close();
}

String getToken(const String fileName) {
  File file = SPIFFS.open(fileName, "r");
  if (!file) {
    Serial.println("Failed to open token file for reading");
    return "";
  }

  String token = file.readString();
  file.close();
  return token;
}

String exchangeCodeForToken(TokenInfo &tokenInfo, const String &code) {
  HTTPClient http;
  String authHeader = "Basic " + base64::encode(String(CLIENT_ID) + ":" +
                                                String(CLIENT_SECRET));

  http.begin("https://accounts.spotify.com/api/token");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Authorization", authHeader);
  http.addHeader("Connection", "keep-alive");

  String postData = "grant_type=authorization_code&code=" + code +
                    "&redirect_uri=" + String(REDIRECT_URI);

  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.printf("[DEBUG] Spotify token exchange HTTP %d\n", httpResponseCode);
    Serial.println("[DEBUG] Token exchange response:");
    Serial.println(response);

    if (httpResponseCode == 200) {
      saveToken(response, tokenInfo.tokenFileName);
      http.end();
      return response;
    } else {
      Serial.println("[ERROR] Token exchange failed, not saving.");
    }
  } else {
    Serial.printf(
        "[ERROR] Failed to connect to Spotify token endpoint. Code: %d\n",
        httpResponseCode);
  }
  http.end();
  return "";
}

void getAuthorizationCode(TokenInfo &tokenInfo) {
  if (!MDNS.begin("esp32")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("[mDNS] esp32.local is now active");

  tokenInfo.webserver->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("[HTTP GET /] Serving login page...");
    String scopes = "user-read-playback-state%20user-modify-playback-state%"
                    "20user-read-currently-playing";
    String html = "<html><body><h1>rsdfgiws</h1><h3>Log in to spotify using "
                  "the link below!</h3>";
    // String encodedRedirectURI = "http%3A%2F%2Fesp32.local%2Fcallback";
    html += "<a href=\"https://accounts.spotify.com/authorize?client_id=" +
            String(CLIENT_ID) +
            "&response_type=code&redirect_uri=" + String(REDIRECT_URI) +
            "&scope=" + scopes + "\">Login Here</a>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  tokenInfo.webserver->on(
      "/callback", HTTP_GET, [&tokenInfo](AsyncWebServerRequest *request) {
        Serial.println("[HTTP GET /callback] Callback triggered");
        if (request->hasParam("code")) {
          String code = request->getParam("code")->value();
          Serial.println("[Callback] Authorization code received:");
          Serial.println(code);

          saveToken(code, tokenInfo.authTokenFileName);
          tokenInfo.loggedIn = true;

          request->send(200, "text/plain",
                        "Login successful! You can now close this window.");

          tft.fillScreen(TFT_BLACK);
          tft.setTextColor(TFT_GREEN, TFT_BLACK);
          tft.setCursor(0, 10);
          tft.println("Login successful!\nToken saved.");
        } else {
          Serial.println("[Callback] Authorization code NOT found!");
          request->send(400, "text/plain", "Authorization code not found!");
        }
      });

  tokenInfo.webserver->begin();
  Serial.println("[Webserver] Started and awaiting login");

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 30);
  tft.print("Go to ");
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.print("http://");
  tft.print(WiFi.localIP().toString());
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.print("\non another device\nto login to Spotify");
  Serial.println("[Login] Waiting for login");
  while (!tokenInfo.loggedIn) {
    delay(100);
  }

  Serial.println("[Login] Completed, token saved.");

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

bool isAccessTokenValid(TokenInfo &tokenInfo) {
  Serial.print("[DEBUG] Access token string: [");
  Serial.print(tokenInfo.accessToken);
  Serial.println("]");

  HTTPClient http;
  if (tokenInfo.accessToken.length() == 0) {
    return false;
  }
  http.begin("https://api.spotify.com/v1/me/player/currently-playing");

  // Set the Authorization header with the access token
  http.addHeader("Authorization", "Bearer " + tokenInfo.accessToken);
  http.addHeader("Connection", "keep-alive");
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    // Token is valid
    http.end();
    return true;
  } else if (httpResponseCode == 401) {
    // Token is invalid or expired (401 Unauthorized)
    http.end();
    return false;
  } else {
    http.end();
    // Serial.println("Unexpected response code: " + String(httpResponseCode));
    ;
  }
  return false;
}

bool isRefreshTokenValid(TokenInfo &tokenInfo) {
  HTTPClient http;
  http.begin("https://accounts.spotify.com/api/token");
  // headers
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Connection", "keep-alive");
  // body
  String postBody = "grant_type=refresh_token";
  postBody += "&refresh_token=" + tokenInfo.refreshToken;
  postBody += "&client_id=" + String(CLIENT_ID);
  postBody += "&client_secret=" + String(CLIENT_SECRET);

  int httpResponseCode = http.POST(postBody);

  if (httpResponseCode == 200) {
    http.end();
    return true;
  } else if (httpResponseCode == 400) {
    http.end();
    return false;
  } else {
    // Serial.println("Unexpected response code: " + String(httpResponseCode));
    ;
  }

  http.end();
  return false;
}
