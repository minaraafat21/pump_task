#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define FLOWMETER_PIN 2  // Flowmeter signal pin
#define PUMP1_PIN 5      // PWM pin for Pump 1
#define PUMP2_PIN 6      // PWM pin for Pump 2
#define EMERGENCY_BTN 7  // Emergency stop button
#define LED_PIN 13       // Status LED

LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust I2C address as needed

volatile int pulseCount = 0;
float flowRate = 0.0; // Flow rate in mL/s
unsigned long lastMillis = 0;

// Pump control
int pumpSpeed = 128; // Default 50% duty cycle
bool pump1Active = true; // Start with Pump 1
unsigned long pumpSwitchInterval = 5000; // 5 seconds per pump
unsigned long lastSwitchTime = 0;
bool emergency_pressed = false;
void pulseCounter() {
  pulseCount++;
}

void setup() {
  pinMode(FLOWMETER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOWMETER_PIN), pulseCounter, RISING);
  
  pinMode(PUMP1_PIN, OUTPUT);
  pinMode(PUMP2_PIN, OUTPUT);
  pinMode(EMERGENCY_BTN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  lcd.begin(16,2);
  lcd.print("Infusion Pump");
  delay(2000);
  lcd.clear();

  // Start with Pump 1
  analogWrite(PUMP1_PIN, pumpSpeed);
  analogWrite(PUMP2_PIN, 0); // Ensure Pump 2 is off
}

void loop() {
  // Emergency button handling
  if (digitalRead(EMERGENCY_BTN) == LOW) {
    emergency_pressed = true;
    analogWrite(PUMP1_PIN, 0);
    analogWrite(PUMP2_PIN, 0);
    lcd.clear();
    lcd.print("Emergency Stop!");
    digitalWrite(LED_PIN, HIGH);
  }else{
    emergency_pressed = false;
    digitalWrite(LED_PIN, LOW);
    lcd.clear();
  }
  

  // Calculate flow rate
  if (millis() - lastMillis >= 5000) { // Every 5 seconds
    flowRate = (pulseCount / 7.5); // Adjust based on flowmeter specs
    pulseCount = 0;
    lastMillis = millis();
    
    lcd.clear();
    if (pump1Active) {
      lcd.setCursor(0, 0);
      lcd.print("Pump1 Flow: ");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Pump2 Flow: ");
    }
    lcd.setCursor(0, 1);
    lcd.print(flowRate, 2);
    lcd.print(" mL/s");
  }

  // Switch pumps at intervals
  if (millis() - lastSwitchTime >= pumpSwitchInterval) {
    pump1Active = !pump1Active; // Toggle active pump
    lastSwitchTime = millis();
    
    if (pump1Active) {
      digitalWrite(PUMP1_PIN, 1);
      analogWrite(PUMP2_PIN, 0);
    } else {
      analogWrite(PUMP1_PIN, 0);
      analogWrite(PUMP2_PIN, pumpSpeed);
    }
  }
}
