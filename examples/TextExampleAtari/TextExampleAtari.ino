// 8-bit Atari computer font example with classic spectrum-cycling logo
// D. Cerisano 2023

// Custom hardware settings.
#define FASTLED_INTERNAL
#define LED_PIN        27
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B
#define MATRIX_WIDTH   8
#define MATRIX_HEIGHT  8
#define MATRIX_TYPE    HORIZONTAL_MATRIX
#define FPS            20

// Include dependencies
#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include "FontAtari.h"

// Display structs
cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;
cLEDText ScrollingMsg;

// Sequence structs
unsigned char TxtDemo1[] = {EFFECT_HSV_AV "\x00\xff\xff\x80\xff\xff" " "}; //vertical spectrum
unsigned char TxtDemo2[] = {EFFECT_SCROLL_LEFT "  ATARI regular font    "};

// Demo variables
int      hue = 0, inc = 2;
boolean  logo = true;

void setup()
{
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(128);

  // Change first char in seq to custom Atari logo (ASCII 127 DEL) in FontAtari.h
  TxtDemo1[7] = 127;
  TxtDemo2[1] = 127;

  ScrollingMsg.SetFont(AtariFontData);
  ScrollingMsg.Init(&leds, leds.Width(), ScrollingMsg.FontHeight() + 1, 0, 0);
  ScrollingMsg.SetText((unsigned char *)TxtDemo1, sizeof(TxtDemo1) - 1);
}

void loop()
{ 
  // Bounce HSV_AV spectrum for the logo sequence then scroll the second sequence, ad infinitum
  
  if (logo) {
    TxtDemo1[1] = hue;
    TxtDemo1[4] = hue + 128;
    ScrollingMsg.SetText((unsigned char *)TxtDemo1, sizeof(TxtDemo1) - 1);
    hue += inc;

    // Bounce spectrum
    if (hue > 127) {
      hue = 127;
      inc = -2;
    }

    // Start the second sequence after logo bounce completes
    if (hue < 0) {
      hue = 0;
      inc = 2;
      logo = false;
      ScrollingMsg.SetText((unsigned char *)TxtDemo2, sizeof(TxtDemo2) - 1);
    }
  }

  // Restart demo after second sequence scrolls.
  if (ScrollingMsg.UpdateText() == -1)
    logo = true;

  // FastLED.delay() calls show() FPS times per second
  FastLED.delay(1000 / FPS); 
}
