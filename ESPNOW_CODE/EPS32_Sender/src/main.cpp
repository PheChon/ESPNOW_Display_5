#include <esp_now.h>
#include <WiFi.h>

hw_timer_t *timer = NULL;
volatile bool sendFlag = false;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // Broadcast address

typedef struct struct_message {
    float speed;
    float rpm;
    float fuel;
    float temp;
    int gear;  
} struct_message;

struct_message myData;

void IRAM_ATTR onTimer() {
    sendFlag = true;
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    esp_now_register_send_cb(OnDataSent);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    timer = timerBegin(0, 80, true);  
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000 , true);  //เปลี่ยนเวลาส่งตรงนี้เด้อ หน่วย us
    timerAlarmEnable(timer);
}

void loop() {
    if (Serial.available()) {
        
        String receivedData = Serial.readStringUntil('\n');
        
        sscanf(receivedData.c_str(), "%f,%f,%f,%f,%d", &myData.speed, &myData.rpm, &myData.fuel, &myData.temp, &myData.gear);

        
        Serial.print("Sending: ");
        Serial.print("Speed: "); Serial.print(myData.speed);
        Serial.print(" RPM: "); Serial.print(myData.rpm);
        Serial.print(" Fuel: "); Serial.print(myData.fuel);
        Serial.print(" Temp: "); Serial.print(myData.temp);
        Serial.print(" Gear: "); Serial.println(myData.gear);

        // Send the data via ESP-NOW
        esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

        if (result == ESP_OK) {
            Serial.println("Sent with success");
        } else {
            Serial.println("Error sending the data");
        }
    }
}
