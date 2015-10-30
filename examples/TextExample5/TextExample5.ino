// This example shows the use of EFFECT_CUSTOM_RC
// This allows you to monitor the return code of the UpdateText() call
// for more than just 0 or -1. It allows you to set a return code of 1-255
// when certain parts of the message reach the displayable area.

#include <FastLED.h>

#include <LEDMatrix.h>
#include <LEDText.h>
#include <FontRobotron.h>

// Change the next 6 defines to match your matrix type and size

#define LED_PIN        2
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B

#define MATRIX_WIDTH   16
#define MATRIX_HEIGHT  16
#define MATRIX_TYPE    HORIZONTAL_MATRIX

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

cLEDText DemoMsg;

const unsigned char TxtDemo[] = { EFFECT_SCROLL_LEFT
                                  EFFECT_FRAME_RATE "\x00"
                                  EFFECT_BACKGND_ERASE
                                  EFFECT_COLR_EMPTY
                                  EFFECT_CUSTOM_RC "\x01"
                                  "   HORIZONTAL   "
                                  EFFECT_CUSTOM_RC "\x02"
                                  "VERTICAL   "
                                  EFFECT_CUSTOM_RC "\x03"
                                  "DIAGONAL   " };

void setup()
{
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
  FastLED.setBrightness(255);
  FastLED.clear(true);
  delay(500);
  FastLED.showColor(CRGB::Red);
  delay(1000);
  FastLED.showColor(CRGB::Lime);
  delay(1000);
  FastLED.showColor(CRGB::Blue);
  delay(1000);
  FastLED.showColor(CRGB::White);
  delay(1000);
  FastLED.show();

  DemoMsg.SetFont(RobotronFontData);
  DemoMsg.Init(&leds, leds.Width(), DemoMsg.FontHeight() + 1, 0, 0);
  DemoMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
}


void loop()
{
  static uint8_t hue;
  static int Mode = 1;
  int16_t x, y;
  uint8_t h;
  int rc;

  h = hue;
  switch (Mode)
  {
    case 1:
      // ** Fill LED's with horizontal stripes
      for (y=0; y<leds.Height(); ++y)
      {
        leds.DrawLine(0, y, leds.Width() - 1, y, CHSV(h, 255, 255));
        h+=16;
      }
      break;
    case 2:
      // ** Fill LED's with vertical stripes
      for (x=0; x<leds.Width(); ++x)
      {
        leds.DrawLine(x, 0, x, leds.Height() - 1, CHSV(h, 255, 255));
        h+=16;
      }
      break;
    case 3:
      // ** Fill LED's with diagonal stripes
      for (x=0; x<(leds.Width()+leds.Height()); ++x)
      {
        leds.DrawLine(x - leds.Height(), leds.Height() - 1, x, 0, CHSV(h, 255, 255));
        h+=16;
      }
      break;
  }
  hue += 4;

  rc = DemoMsg.UpdateText();
  if (rc == -1)
  {
    DemoMsg.SetText((unsigned char *)TxtDemo, sizeof(TxtDemo) - 1);
    rc = DemoMsg.UpdateText();
  }
  if (rc > 0)
    Mode = rc;
  FastLED.show();
  delay(20);
}
