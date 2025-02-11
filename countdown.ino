#include <Adafruit_GFX.h>
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PIN 12
#define BRIGHTNESS 16

// Each panel is 14 columns x 28 rows; we have 3 panels side by side.
#define MATRIX_COLUMNS 14
#define MATRIX_ROWS 28
#define PANEL_COUNT 3

#define WIDTH (MATRIX_COLUMNS * PANEL_COUNT)
#define LED_COUNT (WIDTH * MATRIX_ROWS)

int mw = WIDTH;      // Matrix width (42 pixels)
int mh = MATRIX_ROWS; // Matrix height (28 pixels)

// Create the LED array and initialize the matrix object.
// The remap flags assume panels arranged starting at the TOP-LEFT with "zigzag" wiring.
CRGB leds[LED_COUNT];
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(
  leds, mw, mh, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG);

// Define our text color (bright white)
#define TEXT_COLOR (matrix->Color(255, 255, 255))

// Global variable for the countdown end time (48 hours from startup)
unsigned long countdownEnd;

// --- BURST EFFECT GLOBALS ---
// Every burst is triggered every 5000 ms (5 seconds) and lasts for 1000 ms.
unsigned long lastBurstTime = 0;
unsigned long burstStart = 0;
const unsigned long burstInterval = 5000; // 5 seconds between bursts
const unsigned long burstDuration = 1000; // burst lasts 1 second
const float maxBurstRadius = 30.0;        // maximum burst radius (in pixels)

//---------------------------------------------------------
// customIndex()
// Remaps (x,y) coordinates to the correct LED index for your
// multi-panel wiring with serpentine (zigzag) connections.
uint16_t customIndex(uint16_t x, uint16_t y) {
  int panel = x / MATRIX_COLUMNS;
  int lx = x % MATRIX_COLUMNS;
  int ly = y;
  int i;
  if (ly % 2 == 0) {
    i = ly * MATRIX_COLUMNS + lx;
  } else {
    i = (ly + 1) * MATRIX_COLUMNS - lx - 1;
  }
  return panel * (MATRIX_COLUMNS * MATRIX_ROWS) + i;
}

//---------------------------------------------------------
// drawBackgroundEffect()
// Instead of an intricate moving sine–wave pattern, we now simply
// fill the background with a dark blue so that the burst effect is visible.
void drawBackgroundEffect() {
  // Fill the entire display with a dark blue color.
  matrix->fillScreen(matrix->Color(0, 0, 20));
}

//---------------------------------------------------------
// drawBurstEffect()
// Every 5 seconds this function triggers a burst effect. Over the course
// of 1 second an expanding ring (centered on the display) is drawn.
void drawBurstEffect() {
  unsigned long currentMillis = millis();
  // Trigger a new burst every burstInterval (5 seconds)
  if (currentMillis - lastBurstTime >= burstInterval) {
    burstStart = currentMillis;
    lastBurstTime = currentMillis;
  }
  
  // If we are within the burst duration, animate the burst.
  if (currentMillis - burstStart < burstDuration) {
    // Compute a normalized time (t goes from 0 to 1 over burstDuration)
    float t = (currentMillis - burstStart) / (float)burstDuration;
    // Compute the current radius (from 0 up to maxBurstRadius)
    float radius = t * maxBurstRadius;
    
    // Use the display center as the burst center.
    float centerX = mw / 2.0;
    float centerY = mh / 2.0;
    
    // For every pixel, if its distance from the center is close to the current radius,
    // draw it in a bright color.
    for (int16_t y = 0; y < mh; y++) {
      for (int16_t x = 0; x < mw; x++) {
        float dx = x - centerX;
        float dy = y - centerY;
        float d = sqrt(dx * dx + dy * dy);
        if (fabs(d - radius) < 1.0) {
          // Optionally, you could vary brightness with t.
          uint16_t burstColor = matrix->Color(255, 255, 255); // white burst
          matrix->drawPixel(x, y, burstColor);
        }
      }
    }
  }
}

//---------------------------------------------------------
// drawCondensedText()
// Draws the string 'str' starting at (x,y) using the default font,
// but uses only (textSize*5) pixels per character (instead of the default 6)
// so that the gaps between characters are reduced.
// (The background color is set equal to the text color so that only the "on" pixels are drawn.)
void drawCondensedText(const char* str, int16_t x, int16_t y, uint16_t color, uint8_t textSize) {
  int16_t curX = x;
  for (int i = 0; str[i] != '\0'; i++) {
    // Draw the character with "transparent" background (by passing the same color)
    matrix->drawChar(curX, y, str[i], color, color, textSize);
    // Advance by 5 pixels (times textSize) instead of the usual 6.
    curX += textSize * 5;
  }
}

//---------------------------------------------------------
// displayCountdownTimer()
// Computes the remaining time until countdownEnd and draws it in condensed form.
// The text is scaled (using a custom formula) so that it fits within 90% of the display,
// then centered both horizontally and vertically. A drop shadow is drawn first.
void displayCountdownTimer() {
  // Compute remaining seconds.
  unsigned long now = millis();
  unsigned long remainingSeconds = (now >= countdownEnd) ? 0 : (countdownEnd - now) / 1000UL;
  
  int hours   = remainingSeconds / 3600;
  int minutes = (remainingSeconds % 3600) / 60;
  int seconds = remainingSeconds % 60;
  
  // Format the time as HH:MM:SS (8 characters including colons)
  char timeString[9];
  sprintf(timeString, "%02d:%02d:%02d", hours, minutes, seconds);
  
  // Compute the length of the string.
  int len = strlen(timeString);
  // In our condensed drawing, each character is 5 pixels wide and the font is 8 pixels tall at scale 1.
  int baseWidth = 5 * len;
  int baseHeight = 8;
  
  // Compute the maximum integer scale that fits into 90% of the display.
  int scaleW = (mw * 90 / 100) / baseWidth;
  int scaleH = (mh * 90 / 100) / baseHeight;
  int textSize = (scaleW < scaleH) ? scaleW : scaleH;
  if (textSize < 1) textSize = 1;
  
  // Determine the total drawn width/height.
  int textWidth = textSize * baseWidth;
  int textHeight = textSize * baseHeight;
  
  // Center the text on the display.
  int xpos = (mw - textWidth) / 2;
  int ypos = (mh - textHeight) / 2;
  
  // Draw a drop shadow for a nice effect.
  uint16_t shadowColor = matrix->Color(30, 30, 30);
  drawCondensedText(timeString, xpos + textSize, ypos + textSize, shadowColor, textSize);
  // Draw the main countdown text.
  drawCondensedText(timeString, xpos, ypos, TEXT_COLOR, textSize);
}

//---------------------------------------------------------
void setup() {
  // Set the custom LED remapping function.
  matrix->setRemapFunction(customIndex);
  
  // Initialize FastLED (using the NEOPIXEL definition).
  FastLED.addLeds<NEOPIXEL, PIN>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  
  Serial.begin(9600);
  
  // Set the countdown end time to 48 hours from now.
  countdownEnd = millis() + 48UL * 3600UL * 1000UL;
}

//---------------------------------------------------------
void loop() {
  // First, draw the (static) dark background.
  drawBackgroundEffect();
  
  // Next, draw the burst effect if active.
  drawBurstEffect();
  
  // Then overlay the condensed, centered countdown timer.
  displayCountdownTimer();
  
  // Update the matrix.
  matrix->show();
  
  // A short delay to smooth the animation.
  delay(50);
}

