//Copyright 2014,2015 MCbx, All rights reserved.
//http://mcbx.netne.net/ictester
//This file is part of ICTester.
//ICTester is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 2 of the License, or
//(at your option) any later version.
//ICTester is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//You should have received a copy of the GNU General Public License
//along with ICTester; if not, write to the Free Software Foundation,
//Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// MODIFIED VERSION: Uses line-based protocol with \n terminators
// This version is more reliable and works better with Arduino

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
//#include <QMessageBox>
#include <qstring.h>
#include <QTime>
#include "devicedriver.h"
#ifdef Q_OS_WIN
#include <windows.h>
#include <QDateTime>
#endif

//Errors:
// -  0 - no error, operation completed.
// - -1 - port not opened
// - -2 - device not present
// - -3 - Improper response from device.
// - -4 - user data error. It's your fault.


DeviceDriver::DeviceDriver(QString portName, int baudRate, int timeout)
{
    serialConn = new QSerialPort();
    this->Timeout=timeout;
    this->openConn(portName,baudRate);
}

DeviceDriver::~DeviceDriver()
{
    if (serialConn->isOpen()) serialConn->close();
}

//PRIVATE CONNECTION FUNCTIONS
int DeviceDriver::openConn(QString portName, int baudRate=QSerialPort::Baud19200)
{
    serialConn->setPortName(portName);
    serialConn->setBaudRate(baudRate);
    serialConn->setDataBits(QSerialPort::Data8);
    serialConn->setStopBits(QSerialPort::OneStop);
    serialConn->setFlowControl(QSerialPort::NoFlowControl);
    serialConn->setParity(QSerialPort::NoParity);

    //open connection. Close it at destructor.
     if (!serialConn->open(QIODevice::ReadWrite))
     {
  //       QMessageBox::critical(NULL,"Error", serialConn->errorString());
         return -1;
     }
     return 0;
}

//NEW: Read a line with timeout - more reliable than getResponse()
QString DeviceDriver::readLineResponse()
{
#ifdef Q_OS_WIN
    qint64 start = QDateTime::currentDateTime().toMSecsSinceEpoch();
    QByteArray buffer;
    
    while (QDateTime::currentDateTime().toMSecsSinceEpoch() < start + this->Timeout)
    {
        if (serialConn->waitForReadyRead(100))
        {
            buffer.append(serialConn->readAll());
            
            // Check if we have a complete line
            if (buffer.contains('\n'))
            {
                // Extract up to and including the newline
                int nlPos = buffer.indexOf('\n');
                QByteArray line = buffer.left(nlPos + 1);
                return QString::fromLatin1(line).trimmed();
            }
        }
    }
    return QString(); // Timeout
#else
    QByteArray buffer;
    qint64 start = QDateTime::currentDateTime().toMSecsSinceEpoch();
    
    while (QDateTime::currentDateTime().toMSecsSinceEpoch() < start + this->Timeout)
    {
        if (serialConn->waitForReadyRead(100))
        {
            buffer.append(serialConn->readAll());
            
            if (buffer.contains('\n'))
            {
                int nlPos = buffer.indexOf('\n');
                QByteArray line = buffer.left(nlPos + 1);
                return QString::fromLatin1(line).trimmed();
            }
        }
    }
    return QString();
#endif
}

//OLD getResponse kept for compatibility but not used anymore
QByteArray DeviceDriver::getResponse(int expLength)
{
    for (;;)
    {
#ifdef Q_OS_WIN         //Qt bug unfixed since 2010, Windows only. Writing this in 2015.
        bool ret;       //I have to rewrite waiting routine, the original sometimes doesn't work in Windows.
        qint64 start=QDateTime::currentDateTime().toMSecsSinceEpoch();
        while(QDateTime::currentDateTime().toMSecsSinceEpoch()<start+this->Timeout)
        {
            ret = serialConn->waitForReadyRead(this->Timeout); // wait at least one byte available
            if (ret)
            {
                qint64 bav = serialConn->bytesAvailable();
                if (bav < expLength) continue;

                QByteArray recvData = serialConn->read(expLength);
                serialConn->clear();
                return recvData;
            }
        }
        return QByteArray();
#else                //Linux version works flawlessly
        bool ret = serialConn->waitForReadyRead(this->Timeout); // wait at least one byte available
        if (ret)
        {
            qint64 bav = serialConn->bytesAvailable();
            if (bav < expLength) continue;

            QByteArray recvData = serialConn->read(expLength);
            serialConn->clear();
            return recvData;
        }
        else
        {
            return QByteArray();
        }

#endif
    }
}

/////////////////////////
////PUBLIC INTERFACES////
/////////////////////////

//sleep
void DeviceDriver::qSleep(int ms)
{
#ifdef Q_OS_WIN
    Sleep(uint(ms));
#else
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif
}

//change name and reopen connection
int DeviceDriver::changeName(QString portName)
{

    int rate=serialConn->baudRate();
    if (serialConn->isOpen()) serialConn->close();
    return openConn(portName,rate);
}

//change rate and reopen connection
int DeviceDriver::changeRate(int baudRate)
{

    QString name=serialConn->portName();
    if (serialConn->isOpen())  serialConn->close();
    return openConn(name,baudRate);
}

void DeviceDriver::changeTimeout(int timeout)
{
    this->Timeout=timeout;
}

int DeviceDriver::getTimeout()
{
    return this->Timeout;
}

///////////////////////
////DEVICE COMMANDS////
///////////////////////

//Reset and check is device present
//MODIFIED: New line-based protocol - send "0\n", expect "Ready."
int DeviceDriver::reset()
{
    if (!serialConn->isOpen())
    {
        return -1;
    }
    
    // NEW: Send reset command once with newline terminator
    serialConn->clear();
    QByteArray cmd;
    cmd.append('0');
    cmd.append('\n');
    serialConn->write(cmd);
    serialConn->flush();
    
    // Wait for response line
    QString res = readLineResponse();
    
    if (res == "Ready.")
    {
        return 0;
    }
    if (res.length() == 0)
    {
        return -2;  //no response
    }
    return -3;  //unexpected response
}

//turn IC power ON
//MODIFIED: New line-based protocol - send "1\n", expect "OK."
int DeviceDriver::powerON()
{
    if (!serialConn->isOpen())
    {
        return -1;
    }
    
    serialConn->clear();
    QByteArray cmd;
    cmd.append('1');
    cmd.append('\n');
    serialConn->write(cmd);
    serialConn->flush();
    
    QString res = readLineResponse();
    
    if (res == "OK.")
    {
        return 0;
    }
    if (res.length() == 0)
    {
        return -2;  //no response
    }
    return -3;  //unexpected response
}

//sets I/O config - 0 out, 1 in (out - to tested ic)
//MODIFIED: New line-based protocol - send "2" + 4 bytes + "\n", expect "OK."
int DeviceDriver::setIO(QString pins)
{
  //  pins=pins.replace("X","1"); //This is as in original protocol in command 2 only.
    if (!serialConn->isOpen()) return -1;
    if (pins.length()>24) return -4; //error in inp data

    //This code prepares BIT PATTERN which is string of 24 chars. They can be
    //1 or 0, when 1 means PIC's input, 0 PIC's output.
    //pins is usually shorter, so it must "align" the chip into socket.
    QString sBitPattern;
    sBitPattern=sBitPattern.fill('1',12-(pins.length()/2)); //set unused pins IN - left side
    for (int F=1;F<=pins.length();F++)
    {
        if (pins[F-1]=='0')  //is output supplying signal INTO tested IC.
        {
            sBitPattern=sBitPattern+'0';
        }
        else  //is input taking signal FROM test IC or Vcc or GND or NC.
        {
            sBitPattern=sBitPattern+'1';
        }
    }
    sBitPattern=sBitPattern.leftJustified(24,'1'); //set unused pins IN - right side

    //QMessageBox::information(NULL,"pattern",sBitPattern);
    //Now we can prepare data packet for Device

    //The packet contains of 5 bytes, command (1 byte) and data.
    //Ports B,D,A,E - this order.
    //So you can see that not all pins are not in all ports - PIC has just some ports OC.
    //and they're unsuitable for tester.

    int nTotals[4];
    for (int i=0;i<4;i++) nTotals[i]=0;

    int nMul=128;
    for (int F=0;F<8;F++)
    {
        if (sBitPattern[F]=='1') //PORTB used entirely
        {
            nTotals[0]+=nMul;
        }
        if (sBitPattern[F+8]=='1') //PORTD used entirely
        {
            nTotals[1]+=nMul;
        }
        nMul=nMul/2;
    }

    //PORTA - we're not using PA4 here.
    //we're using PA0,1,2,3,5.
    nMul=32;
    for (int F=0;F<5;F++)
    {
       if (sBitPattern[F+16]=='1')
       {
           nTotals[2]+=nMul;
       }
       if (F==0)
       {
           nMul=nMul/2; //shift out first bit;
       }
       nMul=nMul/2;
    }

    //PORTE - 3 bits are used here.
    //PE0,1,2
    nMul=4;
    for (int F=0;F<3;F++)
    {
        if (sBitPattern[F+21]=='1')
        {
            nTotals[3]+=nMul;
        }
        nMul=nMul/2;
    }

    // NEW: Build complete command with newline terminator
    serialConn->clear();
    QByteArray cmd;
    cmd.append('2');
    for (int F=0;F<4;F++)
    {
        unsigned char arg=nTotals[F];
        cmd.append(arg);
    }
    cmd.append('\n');
    
    serialConn->write(cmd);
    serialConn->flush();

    //wait for an answer
    QString res = readLineResponse();
    
    if (res == "OK.")
    {
        return 0;
    }
    if (res.length() == 0)
    {
        return -2;  //no response
    }
    return -3;  //unexpected response
}

int DeviceDriver::setData(QString pins)
{
   // pins=pins.replace("X","0");
    if (!serialConn->isOpen()) return -1;
    if (pins.length()>24) return -4; //error in inp data
    QString sBitPattern;
    sBitPattern=sBitPattern.fill('1',12-(pins.length()/2)); //set unused pins IN - left side
    sBitPattern+=pins;
   // sBitPattern=sBitPattern.leftJustified(24,'0'); //set unused pins IN - right side.
    //This mess is NOT in source, it's in protocol.
    //I think setting all to 1 helps PIC, as it has not to drain
    //current supplied by pull-ups, a bit more power goes to chip.
     sBitPattern=sBitPattern.leftJustified(24,'1');

    int nTotals[4];
    for (int i=0;i<4;i++) nTotals[i]=0;

    int nMul=128;
    for (int F=0;F<8;F++)
    {
        if (sBitPattern[F]=='1') //PORTB used entirely
        {
            nTotals[0]+=nMul;
        }
        if (sBitPattern[F+8]=='1') //PORTD used entirely
        {
            nTotals[1]+=nMul;
        }
        nMul=nMul/2;
    }

    //PORTA - we're not using PA4 here.
    //we're using PA0,1,2,3,5.
    nMul=32;
    for (int F=0;F<5;F++)
    {
       if (sBitPattern[F+16]=='1')
       {
           nTotals[2]+=nMul;
       }
       if (F==0)
       {
           nMul=nMul/2; //shift out first bit;
       }
       nMul=nMul/2;
    }

    //PORTE - 3 bits are used here.
    //PE0,1,2
    nMul=4;
    for (int F=0;F<3;F++)
    {
        if (sBitPattern[F+21]=='1')
        {
            nTotals[3]+=nMul;
        }
        nMul=nMul/2;
    }

    // NEW: Build complete command with newline terminator
    serialConn->clear();
    QByteArray cmd;
    cmd.append('3');
    for (int F=0;F<4;F++)
    {
        unsigned char arg=nTotals[F];
        cmd.append(arg);
    }
    cmd.append('\n');
    
    serialConn->write(cmd);
    serialConn->flush();

    //wait for an answer
    QString res = readLineResponse();
    
    if (res == "OK.")
    {
        return 0;
    }
    if (res.length() == 0)
    {
        return -2;  //no response
    }
    return -3;  //unexpected response
}

//MODIFIED: getData now expects line-based response
QString DeviceDriver::getData()
{
    serialConn->clear();
    QByteArray cmd;
    cmd.append('4');
    cmd.append('\n');
    serialConn->write(cmd);
    serialConn->flush();
    
    // Wait for response with timeout
    // Response format: 4 data bytes + "OK.\n"
#ifdef Q_OS_WIN
    qint64 start = QDateTime::currentDateTime().toMSecsSinceEpoch();
#else
    qint64 start = QDateTime::currentDateTime().toMSecsSinceEpoch();
#endif
    
    QByteArray res;
    while (res.length() < 7 && (QDateTime::currentDateTime().toMSecsSinceEpoch() - start < this->Timeout))
    {
        serialConn->waitForReadyRead(100);
        res.append(serialConn->readAll());
    }
    
    // Check if we got valid response (4 bytes + "OK.\n")
    if (res.length() >= 7)
    {
        // Check for OK. at position 4-6
        if (res[4] == 'O' && res[5] == 'K' && res[6] == '.')
        {
            // Parse the 4 data bytes
            unsigned char k=res[0];
            QString result=QString::number(k,2).rightJustified(8,'0');
            k=res[1];
            result += QString::number(k,2).rightJustified(8,'0');
            k=res[2];
            QString sTemp= QString::number(k,2).rightJustified(8,'0').right(6);
            result += sTemp.left(1); //A5
            result += sTemp.mid(2);
            k=res[3];
            result += QString::number(k,2).rightJustified(8,'0').right(3);
            
            return result;
        }
    }
    
    return "-3"; //error - no valid response
}

QString DeviceDriver::deviceVersion()
{
    serialConn->clear();
    QByteArray cmd;
    cmd.append('6');
    cmd.append('\n');
    serialConn->write(cmd);
    serialConn->flush();

    // Use readLine for consistent behavior
    QString version = readLineResponse();
    return version;
}
