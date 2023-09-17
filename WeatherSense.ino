#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// Replace with your network credentials
const char* ssid = "Proton";
const char* password = "cuadra123";

#define DHTPIN 5     // Digital pin connected to the DHT sensor

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 10 seconds
const long interval = 10000;  

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

// Replaces placeholder with DHT values
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
  // Serial port for debugging purposes
  Serial.begin(115200);
  dht.begin();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });

  // Start server
  server.begin();
}
 
void loop(){  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      Serial.println(h);
    }
  }
}