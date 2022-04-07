#include <WiFi.h>
#include <HTTPClient.h>

//Wifi Configuration
const char *ssid = "NOWTVV1PD4";
//const char *ssid = "Conor";
//const char *password = "Conor123";
const char *password = "XRTs8mck76Cg";

//SmartThings Configuration
const char *personal_token = "c811d0c2-2ec9-4b37-b956-341b67056479";
const char *setup_routine = "d05abd3c-de90-4a88-a0c7-2b18d711a300";
const char *main_routine = "f8c342ee-6c84-4b28-b29f-0f07f131de1f";
const char *inactive_routine = "49396f64-afc0-4666-a205-9a9b9868b586";

const char *url_pattern = "https://api.smartthings.com/v1/devices/%s/commands";
const char *bearer_pattern = "Bearer: %s";

//Ultrasonic pins
const int trigPin = 4;
const int echoPin = 12;

//LED pins
const int ledBluePin = 25;
const int ledRedPin = 0;
const int ledGreenPin = 26;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

//All Variables
long duration;
float distance;
float presenceClose;
float presenceFar;

bool presence;

char *url;
char *bearer;

char *routNo;

void setup() {
  Serial.begin(115200); // Starts the serial communication
  
  //Pin setup
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(ledBluePin, OUTPUT);
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);

  connectWifi();

  Serial.println("===Smart Desk Start Up===");
  setupRoutine();
  runAlexa();
  delay(5000);
  deskCalibration();
}

void loop() {
  checkPresence();
}

void connectWifi() {
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("");
}

void runAlexa() {
  HTTPClient http;

  http.begin(url);

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", bearer);
  String httpRequestData = "[{\"component\":\"main\",\"capability\":\"switch\",\"command\":\"on\"}]";

  int httpResponseCode = http.POST(httpRequestData);

  http.end();
}

void setupRoutine() {
  url = (char *)malloc(strlen(url_pattern) + strlen(setup_routine));
  bearer = (char *)malloc(strlen(bearer_pattern) + strlen(personal_token));

  sprintf(url, url_pattern, setup_routine);
  sprintf(bearer, bearer_pattern, personal_token);
}

void runRoutine() {
  url = (char *)malloc(strlen(url_pattern) + strlen(main_routine));
  bearer = (char *)malloc(strlen(bearer_pattern) + strlen(personal_token));

  sprintf(url, url_pattern, main_routine);
  sprintf(bearer, bearer_pattern, personal_token);
}

void inactiveRoutine() {
  url = (char *)malloc(strlen(url_pattern) + strlen(inactive_routine));
  bearer = (char *)malloc(strlen(bearer_pattern) + strlen(personal_token));

  sprintf(url, url_pattern, inactive_routine);
  sprintf(bearer, bearer_pattern, personal_token);
}

float findDist() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distance = duration * SOUND_SPEED/2;
  return distance;
}

void deskCalibration() {
  Serial.println("Running Calibration");
  Serial.println("sit at desk as normal");
  Serial.println("");

  distance = findDist();

  //Set values
  presenceClose = distance - 5; //Having a 5cm buffer, just incase
  presenceFar = presenceClose + 40; //Suitable distance away
}

void checkPresence() {
  distance = findDist();
  Serial.println(presenceClose);
  Serial.println(presenceFar);

  //Set LEDs
  digitalWrite(ledRedPin, LOW);
  digitalWrite(ledGreenPin, LOW);
  digitalWrite(ledBluePin, HIGH);
  delay(1000);
  digitalWrite(ledBluePin, LOW);

  if (WiFi.status() == WL_CONNECTED)
    {
      if (distance < presenceClose || distance > presenceFar) {
        if (presence == true) {
          inactiveRoutine();
          runAlexa(); 
        }
        Serial.println(distance);
        presence = false;
        digitalWrite(ledRedPin, HIGH);
        
      } else
        {
          if (presence != true) {
            
            runRoutine();
            runAlexa();
            Serial.println("Detected");
            Serial.println(distance);
            presence = true;
            digitalWrite(ledGreenPin, HIGH);
          } else {
            Serial.println("Still Active");
            Serial.println(distance);
            digitalWrite(ledGreenPin, HIGH);
          }
        }
    }
    else
    {
        Serial.println("WiFi Disconnected");
        delay(5000);
    }

  delay(10000); //Set delay to 600000 for 10 mins
}
