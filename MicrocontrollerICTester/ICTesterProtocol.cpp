/*
 * ICTesterProtocol_V2.cpp
 * Arduino implementation with LINE-BASED protocol
 * Much more reliable than the original byte-based protocol
 * 
 * Protocol: All commands end with \n, all responses end with \n
 */

#include "ICTesterProtocol.h"

// Pin mapping definitions - REQUIRES ARDUINO MEGA 2560!
// Arduino Uno/Nano have insufficient pins (only ~20 usable, we need 24)

// #if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1280__)
//   // ===== ARDUINO MEGA 2560 - RECOMMENDED =====
//   // PORTB: 8 pins (Digital 22-29)
//   const uint8_t ICTesterProtocol::PORTB_PINS[8] = {22, 23, 24, 25, 26, 27, 28, 29};
  
//   // PORTD: 8 pins (Digital 30-37)
//   const uint8_t ICTesterProtocol::PORTD_PINS[8] = {30, 31, 32, 33, 34, 35, 36, 37};
  
//   // PORTA: 5 pins (Digital 38-42)
//   const uint8_t ICTesterProtocol::PORTA_PINS[5] = {38, 39, 40, 41, 42};
  
//   // PORTE: 3 pins (Digital 2, 3, 5 - safe pins with PWM)
//   const uint8_t ICTesterProtocol::PORTE_PINS[3] = {2, 3, 5};

// #else
//   // ===== ARDUINO UNO/NANO - LIMITED PIN COUNT =====
//   // WARNING: Uno/Nano only have ~20 usable pins, but we need 24!
  
//   // PORTB: 8 pins (Digital 2-9)
//   const uint8_t ICTesterProtocol::PORTB_PINS[8] = {2, 3, 4, 5, 6, 7, 8, 9};
  
//   // PORTD: 8 pins (Digital 10-13, A0-A3)
//   const uint8_t ICTesterProtocol::PORTD_PINS[8] = {10, 11, 12, 13, A0, A1, A2, A3};
  
//   // PORTA: 5 pins (A4, A5, A6, A7 if available, else use digital)
//   #if defined(A7)
//     const uint8_t ICTesterProtocol::PORTA_PINS[5] = {A4, A5, A6, A7, 0};
//   #else
//     const uint8_t ICTesterProtocol::PORTA_PINS[5] = {A4, A5, 0, 1, 14};  // Uses TX/RX!
//   #endif
  
//   // PORTE: 3 pins
//   #if defined(A6)
//     const uint8_t ICTesterProtocol::PORTE_PINS[3] = {A6, 14, 15};
//   #else
//     const uint8_t ICTesterProtocol::PORTE_PINS[3] = {14, 15, 16};
//   #endif
  
// #endif

ICTesterProtocol::ICTesterProtocol() : powerState(false), commandBufferIndex(0) {
    // // Compile-time board check
    // #if !defined(__AVR_ATmega2560__) && !defined(__AVR_ATmega1280__)
    //   #warning "Arduino Uno/Nano detected! 24 pins required but limited pins available. Arduino Mega 2560 strongly recommended!"
    // #endif
    
    // Initialize arrays
    for (int i = 0; i < 24; i++) {
        currentIO[i] = 1;    // All inputs by default
        currentData[i] = 1;  // All high by default
    }
    
    commandBuffer[0] = '\0';
}

void ICTesterProtocol::begin() {
    Serial.begin(19200);  // Match Qt default baudrate
    
    // // Initialize all pins as inputs with pullup
    // for (int i = 0; i < 8; i++) {
    //     pinMode(PORTB_PINS[i], INPUT_PULLUP);
    //     pinMode(PORTD_PINS[i], INPUT_PULLUP);
    // }
    // for (int i = 0; i < 5; i++) {
    //     pinMode(PORTA_PINS[i], INPUT_PULLUP);
    // }
    // for (int i = 0; i < 3; i++) {
    //     pinMode(PORTE_PINS[i], INPUT_PULLUP);
    // }
    
    powerState = false;
}

void ICTesterProtocol::handleCommand() {
    static unsigned long lastByteTime = 0;
    const unsigned long COMMAND_TIMEOUT = 1000;  // 1 second timeout
    
    // Read bytes into buffer until we get a newline
    while (Serial.available() > 0) {
        char c = Serial.read();
        lastByteTime = millis();  // Reset timeout on each byte
        
        if (c == '\n') {
            // Complete command received - process it
            processCommand();
            commandBufferIndex = 0;
            commandBuffer[0] = '\0';
        } else if (commandBufferIndex < sizeof(commandBuffer) - 1) {
            // Add to buffer
            commandBuffer[commandBufferIndex++] = c;
            commandBuffer[commandBufferIndex] = '\0';
        }
        // If buffer is full, ignore additional characters until newline
    }
    
    // TIMEOUT: If we have data in buffer but no newline for too long, clear it
    if (commandBufferIndex > 0 && (millis() - lastByteTime > COMMAND_TIMEOUT)) {
        // Timeout - clear buffer and send error
        Serial.println("ERR-TIMEOUT");
        commandBufferIndex = 0;
        commandBuffer[0] = '\0';
    }
}

void ICTesterProtocol::processCommand() {
    if (commandBufferIndex == 0) return;  // Empty command
    
    char cmd = commandBuffer[0];
    
    switch (cmd) {
        case '0':
            cmdReset();
            break;
        case '1':
            cmdPowerON();
            break;
        case '2':
            if (commandBufferIndex >= 5) {  // Need 4 data bytes + command
                cmdSetIO((uint8_t*)&commandBuffer[1]);
            } else {
                Serial.println("ERR");
            }
            break;
        case '3':
            if (commandBufferIndex >= 5) {  // Need 4 data bytes + command
                cmdSetData((uint8_t*)&commandBuffer[1]);
            } else {
                Serial.println("ERR");
            }
            break;
        case '4':
            cmdGetData();
            break;
        case '6':
            cmdVersion();
            break;
        default:
            // Unknown command - send error
            Serial.println("ERR");
            break;
    }
}

// Command '0' - Reset and send "Ready.\n"
void ICTesterProtocol::cmdReset() {
    // // Reset all pins to input with pullup
    // for (int i = 0; i < 8; i++) {
    //     pinMode(PORTB_PINS[i], INPUT_PULLUP);
    //     pinMode(PORTD_PINS[i], INPUT_PULLUP);
    // }
    // for (int i = 0; i < 5; i++) {
    //     pinMode(PORTA_PINS[i], INPUT_PULLUP);
    // }
    // for (int i = 0; i < 3; i++) {
    //     pinMode(PORTE_PINS[i], INPUT_PULLUP);
    // }
    
    powerState = false;
    
    // Reset state arrays
    for (int i = 0; i < 24; i++) {
        currentIO[i] = 1;
        currentData[i] = 1;
    }
    
    Serial.println("Ready.");
}

// Command '1' - Power ON
void ICTesterProtocol::cmdPowerON() {
    powerState = true;
    // In real implementation, you might control a power relay here
    Serial.println("OK.");
}

// Command '2' - Set I/O configuration (4 bytes: PORTB, PORTD, PORTA, PORTE)
void ICTesterProtocol::cmdSetIO(uint8_t* data) {
    // Apply configuration
    // applyPortConfig(PORTB_PINS, 8, data[0], 0);
    // applyPortConfig(PORTD_PINS, 8, data[1], 8);
    // applyPortConfig(PORTA_PINS, 5, data[2], 16);
    // applyPortConfig(PORTE_PINS, 3, data[3], 21);
    
    Serial.println("OK.");
}

// Command '3' - Set Data (4 bytes: PORTB, PORTD, PORTA, PORTE)
void ICTesterProtocol::cmdSetData(uint8_t* data) {
    // Apply data to output pins
    // applyPortData(PORTB_PINS, 8, data[0], 0);
    // applyPortData(PORTD_PINS, 8, data[1], 8);
    // applyPortData(PORTA_PINS, 5, data[2], 16);
    // applyPortData(PORTE_PINS, 3, data[3], 21);
    
    Serial.println("OK.");
}

// Command '4' - Get Data (returns 4 bytes + "OK.\n")
void ICTesterProtocol::cmdGetData() {
    // uint8_t portB = readPortData(PORTB_PINS, 8, 0);
    // uint8_t portD = readPortData(PORTD_PINS, 8, 8);
    // uint8_t portA = readPortData(PORTA_PINS, 5, 16);
    // uint8_t portE = readPortData(PORTE_PINS, 3, 21);
    
    // // Send 4 data bytes (binary, not text)
    // Serial.write(portB);
    // Serial.write(portD);
    // Serial.write(portA);
    // Serial.write(portE);

    // Send 4 data bytes (binary, not text)
    Serial.write(0);
    Serial.write(0);
    Serial.write(0);
    Serial.write(0);
    
    // Send OK confirmation with newline
    Serial.println("OK.");
}

// Command '6' - Get version
void ICTesterProtocol::cmdVersion() {
    Serial.println("ICTester Arduino v2.0 Line-Based");
}

// Helper: Apply I/O configuration to a port
void ICTesterProtocol::applyPortConfig(const uint8_t* pins, uint8_t count, uint8_t configByte, uint8_t startIdx) {
    uint8_t mask = 0x80;  // Start with MSB
    
    for (uint8_t i = 0; i < count; i++) {
        uint8_t mode = (configByte & mask) ? INPUT_PULLUP : OUTPUT;
        pinMode(pins[i], mode);
        currentIO[startIdx + i] = (configByte & mask) ? 1 : 0;
        mask >>= 1;
    }
}

// Helper: Apply data to output pins of a port
void ICTesterProtocol::applyPortData(const uint8_t* pins, uint8_t count, uint8_t dataByte, uint8_t startIdx) {
    uint8_t mask = 0x80;  // Start with MSB
    
    for (uint8_t i = 0; i < count; i++) {
        if (currentIO[startIdx + i] == 0) {  // Only write to output pins
            uint8_t value = (dataByte & mask) ? HIGH : LOW;
            digitalWrite(pins[i], value);
            currentData[startIdx + i] = (dataByte & mask) ? 1 : 0;
        }
        mask >>= 1;
    }
}

// Helper: Read data from a port (all pins, input or output)
uint8_t ICTesterProtocol::readPortData(const uint8_t* pins, uint8_t count, uint8_t startIdx) {
    uint8_t result = 0;
    uint8_t mask = 0x80;
    
    for (uint8_t i = 0; i < count; i++) {
        if (currentIO[startIdx + i] == 1) {  // Input pin - read it
            if (digitalRead(pins[i]) == HIGH) {
                result |= mask;
            }
        } else {  // Output pin - return current state
            if (currentData[startIdx + i] == 1) {
                result |= mask;
            }
        }
        mask >>= 1;
    }
    
    return result;
}
