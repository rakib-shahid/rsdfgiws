#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

extern String authCode;

struct TokenInfo {
  const char *authTokenFileName;
  const char *tokenFileName;
  const char *accessTokenFileName;
  const char *refreshTokenFileName;
  bool loggedIn;
  AsyncWebServer *webserver;
  String authCode;
  String accessToken;
  String refreshToken;

  TokenInfo() : webserver(new AsyncWebServer(80)) {}
};

void readAccessToken(TokenInfo &tokenInfo);
void initializeFileSystem();
bool validateAuthCode(TokenInfo &tokenInfo);
bool isAccessTokenValid(TokenInfo &tokenInfo);
bool isRefreshTokenValid(TokenInfo &tokenInfo);
void saveToken(const String &token, const String fileName);
String getToken(const String fileName);
String exchangeCodeForToken(TokenInfo &tokenInfo, const String &code);
void getAuthorizationCode(TokenInfo &tokenInfo);
