#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

const int THRESHOLD = 500;  // Define gas threshold for easier configuration

// Define constants for pin numbers
const int GAS_SENSOR_PIN = A0;
const int LCD_PINS[] = {2, 3, 4, 5, 6, 7};
const int ALARM_PINS[] = {13, 9, 8, 12};  // {buzzer, redLED, greenLED, relay}
const int GSM_PINS[] = {10, 11};

class GasSensor {
private:
    int pin;
public:
    GasSensor(int sensorPin) : pin(sensorPin) {
        pinMode(pin, INPUT);
    }

    int readGasLevel() {
        return analogRead(pin);
    }
};

class Display {
private:
    LiquidCrystal lcd;
public:
    Display(int pins[6]) : lcd(pins[0], pins[1], pins[2], pins[3], pins[4], pins[5]) {
        lcd.begin(16, 2);
    }

    void showMessage(const char* line1, const char* line2 = "") {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(line1);
        if (line2[0] != '\0') {  // Check if line2 is non-empty
            lcd.setCursor(0, 1);
            lcd.print(line2);
        }
    }

    void showGasLevel(int level) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "Gas Level: %d", level);
        showMessage("Gas Scan is ON", buffer);
    }
};

class Alarm {
private:
    int buzzerPin;
    int redLedPin;
    int greenLedPin;
    int relayPin;
public:
    Alarm(int pins[4]) : buzzerPin(pins[0]), redLedPin(pins[1]), greenLedPin(pins[2]), relayPin(pins[3]) {
        pinMode(buzzerPin, OUTPUT);
        pinMode(redLedPin, OUTPUT);
        pinMode(greenLedPin, OUTPUT);
        pinMode(relayPin, OUTPUT);
    }

    void trigger() {
        digitalWrite(redLedPin, HIGH); 
        digitalWrite(greenLedPin, LOW);
        digitalWrite(relayPin, HIGH);
    }

    void reset() {
        digitalWrite(buzzerPin, LOW);
        digitalWrite(redLedPin, LOW);
        digitalWrite(greenLedPin, HIGH);
        digitalWrite(relayPin, LOW);
    }

    void playStartupSound() {
      tone(buzzerPin, 2000, 150);
      delay(150);
      tone(buzzerPin, 2000, 180);
      delay(150);
      tone(buzzerPin, 2500, 100);
      delay(200);
      tone(buzzerPin, 3000, 150);
      delay(250);
      noTone(buzzerPin);
    }

    void playAlertSound() {
      for(int i = 0; i < 2; i++) {
        for (int i = 0; i < 5; i++) {
          tone(buzzerPin, 2000, 150); 
          delay(100);
          tone(buzzerPin, 4000, 150);
          delay(160);
          tone(buzzerPin, 2500, 150);
          delay(100);
          tone(buzzerPin, 4500,   150);
          delay(160);
      }
      delay(1000);
      }
      noTone(buzzerPin);
    }
};

class GSMModule {
private:
    SoftwareSerial mySerial;
public:
    GSMModule(int rx, int tx) : mySerial(rx, tx) {
        mySerial.begin(9600);
    }

    void sendMessage(const char* message, int gasLevel) {
        mySerial.println("AT+CMGF=1");  // Set SMS mode to text
        delay(1000);
        mySerial.println("AT+CMGS=\"+91xxxxxxxxxx\"\r");  // Replace with real number
        delay(1000);
        mySerial.println(message);

        char gasLevelMessage[30];
        snprintf(gasLevelMessage, sizeof(gasLevelMessage), "Gas Level: %d", gasLevel);
        mySerial.println(gasLevelMessage);

        mySerial.write(26);  // ASCII code for CTRL+Z (end of message)
        delay(1000);
    }
};

class GasLeakageDetector {
private:
    GasSensor& sensor;  // Pass by reference
    Display& display;   // ...
    Alarm& alarm;       
    GSMModule& gsmModule;  
    const int threshold;

public:
    GasLeakageDetector(GasSensor& gasSensor, Display& lcdDisplay, Alarm& gasAlarm, GSMModule& gsm, int threshold = THRESHOLD)
        : sensor(gasSensor), display(lcdDisplay), alarm(gasAlarm), gsmModule(gsm), threshold(threshold) {}

    void detectAndRespond() {
        int gasLevel = sensor.readGasLevel();
        Serial.print("Gas Level: ");
        Serial.println(gasLevel);
        display.showGasLevel(gasLevel);

        if (gasLevel > threshold) {
            alarm.trigger();
            gsmModule.sendMessage("Excess Gas Detected", gasLevel);
            Serial.println("Excess Gas Detected");
            alarm.playAlertSound();
            display.showMessage("Gas Level Exceed", "SMS Sent");
        } else {
            alarm.reset();
            Serial.println("Gas Level Normal");
            display.showMessage("Gas Level Normal");
        }

        delay(1000);
    }
};

// Setup
GasSensor gasSensor(GAS_SENSOR_PIN);
Display lcd(LCD_PINS);
Alarm gasAlarm(ALARM_PINS);
GSMModule gsmModule(GSM_PINS[0], GSM_PINS[1]);
GasLeakageDetector detector(gasSensor, lcd, gasAlarm, gsmModule);

void setup() {
    Serial.begin(9600);

    // Play the startup sound
    gasAlarm.playStartupSound();

    lcd.showMessage(" Gas Leakage ", "Detector Alarm");
    delay(7000);
}

// Main loop
void loop() {
    detector.detectAndRespond();
}
