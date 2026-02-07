/*
 * ICTester_Arduino.ino
 * 
 * Arduino replacement for PIC-based ICTester
 * VERSION 2: Line-based protocol for reliability
 * 
 * IMPROVEMENTS OVER V1:
 * - All commands end with \n for clear boundaries
 * - Non-blocking command processing
 * - No timing issues with reset command
 * - Much more reliable communication
 * 
 * HARDWARE REQUIREMENTS:
 * - Arduino Mega 2560 (REQUIRED - 24 I/O pins needed)
 * - Serial connection to PC (USB)
 * 
 * SETUP:
 * 1. Upload this sketch to Arduino Mega
 * 2. Upload the modified devicedriver.cpp to your Qt project
 * 3. Connect Arduino to PC via USB
 * 4. Run Qt application - it should work reliably!
 * 
 * PIN CONNECTIONS (Arduino Mega 2560):
 * Socket Pin 1-8   -> Mega D22-D29 (PORTB)
 * Socket Pin 9-16  -> Mega D30-D37 (PORTD)
 * Socket Pin 17-21 -> Mega D38-D42 (PORTA)
 * Socket Pin 22-24 -> Mega D2, D3, D5 (PORTE)
 * 
 * PROTOCOL:
 * Commands: "0\n", "1\n", "2<4bytes>\n", "3<4bytes>\n", "4\n", "6\n"
 * Responses: "Ready.\n", "OK.\n", <4bytes>"OK.\n", "version\n"
 * 
 * BAUD RATE: 19200
 */

#include "ICTesterProtocol.h"

ICTesterProtocol tester;

void setup() {
    tester.begin();
    
    // Optional: LED to indicate ready state
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
    // Process incoming commands
    // This is non-blocking and handles line-based protocol
    tester.handleCommand();
    
    // Heartbeat LED to show Arduino is running
    static unsigned long lastBlink = 0;
    static bool ledState = false;
    
    if (millis() - lastBlink > 1000) {
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState);
        lastBlink = millis();
    }
}
