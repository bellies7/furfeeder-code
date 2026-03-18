/*
 * FurFeeder — Smart IoT Pet Treat Dispenser
 * 
 * A motion-activated pet treat dispenser that sends alerts to AWS IoT Core
 * when your pet is detected. The owner can remotely trigger a treat dispense
 * via the AWS MQTT Test Client.
 * 
 * Hardware required:
 * - ESP32-C6-WROOM-1
 * - PIR Sensor (motion detection)
 * - IR Sensor (treat level detection)
 * - SG90 Servo Motor (treat dispensing mechanism)
 * - Red LED + 220Ω resistor (low treat indicator)
 * 
 * To dispense a treat, publish the following to topic esp32/aws-to-esp32:
 * {"command": "dispense"}
 */

#include "secrets.h"
#include <WiFiClientSecure.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <ESP32Servo.h>
#include <time.h>

// ============================================================
// MQTT Topics — change these if you want custom topic names
// ============================================================
#define AWS_IOT_PUBLISH_TOPIC   "esp32/esp32-to-aws"   // ESP32 sends alerts to AWS on this topic
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/aws-to-esp32"   // ESP32 listens for commands on this topic

// ============================================================
// Pin definitions — update these if you use different GPIO pins
// ============================================================
#define PIR_PIN     2    // PIR sensor signal wire
#define IR_PIN      3    // IR sensor signal wire
#define SERVO_PIN   6    // Servo motor signal wire
#define RED_LED_PIN 10   // Red LED (through 220Ω resistor)

// ============================================================
// Servo positions — adjust SERVO_OPEN angle if needed
// 0 = closed (gate shut), 90 = open (gate open, treat drops)
// ============================================================
#define SERVO_CLOSED 0
#define SERVO_OPEN   90

// ============================================================
// Cooldown period in milliseconds between PIR alerts
// Default: 10000 = 10 seconds. Increase to reduce alert frequency
// ============================================================
#define PIR_COOLDOWN 10000

WiFiClientSecure net = WiFiClientSecure();
MQTTClient client = MQTTClient(256);
Servo treatServo;

unsigned long lastPIRAlert = 0;
bool treatAvailable = true;

// ============================================================
// NTP Time settings — update gmtOffset_sec for your timezone
// GMT+8 (Singapore) = 8 * 3600 = 28800
// GMT+0 (UK)        = 0
// GMT-5 (US EST)    = -5 * 3600 = -18000
// ============================================================
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600;  // Update this for your timezone
const int daylightOffset_sec = 0;

// Returns current time as a formatted string e.g. "2024-11-15 14:32:05"
String getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Time unavailable";
  }
  char timeStr[30];
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStr);
}

void setup() {
  Serial.begin(9600);

  // Set pin modes
  pinMode(PIR_PIN, INPUT);
  pinMode(IR_PIN, INPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  // Initialise servo in closed position
  treatServo.attach(SERVO_PIN);
  treatServo.write(SERVO_CLOSED);
  digitalWrite(RED_LED_PIN, LOW);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");

  // Sync time via NTP server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Syncing time...");
  delay(5000); // Wait for time to sync
  Serial.println("Time synced! Current time: " + getFormattedTime());

  // Connect to AWS IoT
  connectToAWS();
}

void loop() {
  client.loop(); // Keep MQTT connection alive

  // Read IR sensor to check treat level
  // IR sensor outputs HIGH when nothing detected (low treats)
  // IR sensor outputs LOW when object detected (treats present)
  int irValue = digitalRead(IR_PIN);
  if (irValue == HIGH) {
    treatAvailable = false;
    digitalWrite(RED_LED_PIN, HIGH); // Red LED ON = treats low
  } else {
    treatAvailable = true;
    digitalWrite(RED_LED_PIN, LOW);  // Red LED OFF = treats ok
  }

  // Read PIR sensor for pet motion
  int pirValue = digitalRead(PIR_PIN);
  if (pirValue == HIGH) {
    unsigned long now = millis();
    // Only send alert if cooldown period has passed
    if (now - lastPIRAlert > PIR_COOLDOWN) {
      lastPIRAlert = now;
      Serial.println("Pet detected! Sending alert to AWS...");
      sendPetAlert();
    }
  }
}

// Publishes a pet detection alert to AWS IoT
void sendPetAlert() {
  StaticJsonDocument<200> message;
  message["event"] = "pet_detected";
  message["time"] = getFormattedTime();

  // Include treat level status in the alert
  if (!treatAvailable) {
    message["treats_available"] = "low, please refill soon";
  } else {
    message["treats_available"] = "ok";
  }

  char messageBuffer[512];
  serializeJson(message, messageBuffer);

  client.publish(AWS_IOT_PUBLISH_TOPIC, messageBuffer);

  Serial.println("Alert sent:");
  Serial.println(messageBuffer);
}

// Rotates servo to dispense one treat then closes again
void dispenseTreat() {
  Serial.println("Dispensing treat!");
  treatServo.write(SERVO_OPEN);  // Open gate
  delay(1000);                   // Wait 1 second for treat to fall
  treatServo.write(SERVO_CLOSED); // Close gate
  Serial.println("Treat dispensed!");
}

// Handles incoming messages from AWS IoT
// To dispense, publish {"command": "dispense"} to esp32/aws-to-esp32
void messageHandler(String &topic, String &payload) {
  Serial.println("Message received from AWS:");
  Serial.println(payload);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);

  const char* command = doc["command"];
  if (strcmp(command, "dispense") == 0) {
    if (treatAvailable) {
      dispenseTreat();
    } else {
      Serial.println("No treats available! Refill the tube.");
    }
  }
}

// Establishes secure connection to AWS IoT Core
void connectToAWS() {
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IoT");
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nAWS IoT Connected!");

  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
}
