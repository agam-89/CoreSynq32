#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "samples.h" 

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int btnPins[8] = {13, 12, 14, 27, 26, 33, 32, 15};
const int potPin = 34;
const int dacPin = 25;

bool patterns[3][8] = {0}; 
int currentTrack = 0; 
int currentStep = 0;
int currentPage = 0;  
int bpm = 120;
bool isPlaying = true;

// --- PCM Sample Player Engine ---
const uint8_t* sampleData[3] = {kick, snare, hihat};
const int sampleLens[3] = {kick_len, snare_len, hihat_len};

volatile int audioPos[3] = {-1, -1, -1};
volatile bool audioActive[3] = {false, false, false};

hw_timer_t* sampleTimer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onSampleTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  int32_t mixed = 0; 
  int activeCount = 0;
  for (int i = 0; i < 3; i++) {
    if (audioActive[i]) {
      int16_t sample = (int16_t)sampleData[i][audioPos[i]] - 128;
      mixed += sample;
      activeCount++;
      audioPos[i]++;
      if (audioPos[i] >= sampleLens[i]) {
        audioPos[i] = -1;
        audioActive[i] = false;
      }
    }
  }
  if (activeCount > 0) {
    int32_t finalOut = mixed + 128;
    if (finalOut > 255) finalOut = 255;
    if (finalOut < 0) finalOut = 0;
    dacWrite(dacPin, (uint8_t)finalOut);
  } else {
    dacWrite(dacPin, 128); 
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}

void triggerSound(int track) {
  portENTER_CRITICAL(&timerMux);
  audioPos[track] = 0;
  audioActive[track] = true;
  portEXIT_CRITICAL(&timerMux);
}

void vSequencer(void *pvParameters) {
  for (;;) {
    if (isPlaying) {
      for (int i = 0; i < 3; i++) {
        if (patterns[i][currentStep]) triggerSound(i);
      }
      currentStep = (currentStep + 1) % 8;
    }
    vTaskDelay(pdMS_TO_TICKS((60000 / bpm) / 4));
  }
}

void vUI(void *pvParameters) {
  for (;;) {
    bpm = map(analogRead(potPin), 0, 4095, 60, 240);
    for (int i = 0; i < 8; i++) {
      if (digitalRead(btnPins[i]) == LOW) {
        if (i < 4) {
          int stepIndex = i + (currentPage * 4);
          patterns[currentTrack][stepIndex] = !patterns[currentTrack][stepIndex];
        }
        else if (i == 4) currentTrack = (currentTrack + 1) % 3;
        else if (i == 5) currentPage = !currentPage;
        else if (i == 6) isPlaying = !isPlaying;
        else if (i == 7) memset(patterns[currentTrack], 0, 8);
        vTaskDelay(pdMS_TO_TICKS(200)); 
      }
    }
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    // --- Header ---
    display.setCursor(0, 0);
    display.printf("TRK:%s", (currentTrack == 0 ? "KICK" : (currentTrack == 1 ? "SNARE" : "HAT")));
    
    display.setCursor(100, 0);
    display.print(currentPage == 0 ? "P:1" : "P:2");

    // --- Editing Bar ---
    int highlightX = currentPage * 64;
    display.drawFastHLine(highlightX + 4, 21, 56, SSD1306_WHITE);
    display.setCursor(highlightX + 12, 12);
    display.print("EDITING");

    // --- Sequencer Steps ---
    for(int s=0; s<8; s++) {
      int xPos = s * 16 + 2;
      display.drawRect(xPos, 25, 12, 12, 1);
      if(patterns[currentTrack][s]) display.fillRect(xPos + 2, 27, 8, 8, 1);
      if(currentStep == s && isPlaying) {
        display.fillTriangle(xPos + 2, 45, xPos + 10, 45, xPos + 6, 40, 1);
      }
    }

    // --- Footer ---
    display.setCursor(0, 56);
    display.printf("BPM:%d", bpm);
    
    display.setCursor(75, 56);
    display.print(isPlaying ? "> PLAYING" : "|| PAUSED");

    display.display();
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void setup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);
  for (int i = 0; i < 8; i++) pinMode(btnPins[i], INPUT_PULLUP);
  pinMode(dacPin, OUTPUT);
  dacWrite(dacPin, 128);

  sampleTimer = timerBegin(1000000); 
  timerAttachInterrupt(sampleTimer, &onSampleTimer);
  timerAlarm(sampleTimer, 45, true, 0); 

  xTaskCreatePinnedToCore(vSequencer, "Seq", 5000, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(vUI,        "UI",  4000, NULL, 1, NULL, 0);
}

void loop() {}