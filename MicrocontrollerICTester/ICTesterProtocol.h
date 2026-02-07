/*
 * ICTesterProtocol.h
 * Arduino implementation with LINE-BASED protocol
 * 
 * Protocol improvements:
 * - All commands end with \n
 * - All responses end with \n
 * - Non-blocking command processing
 * - Much more reliable than byte-based protocol
 * 
 * Pin Mapping (Arduino Mega 2560):
 * PORTB (8 pins) -> Arduino Digital Pins 22-29
 * PORTD (8 pins) -> Arduino Digital Pins 30-37
 * PORTA (5 pins) -> Arduino Digital Pins 38-42
 * PORTE (3 pins) -> Arduino Digital Pins 2, 3, 5
 * 
 * Total: 24 pins
 */

#ifndef ICTESTER_PROTOCOL_H
#define ICTESTER_PROTOCOL_H

#include <Arduino.h>

class ICTesterProtocol {
private:
    // Pin definitions - 24 pins total
    // static const uint8_t PORTB_PINS[8];
    // static const uint8_t PORTD_PINS[8];
    // static const uint8_t PORTA_PINS[5];
    // static const uint8_t PORTE_PINS[3];
    
    // Current pin states
    uint8_t currentIO[24];   // 0=output, 1=input
    uint8_t currentData[24]; // current data values
    
    bool powerState;
    
    // Command buffer for line-based protocol
    char commandBuffer[32];  // Buffer for incoming command
    uint8_t commandBufferIndex;
    
    // Helper functions
    void processCommand();
    void applyPortConfig(const uint8_t* pins, uint8_t count, uint8_t configByte, uint8_t startIdx);
    void applyPortData(const uint8_t* pins, uint8_t count, uint8_t dataByte, uint8_t startIdx);
    uint8_t readPortData(const uint8_t* pins, uint8_t count, uint8_t startIdx);
    
    // Individual command handlers
    void cmdReset();
    void cmdPowerON();
    void cmdSetIO(uint8_t* data);
    void cmdSetData(uint8_t* data);
    void cmdGetData();
    void cmdVersion();
    
public:
    ICTesterProtocol();
    
    // Main protocol commands
    void begin();
    void handleCommand();  // Call this in loop()
};

#endif
