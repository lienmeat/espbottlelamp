#include "lamputil.h"
#include "FastLED.h"

uint8_t* hue_complement(uint8_t hue, uint8_t* hues) {
  hues[0] = hue;
  hues[1] = hue + 127;
  return hues;
}

uint8_t* hue_split_complements(uint8_t hue, uint8_t* hues) {
  hues[0] = hue;
  hues[1] = hue + 110;
  hues[2] = hue + 144;
  return hues;
}

uint8_t* hue_triads(uint8_t hue, uint8_t* hues) {
  hues[0] = hue;
  hues[1] = hue + 85;
  hues[2] = hue + 170;
  return hues;
}

uint8_t* hue_tetrads(uint8_t hue, uint8_t* hues) {
  hues[0] = hue;
  hues[1] = hue + 64;
  hues[2] = hue + 127;
  hues[3] = hue + 191;
  return hues;
}

uint8_t* hue_analogous(uint8_t hue, uint8_t* hues) {
  hues[0] = hue;
  hues[1] = hue + 21;
  hues[2] = hue + 234;
  return hues;
}

CRGB hexToColor(String hex) {
  uint8_t r, g, b;
  char * t_hex = (char *) hex.c_str();
  char * ept;
  for(int i = 1; i < 8; i++) {
    t_hex[i-1] = t_hex[i];
  }
  unsigned long l_hex = strtol(t_hex, &ept, 16);
  r = (l_hex & 0xFF0000) >> 16;
  g = (l_hex & 0x00FF00) >> 8;
  b = (l_hex & 0x0000FF);
  return CRGB(r, g, b);
}

String colorToHex(CRGB color) {
  char hexout[8];
  sprintf(hexout,"#%02x%02x%02x", (char) color.r, (char) color.g, (char) color.b);
  return (String) hexout;
}
