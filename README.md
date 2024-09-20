# Gas Leakage Detector System

## Overview

This project implements a **Gas Leakage Detector** using an Arduino, gas sensor, LCD display, alarm system (buzzer + LEDs), and GSM module to send notifications. The system continuously monitors gas levels and takes appropriate actions (trigger alarms and send SMS notifications) when gas levels exceed a predefined threshold.

### Features:
- **Real-Time Gas Monitoring**: Continuously reads gas levels via an analog sensor.
- **LCD Display**: Provides real-time feedback on gas status.
- **Alarm System**: Visual (LEDs) and audio (buzzer) alerts.
- **GSM Module**: Sends SMS notifications to predefined phone numbers when gas levels are hazardous.
- **Threshold-Based Detection**: Actions are triggered when gas levels exceed a set threshold.

## Components:
- **Gas Sensor** (connected to analog pin A0): Detects gas levels.
- **LiquidCrystal Display** (LCD): 16x2 display for showing status messages and gas levels.
- **Alarm System**: Includes a buzzer, red LED (danger), green LED (safe), and a relay.
- **GSM Module**: Sends SMS alerts to a specified phone number when gas levels are critical.

## Pin Configuration:
| Component        | Pin(s)              | Description                    |
|------------------|---------------------|--------------------------------|
| **Gas Sensor**   | A0                  | Analog pin to read gas levels. |
| **LCD Display**  | 2, 3, 4, 5, 6, 7    | Control pins for 16x2 LCD.     |
| **Buzzer**       | 13                  | Output pin for the buzzer.     |
| **Red LED**      | 9                   | Output pin for red LED (danger).|
| **Green LED**    | 8                   | Output pin for green LED (safe).|
| **Relay**        | 12                  | Output pin for relay control.  |
| **GSM Module**   | 10 (RX), 11 (TX)    | Pins for SoftwareSerial communication. |

## System Flow:
1. **Startup**: The system initializes, plays a startup sound on the buzzer, and displays a welcome message on the LCD.
2. **Gas Level Monitoring**: The gas level is read from the sensor continuously.
3. **Normal Condition**: If gas levels are below the threshold, the green LED stays ON, and the system displays "Gas Level Normal" on the LCD.
4. **Dangerous Condition**: If the gas level exceeds the threshold:
   - The red LED turns ON, and the green LED turns OFF.
   - The relay is activated (to control external equipment if necessary).
   - The buzzer plays an alert sound.
   - An SMS is sent to a predefined number indicating the gas level.
   - The LCD displays a warning message ("Gas Level Exceed", "SMS Sent").
5. **Repeat**: After every check, the system waits for 1 second before the next cycle.

## Code Breakdown

### Constants and Pin Definitions:
```cpp
const int THRESHOLD = 500; // Gas threshold for triggering the alarm.
const int GAS_SENSOR_PIN = A0;  // Analog pin to read gas sensor data.
const int LCD_PINS[] = {2, 3, 4, 5, 6, 7}; // Pins for 16x2 LCD.
const int ALARM_PINS[] = {13, 9, 8, 12};  // {buzzer, redLED, greenLED, relay}.
const int GSM_PINS[] = {10, 11}; // Pins for GSM module (RX, TX).
```

### `GasSensor` Class:
Manages interaction with the gas sensor.
```cpp
class GasSensor {
    int pin;
public:
    GasSensor(int sensorPin) : pin(sensorPin) {
        pinMode(pin, INPUT);
    }

    int readGasLevel() {
        return analogRead(pin); // Returns gas level as analog value (0-1023).
    }
};
```

### `Display` Class:
Manages the LCD for showing messages.
```cpp
class Display {
    LiquidCrystal lcd;
public:
    Display(int pins[6]) : lcd(pins[0], pins[1], pins[2], pins[3], pins[4], pins[5]) {
        lcd.begin(16, 2); // Initialize LCD with 16x2 character display.
    }

    void showMessage(const char* line1, const char* line2 = "") {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(line1);  // Show message on line 1.
        if (line2[0] != '\0') {
            lcd.setCursor(0, 1);
            lcd.print(line2);  // Show message on line 2 if provided.
        }
    }

    void showGasLevel(int level) {
        char buffer[16];
        snprintf(buffer, sizeof(buffer), "Gas Level: %d", level);
        showMessage("Gas Scan is ON", buffer);  // Display current gas level.
    }
};
```

### `Alarm` Class:
Handles the alert system, including buzzer sounds and LED indicators.
```cpp
class Alarm {
    int buzzerPin, redLedPin, greenLedPin, relayPin;
public:
    Alarm(int pins[4]) : buzzerPin(pins[0]), redLedPin(pins[1]), greenLedPin(pins[2]), relayPin(pins[3]) {
        pinMode(buzzerPin, OUTPUT);
        pinMode(redLedPin, OUTPUT);
        pinMode(greenLedPin, OUTPUT);
        pinMode(relayPin, OUTPUT);
    }

    void trigger() {
        digitalWrite(redLedPin, HIGH); // Danger: Red LED ON, green OFF.
        digitalWrite(greenLedPin, LOW);
        digitalWrite(relayPin, HIGH);  // Activate relay.
    }

    void reset() {
        digitalWrite(buzzerPin, LOW);
        digitalWrite(redLedPin, LOW);  // Reset alarm state.
        digitalWrite(greenLedPin, HIGH);
        digitalWrite(relayPin, LOW);
    }

    void playStartupSound() {
        // Plays a series of tones for startup.
    }

    void playAlertSound() {
        // Plays a repeating alert sound sequence.
    }
};
```

### `GSMModule` Class:
Communicates with the GSM module to send SMS messages.
```cpp
class GSMModule {
    SoftwareSerial mySerial;
public:
    GSMModule(int rx, int tx) : mySerial(rx, tx) {
        mySerial.begin(9600);  // Start GSM communication.
    }

    void sendMessage(const char* message, int gasLevel) {
        // Sends SMS message with current gas level.
    }
};
```

### `GasLeakageDetector` Class:
Orchestrates the behavior of the system, including reading gas levels, controlling the display, alarm, and GSM module.
```cpp
class GasLeakageDetector {
    GasSensor& sensor;
    Display& display;
    Alarm& alarm;
    GSMModule& gsmModule;
    const int threshold;
public:
    GasLeakageDetector(GasSensor& gasSensor, Display& lcdDisplay, Alarm& gasAlarm, GSMModule& gsm, int threshold = THRESHOLD)
        : sensor(gasSensor), display(lcdDisplay), alarm(gasAlarm), gsmModule(gsm), threshold(threshold) {}

    void detectAndRespond() {
        int gasLevel = sensor.readGasLevel();
        display.showGasLevel(gasLevel);

        if (gasLevel > threshold) {
            alarm.trigger();
            gsmModule.sendMessage("Excess Gas Detected", gasLevel);
            alarm.playAlertSound();
            display.showMessage("Gas Level Exceed", "SMS Sent");
        } else {
            alarm.reset();
            display.showMessage("Gas Level Normal");
        }

        delay(1000); // Wait before next reading.
    }
};
```

## Setup & Loop:
- **setup()**: Initializes the components and shows the startup message.
- **loop()**: Runs continuously, checking gas levels and responding accordingly.

```cpp
void setup() {
    Serial.begin(9600);
    gasAlarm.playStartupSound();
    lcd.showMessage(" Gas Leakage ", "Detector Alarm");
    delay(7000);  // Display startup message for 7 seconds.
}

void loop() {
    detector.detectAndRespond();  // Continuously monitor gas levels.
}
```

## Usage:
1. Connect all components to the Arduino according to the pin configuration table.
2. Upload the code to your Arduino board.
3. Power up the system, and it will start monitoring the gas levels immediately.

## License:
This project is open-source under the MIT License. Feel free to modify and redistribute the code with proper attribution.
