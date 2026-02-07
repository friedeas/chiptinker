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

#ifndef ROMALGORITHMS_H
#define ROMALGORITHMS_H
#include <QString>
#include <QByteArray>
#include "romlist.h"
#include "devicedriver.h"
#include <QList>

class ROMAlgorithms
{
public:
    ROMAlgorithms(QString port, int baudRate, ROMModel * rom, int waitoverride);
    ~ROMAlgorithms();
    unsigned char readWord(unsigned int address); //read word at address
    unsigned char readByte(); //read byte and go next
    QByteArray readChip();  //Read complete chip, locking
    short PowerOff();  //Reset, power off
    short powerOn();  //Power ON
    short Initialize(); //Reset, prepare IO
    void rewind();  //Move to zero

    //USAGE: Turning power on and off is not automatic.
    //So, create, initialize, power on, read, power off.
    //This is intentional, to check against power outages in future releases

private:
    bool initialized;
    ROMModel * currentROM;
    DeviceDriver * tester;
    QString port;
    int baudRate;
    unsigned int currentAddress;
    QList<short> adressPins;
    QList<short> dataPins;
    QList<short> controlPins;
    int wait_read;
    int wait_enable;
    QString enable_rom;
    QString disable_rom;
};

#endif // ROMALGORITHMS_H
