// Example for library:
// https://github.com/Bodmer/TJpg_Decoder

// This example is for an ESP8266 or ESP32, it fetches a Jpeg file
// from the web and saves it in a SPIFFS file. You must have SPIFFS
// space allocated in the IDE.

// Chenge next 2 lines to suit your WiFi network
#define WIFI_SSID "DukeVisitor"
#define PASSWORD ""

// Include SPIFFS
#define FS_NO_GLOBALS
#include <FS.h>

// Include WiFi and http client
#ifdef ARDUINO_ARCH_ESP8266
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <ESP8266WiFiMulti.h>
  #include <WiFiClientSecureBearSSL.h>
#else
  #include "SPIFFS.h" // Required for ESP32 only
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

// Load tabs attached to this sketch
#include "List_SPIFFS.h"
#include "Web_Fetch.h"

// Include the TFT library https://github.com/Bodmer/TFT_eSPI
#include "SPI.h"
#include <TFT_eSPI.h>              // Hardware-specific library
TFT_eSPI tft = TFT_eSPI();         // Invoke custom library

// This next function will be called during decoding of the jpeg file to
// render each block to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  // Stop further decoding as image is running off bottom of screen
  if ( y >= tft.height() ) return 0;

  // This function will clip the image block rendering automatically at the TFT boundaries
  tft.pushImage(x, y, w, h, bitmap);

  // Return 1 to decode next block
  return 1;
}

void setup()
{
  Serial.begin(115200);

  // Initialise SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  Serial.println("\r\nInitialisation done.");

  // Initialise the TFT
  tft.begin();
  tft.fillScreen(TFT_BLACK);

  WiFi.begin(WIFI_SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();

  // This is for demoe purposes only so that file is fetched each time this is run
  if (SPIFFS.exists("/slider.jpg") == true) {
    Serial.println("For test only, removing file");
    SPIFFS.remove("/slider.jpg");
  }
}

void loop()
{
  Serial.println("MAIN");
  // List files stored in SPIFFS
  listSPIFFS();

  // Time recorded for test purposes
  uint32_t t = millis();

  // Fetch the jpg file from the specified URL, examples only, from imgur
  bool loaded_ok = getFile("https://i.imgur.com/8JKL4Ty.jpeg", "/slider.jpg"); // Note name preceded with "/"
  t = millis() - t;
  if (loaded_ok) { Serial.print(t); Serial.println(" ms to download"); }

  // List files stored in SPIFFS, should have the file now
  listSPIFFS();

  t = millis();

  t = millis() - t;
  Serial.print(t); Serial.println(" ms to draw to TFT");

  // Wait forever
  while(1) yield();
}
// // Example for library:
// // https://github.com/Bodmer/TJpg_Decoder

// // This example is for an ESP8266 or ESP32, it fetches a Jpeg file
// // from the web and saves it in a SPIFFS file. You must have SPIFFS
// // space allocated in the IDE.

// // Chenge next 2 lines to suit your WiFi network
// #define WIFI_SSID "DukeVisitor"
// #define PASSWORD ""

// // Include the jpeg decoder library
// #include <TJpg_Decoder.h>

// // Include SPIFFS
// #define FS_NO_GLOBALS
// #include <FS.h>
// #include "SPIFFS.h" // Required for ESP32 only
// #include <WiFi.h>
// #include <HTTPClient.h>
// // Load tabs attached to this sketch
// #include "List_SPIFFS.h"
// #include "Web_Fetch.h"
// // Include the TFT library https://github.com/Bodmer/TFT_eSPI
// #include "SPI.h"
// #include <TFT_eSPI.h>              // Hardware-specific library
// TFT_eSPI tft = TFT_eSPI();         // Invoke custom library
// #include <JPEGDEC.h>

// JPEGDEC jpeg;
// const char *PHOTO_URI = "/slider.jpg";
// const int BUF_SIZE = 16384;

// // This next function will be called during decoding of the jpeg file to
// // render each block to the TFT.  If you use a different TFT library
// // you will need to adapt this function to suit.
// bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
// {
//   // Stop further decoding as image is running off bottom of screen
//   if ( y >= tft.height() ) return 0;

//   // This function will clip the image block rendering automatically at the TFT boundaries
//   tft.pushImage(x, y, w, h, bitmap);

//   // Return 1 to decode next block
//   return 1;
// }

// void setup()
// {
//   Serial.begin(115200);
//   Serial.println("\n\n Testing TJpg_Decoder library");

//   // Initialise SPIFFS
//   if (!SPIFFS.begin()) {
//     Serial.println("SPIFFS initialisation failed!");
//     while (1) yield(); // Stay here twiddling thumbs waiting
//   }
//   Serial.println("\r\nInitialisation done.");

//   // Initialise the TFT
//   tft.begin();
//   tft.fillScreen(TFT_BLACK);

//   // The jpeg image can be scaled by a factor of 1, 2, 4, or 8
//   TJpgDec.setJpgScale(1);

//   // The byte order can be swapped (set true for TFT_eSPI)
//   TJpgDec.setSwapBytes(true);

//   // The decoder must be given the exact name of the rendering function above
//   TJpgDec.setCallback(tft_output);

//   WiFi.begin(WIFI_SSID, PASSWORD);

//   while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     Serial.print(".");
//   }
//   Serial.println();

//   // This is for demoe purposes only so that file is fetched each time this is run
//   if (LittleFS.exists(PHOTO_URI) == true) {
//     Serial.println("For test only, removing file");
//     LittleFS.remove(PHOTO_URI);
//   }

// }

// void loop()
// {
//   // List files stored in SPIFFS
//   listSPIFFS();

//   // Time recorded for test purposes
//   uint32_t t = millis();

//   // Fetch the jpg file from the specified URL, examples only, from imgur
//   bool loaded_ok = getFile("https://imgur.com/kiyAmOr", PHOTO_URI); // Note name preceded with "/"

//   t = millis() - t;
//   if (loaded_ok) { Serial.print(t); Serial.println(" ms to download"); }

//   // List files stored in SPIFFS, should have the file now
//   listSPIFFS();
//   uint8_t *fb; 
//   // frame buffer for image, it's kinda big so 
//   fb = (uint8_t*)  malloc(BUF_SIZE* sizeof( uint8_t ) );
//   fb[0] = 0;

//   copyFileintoRAM(fb, SPIFFS, PHOTO_URI);
//   drawImagefromRAM((const char*) fb, BUF_SIZE);

//   // Wait forever
//   while(1) yield();
// }

// void copyFileintoRAM(uint8_t* dest, fs::FS &fs, const char * path){
//     Serial.printf("Reading file: %s\r\n", path);

//     File file = fs.open(path);
//     if(!file || file.isDirectory()){
//         Serial.println("- failed to open file for reading");
//         return;
//     }
//     Serial.println("- read from file:");
//     if(file.available()){
//         file.read(dest,BUF_SIZE);
//     }
//     Serial.println();
//     file.close();
// }

// int drawImagefromRAM(const char *imageBuffer, int size) {
//     unsigned long lTime = millis();
//     lTime = millis();
//     jpeg.openRAM((uint8_t *)imageBuffer, size, JPEGDraw);
//     jpeg.setPixelType(1);
//     int imagePositionX = 320/4;
//     int imagePositionY = 240/4;
//     // decode will return 1 on sucess and 0 on a failure
//     // int decodeStatus = jpeg.decode(0, 0, 0);                       //for full size display
//     int decodeStatus = jpeg.decode(imagePositionX, imagePositionY, 0);   //for one-quarter size (half in each dimension)
//     // jpeg.decode(45, 0, 0);
//     jpeg.close();
//     // Serial.print("Time taken to decode and display Image (ms): ");
//     // Serial.println(millis() - lTime);

//     return decodeStatus;
// }

// int JPEGDraw(JPEGDRAW *pDraw) {
//   // Stop further decoding as image is running off bottom of screen
//   if (pDraw->y >= tft.height())
//     return 0;
//   tft.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
//   return 1;
// }
