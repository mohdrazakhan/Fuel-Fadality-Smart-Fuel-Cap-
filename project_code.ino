#include <Wire.h>
#include <U8g2lib.h>

// U8g2 Constructor for 128x64 OLED (I2C)
// Try this first. If display shows nothing, try U8G2_R0 or U8G2_R2.
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

// Pin Definitions
#define FLOW_SENSOR_PIN 18
#define TURBIDITY_PIN 34
#define TEMP_SENSOR_PIN 35
#define LED_ALERT_PIN 2

// Flow Sensor Variables
volatile int pulseCount = 0;
unsigned long lastFlowCheck = 0;
float calibrationFactor =80;
float flowRate = 0;
float totalMilliLitres = 0;

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // OLED init
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 15, "Fuel Smart Cap Init");
  u8g2.sendBuffer();

  // Sensor init
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING);
  pinMode(LED_ALERT_PIN, OUTPUT);
  digitalWrite(LED_ALERT_PIN, LOW);

  Serial.println("✅ U8g2 OLED Init Successful");
}

void loop() {
  if (millis() - lastFlowCheck >= 1000) {
    detachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN));

    flowRate = (pulseCount / calibrationFactor); // L/min
    float flowMilliLitres = (flowRate / 60.0) * 1000.0;
    totalMilliLitres += flowMilliLitres;

    pulseCount = 0;
    lastFlowCheck = millis();
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, FALLING);
  }

  int turbidityValue = analogRead(TURBIDITY_PIN);
  bool badFuel = turbidityValue < 1800;

  int rawTemp = analogRead(TEMP_SENSOR_PIN);
  float voltage = rawTemp * (3.3 / 4095.0);
  float temperatureC = voltage * 100.0;
  bool tempIssue = (temperatureC > 50 || temperatureC < 0);

  if (badFuel || tempIssue) {
    digitalWrite(LED_ALERT_PIN, HIGH);
  } else {
    digitalWrite(LED_ALERT_PIN, LOW);
  }

  // Display Data
  u8g2.clearBuffer();
  u8g2.setCursor(25, 10); u8g2.print("Smart Fuel Cap");
  u8g2.setCursor(1, 20); u8g2.print("Flow: "); u8g2.print(flowRate); u8g2.print(" L/m");
  u8g2.setCursor(1, 30); u8g2.print("Used: "); u8g2.print(totalMilliLitres / 1000.0); u8g2.print(" L");
  u8g2.setCursor(1, 40); u8g2.print("Turb: "); u8g2.print(turbidityValue);
  if (badFuel) {u8g2.setCursor(0,50);  u8g2.print("---------BAD---------");}
  else{ u8g2.setCursor(0,50);                 u8g2.print("---------------------");}
  u8g2.setCursor(60, 40); u8g2.print(" Temp:"); u8g2.print(temperatureC); u8g2.print("C");
  if (tempIssue) {u8g2.setCursor(0,50);  u8g2.print("---------BAD---------");}

  u8g2.setCursor(0,60);                       u8g2.print("Develop By Raza Khan");
  u8g2.sendBuffer();

  // Serial Output
  Serial.println("==== Fuel Smart Cap ====");
  Serial.print("Flow Rate: "); Serial.println(flowRate);
  Serial.print("Used Fuel: "); Serial.println(totalMilliLitres / 1000.0);
  Serial.print("Turbidity: "); Serial.println(turbidityValue);
  Serial.print("Temperature: "); Serial.println(temperatureC);
  Serial.print("LED Alert: "); Serial.println((badFuel || tempIssue) ? "ON" : "OFF");
  Serial.println("------------------------");

  delay(500);
}