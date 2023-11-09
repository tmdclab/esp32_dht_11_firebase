#include "DHT.h"

#include <WiFi.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>


#define DHTPIN 4  // Digital pin connected to the DHT sensor

// Uncomment whatever type you're using!
#define DHTTYPE DHT11  // DHT 11

/* 1. Define the WiFi credentials */
#define WIFI_SSID ""  
#define WIFI_PASSWORD "" 

/* 2. Define the API Key */
#define API_KEY ""

/* 3. Define the project ID */
#define FIREBASE_PROJECT_ID ""

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL ""
#define USER_PASSWORD ""


// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save timestamp
String formattedDate;


void setup() {

  // Initialize serial communication
  Serial.begin(9600);

  // Initialize DHT11
  Serial.println(F("DHTxx test!"));
  dht.begin();

  // Initialize a WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Initialize a Firebase
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  //auth.user.email = USER_EMAIL;
  //auth.user.password = USER_PASSWORD;
  Firebase.signUp(&config, &auth, USER_EMAIL, USER_PASSWORD);

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

  // Initialize a NTPClient to get time
  timeClient.begin();


}

void loop() {


  if (Firebase.ready() && (millis() - dataMillis > 5000 || dataMillis == 0)) {
    dataMillis = millis();

    while (!timeClient.update()) {
      timeClient.forceUpdate();
    }
    // The formattedDate comes with the following format:
    // 2018-05-28T16:00:13Z
    // We need to extract date and time
    formattedDate = timeClient.getFormattedDate();
    Serial.println(formattedDate);

    //temperature sensor
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    // For the usage of FirebaseJson, see examples/FirebaseJson/BasicUsage/Create.ino
    FirebaseJson content;

    String documentPath = "Temperature";

    // timestamp
    content.set("fields/Timestamp/timestampValue", formattedDate);  // RFC3339 UTC "Zulu" format

    // temperature fields double
    content.set("fields/DHT11_temperature/doubleValue", t);

    content.set("fields/DHT11_humidity/doubleValue", h);
    
    Serial.print("Create a document... ");

    if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID, "" /* databaseId can be (default) or empty */, documentPath.c_str(), content.raw()))
      Serial.printf("ok\n % s\n\n", fbdo.payload().c_str());
    else
      Serial.println(fbdo.errorReason());
  }
}
