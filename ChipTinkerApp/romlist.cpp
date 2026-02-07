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
#include "romlist.h"
#include <QFile>
#include <QStringList>
#include <QDebug>
#include <QDateTime>


////////////////////
/// CONSTRUCTION ///
////////////////////
//The file is INI, but its structure is parsed different way than in QtConfig.
//This is a starting parser. Every algorithm has its own parser for arguments.
ROMList::ROMList(QString fileName)
{
    char cr=0x0D;
    char lf=0x0A;   //keeps Windows standard for file
    this->fileName=fileName;
    QFile romFile(this->fileName);
    if (!romFile.exists())
    {
        //create file for editing, to be implemented
    }
    if (romFile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
       this->roms.clear();
       QString line="";
       QTextStream in(&romFile);
       while (!in.atEnd())
       {
            line=in.readLine().trimmed();
            if ((line=="")||(line[0]=='#')) continue;
            if (line=="[GENERAL]")      //General settings
            {
                line=in.readLine().trimmed();
                this->modDate=line.split('=')[1];
            }
            if (line.mid(0,2)=="[-")    //category
            {
                ROMModel tmp;
                tmp.name=line.mid(1,line.length()-2);
                tmp.description="";
                tmp.pinsNo=-1;
                tmp.bits=0;
                tmp.words=0;
                tmp.algorithm="";
                tmp.algorithmData="";
                this->roms.push_back(tmp);
            }
            if ((line[0]=='[')&&(line[1]!='-')) //chip definition
            {
                ROMModel tmp;
                tmp.name=line.mid(1,line.length()-2);
                for (int i=0;i<7;i++)                   //read chip parameters
                {
                    line=in.readLine().trimmed();
                    QString attr=line.split('=')[0];
                    QString arg=line.split('=')[1];
                    if (attr=="DESCRIPTION")
                    {
                        tmp.description=arg;
                    }
                    if (attr=="PINS")
                    {
                        tmp.pinsNo=arg.toInt();
                    }
                    if (attr=="BITS")
                    {
                        tmp.bits=arg.toInt();
                    }
                    if (attr=="WORDS")
                    {
                        tmp.words=arg.toInt();
                    }
                    if (attr=="METHOD")
                    {
                        tmp.algorithm=arg;
                    }
                    if (attr=="GROUND")
                    {
                        QList<short> t;
                        QStringList ppins=arg.split(',');
                        for (int pp=0;pp<ppins.count();pp++)
                        {
                            t.push_back(ppins[pp].toInt());
                        }
                        tmp.GNDPins=t;
                    }
                    if (attr=="POWER")
                    {
                        QList<short> t;
                        QStringList ppins=arg.split(',');
                        for (int pp=0;pp<ppins.count();pp++)
                        {
                            t.push_back(ppins[pp].toInt());
                        }
                        tmp.PowerPins=t;
                    }
                }
                while ((line!="")&&(!in.atEnd()))           //read algorithm commands
                {
                     line=in.readLine().trimmed();
                     tmp.algorithmData+=line+cr+lf;
                }
                this->roms.push_back(tmp);
            }
       }
   }
   romFile.close();
}

/////////////////////////
/// PUBLIC INTERFACES ///
/////////////////////////
int ROMList::saveToFile(QString fileName)
{
    char cr=0x0D;
    char lf=0x0A;   //keeps Windows standard for file

    QFile plik(fileName);
    if (!plik.open(QIODevice::WriteOnly))
    {
        return 1;
    }
    QTextStream out(&plik);

    out<<"[GENERAL]"<<cr<<lf;
    out<<"MODIFIED="<<this->modDate<<cr<<lf<<cr<<lf;

    for (int i=0;i<this->roms.count();i++)
    {
        if (this->roms[i].pinsNo<0)
        {
            out<<"["<<this->roms[i].name<<"]"<<cr<<lf;
            out<<cr<<lf;
            continue;
        }
        out<<"["<<this->roms[i].name<<"]"<<cr<<lf;
        out<<"DESCRIPTION="<<this->roms[i].description<<cr<<lf;
        out<<"PINS="<<QString::number(this->roms[i].pinsNo)<<cr<<lf;

        out<<"GROUND=";
        QString line;
        for (int ii=0;ii<this->roms[i].GNDPins.count();ii++)
        {
            line=line+QString::number(roms[i].GNDPins[ii])+",";
        }
        line=line.mid(0,line.length()-1);
        out<<line<<cr<<lf;


        out<<"POWER=";
        line="";
        for (int ii=0;ii<this->roms[i].PowerPins.count();ii++)
        {
            line=line+QString::number(roms[i].PowerPins[ii])+",";
        }
        line=line.mid(0,line.length()-1);
        out<<line<<cr<<lf;

        out<<"BITS="<<QString::number(this->roms[i].bits)<<cr<<lf;
        out<<"WORDS="<<QString::number(this->roms[i].words)<<cr<<lf;
        out<<"METHOD="<<this->roms[i].algorithm<<cr<<lf;
        out<<this->roms[i].algorithmData;

        out<<cr<<lf;
    }
    out.flush();
    plik.close();
    return 0;
}

//updates Time Mark
void ROMList::timeMark()
{
    QDateTime now = QDateTime::currentDateTime();
    this->modDate=now.toString("yyyyMMddhhmmss");
}

//generates ROM models listing to be used in combo boxes or command-line software
QString ROMList::generateListing(bool commandLine)
{
    QString result="";

    for (int i=0;i<this->roms.count();i++)
    {
        if (this->roms[i].pinsNo>0)
        {
            if (commandLine)
            {
                result+="    ";
            }
            result+=this->roms[i].name+" ("+QString::number(this->roms[i].words)+"x";
            result+=QString::number(this->roms[i].bits)+") - "+this->roms[i].description+"\n";
        }
        else
        {
            result+=this->roms[i].name+"\n";
        }
    }
    return result;
}

void ROMList::overwrite()
{
    this->saveToFile(this->fileName);
}

//get model by name
ROMModel ROMList::getByName(QString name)
{
    for (int i=0;i<this->roms.count();i++)
    {
        if (this->roms[i].name==name)
        {
            return this->roms[i];
        }
    }
    ROMModel dummy;
    dummy.name="ERROR!";
    return dummy;
}

/*
 * INI Format description
 * First section is GENERAL containing MODIFIED, date of modification.
 * like MODIFIED=20150623182301 (YYYYMMDDHHMMSS)
 * Then chip sections come in their proper order.
 * If section is starting with [-, it is taken as unselectable comment.
 * All things else - it is taken as chip model.
 * The chip model consists of at least 6 lines written as INI variables (NAME=VALUE)
 * DESCRIPTION - description of model
 * PINS - numberof pins
 * GROUND - comma-separated list of GND pins
 * POWER -  comma-separated list of Vcc pins
 * BITS  - number of bits
 * WORDS - number of words
 * METHOD - name of algorithm to read chip
 * Next, all parameters for algorithm go. Next thing is a blank line AND IS MANDATORY.
 *
 */

