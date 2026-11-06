#include <Arduino.h>

#define LEADING_ZERO true   // true = แสดง 007, false = แสดง   7

// ขา segment a, b, c, d, e, f, g
const uint8_t SEG_PINS[7] = {2, 3, 4, 5, 6, 7, 8};
// ขาเลือกหลัก: ร้อย, สิบ, หน่วย
const uint8_t DIGIT_PINS[3] = {A0, A2, A4};
const uint8_t SW_PIN = 12;

const uint8_t DIGIT_PATTERN[10] = {
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F  // 9
};

const uint16_t DIGIT_TIME_US = 1000;  // เวลาต่อหลัก

uint8_t digitBuffer[3] = {0, 0, 0};
uint8_t currentDigit = 0;
uint32_t lastSwitch = 0;


void writeSegments(uint8_t pattern) {
  for (uint8_t i = 0; i < 7; i++) {
    digitalWrite(SEG_PINS[i], (pattern >> i) & 0x01);
  }
}

void displayNumber(int num) {

  if (num < 0) num = 0;
  if (num > 999) num = 999;

  uint8_t hundreds = num / 100;
  uint8_t tens     = (num / 10) % 10;
  uint8_t ones     = num % 10;

  digitBuffer[0] = DIGIT_PATTERN[hundreds];
  digitBuffer[1] = DIGIT_PATTERN[tens];
  digitBuffer[2] = DIGIT_PATTERN[ones];

#if !LEADING_ZERO
  if (num < 100) digitBuffer[0] = 0;   // ดับหลักร้อย
  if (num < 10)  digitBuffer[1] = 0;   // ดับหลักสิบ
#endif

}


void refreshDisplay() {
  uint32_t now = micros();
  if (now - lastSwitch < DIGIT_TIME_US) return;
  lastSwitch = now;

  for (uint8_t i = 0; i < 3; i++) {
    digitalWrite(DIGIT_PINS[i], LOW);
  }

  currentDigit = (currentDigit + 1) % 3;
  writeSegments(digitBuffer[currentDigit]);
  digitalWrite(DIGIT_PINS[currentDigit], HIGH);   // HIGH = เลือกหลักนี้
}


void displayInit() {
  for (uint8_t i = 0; i < 7; i++) {
    pinMode(SEG_PINS[i], OUTPUT);
    digitalWrite(SEG_PINS[i], LOW);
  }

  for (uint8_t i = 0; i < 3; i++) {
    pinMode(DIGIT_PINS[i], OUTPUT);
    digitalWrite(DIGIT_PINS[i], LOW);
  }
}

void waiting_debounce(int t) {
  for (int i = 0; i < t; i++) {
    delay(1);
    refreshDisplay();
  }
}

int counter = 0;
uint32_t lastCount = 0;

void setup() {
  displayInit();
  displayNumber(888);
  pinMode(SW_PIN, INPUT_PULLUP);
  waiting_debounce(500);
  displayNumber(0);
}

void loop() {
  refreshDisplay();

  if (digitalRead(SW_PIN) == LOW) {
    while (digitalRead(SW_PIN) == LOW) {
      refreshDisplay();
    }
    waiting_debounce(10);

    while (digitalRead(SW_PIN) == HIGH) {
      if (millis() - lastCount >= 130) {
        lastCount = millis();
        if (counter > 999) {
          counter = 0;
        } else {
          counter += 1;
        }
        displayNumber(counter);
      }
      refreshDisplay();
    }

    waiting_debounce(10);

    while (digitalRead(SW_PIN) == LOW) {
      refreshDisplay();
    }

    waiting_debounce(10);

    while (digitalRead(SW_PIN) == HIGH) {
      refreshDisplay();
    }

    while (digitalRead(SW_PIN) == LOW) {
      counter = 0;
      displayNumber(counter);
      refreshDisplay();
    }

    waiting_debounce(10);
  }
}