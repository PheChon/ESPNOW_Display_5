#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>

#define Height 240
#define Width 240

TFT_eSPI tft = TFT_eSPI();

typedef struct struct_message {
    float speed;
    float rpm;
    float fuel;
    float temp;
    int gear;
} struct_message;

struct_message myData;

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    memcpy(&myData, incomingData, sizeof(myData));

    Serial.print("Received Speed: ");
    Serial.println(myData.speed);
    Serial.print("Received RPM: ");
    Serial.println(myData.rpm);
    Serial.print("Received Fuel: ");
    Serial.println(myData.fuel);
    Serial.print("Received Temp: ");
    Serial.println(myData.temp);
    Serial.print("Received Gear: ");
    Serial.println(myData.gear);
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setFreeFont(&FreeSansBold24pt7b);
    tft.setTextPadding(250);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
    if (myData.gear == 1)
    {
        tft.drawString("Speed:", Width / 2, Height / 3);
        tft.drawFloat(myData.speed, 2, Width / 2, Height / 2);
    }
    else if (myData.gear == 2)
    {
        tft.drawString("RPM:", Width / 2, Height / 3);
        tft.drawFloat(myData.rpm, 2, Width / 2, Height / 2);
    }
    else if (myData.gear == 3)
    {
        tft.drawString("Fuel%:", Width / 2, Height / 3);
        tft.drawFloat(myData.fuel, 2, Width / 2, Height / 2);
    }
    else if (myData.gear == 4)
    {
        tft.drawString("Temp:", Width / 2, Height / 3);
        tft.drawFloat(myData.temp, 2, Width / 2, Height / 2);
    }
    else 
        tft.drawString("NoData:", Width / 2, Height / 2);
        
}