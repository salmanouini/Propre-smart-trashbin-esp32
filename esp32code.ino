
#include <WiFi.h>
#include <ESP32Servo.h>
#include <FirebaseESP32.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

#define TRIG_PIN 17
#define ECHO_PIN 18
#define SERVO_PIN 13
#define NEW_TRIG_PIN 16
#define NEW_ECHO_PIN 2

#define FIREBASE_HOST "wasteflow-9bd12-default-rtdb.europe-west1.firebasedatabase.app"
#define FIREBASE_AUTH "XuXFmyiD6K0wT7qZ4S9wUJqs0fgi4llCbPW1Hy7X"
#define BUTTON_PIN 19



const char* ssid = "Honor";
const char* password = "salma123";

// GPS setup
TinyGPSPlus gps;
HardwareSerial gpsSerial(1); // use Serial1

// Firebase
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// Servo
Servo monServo;

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 4, 5);  // RX = 4, TX = 5
  Serial.println("Waiting for GPS...");

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Activates internal pull-up


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");

  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(NEW_TRIG_PIN, OUTPUT);
  pinMode(NEW_ECHO_PIN, INPUT);
  monServo.attach(SERVO_PIN, 500, 2400);
  monServo.write(0);
}

float measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return (duration * 0.0343) / 2;
}

void loop() {

  float distanceServo = measureDistance(TRIG_PIN, ECHO_PIN);
  float distanceFirebase = measureDistance(NEW_TRIG_PIN, NEW_ECHO_PIN);
  bool isFull = (distanceFirebase < 8);

  float latitude = 0.0;
float longitude = 0.0;

while (gpsSerial.available()) {
  gps.encode(gpsSerial.read());
  if (gps.location.isUpdated()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();

    Serial.print("Lat: ");
    Serial.println(latitude, 6);
    Serial.print("Lng: ");
    Serial.println(longitude, 6);
  }
}

  Serial.print("Distance (Firebase sensor): ");
  Serial.println(distanceFirebase);

  Serial.print("Lat: ");
  Serial.print(latitude);
  Serial.print(" | Lng: ");
  Serial.println(longitude);

  // Update Firebase with group/bin structure
  String group = "group1";
  String bin = "bin1";
  String basePath = "/groups/" + group + "/" + bin;

  Firebase.setFloat(firebaseData, basePath + "/distance", distanceFirebase);
  Firebase.setBool(firebaseData, basePath + "/isFull", isFull);
  Firebase.setFloat(firebaseData, basePath + "/lat", latitude);
  Firebase.setFloat(firebaseData, basePath + "/lng", longitude);


  bool buttonPressed = digitalRead(BUTTON_PIN) == LOW; // LOW = pressed


if ((distanceServo < 180 && !isFull)  || buttonPressed) {
  monServo.write(90);
  delay(10000);
  monServo.write(0);

  Firebase.setBool(firebaseData, basePath + "/openRemotely", false);
}


  delay(2000);
}


























