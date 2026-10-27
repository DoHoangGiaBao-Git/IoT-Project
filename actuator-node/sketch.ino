
#include <Arduino.h>
#include <ESP32Servo.h>

// ==================================================
// FIRE ALARM SYSTEM - ACTUATOR NODE (ESP32)
// He thong dieu khien thiet bi khi co bao chay
// ==================================================

// 1. KHAI BAO CAC CHAN KET NOI

// Nut nhan mo phong tin hieu tu Central Node
const int PIN_ALARM_BUTTON = 18;

// Nut nhan reset he thong
const int PIN_RESET_BUTTON = 19;

// Cong tac mo phong mat dien
const int PIN_POWER_FAIL = 21;

// Relay dieu khien voi phun nuoc
const int PIN_SPRINKLER = 25;

// Servo dieu khien cua thoat hiem
const int PIN_EXIT_DOOR = 26;

// Servo dieu khien cua so
const int PIN_WINDOW = 27;

// Coi bao chay
const int PIN_BUZZER = 14;

// LED do canh bao chay
const int PIN_WARNING_LED = 23;

// LED vang bao nguon du phong
const int PIN_BATTERY_LED = 22;

// LED xanh bao trang thai voi phun
const int PIN_SPRINKLER_LED = 32;


// 2. KHAI BAO DOI TUONG SERVO

Servo exitDoorServo;
Servo windowServo;


// 3. KHAI BAO BIEN TRANG THAI

// false: khong co chay
// true: dang bao chay
bool alarmLatched = false;

// Trang thai mat dien
bool powerFailed = false;

// Trang thai coi va LED canh bao
bool sirenOn = false;

// Bien chong doi nut nhan
bool lastAlarmButton = HIGH;
bool lastResetButton = HIGH;

// Thoi gian nhap nhay cua coi va LED
const unsigned long SIREN_PERIOD_MS = 1200;
const unsigned long SIREN_ON_MS = 700;


// 4. HAM DIEU KHIEN CAC THIET BI

void applyActuators() {

  if (alarmLatched == true) {

    // Bat relay dieu khien voi phun
    digitalWrite(PIN_SPRINKLER, HIGH);

    // Bat LED trang thai voi phun
    digitalWrite(PIN_SPRINKLER_LED, HIGH);

    // Mo cua thoat hiem 90 do
    exitDoorServo.write(90);

    // Mo cua so 90 do
    windowServo.write(90);

    Serial.println("Da kich hoat cac thiet bi!");
    Serial.println("Voi phun: ON");
    Serial.println("Cua thoat hiem: OPEN");
    Serial.println("Cua so: OPEN");

  } else {

    // Tat relay voi phun
    digitalWrite(PIN_SPRINKLER, LOW);

    // Tat LED trang thai voi phun
    digitalWrite(PIN_SPRINKLER_LED, LOW);

    // Dua servo ve vi tri ban dau
    // Chi su dung cho mo phong
    exitDoorServo.write(0);
    windowServo.write(0);

    // Tat coi bao chay
    noTone(PIN_BUZZER);

    // Tat LED canh bao
    digitalWrite(PIN_WARNING_LED, LOW);

    sirenOn = false;
  }
}


// 5. HAM XU LY TIN HIEU BAO CHAY

void setAlarm(bool active) {

  // Neu trang thai khong thay doi thi bo qua
  if (alarmLatched == active) {
    return;
  }

  alarmLatched = active;

  if (alarmLatched == true) {

    Serial.println("======================");
    Serial.println("CANH BAO: PHAT HIEN CHAY!");
    Serial.println("======================");

  } else {

    Serial.println("======================");
    Serial.println("HE THONG DA RESET");
    Serial.println("======================");
  }

  // Cap nhat trang thai thiet bi
  applyActuators();
}


// 6. KIEM TRA MAT DIEN

void checkPower() {

  // LOW: mo phong mat dien
  bool newPowerFailed =
      (digitalRead(PIN_POWER_FAIL) == LOW);

  // Neu trang thai khong thay doi thi bo qua
  if (newPowerFailed == powerFailed) {
    return;
  }

  powerFailed = newPowerFailed;

  if (powerFailed == true) {

    // Bat LED bao nguon du phong
    digitalWrite(PIN_BATTERY_LED, HIGH);

    Serial.println("CANH BAO: MAT DIEN!");
    Serial.println("Chuyen sang nguon du phong (gia lap)");

  } else {

    // Tat LED nguon du phong
    digitalWrite(PIN_BATTERY_LED, LOW);

    Serial.println("Nguon dien da khoi phuc");
  }
}


// 7. DIEU KHIEN COI VA LED CANH BAO

void updateWarning() {

  // Neu khong co bao chay thi khong can canh bao
  if (alarmLatched == false) {
    return;
  }

  // Tao chu ky bat/tat coi va LED
  bool shouldWarn =
      (millis() % SIREN_PERIOD_MS < SIREN_ON_MS);

  // Neu trang thai khong thay doi thi bo qua
  if (shouldWarn == sirenOn) {
    return;
  }

  sirenOn = shouldWarn;

  if (sirenOn == true) {

    // Bat LED do
    digitalWrite(PIN_WARNING_LED, HIGH);

    // Bat coi tan so 1400 Hz
    tone(PIN_BUZZER, 1400);

  } else {

    // Tat LED do
    digitalWrite(PIN_WARNING_LED, LOW);

    // Tat coi
    noTone(PIN_BUZZER);
  }
}


// 8. NHAN LENH TU CENTRAL NODE QUA SERIAL

void readCentralCommand() {

  if (Serial.available() == 0) {
    return;
  }

  // Doc du lieu tu Serial Monitor
  String command = Serial.readStringUntil('\n');

  // Xoa khoang trang va chuyen thanh chu hoa
  command.trim();
  command.toUpperCase();

  // Nhan lenh bao chay
  if (command == "ALARM") {

    setAlarm(true);

  }

  // Nhan lenh reset
  else if (command == "RESET") {

    setAlarm(false);

  }

  // Xem trang thai he thong
  else if (command == "STATUS") {

    Serial.println("----- SYSTEM STATUS -----");

    Serial.print("Fire Alarm: ");
    Serial.println(alarmLatched ? "ON" : "OFF");

    Serial.print("Power Failure: ");
    Serial.println(powerFailed ? "YES" : "NO");

    Serial.println("-------------------------");

  }

  // Lenh khong hop le
  else if (command.length() > 0) {

    Serial.println("Lenh khong hop le!");
    Serial.println("Dung: ALARM, RESET hoac STATUS");

  }
}


// 9. HAM SETUP - KHOI TAO HE THONG

void setup() {

  // Khoi tao Serial Monitor
  Serial.begin(115200);

  // Thoi gian cho du lieu Serial
  Serial.setTimeout(100);

  // Cau hinh nut nhan
  pinMode(PIN_ALARM_BUTTON, INPUT_PULLUP);
  pinMode(PIN_RESET_BUTTON, INPUT_PULLUP);

  // Cau hinh cong tac mat dien
  pinMode(PIN_POWER_FAIL, INPUT_PULLUP);

  // Cau hinh cac chan OUTPUT
  pinMode(PIN_SPRINKLER, OUTPUT);
  pinMode(PIN_SPRINKLER_LED, OUTPUT);
  pinMode(PIN_WARNING_LED, OUTPUT);
  pinMode(PIN_BATTERY_LED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  // Dat cac thiet bi ve trang thai ban dau
  digitalWrite(PIN_SPRINKLER, LOW);
  digitalWrite(PIN_SPRINKLER_LED, LOW);
  digitalWrite(PIN_WARNING_LED, LOW);
  digitalWrite(PIN_BATTERY_LED, LOW);

  // Thiet lap tan so PWM cho Servo
  exitDoorServo.setPeriodHertz(50);
  windowServo.setPeriodHertz(50);

  // Gan chan dieu khien cho Servo
  exitDoorServo.attach(PIN_EXIT_DOOR, 500, 2400);
  windowServo.attach(PIN_WINDOW, 500, 2400);

  // Dua Servo ve goc ban dau
  exitDoorServo.write(0);
  windowServo.write(0);

  // Kiem tra trang thai dien
  // HIGH: co dien, LOW: mat dien
  powerFailed = (digitalRead(PIN_POWER_FAIL) == LOW);
  digitalWrite(PIN_BATTERY_LED,
               powerFailed ? HIGH : LOW);

  // Thong bao khoi dong
  Serial.println("============================");
  Serial.println("FIRE ALARM - ACTUATOR NODE");
  Serial.println("============================");

  Serial.println("System initialized!");
  Serial.println("Waiting for Central Node...");

  Serial.println("Available commands:");
  Serial.println("ALARM  - Activate fire alarm");
  Serial.println("RESET  - Reset fire alarm");
  Serial.println("STATUS - Show system status");
}


// 10. HAM LOOP - CHUONG TRINH CHINH

void loop() {

  // Buoc 1: Kiem tra mat dien
  checkPower();

  // Buoc 2: Kiem tra thong bao tu Central Node
  readCentralCommand();

  // Buoc 3: Doc nut nhan bao chay
  bool alarmButton = digitalRead(PIN_ALARM_BUTTON);

  // Phat hien canh xuong khi nhan nut
  if (lastAlarmButton == HIGH &&
      alarmButton == LOW) {

    // Gia lap nhan lenh bao chay
    setAlarm(true);
  }

  lastAlarmButton = alarmButton;

  // Buoc 4: Kiem tra nut Reset
  bool resetButton = digitalRead(PIN_RESET_BUTTON);

  if (lastResetButton == HIGH &&
      resetButton == LOW) {

    // Chi reset neu nut bao chay khong bi giu
    if (alarmButton == HIGH) {
      setAlarm(false);
    }
  }

  lastResetButton = resetButton;

  // Buoc 5: Duy tri coi va LED canh bao
  updateWarning();

  // Dung ngan de tranh doc nut qua nhanh
  delay(20);
}
