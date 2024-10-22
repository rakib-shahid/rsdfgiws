#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

extern const char *authTokenFileName;
extern const char *tokenFileName;
extern const char *accessTokenFileName;
extern const char *refreshTokenFileName;
extern AsyncWebServer server;
extern String authCode;
extern String accessToken;
extern String refreshToken;

void readAccessToken(const char *filename);
void initializeFileSystem();
bool validateAuthCode();
bool isAccessTokenValid(const String &accessToken);
bool isRefreshTokenValid();
void saveToken(const String &token, const String fileName);
String getToken(const String fileName);
const char *exchangeCodeForToken(const String &code);
void getAuthorizationCode();

#endif // TOKENMANAGER_H
