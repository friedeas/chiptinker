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
#include <QString>
#include <QTime>
#include <qglobal.h>
#include <cmath>


//ADAPTING THIS SCRIPT TO YOUR SRAM:
//First, fill these:
#define CHIP_NAME "6810"
//CHIP_NAME is the name of this chip.
#define MEM_BITS 8
//MEM_BITS is number of bits in chip, maximum 8
#define MEM_ADDR 7
//MEM_ADDR is number of address lines.
#define MEM_SIZE 128
//MEM_SIZE is number of words (or bytes) in chip.
#define POWER_PIN "24 (chip's 24)"
//POWER_PIN is the pin which is shown in program as power pin
#define GND_PIN "1 (chip's 1)"
//GND_PIN is the pin which is shown in program as ground pin
#define PINS_NO 24
//PINS_NO is number of pins in your IC.
#define INITIAL_IO "111111111000000000000001"
//INITIAL IO is the input/output configuration of chip pins (counted from 1) at start.
//Length of this string must be PINS_NO characters.
//0 - output data TO chip
//1 - input data FROM chip or power.
#define WRITE_SET 1
//set to 1 if it's needed to change I/O pins before write
#define WRITE_IO "100000000000000000000001"
//define writing I/O configuration here
#define READ_SET 1
//set to 1 if it's needed to change I/O pins before read
#define READ_IO "111111111000000000000001"
//define reading I/O configuration here

#define TIME_WAIT 0
//TIME_WAIT is a time in ms to wait between device accesses. Default value 0 is usually OK

void writeByteToMem(DeviceDriver * gerat, char data, int address)
{
    //CS disabled, RW on write (low).
    //Address in, data in
    //CS enable, CS disable.

    QString addr=QString::number(address,2).rightJustified(MEM_ADDR,'0'); //generate binary address
    QString dat=QString::number(data,2).right(8).rightJustified(8,'0').right(MEM_BITS);
    //dat is a data word for comparing

    QString state="0"+dat.mid(0,1)+dat.mid(1,1)+dat.mid(2,1)+dat.mid(3,1)+dat.mid(4,1)+
            dat.mid(5,1)+dat.mid(6,1)+dat.mid(7,1)+"011011"+"0"+addr.mid(6,1)+addr.mid(5,1)+
            addr.mid(4,1)+addr.mid(3,1)+addr.mid(2,1)+addr.mid(1,1)+addr.mid(0,1)+"1";
                            //          CS set in disable   R/W
    gerat->setData(state);
    gerat->qSleep(TIME_WAIT);

    state="0"+dat.mid(0,1)+dat.mid(1,1)+dat.mid(2,1)+dat.mid(3,1)+dat.mid(4,1)+
                dat.mid(5,1)+dat.mid(6,1)+dat.mid(7,1)+"100100"+"0"+addr.mid(6,1)+addr.mid(5,1)+
                addr.mid(4,1)+addr.mid(3,1)+addr.mid(2,1)+addr.mid(1,1)+addr.mid(0,1)+"1";

    gerat->setData(state); //bang CS lines down
    gerat->qSleep(TIME_WAIT);

    state="0"+dat.mid(0,1)+dat.mid(1,1)+dat.mid(2,1)+dat.mid(3,1)+dat.mid(4,1)+
                dat.mid(5,1)+dat.mid(6,1)+dat.mid(7,1)+"011011"+"0"+addr.mid(6,1)+addr.mid(5,1)+
                addr.mid(4,1)+addr.mid(3,1)+addr.mid(2,1)+addr.mid(1,1)+addr.mid(0,1)+"1";

    gerat->setData(state); //bang CS lines up
    gerat->qSleep(TIME_WAIT);

}

QString readByteFromMem(DeviceDriver * gerat, int address)
{
    //R/W in high
    //Address in
    //CS active
    //read
    //CS unactive

    gerat->qSleep(TIME_WAIT);
    QString addr=QString::number(address,2).rightJustified(MEM_ADDR,'0'); //generate binary address

    //Here you have to implement reading algorithm which will read data to res variable.
     //"011011"
    QString state=QString("100000000")+"100100"+"1"+
            addr.mid(6,1)+addr.mid(5,1)+addr.mid(4,1)+addr.mid(3,1)+addr.mid(2,1)+
            addr.mid(1,1)+addr.mid(0,1)+"1";

    gerat->setData(state); //address and CS low goes in
    gerat->qSleep(TIME_WAIT);

  //  state=QString("100000000")+"100100"+"1"+
  //         addr.mid(6,1)+addr.mid(5,1)+addr.mid(4,1)+addr.mid(3,1)+addr.mid(2,1)+
  //              addr.mid(1,1)+addr.mid(0,1)+"1";


 //   gerat->setData(state); //address and CS low goes in
 //   gerat->qSleep(TIME_WAIT);
    //reading result. Leave this "mid" as shown.
    QString res=gerat->getData().mid((12-PINS_NO/2),PINS_NO);

    //here is extracting data word from res to res :)
    res=res.mid(1,MEM_BITS);
    return res;
}

//you usually don't need to change anything in test routine itself

void writeToMem(DeviceDriver * gerat, char * data)
{
    qDebug()<<"       Writing data...";

    //Here initialize I/O string for writing. Like in INITIAL_IO defines.
    if (WRITE_SET==1)
        gerat->setIO(WRITE_IO); //initialize IO

    for (int i=0;i<MEM_SIZE;i++)
    {
        writeByteToMem(gerat,data[i],i);
    }
}

int verify(DeviceDriver * gerat, char * data)
{
    qDebug()<<"       Reading and comparing data...";

    //Here initialize I/O string for reading. Like in INITIAL_IO defines.
    if (READ_SET==1)
        gerat->setIO(READ_IO); //initialize IO


    for (int i=0;i<MEM_SIZE;i++)
    {
        QString dat=QString::number(data[i],2).right(8).rightJustified(8,'0').right(MEM_BITS);

        QString res=readByteFromMem(gerat,i);
        if (res!=dat)
        {
            qDebug()<<"      ERROR at "+QString::number(i)+" Expected: "+dat+" Got: "+res;
            return i;
        }

    }
    return -1;
}


void help()
{
            qDebug()<<"USAGE:\n "<<qPrintable(QString(CHIP_NAME)+"test ttySX");
            qDebug()<<"  ttySX - serial port name, or port:baudrate:timeout (e.g. COM1:57600)";
            qDebug()<<"        By default it goes with 19200baud, 1000ms. Timeout is optional.";
            return;
}

void fail(QString message, DeviceDriver * gerat)
{
    qDebug()<<qPrintable(message);
    gerat->reset();
    std::exit(1);
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextStream cin(stdin);

    qDebug()<<"IC Tester tool";
    qDebug()<<"v. 0.03       MCbx, 2019";
    qDebug()<<" For chip: "<<qPrintable(CHIP_NAME);

    if (argc<1)
    {
        help();
        return 0;
    }

    //QTime time = QTime::currentTime();
    //qsrand((uint)time.msec());

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
    DeviceDriver tester (qsport,baudrate,timeout);
    //test is device OK:
    int k = tester.reset();
    if (k<0)
    {
        qDebug()<<"ERROR: Failed to connect with device!\n";
        help();
        return 0;
    }
    qDebug()<<"Identify Device:"<<tester.deviceVersion().replace(QChar(0x0a),' ').replace(QChar(0x0d),' ').trimmed();
    tester.reset();


    qDebug()<<"Please connect your ground to pin "<<qPrintable(GND_PIN)<<"\n and Vcc to pin "<<qPrintable(POWER_PIN);
    qDebug()<<"Press Return to start test, Ctrl-C aborts";
    cin.readLine();

    tester.setIO(INITIAL_IO); //initialize IO
    tester.powerON();//123456789012345678

    for (int pass=1;pass<=10240;pass++)
    {
        qDebug()<<"PASS"<<qPrintable(QString::number(pass))<<"Starting zeros test...";
        char * dat = new char[MEM_SIZE];
        for (int i=0;i<MEM_SIZE;i++) dat[i]=0;
        writeToMem(&tester,dat);    //zero-blank it
        k=verify(&tester,dat);
        if (k==-1)
            qDebug()<<"Completed successfully.";
        else
            fail("There was an error.",&tester);

        qDebug()<<"PASS"<<qPrintable(QString::number(pass))<<"Address: Walking bit 1 test, ones...";
        for (int i=1;i<MEM_SIZE;i=i*2)
        {
            if (WRITE_SET==1)
                tester.setIO(WRITE_IO);
            writeByteToMem(&tester,255,i);
            qDebug()<<"       "<<qPrintable(QString::number(i,2).rightJustified(MEM_ADDR,'0',1));
            if (READ_SET==1)
                tester.setIO(READ_IO);
            for (int j=1;j<MEM_SIZE;j=j*2)
            {
                QString res = readByteFromMem(&tester,MEM_SIZE-1);
                res = readByteFromMem(&tester,j);
                if ((i!=j)&&(res==QString::number(255,2).right(MEM_BITS)))
                    fail("There was an error - found byte which was not supposed to be there",&tester);
                if ((i==j)&&(res!=QString::number(255,2).right(MEM_BITS)))
                    fail("There was an error - byte written not found",&tester);
            }
            if (WRITE_SET==1)
                tester.setIO(WRITE_IO);
            writeByteToMem(&tester,0,i);
        }

        qDebug()<<"PASS"<<qPrintable(QString::number(pass))<<"Address: Walking bit 0 test, ones...";
        for (int i=1;i<MEM_SIZE;i=i*2)
        {
            int k=QString::number(~i,2).right(MEM_ADDR).toInt(NULL,2);
            if (WRITE_SET==1)
                tester.setIO(WRITE_IO);
            writeByteToMem(&tester,255,k);
            qDebug()<<"       "<<qPrintable(QString::number(k,2).rightJustified(MEM_ADDR,'0',1));
            if (READ_SET==1)
                tester.setIO(READ_IO);
            for (int j=1;j<MEM_SIZE;j=j*2)
            {
                int l=QString::number(~j,2).right(MEM_ADDR).toInt(NULL,2);
                QString res = readByteFromMem(&tester,MEM_SIZE-1); //access another cell for minimizing "echo"s
                res = readByteFromMem(&tester,l);
                if ((k!=l)&&(res==QString::number(255,2).right(MEM_BITS)))
                    fail("There was an error - found byte which was not supposed to be there",&tester);
                if ((k==l)&&(res!=QString::number(255,2).right(MEM_BITS)))
                    fail("There was an error - byte written not found",&tester);
            }
            if (WRITE_SET==1)
                tester.setIO(WRITE_IO);
            writeByteToMem(&tester,0,k);
        }


        qDebug()<<"PASS"<<qPrintable(QString::number(pass))<<"Starting ones test...";
        for (int i=0;i<MEM_SIZE;i++) dat[i]=255;
        writeToMem(&tester,dat);
        k=verify(&tester,dat);
        if (k==-1)
            qDebug()<<"Completed successfully.";
        else
            fail("There was an error.",&tester);


        qDebug()<<"PASS"<<qPrintable(QString::number(pass))<<"Address: Walking bit 1 test, zeros...";
        for (int i=1;i<MEM_SIZE;i=i*2)
        {
            if (WRITE_SET==1)
                tester.setIO(WRITE_IO);
            writeByteToMem(&tester,0,i);
            qDebug()<<"       "<<qPrintable(QString::number(i,2).rightJustified(MEM_ADDR,'0',1));
            if (READ_SET==1)
                tester.setIO(READ_IO);
            for (int j=1;j<MEM_SIZE;j=j*2)
            {
                QString res = readByteFromMem(&tester,MEM_SIZE-1);
                res = readByteFromMem(&tester,j);
                if ((i!=j)&&(res==QString::number(0,2).right(MEM_BITS).rightJustified(MEM_BITS,'0')))
                    fail("There was an error - found byte which was not supposed to be there",&tester);
                if ((i==j)&&(res!=QString::number(0,2).right(MEM_BITS).rightJustified(MEM_BITS,'0')))
                    fail("There was an error - byte written not found",&tester);
            }
            if (WRITE_SET==1)
                tester.setIO(WRITE_IO);
            writeByteToMem(&tester,255,i);
        }

        qDebug()<<"PASS"<<qPrintable(QString::number(pass))<<"Address: Walking bit 0 test, zeros...";
        for (int i=1;i<MEM_SIZE;i=i*2)
        {
            int k=QString::number(~i,2).right(MEM_ADDR).toInt(NULL,2);
            if (WRITE_SET==1)
                tester.setIO(WRITE_IO);
            writeByteToMem(&tester,0,k);
            qDebug()<<"       "<<qPrintable(QString::number(k,2).rightJustified(MEM_ADDR,'0',1));
            if (READ_SET==1)
                tester.setIO(READ_IO);
            for (int j=1;j<MEM_SIZE;j=j*2)
            {
                int l=QString::number(~j,2).right(MEM_ADDR).toInt(NULL,2);
                QString res = readByteFromMem(&tester,MEM_SIZE-1); //access another cell for minimizing "echo"s
                res = readByteFromMem(&tester,l);
                if ((k!=l)&&(res==QString::number(0,2).right(MEM_BITS).rightJustified(MEM_BITS,'0')))
                    fail("There was an error - found byte which was not supposed to be there",&tester);
                if ((k==l)&&(res!=QString::number(0,2).right(MEM_BITS).rightJustified(MEM_BITS,'0')))
                    fail("There was an error - byte written not found",&tester);
            }
            if (WRITE_SET==1)
                tester.setIO(WRITE_IO);
            writeByteToMem(&tester,255,k);
        }



        if (MEM_BITS>1)
        {
            qDebug()<<"PASS"<<qPrintable(QString::number(pass))<<"Data: Walking bit 1 test...";
            for (int i=0;i<MEM_BITS;i++)
            {
                if (WRITE_SET==1)
                    tester.setIO(WRITE_IO);
                unsigned char data=1;
                for (int k=0;k<i;k++) data=data*2;
                qDebug()<<"       "<<qPrintable(QString::number(i))<<"("<<qPrintable(QString::number(data))<<","<<QString::number(data,2).rightJustified(MEM_BITS,'0')<<")...";
                writeByteToMem(&tester,data,0);
                tester.qSleep(100);
                if (READ_SET==1)
                    tester.setIO(READ_IO);
                QString res = readByteFromMem(&tester,MEM_SIZE-1); //access another cell for minimizing "echo"s
                res = readByteFromMem(&tester,0);
                if (res.toInt(NULL,2)!=data)
                    fail("There was an error.",&tester);
            }
            qDebug()<<"Data: Walking bit 1 OK";

            qDebug()<<"PASS"<<qPrintable(QString::number(pass))<<"Data: Walking bit 0 test...";
            for (int i=0;i<MEM_BITS;i++)
            {
                if (WRITE_SET==1)
                    tester.setIO(WRITE_IO);
                unsigned char data1=1;
                for (int k=0;k<i;k++) data1=data1*2;
                unsigned char data=QString::number(~data1,2).right(MEM_BITS).toInt(NULL,2);
                qDebug()<<"       "<<qPrintable(QString::number(i))<<"("<<qPrintable(QString::number(data))<<","<<QString::number(data,2).rightJustified(MEM_BITS,'0')<<")...";
                writeByteToMem(&tester,data,0);
                tester.qSleep(100);
                if (READ_SET==1)
                    tester.setIO(READ_IO);
                QString res = readByteFromMem(&tester,MEM_SIZE-1); //access another cell for minimizing "echo"s
                res = readByteFromMem(&tester,0);
                if (res.toInt(NULL,2)!=data)
                    fail("There was an error.",&tester);
            }
            qDebug()<<"Data: Walking bit 0 OK";
        }
        else qDebug()<<"Skipping data bit walk tests - 1-bit memory.";

        for (int j=0;j<10;j++)
        {
            qDebug()<<"PASS"<<qPrintable(QString::number(pass))<<"Starting random data test no "<<qPrintable(QString::number(j+1))<<"/ 10...";
            for (int i=0;i<MEM_SIZE;i++)
                dat[i]=(char)(rand() % 256);
            writeToMem(&tester,dat);
            k=verify(&tester,dat);
            if (k==-1)
                qDebug()<<"Completed successfully.";
            else
              fail("There was an error.",&tester);
        }
        qDebug()<<"End of pass "<<qPrintable(QString::number(pass));
    }
    tester.reset();
    qDebug()<<"End of program.";
    return 0;
}
