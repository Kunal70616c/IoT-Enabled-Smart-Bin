#include "Arduino.h"
#include <EMailSender.h>
#include <ESP8266WiFi.h>

const char* ssid = "wifiname";
const char* password = "wifipassword";

const int irSensorPin1 = D1;
const int irSensorPin2 = D2;
 // Assuming IR sensor is connected to pin D1
const int irThreshold = 80; // Adjust this threshold according to your sensor

uint8_t connection_state = 0;
uint16_t reconnect_interval = 10000;

EMailSender emailSend("sender email", "app password");

uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr)
{
    static uint16_t attempt = 0;
    Serial.print("Connecting to ");
    if(nSSID) {
        WiFi.begin(nSSID, nPassword);
        Serial.println(nSSID);
    }

    uint8_t i = 0;
    while(WiFi.status()!= WL_CONNECTED && i++ < 50)
    {
        delay(200);
        Serial.print(".");
    }
    ++attempt;
    Serial.println("");
    if(i == 51) {
        Serial.print("Connection: TIMEOUT on attempt: ");
        Serial.println(attempt);
        if(attempt % 2 == 0)
            Serial.println("Check if access point available or SSID and Password\r\n");
        return false;
    }
    Serial.println("Connection: ESTABLISHED");
    Serial.print("Got IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void Awaits()
{
    uint32_t ts = millis();
    while(!connection_state)
    {
        delay(50);
        if(millis() > (ts + reconnect_interval) && !connection_state){
            connection_state = WiFiConnect();
            ts = millis();
        }
    }
}

void setup()
{
    Serial.begin(9600);
    pinMode(irSensorPin1, INPUT);
    pinMode(irSensorPin2, INPUT);
    connection_state = WiFiConnect(ssid, password);
    if(!connection_state)  // if not connected to WIFI
        Awaits();          // constantly trying to connect
}

void loop()
{
    // Read the IR sensor value
    int irValue1 = analogRead(irSensorPin1);
    int irValue2 = analogRead(irSensorPin2);

    // If the IR value is below the threshold, the dustbin is full
    if (irValue1 < irThreshold  || irValue2 < irThreshold) {
        EMailSender::EMailMessage message;
        message.subject = "Dustbin is full";
        message.message = "Empty it.";

        EMailSender::Response resp = emailSend.send("reciever address", message);

        Serial.println("Sending status: ");
        Serial.println(resp.status);
        Serial.println(resp.code);
        Serial.println(resp.desc);

        // Add a delay to prevent spamming emails (adjust as needed)
        delay(300000); // 5 minutes delay before sending another email
    }

    // Add additional code or functionality here as needed
}
