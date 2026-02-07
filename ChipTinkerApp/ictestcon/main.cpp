//Copyright 2014,2015..2019 MCbx, All rights reserved.
//http://oldcomputer.info/software/ictester/index.html
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

#include <QCoreApplication>
#include "../devicedriver.h"
#include "../testsheet.h"
#include "../powertable.h"
#include <QDebug>
#include <QDir>
#include <QIODevice>
#include <QTextStream>
#include "signal.h"

DeviceDriver * tester;

void help()
{

            qDebug()<<"USAGE:\n ictestcon ttySx x circuit.mod";
            qDebug()<<"  ttySX - serial port name, or port:baudrate:timeout (e.g. COM1:57600)";
            qDebug()<<"        By default it goes with 19200baud, 1000ms. Timeout is optional.";
            qDebug()<<"   circuit.mod - test sheet file";
            qDebug()<<"x - option:";
            qDebug()<<"   t - normal test, interactive.";
            qDebug()<<"   l - loop test until ctrl-c is pressed or error happens.";
            qDebug()<<"   n - one non-interactive pass - WARNING: Configure DIP switches first!";
            qDebug()<<"   r - reset the device (useful if you've hit Ctrl-C with voltage on)";
            qDebug()<<"   c - Interactive bit-by-bit low-level interface test.";
            return;
}

void fail_gracefully(int k)
{
    qDebug()<<"Aborted. Power off...";
    tester->reset();
    tester->reset();
    tester->reset();
    exit(0);
    return;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, fail_gracefully);

    QCoreApplication a(argc, argv);
    QTextStream cin(stdin);

    qDebug()<<"IC Tester tool";
    qDebug()<<"v. 0.04       MCbx, 2019";

    if (argc<3)
    {
        help();
        return 0;
    }

    int baudrate=QSerialPort::Baud19200;
    QString qsport=argv[1];
    int timeout=1000;
    if (qsport.contains(':'))
    {
        QStringList aa=qsport.split(':');
        qsport=aa.at(0);
        baudrate=aa.at(1).toInt();
        if (aa.length()>2)
        {
            timeout=aa.at(2).toInt();
        }
    }
    qDebug()<<"Port:     "<<qPrintable(qsport);
    qDebug()<<"Baudrate: "<<qPrintable(QString::number(baudrate));
    qDebug()<<"Timeout:  "<<qPrintable(QString::number(timeout));
    //any time, we have to open connection with a device
   // DeviceDriver tester (qsport,baudrate,timeout);
    tester = new DeviceDriver(qsport,baudrate,timeout);
    //test is device OK:
    int k = tester->reset();
    if (k<0)
    {
        qDebug()<<"ERROR: Failed to connect with device!\n";
        help();
        return 0;
    }
    qDebug()<<"Identify Device:"<<tester->deviceVersion().replace(QChar(0x0a),' ').replace(QChar(0x0d),' ').trimmed();
    tester->reset();



    if (argv[2][0]=='r') return 0; //we've already reset a device.
    if (argv[2][0]=='c')                    //hardware check
    {
        qDebug()<<"Interactive test. This test will turn each line HIGH while keeping";
        qDebug()<<"all other lines LOW. Please remove IC from test socket, turn all DIP";
        qDebug()<<"switches OFF and disconnect any power wires.";
        qDebug()<<"Press Return to turn voltage ON";
        cin.readLine();
        if (tester->powerON()==0) qDebug()<<"Voltage turned ON";
        else qDebug()<<"Error while setting voltage on.";
        QString k;
        if (tester->setIO("000000000000000000000000")==0) qDebug()<<"Line o/p set OK";
        else qDebug()<<"There was a problem setting all lines to outputs";
        int currentLine=0;
        while (1)
        {
            qDebug()<<"Line"<<qPrintable(QString::number(currentLine))<<"active.";
            qDebug()<<"Interactive test. Return for next, b then Return for Previous. a to set all HIGH, x Return to exit";
            k=cin.readLine().toLower();
            if (k[0]=='x')
            {
                tester->reset();
                return 0;
            }
            if ((k[0]=='b')&&(currentLine>0)) currentLine--;
            if ((k=="")&&(currentLine<24)) currentLine++;
            if (k[0]=='a')
            {
                if (tester->setData("111111111111111111111111")==0) qDebug()<<"Line set OK";
                else qDebug()<<"There was a problem setting line.";
                qDebug()<<"Press Return to return to normal mode";
                cin.readLine();
            }
            //Now introduce data
            QString inp="000000000000000000000000";
            if (currentLine>0)
            {
                inp="";
                for (int ii=1;ii<25;ii++)
                {
                    if (currentLine!=ii) inp+="0";
                    else inp+="1";
                }
            }
            if (tester->setData(inp)==0) qDebug()<<"Line set OK";
            else qDebug()<<"There was a problem setting line.";

        }
    return 0;
    }

    //IC checking
    if ((argv[2][0]=='t')||(argv[2][0]=='l')||(argv[2][0]=='n'))
    {
        //load sheet
        if (!QFile(argv[3]).exists())
        {
            qDebug()<<"File "<<qPrintable(argv[3])<<" seems not to exist.";
            return 0;
        }
        TestSheet testScene(argv[3]);

        //load table
        QString powerTableFile = "icpower.rc";
        #ifndef Q_OS_WIN32

        QFileInfo newfile1(QDir::homePath()+"/.config/.icpower.rc");
        QFileInfo oldfile1(QDir::homePath()+"/.icpower.rc");
        if ((!newfile1.exists())&&(oldfile1.exists()))
        {
            qDebug()<<"Updating power cfg file from previous version";
            QFile::copy(QDir::homePath()+"/.icpower.rc", QDir::homePath()+"/.config/.icpower.rc");
        }


        powerTableFile = QDir::homePath()+"/.config/.icpower.rc";
        #endif
        PowerTable pLUT(powerTableFile);

        //display info
        qDebug()<<"IC Type:"<<qPrintable(testScene.getName());
        qDebug()<<"IC Description:"<<qPrintable(testScene.getDescription());
        qDebug()<<"Diag steps:"<<qPrintable(QString::number(testScene.script.count()));

        //Now we have to display power information:
        qDebug()<<"\nPOWER CONFIGURATION:";
        QString powercfg=pLUT.checkModel(testScene);
        QString powerPart=powercfg.split(',')[0];
        QString powerLine="";
        if (!powerPart.contains('1')) qDebug()<<"No DIP switches on.";
        else
        {
            for (int i=0;i<powerPart.length();i++)
            {
                if (powerPart[i]=='1')
                        powerLine+=QString::number(i+1)+" ";
            }
        qDebug()<<"DIP switches ON:"<<qPrintable(powerLine);
        }
        powerLine="";
        powerPart=powercfg.split(',')[1];
        if (powerPart.contains('1'))
        {
            for (int i=0;i<powerPart.length();i++)
            {
                if (powerPart[i]=='1')
                        powerLine+=QString::number(i+1)+" ";
            }
        qDebug()<<"Connect the following pins to Vcc:"<<qPrintable(powerLine);
        }
        powerLine="";
        powerPart=powercfg.split(',')[2];
        if (powerPart.contains('1'))
        {
            for (int i=0;i<powerPart.length();i++)
            {
                if (powerPart[i]=='1')
                        powerLine+=QString::number(i+1)+" ";
            }
        qDebug()<<"Connect the following pins to GND:"<<qPrintable(powerLine);
        }

        //if it's normal mode, wait for user.
        if (argv[2][0]!='n')
        {
        qDebug()<<"Press Return to start test, Ctrl-C aborts";
        cin.readLine();
        }
        //infinite loop for test
        QString diagLine;
        int iterNo=1;
        while (1)
        {
            qDebug()<<"Iteration"<<qPrintable(QString::number(iterNo));
            diagLine="0000 Init Device reset... ";
            if (tester->reset()==0) diagLine+="OK";
            else
            {
                qDebug()<<"FAILED!";
                return 1;
            }
            qDebug()<<qPrintable(diagLine);
            diagLine="0000 Init Device IO setup... ";
            if (tester->setIO(testScene.initIO())==0) diagLine+="OK";
            else
            {
                qDebug()<<"FAILED!";
                return 1;
            }
            qDebug()<<qPrintable(diagLine);

            //Execute script. After each reset there must be InitIO.
            QString lastIOState = testScene.initIO();
            int delta=0;
            for (int i=0;i<testScene.script.count();i++)
            {
                tester->qSleep(20);  //levels MUST settle down, if this is not present long tests will fail. 20 is safe.
                if (testScene.script[i].cmd==0) //Reset and configure IO
                {
                    diagLine=QString::number(i+1-delta).rightJustified(4,'0');
                    diagLine+=" Reset... ";
                    if (tester->reset()==0) diagLine+="OK, Config "+lastIOState+"...";
                    else
                    {
                        qDebug()<<"FAILED while resetting!";
                        return 1;
                    }
                    if (tester->setIO(lastIOState)==0) diagLine+="OK";
                    else
                    {
                        qDebug()<<"FAILED! while configuring";
                        tester->reset(); //try to shut voltage down if turned on
                        return 1;
                    }
                    qDebug()<<qPrintable(diagLine);
                }
                if (testScene.script[i].cmd==1) //Power on
                {
                    diagLine=QString::number(i+1-delta).rightJustified(4,'0');
                    diagLine+=" Voltage ON... ";
                    if (tester->powerON()==0) diagLine+="OK";
                    else
                    {
                        qDebug()<<"FAILED while setting voltage!";
                        tester->reset(); //try to shut voltage down if turned on
                        return 1;
                    }
                    qDebug()<<qPrintable(diagLine);
                }
                if (testScene.script[i].cmd==2) //configure IO
                {
                    diagLine=QString::number(i+1-delta).rightJustified(4,'0');
                    diagLine+=" Configure: "+testScene.script[i].arg+" ";
                    if (tester->setIO(testScene.script[i].arg)==0) diagLine+="OK";
                    else
                    {
                        qDebug()<<"FAILED while setting I/O to"<<qPrintable(testScene.script[i].arg);
                        tester->reset();
                        return 1;
                    }
                    lastIOState=testScene.script[i].arg;
                    qDebug()<<qPrintable(diagLine);
                }
                if (testScene.script[i].cmd==3) //set data
                {
                    diagLine=QString::number(i+1-delta).rightJustified(4,'0');
                    diagLine+=" Send: "+testScene.script[i].arg+" ";
                    if (tester->setData(testScene.script[i].arg)==0) diagLine+="OK";
                    else
                    {
                        qDebug()<<"FAILED while sending "<<qPrintable(testScene.script[i].arg);
                        tester->reset();
                        return 1;
                    }
                    qDebug()<<qPrintable(diagLine);
                }
                if (testScene.script[i].cmd==4) //get data and compare
                {
                    //read and compare
                    diagLine=QString::number(i+1-delta).rightJustified(4,'0');
                    diagLine+=" Read, expect: "+testScene.script[i].arg+" ";
                    QString response=tester->getData();
                    if (response=="-3")
                    {
                        qDebug()<<"FAILED, Device responded differently. ";
                        tester->reset();
                        return 1;
                    }
                     response=response.left(testScene.getNumOfPins()+((24-testScene.getNumOfPins())/2));
                     response=response.right(testScene.getNumOfPins());
                     qDebug()<<qPrintable(diagLine);
                     diagLine=QString::number(i+1-delta).rightJustified(4,'0');
                     diagLine+="          Got: "+response;
                     QString comparing=testScene.compareStep(i+1,response);
                     if (comparing=="") diagLine+=" Compare OK";
                     qDebug()<<qPrintable(diagLine);
                     if (comparing!="")
                     {
                         qDebug()<<"Compare error.Bad:"<<qPrintable(comparing);
                         tester->reset();
                         return 2;
                     }
                }
                if (testScene.script[i].cmd==255) //comment
                {
                    if (testScene.script[i].arg.at(testScene.script[i].arg.length()-1)==';')
                    {
                        diagLine=QString::number(i+1-delta).rightJustified(4,'0');
                        diagLine+="  INFO: "+testScene.script[i].arg.left(testScene.script[i].arg.length()-1);
                        qDebug()<<qPrintable(diagLine);
                    }
                    else
                        delta++;
                }


            }

            qDebug()<<"Voltage OFF";
            tester->reset();
            qDebug()<<"IC passed all tests.";
            if ((argv[2][0]=='t')||(argv[2][0]=='n')) return 0;  //if it's non looped-test, exit now.
            iterNo++;
        }

    }
    help();
    return 0;
}
