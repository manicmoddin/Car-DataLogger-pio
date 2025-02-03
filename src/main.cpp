#include <HardwareSerial.h>
#include <TinyGPSPlus.h>
// #include <WiFi.h>
// #include <ESPmDNS.h>
// #include <WiFiUdp.h>
// #include <ArduinoOTA.h>

// SD Card
#include <SD.h>
#include <FS.h>
#include <SPI.h>

bool sdCardPresent = 0;

// Add WiFi Secrets (SECRET_WIFI_SSID / SECRET_WIFI_PASS) in secrets.h
#include <secrets.h>


#define GPSTX 16              //pin number for TX output from ESP32 - RX into GPS
#define GPSRX 17              //pin number for RX input into ESP32 - TX from GPS
#define GPSserial Serial2     //define GPSserial as ESP32 Serial2 

// The TinyGPSPlus object
TinyGPSPlus gps;
unsigned long last = 0UL;
unsigned long lastWrite;

// Current Date
char currentDate[12] = "2000-00-00";

void displayInfo();
// void testFileIO(fs::FS &fs, const char * path);
void checkDate();
void writeLine(fs::FS &fs);


void setup()
{
  Serial.begin(115200);
  // WiFi.mode(WIFI_STA);
  // WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASS);
  // while (WiFi.waitForConnectResult() != WL_CONNECTED) {
  //   Serial.println("Connection Failed! Rebooting...");
  //   delay(5000);
  //   ESP.restart();
  // }

  if(!SD.begin(5)){
    Serial.println("Card Mount Failed");
    // return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    // return;
  }

  else {
    sdCardPresent = 1;
    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
      Serial.println("MMC");
    } else if(cardType == CARD_SD){
      Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
      Serial.println("SDHC");
    } else {
      Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  }
  
  GPSserial.begin(9600, SERIAL_8N1, GPSTX, GPSRX);           //format is baud, mode, UART RX data, UART TX data
  Serial.println("GPS_Echo_ESP32 Starting");

  // ArduinoOTA
  //   .onStart([]() {
  //     String type;
  //     if (ArduinoOTA.getCommand() == U_FLASH)
  //       type = "sketch";
  //     else // U_SPIFFS
  //       type = "filesystem";

  //     // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
  //     Serial.println("Start updating " + type);
  //   })
  //   .onEnd([]() {
  //     Serial.println("\nEnd");
  //   })
  //   .onProgress([](unsigned int progress, unsigned int total) {
  //     Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  //   })
  //   .onError([](ota_error_t error) {
  //     Serial.printf("Error[%u]: ", error);
  //     if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
  //     else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
  //     else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
  //     else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
  //     else if (error == OTA_END_ERROR) Serial.println("End Failed");
  //   });

  // ArduinoOTA.begin();

  // Serial.println("Ready");
  // Serial.print("IP address: ");
  // Serial.println(WiFi.localIP());
  delay(2000);
}

void loop()
{
  // ArduinoOTA.handle();
  unsigned long now = millis();

  while (GPSserial.available())
  {
    // Serial.write(GPSserial.read());
    if (gps.encode(GPSserial.read())){
      displayInfo();
      checkDate();
    }

    if (millis() > 5000 && gps.charsProcessed() < 10)
    {
      Serial.println(F("No GPS detected: check wiring."));
      while(true);
    }
  }
  
  if (now -lastWrite >= 1000) {
    writeLine(SD);
    lastWrite = millis();
  }
}

void displayInfo()
{
  if (gps.location.isUpdated()) {
    Serial.print(F("LOCATION   Fix Age="));
    Serial.print(gps.location.age());
    Serial.print(F("ms Raw Lat="));
    Serial.print(gps.location.rawLat().negative ? "-" : "+");
    Serial.print(gps.location.rawLat().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLat().billionths);
    Serial.print(F(" billionths],  Raw Long="));
    Serial.print(gps.location.rawLng().negative ? "-" : "+");
    Serial.print(gps.location.rawLng().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLng().billionths);
    Serial.print(F(" billionths],  Lat="));
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(" Long="));
    Serial.println(gps.location.lng(), 6);
  }

  else if (gps.date.isUpdated()) {
    Serial.print(F("DATE       Fix Age="));
    Serial.print(gps.date.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.date.value());
    Serial.print(F(" Year="));
    Serial.print(gps.date.year());
    Serial.print(F(" Month="));
    Serial.print(gps.date.month());
    Serial.print(F(" Day="));
    Serial.println(gps.date.day());
  }

  else if (gps.time.isUpdated()) {
    Serial.print(F("TIME       Fix Age="));
    Serial.print(gps.time.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.time.value());
    Serial.print(F(" Hour="));
    Serial.print(gps.time.hour());
    Serial.print(F(" Minute="));
    Serial.print(gps.time.minute());
    Serial.print(F(" Second="));
    Serial.print(gps.time.second());
    Serial.print(F(" Hundredths="));
    Serial.println(gps.time.centisecond());
  }

  else if (gps.speed.isUpdated()) {
    Serial.print(F("SPEED      Fix Age="));
    Serial.print(gps.speed.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.speed.value());
    Serial.print(F(" Knots="));
    Serial.print(gps.speed.knots());
    Serial.print(F(" MPH="));
    Serial.print(gps.speed.mph());
    Serial.print(F(" m/s="));
    Serial.print(gps.speed.mps());
    Serial.print(F(" km/h="));
    Serial.println(gps.speed.kmph());
  }

  else if (gps.course.isUpdated()) {
    Serial.print(F("COURSE     Fix Age="));
    Serial.print(gps.course.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.course.value());
    Serial.print(F(" Deg="));
    Serial.println(gps.course.deg());
  }

  else if (gps.altitude.isUpdated()) {
    Serial.print(F("ALTITUDE   Fix Age="));
    Serial.print(gps.altitude.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.altitude.value());
    Serial.print(F(" Meters="));
    Serial.print(gps.altitude.meters());
    Serial.print(F(" Miles="));
    Serial.print(gps.altitude.miles());
    Serial.print(F(" KM="));
    Serial.print(gps.altitude.kilometers());
    Serial.print(F(" Feet="));
    Serial.println(gps.altitude.feet());
  }

  else if (gps.satellites.isUpdated()) {
    Serial.print(F("SATELLITES Fix Age="));
    Serial.print(gps.satellites.age());
    Serial.print(F("ms Value="));
    Serial.println(gps.satellites.value());
  }

  else if (gps.hdop.isUpdated()) {
    Serial.print(F("HDOP       Fix Age="));
    Serial.print(gps.hdop.age());
    Serial.print(F("ms raw="));
    Serial.print(gps.hdop.value());
    Serial.print(F(" hdop="));
    Serial.println(gps.hdop.hdop());
  }

  else if (millis() - last > 5000) {
    Serial.println();
    if (gps.location.isValid())
    {
      static const double LONDON_LAT = 51.508131, LONDON_LON = -0.128002;
      double distanceToLondon =
        TinyGPSPlus::distanceBetween(
          gps.location.lat(),
          gps.location.lng(),
          LONDON_LAT, 
          LONDON_LON);
      double courseToLondon =
        TinyGPSPlus::courseTo(
          gps.location.lat(),
          gps.location.lng(),
          LONDON_LAT, 
          LONDON_LON);

      Serial.print(F("LONDON     Distance="));
      Serial.print(distanceToLondon/1000, 6);
      Serial.print(F(" km Course-to="));
      Serial.print(courseToLondon, 6);
      Serial.print(F(" degrees ["));
      Serial.print(TinyGPSPlus::cardinal(courseToLondon));
      Serial.println(F("]"));
    }

    Serial.print(F("DIAGS      Chars="));
    Serial.print(gps.charsProcessed());
    Serial.print(F(" Sentences-with-Fix="));
    Serial.print(gps.sentencesWithFix());
    Serial.print(F(" Failed-checksum="));
    Serial.print(gps.failedChecksum());
    Serial.print(F(" Passed-checksum="));
    Serial.println(gps.passedChecksum());

    if (gps.charsProcessed() < 10)
      Serial.println(F("WARNING: No GPS data.  Check wiring."));

    last = millis();
    Serial.println();
  }
   
}

// void testFileIO(fs::FS &fs, const char * path){
//   File file = fs.open(path);
//   static uint8_t buf[512];
//   size_t len = 0;
//   uint32_t start = millis();
//   uint32_t end = start;
//   if(file){
//     len = file.size();
//     size_t flen = len;
//     start = millis();
//     while(len){
//       size_t toRead = len;
//       if(toRead > 512){
//         toRead = 512;
//       }
//       file.read(buf, toRead);
//       len -= toRead;
//     }
//     end = millis() - start;
//     Serial.printf("%u bytes read for %u ms\n", flen, end);
//     file.close();
//   } else {
//     Serial.println("Failed to open file for reading");
//   }


//   file = fs.open(path, FILE_WRITE);
//   if(!file){
//     Serial.println("Failed to open file for writing");
//     return;
//   }

//   size_t i;
//   start = millis();
//   for(i=0; i<2048; i++){
//     file.write(buf, 512);
//   }
//   end = millis() - start;
//   Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
//   file.close();
// }

void checkDate() {
  if (gps.date.isUpdated()){
    int year = gps.date.year();
    int month = gps.date.month();
    int day = gps.date.day();
    char gpsDate[12];
    sprintf(gpsDate,"%i-%i-%i", year , month, day);

    if (strcmp(gpsDate, currentDate) != 0) {
      Serial.println("Need to update current Date");
      sprintf(currentDate,"%i-%i-%i", year , month, day);
    }
  }
}

void writeLine(fs::FS &fs){
  if (gps.location.isValid()) {
    char filename[20];
    sprintf(filename, "/%s.csv", currentDate);
    char msg[255] = "";
    char lat[12];
    char lon[12];
    char speed[9];
    char alt[9];
    char course[5];

    dtostrf(gps.location.lat(),1,6,lat);
    dtostrf(gps.location.lng(),1,6,lon);
    dtostrf(gps.speed.mph(),1,3,speed);
    dtostrf(gps.altitude.feet(),1,3,alt);
    dtostrf(gps.course.deg(),1,2,course);


    sprintf(msg, "%i/%i/%i,%i:%i:%i,%s,%s,%s,%s,%s,%i\n", gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second(), lat, lon, speed, alt, course, gps.satellites.value());

    Serial.println(msg);

    if (sdCardPresent == 1) {
      File file = fs.open(filename, FILE_APPEND);
      if (!file) {
        Serial.println("Failed to open file for writing");
        return;
      }
      if (file.print(msg)) {
        Serial.print("File written: ");
        Serial.println(filename);
      } else {
        Serial.println("Write failed");
      }

      file.close();
    }
  }
  
  else {
    Serial.print("Awaiting Location Details - ");
    Serial.println(gps.satellites.value());
    Serial.println("-----");
  }
}