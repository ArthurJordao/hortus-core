/*Code for Device Hub IoT interaction based on board NodeMCU, module WIFI 1.0(ESP-12E). 
    Code still needs to be calibrated based on Plants.
    Connect D0 (GPIO16) pin to RST for DeepSleep to work!!!!!
 
    by Alexandra Percario 2017
    Based on Leonardo Gonçalves original code with edits by Timothy Woo.
 */
#include <ESP8266WiFi.h>
const char* ssid     = "WIFINAMEHERE";
const char* password = "WIFIPASSHERE";
String pubString;
String pubString2;
const char* host = "api.devicehub.net";//api.devicehub.net
int Analog_Pin = A0; // ADC Pin Will be Multiplexed 
float Water_Level = 0; // values
int Moisture_Level_1 = 0;
int Moisture_Level_2 = 0;
int IN1 = D4;
int IN2 = D5;
int IN3 = D6;
const int sleepTimeS = 1*10; // Set sleep time in seconds for the deep sleep cycle

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(D1, OUTPUT); //WATER_PWM
  pinMode(D2, OUTPUT); //MOISTURE_PWM1
  pinMode(D3, OUTPUT); //MOISTURE_PWM2
  Serial.begin(115200);
  delay(100);

  // Connecting to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); //NodeMCU IP address.

  POST(); // Post the value to the web database
  GET();  

  Serial.println("ESP8266 in sleep mode");
  // Put NodeMCU in deep sleep. When it wakes up it will run setup() again,
  // connect to WiFi, then post and/or get data, then go back to sleep and repeat
  ESP.deepSleep(sleepTimeS * 1000000);
}

void loop()
{
 //No need
}

void POST(void)
{
  delay(1000);
  //init all low
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  
  //WATER_LEVEL
  digitalWrite(D1, HIGH);
  Water_Level = analogRead(Analog_Pin); // Read Water value //MUST CALIBRATE!!!!!
  Serial.print("Water Level: ");
  Serial.println(Water_Level);
  digitalWrite(D1, LOW); //ends low
  
  //MOISTURE_LEVEL1
  digitalWrite(D2, HIGH);
  Moisture_Level_1 = analogRead(Analog_Pin); // Read Mosture1 value //MUST CALIBRATE!!!!!
  Serial.print("Moisture Level 1: ");
  Serial.println(Moisture_Level_1);
  digitalWrite(D2, LOW); //ends low

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  pubString = "{\"value\": " + String(Water_Level) + "}";
  String pubStringLength = String(pubString.length(), DEC);
  // ------------------------------ ?? -------------------------
  pubString2 = "{\"value\": " + String(Moisture_Level_1) + "}";
  String pubStringLength2 = String(pubString2.length(), DEC);
  
  Serial.print("Requesting POST: ");
  client.println("POST /v2/project/13602/device/3f7db2ff-58b3-4078-947a-46783fb1df10/sensor/WaterLevel/data HTTP/1.1");
  client.println("POST /v2/project/13602/device/3f7db2ff-58b3-4078-947a-46783fb1df10/sensor/MoistureLevel1/data HTTP/1.1");
  //parts above are from DeviceHub.net. 
  
  client.println("Host: api.devicehub.net");
  client.print("Api_Key: 5bf991df-e791-4009-a3f6-915c2a7d25f2\r\n"); // Get this from DeviceHub.net
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(pubStringLength);
  client.println();
  client.print(pubString);
  client.println();
  client.println(pubStringLength2);
  client.println();
  client.print(pubString2);
  client.println();
  delay(500); 
  
  //reads all lines from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  Serial.println();
  Serial.println("closing connection");
}

//get data from DeviceHub
void GET(void)
{
  int state_pos;
  String state;
  delay(1000);
  Water_Level = analogRead(Analog_Pin); // Read Water value
  Moisture_Level_1 = analogRead(Analog_Pin); //Read Moisture1 value
  Serial.print("connecting to ");
  Serial.println(host);
  
  
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  pubString = "{\"value\": " + String(Water_Level) + "}";
  String pubStringLength = String(pubString.length(), DEC);
  // ---------------------------------??----------------------
  pubString2 = "{\"value\": " + String(Moisture_Level_1) + "}";
  String pubStringLength2 = String(pubString2.length(), DEC);
  
  Serial.print("Requesting GET: ");
  client.println("GET /v2/project/13602/device/3f7db2ff-58b3-4078-947a-46783fb1df10/actuator/BLYNK/state HTTP/1.1");
  client.println("Host: api.devicehub.net");
  client.print("Api_Key: 5bf991df-e791-4009-a3f6-915c2a7d25f2\r\n");
  client.println("Connection: close");
  client.println();
  delay(500);

  while (client.available()) {
    String line = client.readStringUntil('\r');
    state_pos = line.indexOf("state");
    if (state_pos > 0)
      state = line.substring(state_pos + 7, state_pos + 8);
    else
      state = "nothing";
    Serial.println(state);
    if (state == "1")
      Serial.println("LED ON");
      digitalWrite(D4, LOW);// or whatever pin you want to blink a led
    if (state == "0")
      Serial.println("LED OFF");
      digitalWrite(D4, HIGH);// or whatever pin you want to blink a led
    
  }
  Serial.println();
  Serial.println("closing connection");
}
