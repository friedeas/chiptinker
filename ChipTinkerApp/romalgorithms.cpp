//Copyright 2015 MCbx, All rights reserved.
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
#include "romalgorithms.h"
#include <QStringList>

/////////////////////////
/// UTILITY FUNCTIONS ///
/////////////////////////

//This class is an interface between model and device.
//It allows to read chip in device using specified model.
//In its strange way it should contain all algorithms used.
//The first one is "NORMAL" which is a typical address-data line algorithm.

//reverse QString
QString rev(QString src)
{
    QByteArray ba=src.toLatin1();
    char *d=ba.data();
    std::reverse(d,d+src.length());
    return QString(d);
}

//////////////////////
///  CONSTRUCTION  ///
//////////////////////

//Create algorithm using device parameters and ROM model.
//This does not support unknown methods yet.
ROMAlgorithms::ROMAlgorithms(QString port, int baudRate, ROMModel *rom, int waitoverride=-1)
{
    this->currentROM=rom;
    this->tester= NULL;
    this->port=port;
    this->baudRate=baudRate;
    this->initialized=0;
    this->tester=new DeviceDriver(this->port,this->baudRate,3000);
    //parse algorithm's data
    if (this->currentROM->algorithm=="NORMAL")
    {
        QStringList lines=this->currentROM->algorithmData.split('\n');
        for (int i=0;i<lines.count();i++)
        {
            if (lines[i].length()<4)
            {
                continue;
            }
            QString par = lines[i].split('=')[0];
            QString arg = lines[i].split('=')[1];
            if (par=="WAIT_ENABLE")
            {
                this->wait_enable=arg.toInt();
            }
            if (par=="WAIT_READ")
            {
                this->wait_read=arg.toInt();
            }
            if (waitoverride>-1)
            {
                this->wait_enable=waitoverride;
                this->wait_read=waitoverride;
            }
            if (par=="ADDRESS")
            {
                QStringList addr = arg.split(',');
                for (int ii=0;ii<addr.count();ii++)
                {
                    this->adressPins.push_back(addr[ii].toInt());
                }
            }
            if (par=="DATA")
            {
                QStringList data = arg.split(',');
                for (int ii=0;ii<data.count();ii++)
                {
                    this->dataPins.push_back(data[ii].toInt());
                }
            }
            if (par=="ENABLE_PINS")
            {
                QStringList ena = arg.split(',');
                for(int ii=0;ii<ena.count();ii++)
                {
                    this->controlPins.push_back(ena[ii].toInt());
                }
            }
            //enable_rom and disable_rom are combinations which are sent to enable or disable ROM.
            //they are here used as templates for generating new commands
            if (par=="ENABLE_ON")
            {
                this->enable_rom="";
                this->enable_rom.fill('0',this->currentROM->pinsNo);
                for (int i=0;i<this->currentROM->PowerPins.count();i++)
                {
                    this->enable_rom.replace(this->currentROM->PowerPins[i]-1,1,'1'); //fill power
                }
                for (int i=0;i<arg.length();i++)
                {
                    if (arg.at(i)=='1')
                    {
                        this->enable_rom.replace(this->controlPins[i]-1,1,'1');
                    }
                }
            }
            if (par=="ENABLE_OFF")
            {
                this->disable_rom="";
                this->disable_rom.fill('0',this->currentROM->pinsNo);
                for (int i=0;i<this->currentROM->PowerPins.count();i++)
                {
                    this->disable_rom.replace(this->currentROM->PowerPins[i]-1,1,'1'); //fill power
                }
                for (int i=0;i<arg.length();i++)
                {
                    if (arg.at(i)=='1')
                    {
                        this->disable_rom.replace(this->controlPins[i]-1,1,'1');
                    }
                }
            }

        }
    }
}

ROMAlgorithms::~ROMAlgorithms()
{
    if (this->tester) tester->reset();
    delete tester;
}


/////////////////////////
/// PUBLIC INTERFACES ///
/////////////////////////
// WARNING: Usage is Initialize->PowerOn->Read->PowerOff.
// No automatism here except auto-initialization in some cases.
// If you initialize already initialized device it should not blow up.
// All functions below returning short return device command state.

short ROMAlgorithms::PowerOff()
{
    return tester->reset();
    this->initialized=0;
}

short ROMAlgorithms::powerOn()
{
    return tester->powerON();
}

//Function resets device, initializes I/O.
short ROMAlgorithms::Initialize()
{
    int stat=0;

    if (this->currentROM->algorithm=="NORMAL")  //for NORMAL algorithm:
    {
        stat=tester->reset(); //reset
        if (stat!=0)
        {
            return stat;
        }
        //set IO
        QString IOString="";
        IOString.fill('1',this->currentROM->pinsNo); //1 is data, power, NC
        for (int i=0;i<this->adressPins.count();i++) //we have to zero out address and control pins.
        {
            IOString.replace(this->adressPins[i]-1,1,'0');
        }
        for (int i=0;i<this->controlPins.count();i++)
        {
            IOString.replace(this->controlPins[i]-1,1,'0');
        }
        stat=tester->setIO(IOString);
        if (stat!=0)
        {
            return stat;
        }
    }
    this->initialized=1;
    return 0;
}

//buffer pointer to zero
void ROMAlgorithms::rewind()
{
    this->currentAddress=0;
}

//read single word
unsigned char ROMAlgorithms::readWord(unsigned int address)
{
    unsigned char a=0;

    if (!this->initialized)
    {
        this->Initialize();
    }

    //Reading single byte algorithm
    if (this->currentROM->algorithm=="NORMAL")
    {
        QString addr=QString::number(address,2).rightJustified(this->adressPins.count(),'0'); //generate binary address
        addr=rev(addr);
        QString order="";
        if (this->enable_rom!=this->disable_rom)  //if we need to stuff address to disabled chip
        {
            //formulate order from "disable ROM" template
            order=this->disable_rom;
            for (int i=0;i<this->adressPins.count();i++)
            {
                order.replace(this->adressPins[i]-1,1,addr.mid(i,1));
            }
            tester->setData(order); //send order
            tester->qSleep(this->wait_enable);  //wait
        }

        order=this->enable_rom;
        for (int i=0;i<this->adressPins.count();i++)
        {
            order.replace(this->adressPins[i]-1,1,addr.mid(i,1));
        }
        tester->setData(order); //send order
        tester->qSleep(this->wait_read);  //wait

        //read result
        QString res=tester->getData().mid((12-(this->currentROM->pinsNo/2)),this->currentROM->pinsNo);
        QString resdata="";

        for (int i=0;i<this->dataPins.count();i++)
        {
            resdata+=res.mid(this->dataPins[i]-1,1);
        }
        QString aa="";
        aa.fill('0',8-this->currentROM->bits);
        resdata=aa+rev(resdata);
        return resdata.toInt(NULL,2);
    }

    this->currentAddress=address;
    return a;
}

//reads one word and goes next.
unsigned char ROMAlgorithms::readByte()
{
    if (this->currentROM->algorithm=="NORMAL") //imagine chip with addressing based on counter
    {                                       //that's why it's made this way.
        unsigned char k=this->readWord(this->currentAddress);
        this->currentAddress++;
        if (this->currentAddress>(unsigned int)this->currentROM->words)
        {
            this->currentAddress=0;
        }
        return k;
    }
    return 0;
}

//Reads everything. This is a locking function that will hang program during reading
QByteArray ROMAlgorithms::readChip()
{
    QByteArray data;
    if (this->currentROM->algorithm=="NORMAL")
    {
        for (int i=0;i<this->currentROM->words;i++)
        {
            unsigned char k=this->readWord(i);
            data.push_back(k);
        }
        tester->reset();
    }

    return data;
}
