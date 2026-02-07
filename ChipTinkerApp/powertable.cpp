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



#include "powertable.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include "testsheet.h"

//The table consists of rows of 3 values comma-separated:
// - number of switch
// - Type, 0,1,2 - 0 - links to GND, 1 - links to Vcc, 2 - not connected.
// - pin - pin linked by switch

PowerTable::PowerTable(QString tableFile)
{
    this->filePath=tableFile;
    QFile powerFile(this->filePath);
    if (!powerFile.exists())
    {
        //if file does not exist, create default config and save it
        this->createDefaultTable();
        this->saveTable();
        return;
    }
    if (powerFile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        entries.clear();
        QString line="";
        QTextStream in(&powerFile);
        while (!in.atEnd())
        {
            line=in.readLine().trimmed();
            if ((line=="")||(line[0]=='#')) continue;
            powerEntry tmp;
            tmp.number=line.split(",")[0].toInt();
            tmp.type=line.split(",")[1].toInt();
            tmp.pin=line.split(",")[2].toInt();
            entries.push_back(tmp);
        }
    }
    //if exists, read it

}

//creates default hardware table
//This is from schematic
void PowerTable::createDefaultTable()
{
    powerEntry tmp;
    tmp.number=1;
    tmp.type=1;
    tmp.pin=22;
    entries.push_back(tmp);
    tmp.number=2;
    tmp.type=1;
    tmp.pin=20;
    entries.push_back(tmp);
    tmp.number=3;
    tmp.type=1;
    tmp.pin=19;
    entries.push_back(tmp);
    tmp.number=4;
    tmp.type=0;
    tmp.pin=12;
    entries.push_back(tmp);
}

//gets entries count - to use for example in DIP sw visualization
int PowerTable::getCount()
{
    return this->entries.count();
}

//save table to file
int PowerTable::saveTable()
{
    char cr=0x0D;
    char lf=0x0A;   //keeps Windows standard for file
    QFile outFile(filePath);
    outFile.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream out(&outFile);
    for (int i=0;i<this->entries.count();i++)
    {
        out<<QString::number(this->entries[i].number)<<","<<QString::number(this->entries[i].type);
        out<<","<<QString::number(this->entries[i].pin)<<cr<<lf;
    }
    outFile.close();
    return 0;
}

//This thing tests model. It throws a string in format:
//xxxx,yyyyyyyyyyyyyyyyyyyyyyyy,zzzzzzzzzzzzzzzzzzzzzzzz
//Where: x - is the DIP setting - 0 off 1 on;
//       y - is the pin connected to power pin - always 24
//       z - is the pin connected to ground pin - always 24
QString PowerTable::checkModel(TestSheet model)
{
    QString DIPPos="";
    for (int i=0;i<this->entries.count();i++) DIPPos+="0";
    QString VccPos="000000000000000000000000";
    QString GNDPos="000000000000000000000000";
    for (int i=0;i<model.getNumOfPins();i++)
    {
        if (model.pins[i].pinType==3)
        {
            bool foundInDIP=0;
            //GND found on pin i+1
            int lookedPin = (i+1)+(12-(model.getNumOfPins()/2)); //get socket's pin from device pin
            for (int j=0;j<this->entries.count();j++) //check switches
            {
                if ((this->entries[j].type==0)&&(this->entries[j].pin==lookedPin))
                {
                    DIPPos[j]='1';
                    foundInDIP=1;

                }
            }
            //if foundInDip==0 then install the proper link in GNDPos
            if (!foundInDIP)
            {
                GNDPos[i]='1';
            }
        }
        if (model.pins[i].pinType==4)
        {
            bool foundInDIP=0;
            //Vcc found on pin i+1
            int lookedPin = (i+1)+(12-(model.getNumOfPins()/2)); //get socket's pin from device pin
            for (int j=0;j<this->entries.count();j++) //check switches
            {
                if ((this->entries[j].type==1)&&(this->entries[j].pin==lookedPin))
                {
                    DIPPos[j]='1';
                    foundInDIP=1;
                }

            }                //if foundInDIP==0 then install the proper link in VccPos
            if (!foundInDIP)
            {
                VccPos[i]='1';
            }
        }
    }
    return(DIPPos+","+VccPos+","+GNDPos);
}

int PowerTable::getEntryFunc(int i)
{
    return this->entries[i].type;
}

int PowerTable::getEntryPin(int i)
{
    return this->entries[i].pin;
}

bool PowerTable::setEntry(powerEntry ent, int i)
{
    if (i>this->entries.count()+1) return 0;
    if (i>this->entries.count()) this->entries.push_back(ent);
    else this->entries[i]=ent;
    return 1;
}

void PowerTable::revertToDefault()
{
    this->entries.clear();
    this->createDefaultTable();
    return;
}

void PowerTable::resize(int len)
{
    if (len==this->entries.count()) return;
    if (len<this->entries.count())
    {
        while (this->entries.count()!=len)
            this->entries.remove(this->entries.count()-1);
        return;
    }
    if (len>this->entries.count())
    {
        while (this->entries.count()!=len)
        {
            powerEntry ent;
            ent.number=this->entries.count()+1;
            ent.pin=1;
            ent.type=2;
            entries.push_back(ent);
        }
        return;
    }
}
