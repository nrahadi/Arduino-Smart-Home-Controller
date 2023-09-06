#include <LiquidCrystal.h>
#include <Keypad.h>
#include <Servo.h>
#include "DHT.h"
#include "SafeState.h"
#include "RTClib.h"

#define SERVO_PIN 2
#define SERVO_LOCK 20
#define SERVO_UNLOCK 90

#define DHT22_PIN1 28
#define DHT22_PIN2 29
#define DHTTYPE DHT22

#define LDR_IN1 A0
#define LDR_IN2 A1
#define LDR_OUT A2

#define FAN_LED 26
#define LED_IN1 33
#define LED_IN2 34
#define LED_OUT 35
#define GATE_OP 36
#define GATE_CL 37

#define GATE_PB 39

#define PIR_INDOOR 27
int PIR_INDOOR_Stt = LOW;
int PIR_INDOOR_Val = 0;
#define PIR_GATE 30
int PIR_GATE_Stt = LOW;
int PIR_GATE_Val = 0;

int ttreshold;
int ltreshold;
int ttreshold_v;
int ltreshold_v;

int TaskTimer1 = 0;
int TaskTimer2 = 0;
int TaskTimer3 = 0;
//TaskTimer4 = 0;

bool TaskFlag1 = 0;
bool TaskFlag2 = 0;
bool TaskFlag3 = 0;
//TaskFlag4 = 0;

Servo doorLock;

RTC_DS1307 rtc;

DHT DHT1(DHT22_PIN1, DHTTYPE);
DHT DHT2(DHT22_PIN2, DHTTYPE);

LiquidCrystal lcd(5, 4, 3, 22, 23, 24, 25);

int ROWS = 2;
int COLS = 16;

const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 4;
byte rowPins[KEYPAD_ROWS] = {13, 12, 11, 10};
byte colPins[KEYPAD_COLS] = {9, 8, 7, 6};
char keys[KEYPAD_ROWS][KEYPAD_COLS] = {
{'1', '2', '3', 'A'},
{'4', '5', '6', 'B'},
{'7', '8', '9', 'C'},
{'*', '0', '#', 'D'}
};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS);

SafeState safeState;

void PIRgate () {
  PIR_GATE_Val = digitalRead(PIR_GATE);
  if (PIR_GATE_Val == HIGH) {
    digitalWrite(GATE_CL, LOW);
    if (PIR_GATE_Stt == LOW) {
      PIR_GATE_Stt == HIGH;
    }
  }
  else {
    digitalWrite(GATE_CL, HIGH);
    delay(5000);
    digitalWrite(GATE_CL, LOW);
    if (PIR_GATE_Stt == HIGH) {
      PIR_GATE_Stt = LOW;
    }
  }
}

void PIRlight () {
  PIR_INDOOR_Val = digitalRead(PIR_INDOOR);
  if (PIR_INDOOR_Val == HIGH) {
    digitalWrite(LED_IN1, HIGH);
    digitalWrite(LED_IN2, HIGH);
    delay(3000);
    if (PIR_INDOOR_Stt == LOW) {
      userAct();
      PIR_INDOOR_Stt == HIGH;
    }
  }
  else {
    digitalWrite(LED_IN1, LOW);
    digitalWrite(LED_IN2, LOW);
    if (PIR_INDOOR_Stt == HIGH) {
      PIR_INDOOR_Stt = LOW;
    }
  }
}

void userAct() {
  DateTime now = rtc.now();

  Serial.print("AT TIME: (");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print("-");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.print(") USER ACTIVITY RECORDED");
  Serial.println();
}

void userLog() {
  DateTime now = rtc.now();

  Serial.print("AT TIME: (");
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print("-");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.print(") A USER HAS LOGGED IN");
  Serial.println();
}

void gateControl() {
  int GATE_val = digitalRead(GATE_PB);
  if (GATE_val == 0) {
    DateTime now = rtc.now();
    Serial.print("AT TIME: (");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print("-");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.print(") DOMAIN ACCESSED VIA FRONT GATE");
    Serial.println();
    digitalWrite(GATE_OP, HIGH);
    delay(5000);
    digitalWrite(GATE_OP, LOW);
    PIRgate();
  }
}

void lightLEDControl() {
  ltreshold = 100; // default
  if (ltreshold_v > 0) {
    ltreshold = ltreshold_v;
  }
  const float GAMMA = 0.7;
  const float RL10 = 50;

  int LDR_IN1_READ = analogRead(LDR_IN1);
  float LDR_IN1_V = LDR_IN1_READ / 1024. * 5;
  float LDR_IN1_R = 2000 * LDR_IN1_V / (1 - LDR_IN1_V / 5);
  float LDR_IN1_LUX = pow(RL10 * 1e3 * pow(10, GAMMA) / LDR_IN1_R, (1 / GAMMA));
  int LUX_IN1 = LDR_IN1_LUX;

  int LDR_IN2_READ = analogRead(LDR_IN2);
  float LDR_IN2_V = LDR_IN2_READ / 1024. * 5;
  float LDR_IN2_R = 2000 * LDR_IN2_V / (1 - LDR_IN2_V / 5);
  float LDR_IN2_LUX = pow(RL10 * 1e3 * pow(10, GAMMA) / LDR_IN2_R, (1 / GAMMA));
  int LUX_IN2 = LDR_IN2_LUX;

  int LDR_OUT_READ = analogRead(LDR_OUT);
  float LDR_OUT_V = LDR_OUT_READ / 1024. * 5;
  float LDR_OUT_R = 2000 * LDR_OUT_V / (1 - LDR_OUT_V / 5);
  float LDR_OUT_LUX = pow(RL10 * 1e3 * pow(10, GAMMA) / LDR_OUT_R, (1 / GAMMA));
  int LUX_OUT = LDR_OUT_LUX;

  if (LUX_IN1 < ltreshold) {
    digitalWrite(LED_IN1, HIGH);
  } 
  if (LUX_IN1 > ltreshold) {
    digitalWrite(LED_IN1, LOW);
  }

  if (LUX_IN2 < ltreshold) {
    digitalWrite(LED_IN2, HIGH);
  } 
  if (LUX_IN2 > ltreshold) {
    digitalWrite(LED_IN2, LOW);
  }

  if (LUX_OUT < ltreshold) {
    digitalWrite(LED_OUT, HIGH);
  } 
  if (LUX_OUT > ltreshold) {
    digitalWrite(LED_OUT, LOW);
  }
  PIRlight();
  delay(100);
}

void tempFanControl() {
  ttreshold = 25; // default
  if (ttreshold_v > 0) {
    ttreshold = ttreshold_v;
  }
  float t1 = DHT1.readTemperature();
  float t2 = DHT2.readTemperature();
  int intt1 = t1;
  int intt2 = t2;
  String strt1 = String(intt1);
  String strt2 = String(intt2);

  if (ttreshold <= intt1 || ttreshold <= intt2) {
    digitalWrite(FAN_LED, HIGH);
  } else {
    digitalWrite(FAN_LED, LOW);
  }
}

void lock() {
  doorLock.write(SERVO_LOCK);
  safeState.lock();
}

void unlock() {
  doorLock.write(SERVO_UNLOCK);
}

String inputTtreshold() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("NEW TMP TRESHOLD");
  lcd.setCursor(5, 1);
  lcd.print("[__*C]");
  lcd.setCursor(6, 1);
  String temp = "";
  while (temp.length() < 2) {
    char key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      lcd.print(key);
      temp += key;
    }
  }
  return temp;
}

String inputLtreshold() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("NEW LUX TRESHOLD");
  lcd.setCursor(4, 1);
  lcd.print("[___lux]");
  lcd.setCursor(5, 1);
  String lux = "";
  while (lux.length() < 3) {
    char key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      lcd.print(key);
      lux += key;
    }
  }
  return lux;
}

String inputPassword() {
  lcd.setCursor(5, 1);
  lcd.print("[____]");
  lcd.setCursor(6, 1);
  String result = "";
  while (result.length() < 4) {
    char key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      lcd.print('*');
      result += key;
    }
  }
  return result;
}

bool registerPassword() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("REGISTER");
  String newCode = inputPassword();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CONFIRM PASSWORD");
  String confirmCode = inputPassword();

  if (newCode.equals(confirmCode)) {
    safeState.setCode(newCode);
    return true;
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("REGISTER FAILED!");
    lcd.setCursor(1, 1);
    lcd.print("RETURN ATTEMPT!");
    delay(2000);
    return false;
  }
}

void setMessage() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("LOCKED SUCCESS");
  delay(1000);
}

void unlockMessage() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("ACCESS GRANTED");
  delay(1000);
}

void tresholdSetup() {
  auto keyInput = keypad.getKey();
  while (keyInput != 'A' && keyInput != '#') {
    keyInput = keypad.getKey();
    while (keyInput == 'B') {
      lcd.setCursor(0, 0);
      lcd.print("C= TEMP TRESHOLD");
      lcd.setCursor(0, 1);
      lcd.print("D= LUX TRESHOLD");
      auto keyInput = keypad.getKey();
      if (keyInput == 'C') {
        String tempo = inputTtreshold();
        ttreshold_v = tempo.toInt();
      }
      if (keyInput == 'D') {
        String luxo = inputLtreshold();
        ltreshold_v = luxo.toInt();
      }
      else if (keyInput == '#') {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("PRESS # TO LOCK!");
        return true;
      }
    }
  }
}

void unlockedLogic() {
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("INITIALIZATION");
  delay(1000);
  lcd.setCursor(0, 0);
  lcd.print("PRESS # TO LOCK!");
  
  bool newCodeNeeded = true;

  if (safeState.hasCode()) {
    lcd.setCursor(0, 1);
    lcd.print("B = SET TRESHOLD");
    newCodeNeeded = false;
    tresholdSetup(); 
  }

  auto key = keypad.getKey();
  while (key != 'A' && key != '#') {
    key = keypad.getKey();
  }

  bool readyToLock = true;
  if (key == 'A' || newCodeNeeded) {
    readyToLock = registerPassword();
  }

  if (readyToLock) {
    lcd.clear();
    setMessage();
    safeState.lock();
    lock();
  }
}

void lockedLogic() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("ENTER PASSWORD");

  String userCode = inputPassword();
  bool unlockedSuccessfully = safeState.unlock(userCode);

  if (unlockedSuccessfully) {
    unlockMessage();
    unlock();
    userLog();
  } else {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("ACCESS DENIED!");
    delay(1000);
  }
}

void setup() {
  pinMode(LDR_IN1, INPUT);
  pinMode(LDR_IN2, INPUT);
  pinMode(LDR_OUT, INPUT);
  pinMode(PIR_INDOOR, INPUT);
  pinMode(PIR_GATE, INPUT);
  pinMode(GATE_PB, INPUT_PULLUP);
  pinMode(FAN_LED, OUTPUT);
  pinMode(LED_IN1, OUTPUT);
  pinMode(LED_IN2, OUTPUT);
  pinMode(LED_OUT, OUTPUT);
  pinMode(GATE_OP, OUTPUT);
  pinMode(GATE_CL, OUTPUT);

  Serial.begin(115200);
  DHT1.begin();
  DHT2.begin();
  lcd.begin(COLS, ROWS);
  doorLock.attach(SERVO_PIN);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

    noInterrupts();
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2B |= (1 << CS22) |
  (1 << CS20) ;
  TCNT2 = 0;
  OCR2A = 125 - 1;
  TIMSK2 |= (1 << OCIE2A);
  interrupts();

  if (safeState.locked()) {
    lock();
  } else {
    unlock();
  }
}

void loop() {
  if (TaskFlag1) {
    TaskFlag1 = false;
    gateControl();
  }
  if (TaskFlag2) {
    TaskFlag2 = false;
    tempFanControl();
  }
  if (TaskFlag3) {
    TaskFlag3 = false;
    lightLEDControl();
  }
   if (safeState.locked()) {
    lockedLogic();
  } else {
    unlockedLogic();
  }
}

ISR(TIMER2_COMPA_vect)
{
  TaskTimer1++;
  TaskTimer2++;
  TaskTimer3++;

  if (TaskTimer1 > 100 - 1) {
    TaskTimer1 = 0;
    TaskFlag1 = true;
  }

  if (TaskTimer2 > 100 - 1) {
    TaskTimer2 = 0;
    TaskFlag2 = true;
  }

  if (TaskTimer3 > 100 - 1) {
    TaskTimer3 = 0;
    TaskFlag3 = true;
  }
}
