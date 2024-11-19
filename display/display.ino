#include <FS.h>
#include "SPIFFS.h" // Required for ESP32 only
#include <JPEGDEC.h>
#include <Utils.h>
#include "SPI.h"
#include <TFT_eSPI.h>    
#include <XPT2046_Bitbang.h>
#include <Wire.h>

// Pins for accelerometer and buzzer
#define I2C_SDA 27
#define I2C_SCL 22
#define buzzerPin 3

// Pins for the touch screen
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
XPT2046_Bitbang ts(XPT2046_MOSI, XPT2046_MISO, XPT2046_CLK, XPT2046_CS);

// Setting calibration constants for touchscreen
int xMin = 200;
int xMax = 3700;
int yMin = 340;
int yMax = 3800;

TFT_eSPI tft = TFT_eSPI();         

// Constants for slider pic
JPEGDEC jpeg;
const char *PHOTO_URI = "/slider.jpg";
const int BUF_SIZE = 16384;
fs::File myfile;
// frame buffer for image
uint8_t *fb; 

const int Y_SLIDER_MAX = 220;
const int Y_SLIDER_MIN = 180; 
int distance_selected = 0;
int angle_min = 0, angle_max = 0;

const int MPU=0x68; 
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
float pitch = 0, roll = 0; // angle to display
const int rate = 500;  // how often to update the angle
unsigned long prevTime = 0;
bool wasGreen = false;
bool upArrow = false;
bool displayedDist = false;

void setup()
{
  //Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT); // buzzerpin
  Wire.begin(I2C_SDA, I2C_SCL);

  // gyro/accel sensor
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(true);

  // Initialise SPIFFS
  if (!SPIFFS.begin()) {
    //Serial.println("SPIFFS initialisation failed!");
    while (1) yield(); // Stay here twiddling thumbs waiting
  }
  //Serial.println("\r\nInitialisation done.");

  // Initialise the TFT
  tft.begin();
  tft.setRotation(1);
  tft.invertDisplay(1);
  tft.fillScreen(TFT_WHITE);

  ts.begin();
  ts.setCalibration(xMin, xMax, yMin, yMax);

  fb = (uint8_t*)malloc(BUF_SIZE*sizeof(uint8_t));
  fb[0] = 0;
  copyFileintoRAM(fb, SPIFFS, PHOTO_URI);
  displaySliderImage();

  drawSlider(100);
  displayText();
}

void loop()
{ 
  // get accel and gyro from sensor
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,12,true);  
  AcX=Wire.read()<<8|Wire.read();    
  AcY=Wire.read()<<8|Wire.read();  
  AcZ=Wire.read()<<8|Wire.read();  
  GyX=Wire.read()<<8|Wire.read();  
  GyY=Wire.read()<<8|Wire.read();  
  GyZ=Wire.read()<<8|Wire.read();  

  updateAngles(AcX, AcY, AcZ);
  unsigned long currentTime = millis();
  if (currentTime - prevTime >= rate) {
    prevTime = currentTime;
    updateAngleDisplay();
  }
  
  TouchPoint p = ts.getTouch();
  if(p.zRaw > 0 && p.y >= 180 && p.y <= 220) {
    // calculating distance scaling
    if(p.x < 79 && p.x > 65) {
      distance_selected = 20; 
      displaySliderImage();
      drawSlider(79);
      updateDistance();
    }
    else if(p.x <= 65) {

    }
    else if(p.x > 315) {
      distance_selected = 100;
      displaySliderImage();
      drawSlider(315);
      updateDistance();
    }
    else {
      distance_selected =  p.x/3 - 5; 
      displaySliderImage();
      drawSlider(p.x);
      updateDistance();
    }
  }
}

void displayText() {
  tft.setTextColor(TFT_BLACK); // text color
  tft.setTextSize(1); // text size
  tft.drawString("Distance: ", 0, 0, 4); // (String, x, y, font)
  tft.drawString("Range: ", 0, 45, 4); 
  tft.drawString("Angle: ", 0, 90, 4); 
  tft.drawString("Select distance of person: ", 0, 137, 4);
}

void updateDistance() {
  tft.fillRect(110, 0, 70, 40, TFT_WHITE); // (x, y, width, height, color) // clearing the distance
  tft.drawString(String(distance_selected) + " ft", 110, 0, 4);
  tft.fillRect(85, 45, 75, 40, TFT_WHITE); // clearing the range area
  // calculating angle range based on distance selected
  int distance_max = distance_selected + 20;
  if(distance_selected >= 84) {
    angle_min = 40;
    angle_max = 50;
  } 
  else if(distance_selected >= 68) {
    angle_min = 25;
    angle_max = 40;
  }     
  else if(distance_selected >= 52) {
    angle_min = 13;
    angle_max = 25;
  }               
  else if(distance_selected >= 36) {
    angle_min = 0;
    angle_max = 13;
  }               
  else if(distance_selected >= 20) {
    angle_min = -10;
    angle_max = 0;
  }                                                                                                                                                                                                                                                                                                                                                                              
  // gravity = 32.164 ft/s^2
  // assumign initial velocity = 70 ft/s
  // angle = 0.5*asin(G * d / v^2) (also convert angle from radians to deg, *180/pi)
  // angle_max = 0.5*asin(32.174 * distance_selected / (55*55))*180/3.14; 
  // if(32.174 * distance_max / (4900) >= 1) {
  //   angle_min = 45;
  // }
  // else {
  //   angle_min = 0.5*asin(32.174 * distance_max / (4900))*180/3.14;
  // } 
  tft.drawString(String(angle_min) + "-" + String(angle_max) + "`", 85, 45, 4);

  if(!displayedDist) {
    displayedDist = true; 
    if(pitch > angle_max) {
      drawDownArrow();
      noTone(buzzerPin);
      wasGreen = false;
      upArrow = false;
    }
    else if(pitch < angle_min) {
      drawUpArrow();
      noTone(buzzerPin);
      wasGreen = false;
      upArrow = true;
    }
    else if (pitch >= angle_min && pitch <= angle_max) {
      tft.fillRect(180, 0, 160, 130, TFT_WHITE); // the arrow bocx
      tft.fillRect(180, 0, 160, 130, TFT_GREEN);
      tone(buzzerPin, 1000);
      wasGreen = true;
      upArrow = false;
    }
  }
}

void drawDownArrow() {
  tft.fillRect(180, 0, 160, 130, TFT_WHITE); // the arrow bocx
  tft.fillTriangle(250, 125, 285, 80, 215, 80, TFT_BLACK); // Triangle pointing down
  tft.fillRect(235, 20, 30, 60, TFT_BLACK); // Rectangle body above the triangle
}

void drawUpArrow() {
  tft.fillRect(180, 0, 160, 130, TFT_WHITE); // the arrow box
  tft.fillTriangle(250, 20, 285, 65, 215, 65, TFT_BLACK);
  tft.fillRect(235, 65, 30, 60, TFT_BLACK);
}

void updateAngleDisplay() {
  tft.fillRect(85, 90, 80, 25, TFT_WHITE); // the angle number
  tft.drawString(String(pitch) + "`", 85, 90, 4);
  if(displayedDist) {
    if(pitch > angle_max && (upArrow || wasGreen)) {
      drawDownArrow();
      noTone(buzzerPin);
      wasGreen = false;
      upArrow = false;
    }
    else if(pitch < angle_min && (wasGreen || !upArrow)) {
      drawUpArrow();
      noTone(buzzerPin);
      wasGreen = false;
      upArrow = true;
    }
    else if (pitch >= angle_min && pitch <= angle_max && !wasGreen) {
      tft.fillRect(180, 0, 160, 130, TFT_WHITE); // the arrow bocx
      tft.fillRect(180, 0, 160, 130, TFT_GREEN);
      tone(buzzerPin, 1000);
      wasGreen = true;
      upArrow = false;
    }
  }
}

void displaySliderImage() {
  drawImagefromRAM((const char*) fb, BUF_SIZE);
}

void drawSlider(int x) {
  tft.fillEllipse(x-4, 196, 4, 10, TFT_RED); // (x, y, radius_x, radius_y, color)
  tft.drawEllipse(x-4, 196, 4, 10, TFT_BLACK);
}

void copyFileintoRAM(uint8_t* dest, fs::FS &fs, const char * path){
    //Serial.printf("Reading file: %s\r\n", path);
    File file = fs.open(path);
    if(!file || file.isDirectory()){
        //Serial.println("- failed to open file for reading");
        return;
    }
    //Serial.println("- read from file:");
    if(file.available()){
        file.read(dest,BUF_SIZE);
    }
    //Serial.println();
    file.close();
}

int drawImagefromRAM(const char *imageBuffer, int size) {
    unsigned long lTime = millis();
    lTime = millis();
    jpeg.openRAM((uint8_t *)imageBuffer, size, JPEGDraw);
    jpeg.setPixelType(1);
    int imagePositionX = 0;
    int imagePositionY = 160;
    // (x_pos, y_pos, scale)
    int decodeStatus = jpeg.decode(imagePositionX,imagePositionY, 0);   // JPEG_SCALE_HALF for one-quarter size (half in each dimension)
    jpeg.close();
    return decodeStatus;
}

int JPEGDraw(JPEGDRAW *pDraw) {
  // Stop further decoding as image is running off bottom of screen
  if (pDraw->y >= tft.height())
    return 0;
  tft.pushImage(pDraw->x, pDraw->y, pDraw->iWidth, pDraw->iHeight, pDraw->pPixels);
  return 1;
}

void updateAngles(float a_x, float a_y, float a_z) {
  // Calculate angles from accelerometer
  pitch = atan2(a_y, sqrt(a_x * a_x + a_z * a_z)) * (180.0 / PI) - 12;
  roll = atan2(-a_x, sqrt(a_y * a_y + a_z * a_z)) * (180.0 / PI);
}
