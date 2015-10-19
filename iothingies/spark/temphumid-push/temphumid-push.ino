// Device: Particle Core or Photon
// Sensor: Thermocouple needs to be attached via Adafruit MAX31855
// Function: Testing out the push functionality from device to iothingies using http request
// Tested: Not Tested

#include "HttpClient/HttpClient.h"
#include "Adafruit_MAX31855/Adafruit_MAX31855.h"
#include "application.h"
#include <math.h>

#define DO   3
#define CS   4
#define CLK  5

Adafruit_MAX31855 thermocouple(CLK, CS, DO);

#define TEMP_UNIT "C"

// Rick Home IP
#define METEOR_SERVER_IP "192.168.1.9"
#define METEOR_SERVER_PORT 3000

// Hard code this device ID for now
#define DEVICEID "67tCTgueyGXBAEg7p"

HttpClient http;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    { "Content-Type", "application/json" },
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;

double tempC = 0;
double humid = 0;
unsigned int nextTime = 0;    // Next time to contact the server

void setup() {
    Serial.begin(9600);
    Particle.function("readTemps", readTemp);
    Particle.variable("temp", &tempC, DOUBLE);
    Particle.variable("humidity", &humid, DOUBLE);

    request.hostname = METEOR_SERVER_IP;
    request.port = METEOR_SERVER_PORT;
}

void loop() {
    if (nextTime > millis()) {
        return;
    }
    readTemp("Spark");
    sendDataToCloud();
    nextTime = millis() + 5000;
}

double readTemp(String input) {
    double internalTemp;
    double c;

    if (humid == 0) {
        humid = random(20,40);
    } else if (humid < 20) {
        humid = humid + random(0,5);
    } else if (humid > 40) {
        humid = humid + random(-5,0);
    } else {
        humid = humid + random (-1,1);
    }

    for (int i = 1; i <= 10; i++) {
        Serial.print("Internal Temp = ");
        c = thermocouple.readInternal();
        Serial.println(thermocouple.readInternal());
        internalTemp = c;

        double c = thermocouple.readCelsius();
        if (isnan(c) and isnan(internalTemp)) {
            Serial.println("Something wrong with thermocouple!");
            return 0;
        } else if (isnan(internalTemp)) {
            Serial.print("C = ");
            Serial.println(c);
            tempC = c;
            return c;
        } else {
            Serial.print("C = ");
            Serial.println(internalTemp);
            tempC = internalTemp;
            return internalTemp;
        }
    }

    Serial.println("Somebody trying to read temperature...");
    Serial.print("temperature to be sent is ");
    Serial.println(tempC);
    return tempC;
}

void sendDataToCloud() {
    request.path = "/api/device/" + DEVICEID + "/data";
    request.body = "{\"readings\": [{\"name\": \"humidity\", \"value\":" + String(humid) + "},{\"name\": \"temp\", \"value\": " + String(tempC) +"}]}";

    Serial.println("REQUEST===================>");
    Serial.println(request.body);
    Serial.println("=========================>");

    http.post(request, response, headers);

    Serial.println("RESPONSE==================>");
    Serial.println(response.body);
    Serial.println("=========================>");
}

String twoChar(String input) {
    if (input.length() < 2) {
        return String("0" + input);
    } else {
        return String(input);
    }
}

String getCurrentTimeString() {
    String timeString = twoChar(String(Time.year())) +
    "-" + twoChar(String(Time.month())) + "-" + twoChar(String(Time.day())) +
    "T" + twoChar(String(Time.hour())) + ":" + twoChar(String(Time.minute())) +
    ":" + twoChar(String(Time.second())) + ".700Z";
    return timeString;
}
