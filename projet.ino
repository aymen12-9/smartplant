#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <DHT.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// Initialisation de l'écran LCD avec adresse I2C 0x3F
LiquidCrystal_PCF8574 lcd(0x3F);

// Initialisation du capteur DHT
DHT dht(2, DHT11);  // Changez la broche si nécessaire

// Définir les broches
#define SOIL_SENSOR_PIN 34
#define PIR_PIN 13
#define RELAY_PIN_1 12
#define PUSH_BUTTON_1 14

// État initial du relais et du bouton
int relay1State = LOW;
int pushButton1State = HIGH;

// Informations Wi-Fi et Firebase
const char* ssid = "iPhone de aysser";
const char* password = "AYSSER1234";
#define FIREBASE_HOST "https://console.firebase.google.com/u/2/project/smart-plant-44b10/settings/general/android:com.example.smartplant1"  // Remplace par ton URL Firebase
#define FIREBASE_AUTH "AlzaSyCLUPMbnZxKF7EmCPuQOKXBosjyO6UPQNA"


// Initialisation Firebase
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

void setup() {
  Serial.begin(115200);

  // Initialisation LCD
  lcd.begin(16, 2);
  lcd.setBacklight(255);
  lcd.print("Initialisation...");
  
  // Configurer les broches
  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_PIN_1, OUTPUT);
  digitalWrite(RELAY_PIN_1, relay1State);
  pinMode(PUSH_BUTTON_1, INPUT_PULLUP);

  // Initialiser le capteur DHT
  dht.begin();

  // Connexion Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connexion au WiFi...");
    lcd.setCursor(0, 1);
    lcd.print("WiFi...");
  }
  Serial.println("Connecté au WiFi");
  lcd.setCursor(0, 1);
  lcd.print("WiFi OK!");

  // Configuration Firebase
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.api_key = FIREBASE_AUTH;

  Firebase.begin(&firebaseConfig, &firebaseAuth);
  lcd.clear();
  lcd.print("Firebase OK!");
  delay(1000);
  lcd.clear();
}

void DHT11sensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Erreur lecture DHT!");
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t, 1);
  lcd.print(" H:");
  lcd.print(h, 1);

  FirebaseJson json;
  json.add("temperature", t);
  json.add("humidity", h);
  if (!Firebase.setJSON(firebaseData, "/capteurs/DHT11", json)) {
    Serial.println("Erreur Firebase : " + firebaseData.errorReason());
  }
}

void soilMoistureSensor() {
  int value = analogRead(SOIL_SENSOR_PIN);
  value = map(value, 0, 4095, 0, 100);
  value = (value - 100) * -1;

  lcd.setCursor(0, 1);
  lcd.print("S:");
  lcd.print(value);
  lcd.print("% ");

  FirebaseJson json;
  json.add("soilMoisture", value);
  if (!Firebase.setJSON(firebaseData, "/capteurs/soilMoisture", json)) {
    Serial.println("Erreur Firebase : " + firebaseData.errorReason());
  }
}

void PIRsensor() {
  bool value = digitalRead(PIR_PIN);

  lcd.setCursor(10, 1);
  if (value) {
    lcd.print("M:ON ");
    Serial.println("Mouvement détecté!");
  } else {
    lcd.print("M:OFF");
  }

  FirebaseJson json;
  json.add("motionDetected", value ? "Yes" : "No");
  if (!Firebase.setJSON(firebaseData, "/capteurs/motionDetected", json)) {
    Serial.println("Erreur Firebase : " + firebaseData.errorReason());
  }
}

void checkPhysicalButton() {
  if (digitalRead(PUSH_BUTTON_1) == LOW) {
    if (pushButton1State != LOW) {
      relay1State = !relay1State;
      digitalWrite(RELAY_PIN_1, relay1State);
    }
    pushButton1State = LOW;
  } else {
    pushButton1State = HIGH;
  }

  FirebaseJson json;
  json.add("relayState", relay1State == HIGH ? "ON" : "OFF");
  if (!Firebase.setJSON(firebaseData, "/capteurs/relayState", json)) {
    Serial.println("Erreur Firebase : " + firebaseData.errorReason());
  }
}

void loop() {
  DHT11sensor();
  soilMoistureSensor();
  PIRsensor();
  checkPhysicalButton();

  lcd.setCursor(14, 1);
  lcd.print(relay1State == HIGH ? "ON" : "OFF");

  delay(1000);
}
