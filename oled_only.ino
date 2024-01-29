#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHTPIN D4         // DHT11 pin connected to nodemcu D4 pin
#define DHTTYPE    DHT11     // DHT 11
DHT dht(DHTPIN, DHTTYPE);

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32 

#define OLED_RESET     LED_BUILTIN 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float t = 0.0; // Global temperature variable
float h = 0.0; // Global humidity variable

void setup(){
  Serial.begin(115200);
  dht.begin();              // Initializing DHT11 Sensor

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {   // Address of OLED Display
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
}

void loop() {
  unsigned long currentMillis = millis();
  static unsigned long previousMillis = 0;
  const long interval = 1000;  // Update every 1 second

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float newT = dht.readTemperature();
    if (!isnan(newT)) {
      t = newT;

      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);

      // Center the title "WEATHERSENSE" horizontally
      String title = "WEATHERSENSE";
      int16_t titleWidth = title.length() * 6; // Approximate width of each character
      int16_t titleX = (SCREEN_WIDTH - titleWidth) / 2;

      display.setCursor(titleX, 0);
      display.print(title);

      // Draw a line underneath the title
      int16_t lineStartX = titleX - 10;
      int16_t lineEndX = titleX + titleWidth + 10;
      display.drawLine(lineStartX, 9, lineEndX, 9, WHITE); // Line is placed 9 pixels below the title

      // Display temperature
      String tempString = "Temp: " + String(t) + "C";
      int16_t tWidth = tempString.length() * 6; // Approximate width of each character
      int16_t tX = (SCREEN_WIDTH - tWidth) / 2;

      display.setCursor(tX, 15);
      display.print(tempString);
    }

    float newH = dht.readHumidity();
    if (!isnan(newH)) {
      h = newH;

      // Center the humidity text horizontally
      String humidityString = "Humidity: " + String(h) + "%";
      int16_t hWidth = humidityString.length() * 6; // Approximate width of each character
      int16_t hX = (SCREEN_WIDTH - hWidth) / 2;

      display.setCursor(hX, 24);
      display.print(humidityString);
    }

    display.display();
  }
}
