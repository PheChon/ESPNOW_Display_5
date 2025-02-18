#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Wire.h>

#define HEIGHT 240
#define WIDTH  240

// Number of displays you have
#define NUM_DISPLAYS 5
// Define a CS pin for each display. 
// Make sure your wiring corresponds exactly to these pin assignments.
const int CS_PINS[NUM_DISPLAYS] = { 13, 33, 32, 25, 21 };

TFT_eSPI tft = TFT_eSPI();  // Single instance for all displays (using the shared SPI bus)

// Structure for incoming ESP-NOW data
typedef struct struct_message {
  float speed;
  float rpm;
  float fuel;
  float temp;
  int gear;
} struct_message;

// Shared data (accessed both in callback and loop)
volatile struct_message myData;
// A local copy to compare with what’s currently shown
struct_message lastData = { -1.0, -1.0, -1.0, -1.0, -1 };

// A flag set in the ESP-NOW callback to indicate new data is available
volatile bool newDataAvailable = false;

// This function updates one display. It manually toggles the CS pin and wraps
// drawing commands inside an SPI transaction.
void updateDisplay(int index, const char* label, float value) {
  // Activate the display by pulling its CS LOW
  digitalWrite(CS_PINS[index], LOW);
  tft.startWrite(); // Begin SPI transaction

  // Clear only the area where text is shown.
  // (Adjust the rectangle parameters to suit your layout.)
  tft.fillRect(0, HEIGHT/3 - 20, WIDTH, HEIGHT/3 + 50, TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setFreeFont(&FreeSansBold24pt7b);
  tft.drawString(label, WIDTH / 2, HEIGHT / 3);
  tft.drawFloat(value, 2, WIDTH / 2, HEIGHT / 2);

  tft.endWrite(); // End SPI transaction
  digitalWrite(CS_PINS[index], HIGH); // Deactivate the display
}

// ESP-NOW callback: simply store the data and signal new data is available.
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy((void *)&myData, incomingData, sizeof(myData));
  
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

  // Signal that new data is available.
  newDataAvailable = true;
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  tft.begin();
  tft.setRotation(0);

  // Initialize each CS pin as an OUTPUT and clear each display
  for (int i = 0; i < NUM_DISPLAYS; i++) {
    pinMode(CS_PINS[i], OUTPUT);
    digitalWrite(CS_PINS[i], HIGH);
    digitalWrite(CS_PINS[i], LOW);
    tft.startWrite();
    tft.fillScreen(TFT_BLACK);
    tft.endWrite();
    digitalWrite(CS_PINS[i], HIGH);
  }

  // Initialize ESP-NOW and register the receive callback
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
    // If new data has been received, update the displays one-by-one.
    if (newDataAvailable) {
      // เราปิดการขัดจังหวะชั่วคราวเพื่อป้องกัน race condition
      noInterrupts();
      struct_message localData;
      memcpy((void*)&localData, (void*)&myData, sizeof(struct_message));
      newDataAvailable = false;
      interrupts();
  
      // ตรวจสอบและอัปเดตหน้าจอที่เปลี่ยนแปลง
      if (localData.speed != lastData.speed) {
        updateDisplay(0, "Speed:", localData.speed);
        lastData.speed = localData.speed;
      }
      if (localData.rpm != lastData.rpm) {
        updateDisplay(1, "RPM:", localData.rpm);
        lastData.rpm = localData.rpm;
      }
      if (localData.fuel != lastData.fuel) {
        updateDisplay(2, "Fuel%:", localData.fuel);
        lastData.fuel = localData.fuel;
      }
      if (localData.temp != lastData.temp) {
        updateDisplay(3, "Temp:", localData.temp);
        lastData.temp = localData.temp;
      }
      if (localData.gear != lastData.gear) {
        updateDisplay(4, "Gear:", localData.gear);
        lastData.gear = localData.gear;
      }
    }
}