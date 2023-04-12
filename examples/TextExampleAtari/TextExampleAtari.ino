// Atari font example with classic spectrum cycling Atari logo

#define FASTLED_INTERNAL

#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDText.h>
#include "FontAtari.h"

#define LED_PIN        27
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B
#define MATRIX_WIDTH   8
#define MATRIX_HEIGHT  8
#define MATRIX_TYPE    HORIZONTAL_MATRIX


cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;
cLEDText ScrollingMsg;

unsigned char TxtDemo1[] = {EFFECT_HSV_AV "\x00\xff\xff\x80\xff\xff" " "}; //vertical spectrum
unsigned char TxtDemo2[] = {EFFECT_SCROLL_LEFT "  ATARI regular font    "};

int      counter = 0, c = 2;
boolean  logo    = true;


void setup()
{
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(255);

  // Change first char in seq to Atari logo (ASCII 127 DEL) in FontAtari.h
  TxtDemo1[7] = 127; 
  TxtDemo2[1] = 127; 
  
  ScrollingMsg.SetFont(AtariFontData);
  ScrollingMsg.Init(&leds, leds.Width(), ScrollingMsg.FontHeight() + 1, 0, 0);
  ScrollingMsg.SetText((unsigned char *)TxtDemo1, sizeof(TxtDemo1) - 1);
 // ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
}


void loop()
{
  if (logo) {
    // Bounce the HSV_AV spectrum
    TxtDemo1[1] = counter;
    TxtDemo1[4] = counter + 128;
    ScrollingMsg.SetText((unsigned char *)TxtDemo1, sizeof(TxtDemo1) - 1);

    counter += c;
    if (counter < 0) {
      counter = 0;
      c = 2;
      logo = false; // One spectrum bounce then scroll
      ScrollingMsg.SetText((unsigned char *)TxtDemo2, sizeof(TxtDemo2) - 1);
    }

    if (counter > 127) {
      counter = 127;
      c = -2;
    }
  }

  if (ScrollingMsg.UpdateText() == -1)
  {
    logo = true;
  }

  FastLED.delay(1000 / 20); //fps
}
