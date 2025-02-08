#include <FastLED.h>

// ***
// *** Define the pin for the RGB matrix.
// ***
#define RGB_MATRIX_PIN 12

// ***
// *** Define the size of the matrix. The LED strip is
// *** still wired as a single strip, but arranged in a
// *** matrix (square or rectangle).
// ***
#define MATRIX_ROWS     28
#define MATRIX_COLUMNS  14

#define NUM_LEDS        MATRIX_ROWS * MATRIX_COLUMNS
#define BRIGHTNESS      32
#define LED_TYPE        WS2812B
#define COLOR_ORDER     RGB
CRGB leds[NUM_LEDS];



#define UPDATES_PER_SECOND 20

void setup()
{

  Serial.println("Initializing matrix...");
  FastLED.addLeds<LED_TYPE, RGB_MATRIX_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  FastLED.clear(true);
}

int index(int x, int y) {
    if (y % 2 == 0) {
        return y * MATRIX_COLUMNS + x; // Normal order
    } else {
        return (y + 1) * MATRIX_COLUMNS - x - 1; // Reverse order
    }
}


uint16_t _offset = 0;
const int DIGIT_WIDTH = 8;
const int DIGIT_HEIGHT = 16;
const int offsetX = (MATRIX_COLUMNS - DIGIT_WIDTH) / 2;   // 3
const int offsetY = (MATRIX_ROWS - DIGIT_HEIGHT) / 2;   // 6

// Each segment is drawn in the digit coordinate space (0..7 in x, 0..15 in y)
// where the top-left corner of the digit is (0,0). Later, we add offsetX and offsetY.

// Segment A: top horizontal (from (1,0) to (6,0))
void drawSegmentA() {
  for (int x = 1; x <= 6; x++) {
    int xx = offsetX + x;
    int yy = offsetY + 0;
    leds[index(xx, yy)] = CRGB::White;
  }
}

// Segment B: top-right vertical (from (7,1) to (7,7))
void drawSegmentB() {
  for (int y = 1; y <= 7; y++) {
    int xx = offsetX + 7;
    int yy = offsetY + y;
    leds[index(xx, yy)] = CRGB::White;
  }
}

// Segment C: bottom-right vertical (from (7,8) to (7,14))
void drawSegmentC() {
  for (int y = 8; y <= 14; y++) {
    int xx = offsetX + 7;
    int yy = offsetY + y;
    leds[index(xx, yy)] = CRGB::White;
  }
}

// Segment D: bottom horizontal (from (1,15) to (6,15))
void drawSegmentD() {
  for (int x = 1; x <= 6; x++) {
    int xx = offsetX + x;
    int yy = offsetY + 15;
    leds[index(xx, yy)] = CRGB::White;
  }
}

// Segment E: bottom-left vertical (from (0,8) to (0,14))
void drawSegmentE() {
  for (int y = 8; y <= 14; y++) {
    int xx = offsetX + 0;
    int yy = offsetY + y;
    leds[index(xx, yy)] = CRGB::White;
  }
}

// Segment F: top-left vertical (from (0,1) to (0,7))
void drawSegmentF() {
  for (int y = 1; y <= 7; y++) {
    int xx = offsetX + 0;
    int yy = offsetY + y;
    leds[index(xx, yy)] = CRGB::White;
  }
}

// Segment G: middle horizontal (from (1,8) to (6,8))
void drawSegmentG() {
  for (int x = 1; x <= 6; x++) {
    int xx = offsetX + x;
    int yy = offsetY + 8;
    leds[index(xx, yy)] = CRGB::White;
  }
}

// Draw the digit (0-9) by lighting the proper segments.
// (For this countdown we only need digits 9 to 1.)
void drawDigit(int digit) {
  // Clear the display
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  
  // Use the seven-segment mapping:
  switch (digit) {
    case 9:
      // Segments: A, B, C, D, F, G
      drawSegmentA();
      drawSegmentB();
      drawSegmentC();
      drawSegmentD();
      drawSegmentF();
      drawSegmentG();
      break;
    case 8:
      // Segments: A, B, C, D, E, F, G
      drawSegmentA();
      drawSegmentB();
      drawSegmentC();
      drawSegmentD();
      drawSegmentE();
      drawSegmentF();
      drawSegmentG();
      break;
    case 7:
      // Segments: A, B, C
      drawSegmentA();
      drawSegmentB();
      drawSegmentC();
      break;
    case 6:
      // Segments: A, C, D, E, F, G
      drawSegmentA();
      drawSegmentC();
      drawSegmentD();
      drawSegmentE();
      drawSegmentF();
      drawSegmentG();
      break;
    case 5:
      // Segments: A, C, D, F, G
      drawSegmentA();
      drawSegmentC();
      drawSegmentD();
      drawSegmentF();
      drawSegmentG();
      break;
    case 4:
      // Segments: B, C, F, G
      drawSegmentB();
      drawSegmentC();
      drawSegmentF();
      drawSegmentG();
      break;
    case 3:
      // Segments: A, B, C, D, G
      drawSegmentA();
      drawSegmentB();
      drawSegmentC();
      drawSegmentD();
      drawSegmentG();
      break;
    case 2:
      // Segments: A, B, D, E, G
      drawSegmentA();
      drawSegmentB();
      drawSegmentD();
      drawSegmentE();
      drawSegmentG();
      break;
    case 1:
      // Segments: B, C
      drawSegmentB();
      drawSegmentC();
      break;
    case 0:
      // For 0, light all segments except G.
      drawSegmentA();
      drawSegmentB();
      drawSegmentC();
      drawSegmentD();
      drawSegmentE();
      drawSegmentF();
    default:
      // For any unsupported digit, do nothing.
      break;
  }
  
  FastLED.show();
}
void loop()
{
    for (int i = 9; i >= 0; i--) {
        drawDigit(i);
        delay(1000);  // Wait 1 second
    }
   
  
}