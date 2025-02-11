#include <Adafruit_GFX.h>
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>

// Choose your prefered pixmap
#include "heart24.h"
#include "google32.h"

#define PIN 12

// Max is 255
#define BRIGHTNESS 16

#define MATRIX_COLUMNS 14
#define MATRIX_ROWS 28
#define PANEL_COUNT 3

#define WIDTH MATRIX_COLUMNS * PANEL_COUNT
#define LED_COUNT WIDTH * MATRIX_ROWS

int mw = WIDTH;
int mh = MATRIX_ROWS;

CRGB leds[LED_COUNT];
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, mw, mh,
                                                  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG);

// This could also be defined as matrix->color(255,0,0) but those defines
// are meant to work for adafruit_gfx backends that are lacking color()
#define LED_BLACK 0

#define LED_RED_VERYLOW (3 << 11)
#define LED_RED_LOW (7 << 11)
#define LED_RED_MEDIUM (15 << 11)
#define LED_RED_HIGH (31 << 11)

#define LED_GREEN_VERYLOW (1 << 5)
#define LED_GREEN_LOW (15 << 5)
#define LED_GREEN_MEDIUM (31 << 5)
#define LED_GREEN_HIGH (63 << 5)

#define LED_BLUE_VERYLOW 3
#define LED_BLUE_LOW 7
#define LED_BLUE_MEDIUM 15
#define LED_BLUE_HIGH 31

#define LED_ORANGE_VERYLOW (LED_RED_VERYLOW + LED_GREEN_VERYLOW)
#define LED_ORANGE_LOW (LED_RED_LOW + LED_GREEN_LOW)
#define LED_ORANGE_MEDIUM (LED_RED_MEDIUM + LED_GREEN_MEDIUM)
#define LED_ORANGE_HIGH (LED_RED_HIGH + LED_GREEN_HIGH)

#define LED_PURPLE_VERYLOW (LED_RED_VERYLOW + LED_BLUE_VERYLOW)
#define LED_PURPLE_LOW (LED_RED_LOW + LED_BLUE_LOW)
#define LED_PURPLE_MEDIUM (LED_RED_MEDIUM + LED_BLUE_MEDIUM)
#define LED_PURPLE_HIGH (LED_RED_HIGH + LED_BLUE_HIGH)

#define LED_CYAN_VERYLOW (LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_CYAN_LOW (LED_GREEN_LOW + LED_BLUE_LOW)
#define LED_CYAN_MEDIUM (LED_GREEN_MEDIUM + LED_BLUE_MEDIUM)
#define LED_CYAN_HIGH (LED_GREEN_HIGH + LED_BLUE_HIGH)

#define LED_WHITE_VERYLOW (LED_RED_VERYLOW + LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_WHITE_LOW (LED_RED_LOW + LED_GREEN_LOW + LED_BLUE_LOW)
#define LED_WHITE_MEDIUM (LED_RED_MEDIUM + LED_GREEN_MEDIUM + LED_BLUE_MEDIUM)
#define LED_WHITE_HIGH (LED_RED_HIGH + LED_GREEN_HIGH + LED_BLUE_HIGH)

// Convert a BGR 4/4/4 bitmap to RGB 5/6/5 used by Adafruit_GFX
void fixdrawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h) {
  uint16_t RGB_bmp_fixed[w * h];
  for (uint16_t pixel = 0; pixel < w * h; pixel++) {
    uint8_t r, g, b;
    uint16_t color = pgm_read_word(bitmap + pixel);

    b = (color & 0xF00) >> 8;
    g = (color & 0x0F0) >> 4;
    r = color & 0x00F;

    // expand from 4/4/4 bits per color to 5/6/5
    b = map(b, 0, 15, 0, 31);
    g = map(g, 0, 15, 0, 63);
    r = map(r, 0, 15, 0, 31);

    RGB_bmp_fixed[pixel] = (r << 11) + (g << 5) + b;
  }
  matrix->drawRGBBitmap(x, y, RGB_bmp_fixed, w, h);
}

uint16_t customIndex(uint16_t x, uint16_t y) {
  // Find panel we're located at
  int panel = x / MATRIX_COLUMNS;
  int lx = x % MATRIX_COLUMNS;
  int ly = y;  // y is the same in every panel

  // Now, within a single panel the LEDs are wired in a "serpentine".
  int i;
  if (ly % 2 == 0) {
      i = ly * MATRIX_COLUMNS + lx;
  } else {
      i = (ly + 1) * MATRIX_COLUMNS - lx - 1;
  }
  
  // Finally, add an offset equal to the number of LEDs in all previous panels.
  return panel * (MATRIX_COLUMNS * MATRIX_ROWS) + i;
}


void loop() {
  matrix->clear();
  matrix->setTextWrap(false);
  matrix->setBrightness(BRIGHTNESS);
  
  // Set text color to white
  matrix->setTextColor(LED_RED_HIGH);
  
  // Set text size (adjust as needed for your matrix size)
  matrix->setTextSize(2);
  
  // Calculate position to center "36:00:00"
  int16_t x1, y1;
  uint16_t w, h;
  matrix->getTextBounds("36:00:00", 0, 0, &x1, &y1, &w, &h);
  int xpos = (mw - w) / 2;
  int ypos = (mh + h) / 2;
  
  // Draw the text
  matrix->setCursor(xpos, ypos);
  matrix->print("36:00:00");
  matrix->show();
  
  // Add a small delay to prevent flickering
  delay(100);
}

void setup() {
  matrix->setRemapFunction(customIndex);

  FastLED.addLeds<NEOPIXEL, PIN>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
  Serial.print("Setup serial: ");
  Serial.println(LED_COUNT);
}
