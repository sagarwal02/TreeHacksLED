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
#define BRIGHTNESS      64
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

const char* catBitmap[MATRIX_ROWS] = {
  "00000000000000",  // Row 0 (empty)
  "00000000000000",  // Row 1 (empty)
  "00010000001000",  // Row 2: head top (ear tips at col 3 and col 10)
  "00111000111000",  // Row 3: expanded ears
  "01111101111100",  // Row 4: upper face (body of head)
  "01111111111110",  // Row 5: head full
  "00111111111100",  // Row 6: chin/neck
  "00111111111100",  // Row 7: beginning of body
  "00111111111100",  // Row 8
  "00111111111100",  // Row 9
  "00111111111100",  // Row 10
  "00011111111000",  // Row 11: body tapers slightly
  "00011111111000",  // Row 12
  "00011111111000",  // Row 13
  "00011111111000",  // Row 14
  "00011111111000",  // Row 15
  "00001111110000",  // Row 16: lower body/legs
  "00001111110000",  // Row 17
  "00001111110000",  // Row 18
  "00001111110000",  // Row 19
  "00001111110000",  // Row 20
  "00000111111000",  // Row 21: start of tail region
  "00000011111000",  // Row 22: tail shifts right
  "00000001111000",  // Row 23: tail thins out
  "00000000111000",  // Row 24: tail tapering
  "00000000111000",  // Row 25
  "00000000111000",  // Row 26
  "00000000111000"   // Row 27
};

const char* treeBitmap[MATRIX_ROWS] = {
  "00000000000000",  // Row 0: Blank
  "00000000000000",  // Row 1: Blank
  
  // CANOPY (Rows 2–17)
  "00000LLLL00000",  // Row 2:  4 L's (5 off, 4 L, 5 off)
  "0000LLLLLL0000",  // Row 3:  6 L's (4 off, 6 L, 4 off)
  "000LLLLLLLL000",  // Row 4:  8 L's (3 off, 8 L, 3 off)
  "00LLLLLLLLLL00",  // Row 5: 10 L's (2 off, 10 L, 2 off)
  "0LLLLLLLLLLLL0",  // Row 6: 12 L's (1 off, 12 L, 1 off)
  "LLLLLLLLLLLLLL",  // Row 7: Full 14 L's
  "LLLLLLLLLLLLLL",  // Row 8: Full 14 L's
  "LLLLLLLLLLLLLL",  // Row 9: Full 14 L's
  "LLLLLLLLLLLLLL",  // Row 10: Full 14 L's
  "LLLLLLLLLLLLLL",  // Row 11: Full 14 L's
  "0LLLLLLLLLLLL0",  // Row 12: 12 L's
  "00LLLLLLLLLL00",  // Row 13: 10 L's
  "000LLLLLLLL000",  // Row 14:  8 L's
  "0000LLLLLL0000",  // Row 15:  6 L's
  "00000LLLL00000",  // Row 16:  4 L's
  "00000LLLL00000",  // Row 17:  4 L's
  
  // TRUNK (Rows 18–27)
  "00000TTTT00000",  // Row 18
  "00000TTTT00000",  // Row 19
  "00000TTTT00000",  // Row 20
  "00000TTTT00000",  // Row 21
  "00000TTTT00000",  // Row 22
  "00000TTTT00000",  // Row 23
  "00000TTTT00000",  // Row 24
  "00000TTTT00000",  // Row 25
  "00000TTTT00000",  // Row 26
  "00000TTTT00000"   // Row 27
};


void loop()
{
    for (int i = 9; i >= 0; i--) {
        drawDigit(i);
        delay(1000);  // Wait 1 second
    }
    // CRGB leafColor = CRGB::Green;
    // CRGB trunkColor = CRGB::SaddleBrown; // A brown tone

    // // Draw the tree: iterate over each row and column.
    // for (int y = 0; y < MATRIX_ROWS; y++) {
    //   for (int x = 0; x < MATRIX_COLUMNS; x++) {
    //     char pixel = treeBitmap[y][x];
    //     if (pixel == 'L') {
    //       leds[index(x, y)] = leafColor;
    //     } else if (pixel == 'T') {
    //       leds[index(x, y)] = trunkColor;
    //     } else {
    //       leds[index(x, y)] = CRGB::Black;
    //     }
    //   }
    // }
    
    FastLED.show();
   
  
}