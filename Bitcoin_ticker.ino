#include <WiFiManager.h>
#include <Arduino.h>

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
//>>TIRA DE LED RGB (5V)
int redPin = D3;
int greenPin = D4;
int bluePin = D1;
 

 

const uint16_t WAIT_TIME = 1000;

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 6
#define CLK_PIN   D5
#define DATA_PIN  D7
#define CS_PIN    D2
#define MAX_MESG  20
#define LOOPDURATION    15000

// Hardware SPI connection
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

const char* ssid = "*****";
const char* password = "******";
const char* host = "api.coindesk.com";
float previousValue = 0.00;
float threshold = 0.05;

uint8_t scrollSpeed = 25;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_CENTER;
uint16_t scrollPause = 1500; // in milliseconds
char szMesg[MAX_MESG + 1] = "JardinFinanciero.com";

unsigned long currentMillis = 0;
unsigned long previousMillis = 0;

int bufferSize = 3000; // for ArduinoJSON

bool dataAvailable = false;

long hours = 0;
long minutes = 0;
long seconds = 0;

char currencyInfos[3000];


void setup()
{
 
  pinMode(redPin, OUTPUT); // red
  pinMode(greenPin, OUTPUT); // green
  pinMode(bluePin, OUTPUT); // blue
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);
  
  pinMode(D3, OUTPUT); //Price down
  pinMode(D4, OUTPUT); //Price up
 
 
  Serial.begin(115200);               
  delay(10);
 // case '1':
//    digitalWrite(redPin, LOW);
//    break;
//  case '2':
//    digitalWrite(redPin, HIGH);
//    break;
//  case '3':
//    digitalWrite(greenPin, LOW);
//    break;
//  case '4':
//    digitalWrite(greenPin, HIGH);
//    break;
//  case '5':
    digitalWrite(bluePin, LOW);
//    break;
//  case '6':
//    digitalWrite(bluePin, HIGH);
//    break;
//  }
 //WiFi.begin(ssid, password);
//  WiFiManager wifiManager;
 //Uncomment for reset ssid & password every reboot.
//  wifiManager.resetSettings();

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  P.begin();
  P.displayText(szMesg, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);

}

void loop() {
  
  // Every LOOPDURATION seconds
      currentMillis = millis();
      if(currentMillis - previousMillis >= LOOPDURATION) {
        previousMillis = currentMillis;    
    
        getCurrencyInfos();
      }

  if (P.displayAnimate())
  {
    P.displayReset();
  }
      
}


void getCurrencyInfos(){
  // Connect to API
  Serial.print("connecting to ");
  Serial.println(host);
 
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
 
  // We now create a URI for the request
  String url = "/v1/bpi/currentprice.json";
 
  Serial.print("Requesting URL: ");
  Serial.println(url);
 
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(500); //more than 100. Recommended 500.
 
  // Read all the lines of the reply from server and print them to Serial
  String answer;
  while(client.available()){
    String line = client.readStringUntil('\r');
    answer += line;
  }

  client.stop();
  Serial.println();
  Serial.println("closing connection");

  // Process answer
  Serial.println();
  Serial.println("Answer: ");
  Serial.println(answer);

  // Convert to JSON
  String jsonAnswer;
  int jsonIndex;

  for (int i = 0; i < answer.length(); i++) {
    if (answer[i] == '{') {
      jsonIndex = i;
      break;
    }
  }

  // Get JSON data
  jsonAnswer = answer.substring(jsonIndex);
  Serial.println();
  Serial.println("JSON answer: ");
  Serial.println(jsonAnswer);
  jsonAnswer.trim();

  // Get rate as float
  int rateIndex = jsonAnswer.indexOf("rate_float");
  String priceString = jsonAnswer.substring(rateIndex + 12, rateIndex + 18);
  priceString.trim();
  float price = priceString.toFloat();

  // Print price
  Serial.println();
  Serial.println("Bitcoin price: ");
  Serial.println(price);
    

  // Init previous value
  if (previousValue == 0.00) {
    previousValue = price;
  }

  // Alert down ?
  if (price < (previousValue - threshold)) {

    // Flash LED
    digitalWrite(bluePin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(redPin, LOW);

  }

  // Alert up ?
 if (price > (previousValue + threshold)) {

    // Flash LED
    digitalWrite(bluePin, HIGH);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);

  }
      previousValue = price;
      dtostrf(price, 1, 0, szMesg);
      strcat(szMesg, "$");
}


 
 
