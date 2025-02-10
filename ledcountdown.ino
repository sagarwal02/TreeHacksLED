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
#define PANEL_WIDTH 14
#define MATRIX_ROWS     28
#define MATRIX_COLUMNS  N*PANEL_WIDTH

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

// Map individual panels as singular board
int index(int x, int y) {
    // Find panel we're located at
    int panel = x / MATRIX_COLUMNS;
    int lx = x % MATRIX_COLUMNS;
    int ly = y;  // y is the same in every panel

    // In our wiring the panels are connected so that the bottom‐right LED
    // of one panel goes to the top–left of the next. To get a continuous
    // overall index we “flip” (rotate 180°) every other panel.
    if (panel & 1) {
        lx = MATRIX_COLUMNS - 1 - lx;
        ly = MATRIX_ROWS - 1 - ly;
    }

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

const uint8_t font5x7[][5] = {
  // ' ' (space)
  { 0x00, 0x00, 0x00, 0x00, 0x00 },
  // 'A'
  { 0x7E, 0x11, 0x11, 0x7E, 0x00 },
  // 'B'
  { 0x7F, 0x49, 0x49, 0x36, 0x00 },
  // 'C'
  { 0x3E, 0x41, 0x41, 0x22, 0x00 },
  // 'D'
  { 0x7F, 0x41, 0x41, 0x3E, 0x00 },
  // 'E'
  { 0x7F, 0x49, 0x49, 0x41, 0x00 },
  // 'F'
  { 0x7F, 0x09, 0x09, 0x01, 0x00 },
  // 'G'
  { 0x3E, 0x41, 0x51, 0x32, 0x00 },
  // 'H'
  { 0x7F, 0x08, 0x08, 0x7F, 0x00 },
  // 'I'
  { 0x00, 0x41, 0x7F, 0x41, 0x00 },
  // 'J'
  { 0x20, 0x40, 0x41, 0x3F, 0x00 },
  // 'K'
  { 0x7F, 0x08, 0x14, 0x63, 0x00 },
  // 'L'
  { 0x7F, 0x40, 0x40, 0x40, 0x00 },
  // 'M'
  { 0x7F, 0x06, 0x06, 0x7F, 0x00 },  // (a rough approximation)
  // 'N'
  { 0x7F, 0x06, 0x18, 0x7F, 0x00 },  // (a rough approximation)
  // 'O'
  { 0x3E, 0x41, 0x41, 0x3E, 0x00 },
  // 'P'
  { 0x7F, 0x09, 0x09, 0x06, 0x00 },
  // 'Q'
  { 0x3E, 0x41, 0x61, 0x7E, 0x00 },
  // 'R'
  { 0x7F, 0x09, 0x19, 0x66, 0x00 },
  // 'S'
  { 0x26, 0x49, 0x49, 0x32, 0x00 },
  // 'T'
  { 0x01, 0x7F, 0x01, 0x01, 0x00 },
  // 'U'
  { 0x3F, 0x40, 0x40, 0x3F, 0x00 },
  // 'V'
  { 0x1F, 0x60, 0x60, 0x1F, 0x00 },
  // 'W'
  { 0x7F, 0x30, 0x30, 0x7F, 0x00 },
  // 'X'
  { 0x63, 0x1C, 0x1C, 0x63, 0x00 },
  // 'Y'
  { 0x07, 0x78, 0x78, 0x07, 0x00 },
  // 'Z'
  { 0x61, 0x51, 0x49, 0x45, 0x43 }
};

const int fontWidth = 5;
const int fontHeight = 7;

void drawChar(char c, int startX, int startY, CRGB color) {
  // We support only ' ' and A–Z.
  int fontIndex;
  if (c == ' ') {
    fontIndex = 0;
  } else if (c >= 'A' && c <= 'Z') {
    fontIndex = c - 'A' + 1;
  } else {
    // For unsupported characters, render as a space.
    fontIndex = 0;
  }
  
  // For each of the 5 columns in the character...
  for (int col = 0; col < fontWidth; col++) {
    // Each byte holds the vertical pixels; bit 0 is the top pixel.
    uint8_t colData = font5x7[fontIndex][col];
    for (int row = 0; row < fontHeight; row++) {
      // If the bit at 'row' is set, light that pixel.
      if (colData & (1 << row)) {
        int x = startX + col;
        int y = startY + row;
        // Make sure we stay within bounds.
        if (x >= 0 && x < MATRIX_COLUMNS && y >= 0 && y < MATRIX_ROWS) {
          leds[index(x, y)] = color;
        }
      }
    }
  }
}

/*
  drawText(text, startX, startY, color):
  Draws a null-terminated string starting at position (startX, startY).
  Each character is drawn using drawChar(). Characters are drawn left-to-right,
  with a 1‑pixel column of spacing between them.
*/
void drawText(const char* text, int startX, int startY, CRGB color) {
  int x = startX;
  while (*text) {
    drawChar(*text, x, startY, color);
    x += fontWidth + 1; // 5 columns per character plus 1 column spacing
    text++;
  }
}


void animateText(const char* text, int startY, CRGB color) {
  int textLen = strlen(text);
  // Total width: each character is (fontWidth + 1) pixels wide, minus 1 for the final space.
  int textWidth = textLen * (fontWidth + 1) - 1;
  
  // Start with the text completely off-screen to the right.
  int xOffset = MATRIX_COLUMNS;
  
  //We should probably add a better clearing mechanism for 
  while (true) {
    FastLED.clear();
    drawText("HI", 2, 0, CRGB::Aquamarine);
    drawText(text, xOffset, startY, color);
    FastLED.show();
    delay(100);  // Let's adjust the scroll speed
    
    // Decrement xOffset to move the text left.
    xOffset--;
    
    // Once the text has fully scrolled off the left edge, reset xOffset.
    if (xOffset < -textWidth) {
      xOffset = MATRIX_COLUMNS;
    }
  }
}


void loop()
{
    // for (int i = 9; i >= 0; i--) {
    //     drawDigit(i);
    //     delay(1000);  // Wait 1 second
    // }
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

    animateText("JOSH", 11, CRGB::Red);
    
    FastLED.show();
   
  
}