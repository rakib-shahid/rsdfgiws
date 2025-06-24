#pragma once

#include <Arduino.h>

struct PlayerData {
  String name;
  String artist;
  bool is_playing;
  int progress_ms;
  int total_ms;
  int volume;
  String album_art_url;
};
