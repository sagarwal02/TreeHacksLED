#include <Adafruit_GFX.h>
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <WiFi.h>
#include "time.h"

#define PIN 12
#define BRIGHTNESS 255

// Each panel is 14 columns x 28 rows; we have 3 panels side by side.
#define MATRIX_COLUMNS 14
#define MATRIX_ROWS 28
#define PANEL_COUNT 4

#define WIDTH (MATRIX_COLUMNS * PANEL_COUNT)
#define LED_COUNT (WIDTH * MATRIX_ROWS)

const char* ssid = "Thijs's Phone";
const char* password = "cultuslake";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;
const time_t end_time = 1739725200000;

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
// Draws the string 'str' starting at (x,y) using the default font.
// For colons, we allocate a narrower cell so that they don’t take up 5 pixels.
void drawCondensedText(const char* str, int16_t x, int16_t y, uint16_t color, uint8_t textSize) {
  int16_t curX = x;
  for (int i = 0; str[i] != '\0'; i++) {
    int cellWidth = 5; // default allocated cell width
    int charWidth = 5; // default drawn glyph width
    int xOffset = 0;
    
    if (str[i] == ':') {
      // For colons, allocate a smaller cell and adjust the offset.
      charWidth = 2;      
      cellWidth = 2;
      // Normally, (cellWidth - charWidth) / 2 centers it,
      // but subtracting an extra 1 shifts it left a bit.
      xOffset = ((cellWidth - charWidth) / 2) - 1;
    }
    
    // Draw the character with the calculated offset.
    matrix->drawChar(curX + textSize * xOffset, y, str[i], color, color, textSize);
    
    // Advance the X position by the allocated cell width.
    curX += textSize * cellWidth;
  }
}


void displayCountdownTimer() {
  // Compute remaining seconds.
  unsigned long now = millis();
  unsigned long remainingSeconds = (now >= countdownEnd) ? 0 : (countdownEnd - now) / 1000UL;

  int hours   = remainingSeconds / 3600;
  int minutes = (remainingSeconds % 3600) / 60;
  int seconds = remainingSeconds % 60;

  // Format the time as HH:MM:SS.
  char timeString[9];
  sprintf(timeString, "%02d:%02d:%02d", hours, minutes, seconds);

  float textSize = 2;

  // Dynamically calculate the total width of the string.
  float textWidth = 0;
  for (int i = 0; timeString[i] != '\0'; i++) {
    if (timeString[i] == ':')
      textWidth += 2 * textSize;  // cell width for colons
    else
      textWidth += 6 * textSize;  // cell width for other characters
  }
  
  float textHeight = textSize * 8;  // font height remains constant

  // Center the text on the display.
  float xpos = (mw - textWidth) / 2;
  float ypos = (mh - textHeight) / 2;

  // Draw a drop shadow.
  uint16_t shadowColor = matrix->Color(30, 30, 30);
  drawCondensedText(timeString, xpos + textSize, ypos + textSize, shadowColor, textSize);

  // Draw the main countdown text.
  drawCondensedText(timeString, xpos, ypos, TEXT_COLOR, textSize);
}

time_t now;
unsigned long started_at;

//---------------------------------------------------------
void setup() {
  delay(1000);
  Serial.begin(9600);
  
  // Set the custom LED remapping function.
  matrix->setRemapFunction(customIndex);
  
  // Initialize FastLED (using the NEOPIXEL definition).
  FastLED.addLeds<NEOPIXEL, PIN>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  Serial.println("\nConnecting");
  
  matrix->clear();
	matrix->setCursor(0,0);
	matrix->setTextColor(matrix->Color(255, 255, 255));
	matrix->print("Connecting...");

  // Connect the wifi
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  matrix->clear();
	matrix->setTextColor(matrix->Color(0, 255, 0));
	matrix->print("Connected!");
  Serial.println("Connected!");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;

  if(!getLocalTime(&timeinfo)){
    matrix->clear();
    matrix->setTextColor(matrix->Color(255, 0, 0));
    matrix->print("Time failed");
    return;
  }

  time(&now);
  started_at = millis();

  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.println(now);
  
  // Set the countdown end time to 48 hours from now.
  countdownEnd = (end_time - (now * 1000UL) + millis());
  Serial.println("got here...");
  Serial.println(countdownEnd);
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
