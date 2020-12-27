#include "board_secrets.h"
#include "board_builtins.h"

WiFiSSLClient client;

unsigned long prevMillis = 0;
char strBuffer[20];

void setup() {
  Board_LED_Init();
  Board_Serial_Init();
  Board_Temp_Init();
  Board_Light_Init();
  Board_Wifi_Init(WIFI_SSID, WIFI_PASS);

  // Press button to continue
  Board_Serial.println("Press button to start");
  pinMode(BUTTON_SW1, INPUT);
  while (digitalRead(BUTTON_SW1) != LOW) {
    digitalWrite(LED_RED, !digitalRead(LED_RED));
    digitalWrite(LED_YELLOW, !digitalRead(LED_YELLOW));
    digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
    digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
    delay(250);
  }
  Board_Serial.println("Starting up...");
}

uint64_t tagId = 0;

void loop() {
  // Found tag ID
  uint64_t currentTagId = Gwiot7941e_update(&Board_Serial);
  if (currentTagId != 0) {
    tagId = currentTagId;
  }

  // Temperature sensor
  float tempC = tempsensor.readTempC();
  Board_Serial.print("Temp: "); 
  Board_Serial.print(tempC, 4);
  Board_Serial.println(" Celsius"); 

  // Light sensor
  int light_adc = analogRead(LIGHTSENSORPIN); //Read light level
  float light_percent = light_adc / 1023.0 * 100;      //Get percent of maximum value (10 bits)
  Board_Serial.print("Light: "); 
  Board_Serial.println(light_percent);

  // Seperator
  Board_Serial.println("");

  // LEDs 
  digitalWrite(LED_RED, !digitalRead(LED_RED));
  delay(100);
  digitalWrite(LED_YELLOW, !digitalRead(LED_YELLOW));
  delay(100);
  digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
  delay(100);
  digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
  delay(100);

  // Update cloud every 5 seconds
  if ((millis() - prevMillis) >= 5000) {
    // Setup LEDs
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);

    Board_Serial.print("Pushing to cloud - "); 
    Board_Serial.println(millis() / 1000); 

    String payloadLight = "value=" + String(light_percent, 2);
    sendToCloud("light", payloadLight);
    digitalWrite(LED_GREEN, HIGH);

    String payloadTemp = "value=" + String(tempC, 4);
    sendToCloud("temp", payloadTemp);

    if (tagId) {
      digitalWrite(LED_BLUE, HIGH);
      String payloadLog = "value=Found tag: " + String((uint32_t) (tagId >> 32), HEX) + String((uint32_t) (tagId), HEX);
      sendToCloud("entry-log", payloadLog);
      tagId = 0;
    }
    

    prevMillis = millis();

    // Reset LEDs
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_BLUE, LOW);
  }

  // Check for disconnection
  if (WiFi.status() != WL_CONNECTED) {
    Board_Serial.println();
    Board_Serial.println("Disconnected form server. Halting.");
    client.stop();
    while (true) {
      digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
      delay(250);
    }
  }

}


void sendToCloud(String feed, String data) {
  char * host = "io.adafruit.com"; // hostname address
  String path = String("/api/v2/" IO_USERNAME "/feeds/") + feed + String("/data?X-AIO-Key=" IO_KEY);

  if (client.connectSSL(host, 443)) {
    Board_Serial.println("Connected to cloud...");
    
    Board_Serial.println("POST " + path + " HTTP/1.1");
    Board_Serial.print("Host: "); Board_Serial.println(host);
    Board_Serial.println("Content-Type: application/x-www-form-urlencoded");
    Board_Serial.print("Content-Length: "); Board_Serial.println(data.length());
    Board_Serial.println();
    Board_Serial.print(data);
    Board_Serial.println();
    
    client.println("POST " + path + " HTTP/1.1");
    client.print("Host: "); client.println(host);
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: "); client.println(data.length());
    client.println();
    client.print(data);
    client.stop();

    Board_Serial.println("Data sent successfully to cloud...");
  } else {
    Board_Serial.println("Failed to connect to cloud...");
  }
}
