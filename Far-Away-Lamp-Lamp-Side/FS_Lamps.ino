// Library Imports
#include "defines.h"
#include <WebSockets2_Generic.h>
#include <ESP8266WiFi.h>  
#include <ArduinoJson.h>
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager

// Pin Declerations
#define BUTTON_PIN D1
#define LED_PIN D2

// Give Credit for Websocket Library
using namespace websockets2_generic;

// Websocket Vars. 
WebsocketsClient client;
bool connected = false;

// Button Input Vars.
bool lastButtonState = false;
bool lampState = false;

void setup()
{
  // Serial Code
  Serial.begin(9600);
  while (!Serial);

  Serial.println("\n_________________________");

  // Defining Pinmodes and Interrupt
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  wifiManagerSetup();

  Serial.println("\nStarting Secured-ESP8266-Client on " + String(ARDUINO_BOARD));
  Serial.println(WEBSOCKETS2_GENERIC_VERSION);

  // Wait some time to connect to wifi
  for (int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++)
  {
    Serial.print(".");
    delay(300);
  }

  // Check if connected to wifi
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("No Wifi!");
    return;
  }

  //Serial.print("Connected to Wifi, Connecting to WebSockets Server @");
  //Serial.println(websockets_connection_string);

  // Run Callbacks
  client.onMessage(onMessageCallback);
  client.onEvent(onEventsCallback);

  // Before connecting, set the ssl fingerprint of the server
  client.setFingerprint(echo_org_ssl_fingerprint);

  Serial.println("Called Connect to server.");
  connectServer();

  // Send ping and then message with current lamp state
  client.ping();
  sendMessage();
}

void loop()
{
  bool curButtonState = !digitalRead(BUTTON_PIN);

  if (curButtonState != lastButtonState)
  {
    sendMessage();
  }
  
  client.poll();
  lastButtonState = curButtonState;

  

}

void sendMessage() {

  // Change Lamp State
  StaticJsonDocument<200> message;
  updateLampState(!digitalRead(BUTTON_PIN));
  message["groupId"] = GROUP_ID;
  message["lampState"] = lampState;

  // Send Message
  String messageAsText = "";
  serializeJson(message, messageAsText);
  Serial.print("Sent Message: ");
  Serial.println(messageAsText);
  client.send(messageAsText);
}

void onMessageCallback(WebsocketsMessage message)
{
  Serial.print("Got Message: ");
  Serial.println(message.data());

  String messageAsText = message.data();

  StaticJsonDocument<200> doc;

  DeserializationError  error = deserializeJson(doc, messageAsText);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  // Fetch values.
  bool isText = doc["isText"];
  if (isText) {
    String text = doc["text"];
    Serial.println(text);
  }
  else {
    bool newLampState = doc["lampState"];;
    updateLampState(newLampState);
  }

}

void updateLampState(bool newLampState) {
  lampState = newLampState;
  digitalWrite(LED_PIN, lampState);
}


void onEventsCallback(WebsocketsEvent event, String data)
{
  (void) data;

  if (event == WebsocketsEvent::ConnectionOpened)
  {
    Serial.println("Connnection Opened");
  }
  else if (event == WebsocketsEvent::ConnectionClosed)
  {
    Serial.println("Connnection Closed");
    connectServer();
  }
  else if (event == WebsocketsEvent::GotPing)
  {
    Serial.println("Got a Ping!");
    client.pong();
    client.ping();
  }
  else if (event == WebsocketsEvent::GotPong)
  {
    Serial.println("Got a Pong!");
  }

}

void wifiManagerSetup() {

  // WiFiManager
  // Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  wifiManager.setDebugOutput(DEBUG_STATE);

  // Uncomment and run it once, if you want to erase all the stored information
  //wifiManager.resetSettings();

  // set custom ip for portal
  //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

  // fetches ssid and pass from eeprom and tries to connect
  // if it does not connect it starts an access point with the specified name
  // and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("Far Away Lamp Wifi Portal");
  // or use this for auto generated name ESP + ChipID
  //wifiManager.autoConnect();

  // if you get here you have connected to the WiFi
  Serial.println("Connected.");


}

void connectServer(){
  // Connect to Server, re-do if it fails
  while (!connected) {
    connected = client.connect(WEBSOCKETS_CONNECTION_STRING);
    delay(6000 * connected);
    if (!connected){
      Serial.println("Not Connected!");
    }
  }
}


/*
  Tutorial Code is Based On: https://github.com/khoih-prog/WebSockets2_Generic/blob/master/examples/ESP8266/Secured-Esp8266-Client/Secured-Esp8266-Client.ino
  Aruino JSON Documentation: https://arduinojson.org/v6/example/parser/
*/
