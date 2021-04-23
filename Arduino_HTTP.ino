/*  The Workshop - Arduino HTTP Control V1.0 - April 2021
 *   
 *  This sketch will create an HTTP server on the arduino.
 *  The server can be accessed using a client browser.
 *  The pins can be controlled depending on the request sent.
 *    http://arduino_ip/pin/0 will set the OUTPUT_PIN to LOW,
 *    http://arduino_ip/pin/1 will set the OUTPUT_PIN to HIGH
 *  arduino_ip is the IP address of the ESP8266 module, 
 *  The IP will be printed to Serial when connected.
 *  
 *  The Temperature and Humidity will also be printed.
 *  To update the readings, refresh the browser page
 *  The Sensor used is a DHT11 sensors
 *  The needed libraries can be downloaded here: 
 *  - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
 *  - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor
 *  See guide for details on sensor wiring and usage: 
 *  https://learn.adafruit.com/dht/overview
 */

#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

//Wifi Credentials
const char* ssid = "Your WiFi Name";
const char* password = "Your WiFi Password";

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

#define OUTPUT_PIN 12   //[D6 on ESP8266] Digital pin connected to the Output to be controlled (LED light)
#define DHTPIN 2        //[D4 on ESP8266] Digital pin connected to the DHT sensor 
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11
//#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT_Unified dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(OUTPUT_PIN, OUTPUT);      // Initialize the LED_BUILTIN pin as an output
  pinMode(0, OUTPUT);               // prepare GPIO2
  delay(2000);

  dht.begin(); //Initialize DHT
  Serial.println("");
  Serial.println("Arduino HTTP Control V1.0");
  Serial.println("The Workshop - March 2021");
  Serial.println("If you enjoy leave a LIKE!");
  
  sensor_t sensor;      // Print sensor details.
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature & Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Temperature Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Temperature Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Temperature Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  dht.humidity().getSensor(&sensor);
  Serial.print  (F("Humidity Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Humidity Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Humidity Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.println("Connecting to WiFi");
  //Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  server.begin();                   // Start the server
  Serial.println("Server started");
  Serial.println(WiFi.localIP());   // Print the IP address
}

void loop() {
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  float curr_temp = 0;
  float curr_hum = 0;
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    //Serial.print(F("Temperature: "));
    curr_temp = event.temperature;
    //Serial.print(curr_temp);
    //Serial.println(F("°C"));
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    //Serial.print(F("Humidity: "));
    curr_hum = event.relative_humidity;
    //Serial.print(curr_hum);
    //Serial.println(F("%"));
  }
  
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) { return;}
  
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){ delay(1);}
  
  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  int val;
  if (req.indexOf("/pin/0") != -1){
    val = 0;
    Serial.print(F("Temperature: "));
    Serial.print(curr_temp);
    Serial.println(F("°C"));
    Serial.print(F("Humidity: "));
    Serial.print(curr_hum);
    Serial.println(F("%"));
  }
  else if (req.indexOf("/pin/1") != -1){
    val = 1;
    Serial.print(F("Temperature: "));
    Serial.print(curr_temp);
    Serial.println(F("°C"));
    Serial.print(F("Humidity: "));
    Serial.print(curr_hum);
    Serial.println(F("%"));
  }
  else {
    Serial.println("invalid request");
    client.stop();
    return;
  }

  // Set OUTPUT_PIN according to the request
  digitalWrite(0, val);
   digitalWrite(OUTPUT_PIN, val);   // Turn the LED on (Note that LOW is the voltage level
  client.flush();
  
  // Prepare the response
  String s =  
  s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\n";
  s +="<h1>The Workshop - Arduino HTTP Control V1.0<h1>";
  s +="<h2><br>Output_Pin is now ";
  s += (val)?"High":"Low";
  s += "<br>Current Temperature is ";
  s += curr_temp;
  s += "&degC<br>Current Humidity is ";
  s += curr_hum;
  s += "%<br><br>___________________________________________________<br>Support me by LIKING the video!<br>SUBSCRIBE so you don't miss any future episode!<h2>";   //Actually LIKE and SUBSCRIBE, this means a lot to me!
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(10);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}
