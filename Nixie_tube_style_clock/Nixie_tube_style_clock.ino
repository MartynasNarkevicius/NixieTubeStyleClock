#include <SPI.h>
#include <TFT_eSPI.h>

#include <WiFi.h>
#include "time.h"

// Include the PNG decoder library
#include <PNGdec.h>

// Include the arrays (stored here in an 8-bit array)
#include "digit_0.h"
#include "digit_1.h"
#include "digit_2.h"
#include "digit_3.h"
#include "digit_4.h"
#include "digit_5.h"
#include "digit_6.h"
#include "digit_7.h"
#include "digit_8.h"
#include "digit_9.h"
#include "separatorOn.h"
#include "separatorOff.h"

PNG png;  // PNG decoder instance

#define MAX_IMAGE_WIDTH 240  // Adjust for your images

int16_t xpos = 0;
int16_t ypos = 0;

// each display has its own CS pin
#define screen_0_CS 26
#define screen_1_CS 14
#define screen_2_CS 16
#define screen_3_CS 17
#define screen_4_CS 27
TFT_eSPI tft = TFT_eSPI();

const char *ssid = "";
const char *password = "";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7200;
const int daylightOffset_sec = 3600;

bool separatorActive = true;
int previousMinutes = -1; // Initialize with an invalid value

void setup() {

  Serial.begin(9600);

  connectToWifi();

  initializeScreens();

  digitalWrite(screen_2_CS, LOW);
  png.openFLASH((uint8_t *)separatorOn, sizeof(separatorOn), pngDraw);
  tft.startWrite();
  png.decode(NULL, 0);
  tft.endWrite();
  digitalWrite(screen_2_CS, HIGH);
}


void loop() {
  updateClock();
  delay(1000);
}

void drawDigit(int digit, int csPin) {
  digitalWrite(csPin, LOW);  // Select the screen
  switch (digit) {
    case 0:
      png.openFLASH((uint8_t *)digit_0, sizeof(digit_0), pngDraw);
      break;
    case 1:
      png.openFLASH((uint8_t *)digit_1, sizeof(digit_1), pngDraw);
      break;
    case 2:
      png.openFLASH((uint8_t *)digit_2, sizeof(digit_2), pngDraw);
      break;
    case 3:
      png.openFLASH((uint8_t *)digit_3, sizeof(digit_3), pngDraw);
      break;
    case 4:
      png.openFLASH((uint8_t *)digit_4, sizeof(digit_4), pngDraw);
      break;
    case 5:
      png.openFLASH((uint8_t *)digit_5, sizeof(digit_5), pngDraw);
      break;
    case 6:
      png.openFLASH((uint8_t *)digit_6, sizeof(digit_6), pngDraw);
      break;
    case 7:
      png.openFLASH((uint8_t *)digit_7, sizeof(digit_7), pngDraw);
      break;
    case 8:
      png.openFLASH((uint8_t *)digit_8, sizeof(digit_8), pngDraw);
      break;
    case 9:
      png.openFLASH((uint8_t *)digit_9, sizeof(digit_9), pngDraw);
      break;
    default:
      // Handle invalid digit (optional)
      Serial.println("Invalid digit value!");
      png.openFLASH((uint8_t *)separatorOn, sizeof(separatorOn), pngDraw);
      break;
  }

  tft.startWrite();
  png.decode(NULL, 0);
  tft.endWrite();

  digitalWrite(csPin, HIGH);  // Deselect the screen
}

void updateSeparator() {
  digitalWrite(screen_2_CS, LOW);
  if (separatorActive) {
    png.openFLASH((uint8_t *)separatorOn, sizeof(separatorOn), pngDraw);
    separatorActive = !separatorActive;
  } else {
    png.openFLASH((uint8_t *)separatorOff, sizeof(separatorOff), pngDraw);
    separatorActive = !separatorActive;
  };

  tft.startWrite();
  png.decode(NULL, 0);
  tft.endWrite();
  digitalWrite(screen_2_CS, HIGH);
}

void connectToWifi() {
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void initializeScreens() {
  pinMode(screen_0_CS, OUTPUT);
  pinMode(screen_1_CS, OUTPUT);
  pinMode(screen_2_CS, OUTPUT);
  pinMode(screen_3_CS, OUTPUT);
  pinMode(screen_4_CS, OUTPUT);

  // we need to 'init' all displays
  // at the same time - so set all cs pins low
  digitalWrite(screen_0_CS, LOW);
  digitalWrite(screen_1_CS, LOW);
  digitalWrite(screen_2_CS, LOW);
  digitalWrite(screen_3_CS, LOW);
  digitalWrite(screen_4_CS, LOW);
  tft.init();

  // set all CS pins HIGH, or 'inactive'
  digitalWrite(screen_0_CS, HIGH);
  digitalWrite(screen_1_CS, HIGH);
  digitalWrite(screen_2_CS, HIGH);
  digitalWrite(screen_3_CS, HIGH);
  digitalWrite(screen_4_CS, HIGH);
}

void updateClock() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);  // Get hours in string format

  char timeMinute[3];
  strftime(timeMinute, 3, "%M", &timeinfo);  // Get minutes in string format

  // Extract tens and ones digits from minutes string
  int onesMinutes = timeMinute[1] - '0';
  int tensMinutes = timeMinute[0] - '0';

  // Extract tens and ones digits from hours string
  int onesHours = timeHour[1] - '0';
  int tensHours = timeHour[0] - '0';

  updateSeparator(); 

  // Check if the minutes value has changed
  int currentMinutes = tensMinutes * 10 + onesMinutes;
  if (currentMinutes == previousMinutes) {
    return; // Exit the function if minutes value hasn't changed
  }
  previousMinutes = currentMinutes; // Update previous minutes value

  digitalWrite(screen_0_CS, HIGH);
  digitalWrite(screen_1_CS, HIGH);
  digitalWrite(screen_3_CS, HIGH);
  digitalWrite(screen_4_CS, HIGH);

  Serial.println("Minutes: " + String(tensMinutes) + String(onesMinutes));
  Serial.println("Hours: " + String(tensHours) + String(onesHours));

  // Draw tens digit on screen 1
  drawDigit(onesMinutes, screen_0_CS);
  // Draw ones digit on screen 2
  drawDigit(tensMinutes, screen_1_CS);

  // Draw tens digit on screen 4
  drawDigit(onesHours, screen_3_CS);
  // Draw ones digit on screen 5
  drawDigit(tensHours, screen_4_CS);
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
}

//=========================================v==========================================
//                                      pngDraw
//====================================================================================
// This next function will be called during decoding of the png file to
// render each image line to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
// Callback function to draw pixels to the display
void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}
