#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>   // Include the SPIFFS library
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>

const char* ssid = "IoT Home";      // replace with your desired SSID for the hotspot
const char* password = "cuadra123"; // replace with your desired password for the hotspot

#define DHTPIN D4         // DHT11 pin connected to nodemcu D4 pin
#define DHTTYPE    DHT11  // DHT 11
DHT dht(DHTPIN, DHTTYPE);

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32 

#define OLED_RESET     LED_BUILTIN 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float t = 0.0;
float h = 0.0;

AsyncWebServer server(80);      // Wifi Server
unsigned long previousMillis = 0;   
const long interval = 1000;  

unsigned long lastClientCheckMillis = 0;
const long clientCheckInterval = 600000; // 10 minutes in milliseconds

void setup(){
  Serial.begin(115200);
  dht.begin();              // Initializing DHT11 Sensor
  WiFi.softAP(ssid, password);  // Set up ESP8266 as a WiFi hotspot
  Serial.println("WiFi hotspot is up");
  Serial.println(WiFi.softAPIP());  // Print the IP address of the ESP8266

  if (!SPIFFS.begin()) {  // Initialize SPIFFS
    Serial.println("An error has occurred while mounting SPIFFS");
    return;
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(t).c_str());
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(h).c_str());
  });

  server.begin();               // Initializing Wifi Server

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {   // Address of OLED Display
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
}

void loop(){  
  unsigned long currentMillis = millis();
  
  // Check the DHT sensor at regular intervals
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    float newT = dht.readTemperature();
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");   
    } else {
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

    display.display(); // Make sure to update the display
  }
  
  // Check for connected clients
  if (WiFi.softAPgetStationNum() > 0) {
    lastClientCheckMillis = currentMillis; // Reset the timer if a client is connected
  } else if (currentMillis - lastClientCheckMillis >= clientCheckInterval) {
    WiFi.softAPdisconnect(true); // Disconnect the AP if no clients have connected within the interval
    Serial.println("No clients connected for 10 minutes. AP turned off.");
  }
}
