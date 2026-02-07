/*
 * devicedriver.h
 * Header file for ICTester DeviceDriver
 * 
 * MODIFIED VERSION: Added readLine() method for line-based protocol
 */

#ifndef DEVICEDRIVER_H
#define DEVICEDRIVER_H

#include <QtSerialPort/QSerialPort>
#include <QString>
#include <QByteArray>

class DeviceDriver
{
private:
    QSerialPort *serialConn;
    int Timeout;
    
    // Private helper functions
    int openConn(QString portName, int baudRate);
    QByteArray getResponse(int expLength);  // Old method, kept for compatibility
    QString readLineResponse();  // NEW: Line-based reading for reliable protocol
    
public:
    // Constructor & Destructor
    DeviceDriver(QString portName, int baudRate, int timeout);
    ~DeviceDriver();
    
    // Configuration methods
    int changeName(QString portName);
    int changeRate(int baudRate);
    void changeTimeout(int timeout);
    int getTimeout();
    
    // Utility
    void qSleep(int ms);
    
    // Device Commands
    int reset();
    int powerON();
    int setIO(QString pins);
    int setData(QString pins);
    QString getData();
    QString deviceVersion();
};

#endif // DEVICEDRIVER_H
