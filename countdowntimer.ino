#include <Adafruit_GFX.h>
#include <FastLED_NeoMatrix.h>
#include <FastLED.h>
#include <WiFi.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ------------------------------
// Hardware & Matrix configuration
// ------------------------------
#define PIN 12
#define BRIGHTNESS 255

// Each panel is 14 columns x 28 rows; we have 6 panels side by side.
#define MATRIX_COLUMNS 14
#define MATRIX_ROWS 28
#define PANEL_COUNT 6

#define WIDTH (MATRIX_COLUMNS * PANEL_COUNT)
#define LED_COUNT (WIDTH * MATRIX_ROWS)

int mw = WIDTH;      // Matrix width
int mh = MATRIX_ROWS; // Matrix height
signed long remainingTime;

// Create the LED array and initialize the NeoMatrix object.
// The remap flags assume panels arranged from the TOP-LEFT with zigzag wiring.
CRGB leds[LED_COUNT];
FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(
  leds, mw, mh, NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG);

// ------------------------------
// Custom LED remapping function.
// Maps (x,y) on the overall matrix to the correct LED index.
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

// ------------------------------
// WiFi and NTP configuration
// ------------------------------
const char* ssid = "Treehacks-2025";         // <<< Replace with your WiFi network name
const char* password = "treehacks2025!";
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;          // Adjust to your time zone
const int daylightOffset_sec = 0;      // Adjust for DST if needed

// ------------------------------
// Countdown timer configuration
// ------------------------------
// Define the target end time as a Unix timestamp in milliseconds.
// (Adjust this constant to your event’s time.)
const time_t END_TIME = 1739725200000; // Example: adjust to your event’s Unix time

// We'll capture the total duration at startup.
signed long endTime = 0;
time_t now;

// This global will store the initial countdown duration in seconds.
signed long initialRemainingSeconds = 0;

// ------------------------------
// Fire simulation globals
// ------------------------------

// ------------------------------
// Tree sprite for the running tree around the countdown display
// 3x3 sprite: 0 = transparent, 1 = leaves, 2 = trunk.
const uint8_t treeSprite[3][3] = {
  {0, 1, 0},
  {1, 1, 1},
  {0, 2, 0}
};

// Draw the tree sprite centered at (x, y)
void drawTreeAt(int x, int y) {
  // Define colors for leaves and trunk
  uint16_t leavesColor = matrix->Color(0, 200, 0);
  uint16_t trunkColor = matrix->Color(139, 69, 19);
  
  // Sprite dimensions
  int spriteW = 3;
  int spriteH = 3;
  
  // Top-left corner so that the sprite is centered at (x, y)
  int startX = x - spriteW / 2;
  int startY = y - spriteH / 2;
  
  for (int i = 0; i < spriteH; i++) {
    for (int j = 0; j < spriteW; j++) {
      uint8_t pixel = treeSprite[i][j];
      if (pixel != 0) {
        uint16_t color = (pixel == 1) ? leavesColor : trunkColor;
        matrix->drawPixel(startX + j, startY + i, color);
      }
    }
  }
}

// Given a progress (0.0 to 1.0) and a rectangle, compute a point along its border.
void getTreePosition(float progress, int rectX, int rectY, int rectW, int rectH, int &treeX, int &treeY) {
  int perimeter = 2 * (rectW + rectH);
  int distance = progress * perimeter;
  
  if (distance < rectW) {
    treeX = rectX + distance;
    treeY = rectY;
  } else if (distance < rectW + rectH) {
    treeX = rectX + rectW;
    treeY = rectY + (distance - rectW);
  } else if (distance < rectW + rectH + rectW) {
    treeX = rectX + rectW - (distance - (rectW + rectH));
    treeY = rectY + rectH;
  } else {
    treeX = rectX;
    treeY = rectY + rectH - (distance - (rectW + rectH + rectW));
  }
}

// Draw the running tree around the countdown text rectangle.
void drawRunningTree(int rectX, int rectY, int rectW, int rectH) {
  // Cycle duration in milliseconds
  const unsigned long cycleDuration = 10000;
  float treeProgress = (millis() % cycleDuration) / (float) cycleDuration;
  
  int treeX, treeY;
  getTreePosition(treeProgress, rectX, rectY, rectW, rectH, treeX, treeY);
  
  drawTreeAt(treeX, treeY);
}

// --------------------------------------------------------------------
// Custom function to draw a colon with 2-pixel gaps on each side.
// The total cell width for the colon is: 2 (left gap) + (2*textSize) + 2 (right gap)
// This ensures that the digit before and after are 2 pixels away from the colon.
void drawColon(int x, int y, uint8_t textSize, uint16_t color) {
  // x is the left edge of the colon cell.
  // The colon drawing area will start at x+2.
  int dotX = x + 2;
  // For an 8*textSize tall text, we place the two dots roughly at:
  int dotY1 = y + 2 * textSize;
  int dotY2 = y + 5 * textSize;
  // Draw each dot as a filled square of size textSize x textSize.
  for (int i = 0; i < textSize; i++) {
    for (int j = 0; j < textSize; j++) {
      matrix->drawPixel(dotX + j, dotY1 + i, color);
      matrix->drawPixel(dotX + j, dotY2 + i, color);
    }
  }
}

// --------------------------------------------------------------------
// Draw the countdown timer (HH:MM:SS) centered on the matrix,
// ensuring that each digit adjacent to a colon is exactly 2 pixels away.
// Also, the text is drawn within the inner area defined by the border.
void drawCountdown() {
  if(remainingTime < 0) remainingTime = 0;
  
  int hours = remainingTime / 3600;
  int minutes = (remainingTime % 3600) / 60;
  int seconds = remainingTime % 60;
  
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", hours, minutes, seconds);
  
  uint8_t textSize = 2;
  // Define dimensions:
  int digitCellWidth = 5 * textSize;             // drawn digit width
  int colonCellWidth = 2 * textSize + 4;           // 2-pixel gap on left/right plus colon area (2*textSize)
  // Total width for "HH:MM:SS":
  int textWidth = 6 * digitCellWidth + 2 * colonCellWidth;
  int textHeight = 8 * textSize;                   // standard character height
  
  // The green border is inset by 1 pixel; so available drawing area is from x=2 to x=mw-3.
  int availableWidth = mw - 4;
  int availableHeight = mh - 4;
  
  // Center the text in the available area.
  int startX = (availableWidth - textWidth) / 2;
  int startY = 3 + (availableHeight - textHeight) / 2;
  
  uint16_t white = matrix->Color(255, 255, 255);
  
  int x = startX+1;
  // Draw HH (digits 0 and 1)
  matrix->drawChar(x, startY, timeStr[0], white, white, textSize);
  x += digitCellWidth+2;
  matrix->drawChar(x, startY, timeStr[1], white, white, textSize);
  x += digitCellWidth;
  
  // Draw first colon with 2-pixel gap on left/right.
  drawColon(x, startY, textSize, white);
  x += colonCellWidth-2;
  
  // Draw MM (digits 3 and 4; skipping the colon in timeStr[2])
  matrix->drawChar(x, startY, timeStr[3], white, white, textSize);
  x += digitCellWidth+2;
  matrix->drawChar(x, startY, timeStr[4], white, white, textSize);
  x += digitCellWidth;
  
  // Draw second colon.
  drawColon(x, startY, textSize, white);
  x += colonCellWidth-2;
  
  // Draw SS (digits 6 and 7)
  matrix->drawChar(x, startY, timeStr[6], white, white, textSize);
  x += digitCellWidth+2;
  matrix->drawChar(x, startY, timeStr[7], white, white, textSize);
  // x += digitCellWidth;  // Not needed after last digit.
  
  // Draw the running tree around the bounding rectangle of the countdown text.
  drawRunningTree(startX, startY-4, 79, 21);
}

// --------------------------------------------------------------------
// Draw a green border inset by one row/column from the edge.
void drawGreenBorder() {
  uint16_t green = matrix->Color(0, 255, 0);
  // Draw top and bottom borders (y = 1 and y = mh-2).
  for (int x = 0; x < mw; x++) {
    matrix->drawPixel(x, 0, green);
    matrix->drawPixel(x, mh - 1, green);
  }
  // Draw left and right borders (x = 1 and x = mw-2).
  for (int y = 0; y < mh - 1; y++) {
    matrix->drawPixel(0, y, green);
    matrix->drawPixel(mw - 1, y, green);
  }
}

// --------------------------------------------------------------------
// Update the fire simulation and draw it onto the matrix.
// The intensity is modulated by progress (0.0 at the start, 1.0 at END_TIME).


void setup() {
  Serial.begin(115200);
  matrix->setRemapFunction(customIndex);

  FastLED.addLeds<WS2812B, PIN, RGB>(leds, LED_COUNT).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  // ---------------
  // Connect to WiFi
  // ---------------
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected.");

  // -----------------------------
  // Initialize time using NTP.
  // -----------------------------
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Waiting for time sync...");

  struct tm timeinfo;
  // Wait until time is successfully obtained.
  while(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time, retrying...");
    delay(500);
  }
  now = mktime(&timeinfo);
  Serial.println("Time synchronized.");

  // Set initial countdown (calculate the event time in millis relative to millis())
  endTime = END_TIME - (now * 1000UL) + millis();
  initialRemainingSeconds = (endTime - millis()) / 1000UL;

  // Seed random number generator.
  randomSeed(analogRead(0));
}

void loop() {
  // Calculate the remaining time in seconds.
  remainingTime = (endTime - millis()) / 1000UL;
  if(remainingTime < 0) remainingTime = 0;
  
  // Compute progress: 0.0 at start, 1.0 when the countdown reaches 0.
  float progress = 0.0;
  if(initialRemainingSeconds > 0) {
    progress = (float)(initialRemainingSeconds - remainingTime) / initialRemainingSeconds;
    if(progress < 0) progress = 0;
    if(progress > 1) progress = 1;
  } else {
    progress = 1.0;
  }
  
  matrix->clear();
  // Update the fire simulation background with intensity based on progress.
  
  // Overlay the countdown timer text (with proper 2-pixel gaps around colons)
  // and the running tree.
  drawCountdown();
  
  // Draw an inset green border (1 pixel in from each edge).
  drawGreenBorder();
  
  matrix->show();
  delay(75);
}
