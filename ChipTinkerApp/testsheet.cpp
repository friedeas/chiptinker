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

#include "testsheet.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDir>
//#include <QMessageBox>

//Creates empty sheet
TestSheet::TestSheet(QString name, QString description, int numOfPins)
{
    this->name=name;
    this->description=description;
    this->numOfPins=numOfPins;
    pinDesignation temp;
    temp.pinTag="";
    temp.pinType=0;
    for (int i=0;i<numOfPins;i++)
        this->pins.push_back(temp);
}

//load sheet from file
TestSheet::TestSheet(QString fileName)
{
    this->pins.clear();
    this->script.clear();
    this->numOfPins=0;
    QFile inpFile(fileName);
    if (inpFile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        int lineCount=1;
        QString line="";
        QTextStream in(&inpFile);
        while (!in.atEnd())
        {
            line=in.readLine().trimmed();
       //     if ((line=="")||(line[0]=='#')) continue;
            if (line=="") continue;
            if (lineCount==1) this->name=line;
            if (lineCount==2) this->description=line;
            if (lineCount==3) this->numOfPins=line.toInt();
            if ((lineCount>5)&&(lineCount<6+this->numOfPins)) //pin identification chart
            {
                pinDesignation tmp;
                tmp.pinTag=line.split(",")[1];
                tmp.pinType=line.split(",")[2].toInt();
                this->pins.push_back(tmp);
            }
            if (lineCount>=6+this->numOfPins)
            {
                //Read script
                testCommand tmp;
                if (line.at(0)=='#')
                {
                    tmp.cmd=255;
                }
                else
                {
                    line=line.split(",")[1];
                    QString a=line.at(0);
                    tmp.cmd=a.toInt();
                }
                if (tmp.cmd>1)
                {
                    if (tmp.cmd<255) //command
                        tmp.arg=line.right(line.length()-1).left(this->numOfPins).replace('x','X');
                    else
                    {
                        tmp.arg=line.right(line.length()-1); //comments with id 255 go whole
                    }
                }
                this->script.push_back(tmp);
            }
            lineCount++;
        }
    }
    else
    {
        //This is a terrible thing for portability - if the file load is failed,
        //the number of pins goes to 0.
        this->numOfPins=0;
    }
    inpFile.close();
    this->fileName=fileName;
}

//get name
QString TestSheet::getName() const
{
    return name;
}

//set name
void TestSheet::setName(const QString &value)
{
    name = value;
    QString a=QFileInfo(this->fileName).dir().absolutePath();
    if (a[a.length()-1]!=QDir::separator()) a=a+QDir::separator();
    a=a+value+".mod";
    this->fileName=a;
}

//get description
QString TestSheet::getDescription() const
{
    return description;
}

//set description
void TestSheet::setDescription(const QString &value)
{
    description = value;
}

//get number of pins
int TestSheet::getNumOfPins() const
{
    return numOfPins;
}

//set number of pins - WARNING: This deletes the script!
//in VB software it was not, it was messing the script.
void TestSheet::setNumOfPins(int value)
{
    if (value==this->numOfPins) return;
    //this is quite more difficult to do.
    if (value>this->numOfPins)
    {
        //add pins to pins set
       pinDesignation tmp;
       tmp.pinTag="";
       tmp.pinType=0;
       for (int i=0;i<value-this->numOfPins;i++)
           pins.insert(((this->numOfPins/2)+1),tmp);
    }
    else
    {
        //remove pins from pins set
        for (int i=0;i<this->numOfPins-value;i++)
            pins.remove(((this->numOfPins/2)));
    }
    script.clear();
    numOfPins = value;
}

//returns Power pins
QByteArray TestSheet::getPowerPins()
{
    QByteArray a;
    for (int i=0;i<this->pins.count();i++)
    {
        if (this->pins[i].pinType==4)
            a.push_back(i+1);
    }
    return a;
}

//returns GND pins
QByteArray TestSheet::getGNDPins()
{
    QByteArray a;
    for (int i=0;i<this->pins.count();i++)
    {
        if (this->pins[i].pinType==3)
            a.push_back(i+1);
    }
    return a;
}

//save sheet to file
int TestSheet::saveToFile(QString fileName)
{
    char cr=0x0D;
    char lf=0x0A;   //keeps Windows standard for file
    QFile outFile(fileName);
    outFile.open(QIODevice::WriteOnly|QIODevice::Text);
    QTextStream out(&outFile);
    out<<this->name<<cr<<lf;
    out<<this->description<<cr<<lf;
    out<<" "<<QString::number(this->numOfPins)<<" "<<cr<<lf;
    if(getPowerPins().length()>0)
    {
        out<<" "<<QString::number(getPowerPins().at(0))<<" "<<cr<<lf;
    } else {
        out<<"-1"<<cr<<lf;
    }
    if (getGNDPins().length()>0)
    {
        out<<" "<<QString::number(getGNDPins().at(0))<<" "<<cr<<lf;
    }
    else
        out<<" -1"<<cr<<lf;
    for (int i=0;i<this->numOfPins;i++)
    {
        out<<QString::number(i+1)<<","<<this->pins[i].pinTag<<","<<QString::number(this->pins[i].pinType)<<cr<<lf;
    }
    int delta=0; //how many comments are out?
    for (int i=0;i<this->script.count();i++)
    {
        if (this->script[i].cmd==255)
        {
            out<<"#"<<this->script[i].arg<<cr<<lf;
            delta++;
        }
        else
        {
            out<<QString::number(i+1-delta)<<","<<QString::number(this->script[i].cmd)<<this->script[i].arg<<cr<<lf;
        }
    }

    out.flush();
    outFile.close();
    this->fileName=fileName;
    return 0;
}

//generates IO configuration from pins description
//This code should be configured after each reset of a Device.
QString TestSheet::initIO(bool visualization)
{
    QString pattern="";
    pattern=pattern.fill('1',12-(this->numOfPins/2));
    for (int i=0;i<this->numOfPins;i++)
    {
        if (this->pins[i].pinType==0) pattern+="0";
        if (this->pins[i].pinType>0)
        {
            if (!visualization) pattern+="1";
            else pattern+=QString::number(pins[i].pinType);

        }
    }
    pattern=pattern.leftJustified(24,'1');
    return pattern;
}

//performs comparation of data from readout (QString data)
//with data in selected script step. Step must have Command 4.
//returns empty str if OK. If not - 1 is where error occurred.
QString TestSheet::compareStep(int step, QString data)
{
    step--;
    if (data.length()>this->numOfPins)
    {
        //trim data down
        data=data.left(this->numOfPins+((24-this->numOfPins)/2));
        data=data.right(this->numOfPins);
    }
    if (this->script[step].arg.length()<this->numOfPins) return "2";
    QString res;
    for (int i=0;i<this->numOfPins;i++)
    {
        if (this->script[step].arg[i]==data[i])
        {
            res+="0";
        }
        else
        {
            //last chance not throwing error: is it X or NC pin?
            if ((this->script[step].arg[i]=='X')||(this->pins[i].pinType==5))
                res+="0";
            else
                res+="1";
        }
    }
    if (!res.contains('1')) res="";
    return res;
}

QStringList TestSheet::getPinsDescriptions()
{
    QStringList a;
    for (int i=0;i<this->pins.count();i++)
    {
        a.append(pins[i].pinTag);
    }
    return a;
}

QString TestSheet::getFN()
{
    return this->fileName;
}
