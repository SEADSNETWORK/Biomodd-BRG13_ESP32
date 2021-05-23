//http://biomodd.xyz/push?author=jeronimo&device=ESP32&sensor=ADS1115&value=A0
//http://biomodd.xyz/data?device=ESP32
//ref.: https://randomnerdtutorials.com/esp32-http-get-post-arduino/
//
//Comparator + ADS1115
//
// Push once a minute
//

#include <Adafruit_ADS1X15.h>
Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

#include <WiFi.h>
//#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

HTTPClient http;
const char* ssid = "telenet-2EC9A";
const char* password = "7k86G8MwANF5";

unsigned long lastTime = 0;
unsigned long timerDelay = 60000; // 60 seconds-> 60 datapoints/hr -> 1440 datapoint/24hrs.

// BIOMODD.xyz variables
//String serverPath = "https://biomodd.xyz/push?author=jeronimo&device=ESP32&sensor=ADS1115&value=BIOMODDTEST";
String serverPath = "https://biomodd.xyz/push?author=jeronimo&device=ESP32&sensor=ADS1115"; // &value=XX";
//String serverPath;

// ThingSpeak variables
const char* speakName = "http://api.thingspeak.com/update";
// Service API Key
String apiKey = "Q9AJL9FHKDLC6O45";


void setup() {

  // Connect to WIFI
  Serial.println("Good Day :)");
  Serial.begin(115200);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(2, HIGH);
    delay(500);
    Serial.print(".");
    digitalWrite(2, LOW);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //digitalWrite(LED_BUILTIN, HIGH);

  // ADS1115 Setup
  Serial.println("Single-ended readings from AIN0 with >3.0V comparator");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");
  Serial.println("Comparator Threshold: 1000 (3.000V)");

  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                          ADS1115
  //                                                          -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V    0.1875mV (default)
  ads.setGain(GAIN_ONE);          //  1x gain   +/- 4.096V    0.125mV

  //ads.setGain(GAIN_TWO);        //  2x gain   +/- 2.048V    0.0625mV
  //ads.setGain(GAIN_FOUR);       //  4x gain   +/- 1.024V    0.03125mV
  // ads.setGain(GAIN_EIGHT);     //  8x gain   +/- 0.512V    0.015625mV
  // ads.setGain(GAIN_SIXTEEN);   //  16x gain  +/- 0.256V    0.0078125mV

  ads.begin();

  // Setup 3V comparator on channel 0
  ads.startComparator_SingleEnded(0, 1000);

}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    //Get value from ADS1115
    int16_t adc0;

    adc0 = ads.getLastConversionResults();

    //Serial Print
    //Serial.print("AIN0: "); Serial.println(adc0 * 0.125); // 1x gain example
    Serial.println("----------------");
    Serial.println("Hello, this is my pulse:"); 
    Serial.println(adc0 * 0.125); // single value (ico. Processing)
    Serial.println("- - - - - - - - ");
    
    //Check WiFi connection status
    if ((WiFi.status() == WL_CONNECTED)) //Check the current connection status
    {
      // BIOMODD.....................................................
      // http.begin("wss://biomodd.xyz/data?device=ESP32");
      // http.begin("wss://biomodd.xyz/push?author=Hieronimo&device=ESP32&sensor=ADS1115&value=99");
      // HTTPS push via Restful API:

      serverPath +=  String(adc0 * 0.125);

      Serial.println("BIOMODD");
      
      //  http.begin(serverPath.c_str());
      int httpCode =  http.GET();
      //delay(1000);

      if (httpCode > 0)
      {
        //digitalWrite(2, HIGH);// pin 2 = led on esp32
        String payload =  http.getString();
        Serial.println("\nStatuscode: " + String(httpCode));
        //            Serial.println(payload);
        //      delay(1000);
        //digitalWrite(2, LOW);
      }
      else
      {
        Serial.println("Error on biomodd.xyz HTTP request");
      }
    }
    else
    {
      Serial.println("biomodd.xyz Connection lost!!");
    }

    // ThingSpeak.....................................................
    Serial.println("- - - - - - - - ");
    Serial.println("ThingSpeak");
    Serial.println("");
    
    http.begin(speakName);

    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // Data to send with HTTP POST
    String httpRequestData = "api_key=" + apiKey + "&field1=" + String(adc0 * 0.125);
    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.println("----------------");
    Serial.println("");
    
    //delay(3000);
    lastTime = millis();
    
//    // Free resources
//    http.end();
  }

}
