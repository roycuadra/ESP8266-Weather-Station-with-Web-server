#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_I2CDevice.h>

const char* ssid = "nice";      // replace your Wifi Username
const char* password = "cuadra123";    // replace your wifi password

#define DHTPIN D4         // DHT11 pin connected to nodemcu D4 pin
#define DHTTYPE    DHT11     // DHT 11
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

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
   <meta name="author" content="Roy Cuadra">
   <meta name="description" content="Weather-Station">
   <meta name="viewport" content="width=device-width, initial-scale=1">
   <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" 
   integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
   <title>WeatherSense</title>
   <style>
     body {
       font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
       background-image: url("https://p4.wallpaperbetter.com/wallpaper/92/95/748/5bd0eb701f715-wallpaper-preview.jpg");
       margin: 0;
       background-size: cover;
       background-repeat: no-repeat;
       padding: 0;
       display: flex;
       justify-content: center;
       align-items: center;
       height: 100vh;
     }
     .container {
  text-align: center;
  background-color: rgba(231, 219, 219, 0.5); /* Adjust the alpha (0.7) for transparency */
  border-radius: 20px;
  padding: 20px;
  box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.2);
}
     h2 {
       font-size: 2rem;
       color: #333;
     }
     p {
       font-size: 1.5rem;
       color: #555;
       display: flex;
       align-items: center;
     }
     .dht-labels {
       font-size: 1.2rem;
       vertical-align: middle;
       padding-left: 10px;
       color: #2c2b2b;
     }
     i {
       font-size: 2rem;
       margin-right: 10px;
     }
     span {
       font-size: 2rem;
       color: #333;
     }
     sup.units {
       font-size: 1.2rem;
       color: #777;
     }
     .icon-temperature {
       color: #059e8a;
     }
     .icon-humidity {
       color: #00add6;
     }
     .digital-clock {
  font-size: 2rem;
  font-weight: bold;
  color: #333;
  margin-top: 20px;
}

   </style>
 </head>
 <body>
   <div class="container">
     <h2>WeatherSense</h2>
     <div id="digitalClock" class="digital-clock"></div>

     <p>
       <i class="fas fa-thermometer-half icon-temperature"></i>
       <span class="dht-labels">Temperature</span>
       <span id="temperature">%TEMPERATURE%</span>
       <sup class="units">&deg;C</sup>
     </p>
     <p>
       <i class="fas fa-tint icon-humidity"></i>
       <span class="dht-labels">Humidity</span>
       <span id="humidity">%HUMIDITY%</span>
       <sup class="units">%</sup>
     </p>
   </div>
<script>

function updateDigitalClock() {
  const digitalClock = document.getElementById("digitalClock");
  const now = new Date();
  let hours = now.getHours();
  const minutes = now.getMinutes().toString().padStart(2, '0');
  const seconds = now.getSeconds().toString().padStart(2, '0');
  let ampm = hours >= 12 ? 'PM' : 'AM';

  // Convert hours to 12-hour format
  if (hours > 12) {
    hours -= 12;
  }
  if (hours === 0) {
    hours = 12;
  }

  digitalClock.textContent = `${hours}:${minutes}:${seconds} ${ampm}`;
}

// Update the clock every second
setInterval(updateDigitalClock, 1000);

// Initialize the clock
updateDigitalClock();
</script>
<script>
    // For Data 
    setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</body>
</html>)rawliteral";
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  return String();
}

void setup(){
  Serial.begin(115200);
  dht.begin();              // Initializing DHT11 Sensor
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });
  server.begin();               // Initiz=alizing Wifi Server

 if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {   // Address of OLED Display
 Serial.println(F("SSD1306 allocation failed"));
 for(;;);
 }
}

 
void loop(){  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    float newT = dht.readTemperature();
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");   
    }
    else {
      t = newT;

      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.print("Temp:");
      display.print(newT);
      Serial.println(t);
    }
    float newH = dht.readHumidity(); 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;

     display.setCursor(0,17);  
     display.print("Humi:");
     display.print(newH);
     display.display();
     Serial.println(h);
    }
  }
}
