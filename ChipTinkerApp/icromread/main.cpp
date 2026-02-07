#include <QCoreApplication>
#include <QByteArray>
#include "../devicedriver.h"
#include "../romalgorithms.h"
#include "../romlist.h"
#include <QDebug>
#include <QString>
#include <qglobal.h>
#include <cmath>
#include <QStringList>
#include <QFile>

//IC ROM READ
// A new IC ROM reader. (2015)
// Does not need programming to have all typical capabilities.


void help()
{
    qDebug() << " Usage: ";
    qDebug() << "    icromread FILE.INI [r/s/d] port MODEL FILE.BIN";
    qDebug() << " Where: ";
    qDebug()<<"  port - serial port name, or port:baudrate (e.g. COM1:57600)";
    qDebug()<<"        By default it goes with 19200baud. Timeout is optional.";
    qDebug() << " FILE.INI - file with chip definitions.";
    qDebug() << "  d - displays contents of FILE.INI, does not need other parameters. \n";
    qDebug() << "  r - reads ROM contents from MODEL circuit to FILE.BIN ";
    qDebug() << "  s - reads ROM contents from MODEL circuit to FILE.BIN, non-interactive";
    qDebug() << "  v - verifies ROM contents from MODEL circuit against FILE.BIN. \n";
    qDebug() << "  f - as above, non-interactive way. \n";

    return;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug()<<"IC ROM/PROM/EPROM reading utility";
    qDebug()<<"v. 0.05       MCbx, 2015\n";

    if ((argc!=3)&&(argc!=6))
    {
        help();
        return 2;
    }

    QString fn = argv[1];
    QString par=argv[2];

    int baudrate=QSerialPort::Baud19200;
    QString port=argv[3];
    int timeout=1000;
    if (port.contains(':'))
    {
        QStringList aa=port.split(':');
        port=aa.at(0);
        baudrate=aa.at(1).toInt();
        if (aa.length()>2)
        {
            timeout=aa.at(2).toInt();
        }
    }
    qDebug()<<"Port:     "<<qPrintable(port);
    qDebug()<<"Baudrate: "<<qPrintable(QString::number(baudrate));
    qDebug()<<"Timeout:  "<<qPrintable(QString::number(timeout));
    //any time, we have to open connection with a device

    if (!((par=="r")||(par=="s")||(par=="d")||(par=="v")||(par=="f")))
    {
        qDebug() << "Unknown action!";
        return 2;
    }

    //try to load model list
    if (!QFile::exists(fn))
    {
        qDebug() <<"Error. Datafile does not exist.";
        return -1;
    }

    //load datafile
    ROMList roms(fn);
    if (par=="d")
    {
        qDebug() <<"Supported chips:";
        qDebug()<<qPrintable(roms.generateListing(1));
        return 0;
    }

    if ((par=="r")||(par=="s"))
    {
        if (argc!=6)
        {
            qDebug() << "Insufficient parameters.";
            return -1;
        }
        //check if model exists
        QString model=argv[4];
        ROMModel current=roms.getByName(model);
        if (current.name=="ERROR!")
        {
            qDebug() << "No such model.";
            return -2;
        }

        //power
        if (par!="s")
        {
            //display power requirements
            QString Pins="";
            for (int i=0;i<current.GNDPins.count();i++)
            {
                Pins+=QString::number(current.GNDPins[i]+(12-(current.pinsNo/2)))+", ";
            }
            Pins=Pins.left(Pins.length()-2);
            qDebug()<<" Connect GND to SOCKET's pins: "<<qPrintable(Pins);
            Pins="";
            for (int i=0;i<current.PowerPins.count();i++)
            {
                Pins+=QString::number(current.PowerPins[i]+(12-(current.pinsNo/2)))+", ";
            }
            Pins=Pins.left(Pins.length()-2);
            qDebug()<<" Connect Vcc to SOCKET's pins: "<<qPrintable(Pins);
            qDebug() <<"And press Return to read chip.\n";
            QTextStream cin(stdin);
            cin.readLine();
        }

        //read ROM
        QByteArray buf;
        ROMAlgorithms ra(port,baudrate,&current,-1);
        int s=ra.Initialize();
        if (s!=0)
        {
            qDebug() <<"Device error "<<QString::number(s);
            return -4;
        }
        ra.rewind();
        ra.powerOn();
        qDebug() <<"Reading chip";
        for (int i=0;i<current.words;i++)
        {
            unsigned char z=ra.readByte();
            buf.push_back(z);

            if ((i+1)%64==0)
            {
                qDebug()<< i+1;
            }
        }
        ra.PowerOff();

        //save file
        qDebug() << "Saving...";
        QFile file(argv[5]);
        file.open(QIODevice::WriteOnly);
        file.write(buf);
        file.close();
        qDebug()<<"Successfully saved "<<argv[5];
        return 0;
    }

    if ((par=="v")||(par=="f"))
    {

        if (argc!=6)
        {
            qDebug() << "Insufficient parameters.";
            return -1;
        }
        //check if model exists
        QString model=argv[4];
        ROMModel current=roms.getByName(model);
        if (current.name=="ERROR!")
        {
            qDebug() << "No such model.";
            return -2;
        }

        //power
        if (par!="f")
        {
            //display power requirements
            QString Pins="";
            for (int i=0;i<current.GNDPins.count();i++)
            {
                Pins+=QString::number(current.GNDPins[i]+(12-(current.pinsNo/2)))+", ";
            }
            Pins=Pins.left(Pins.length()-2);
            qDebug()<<" Connect GND to SOCKET's pins: "<<qPrintable(Pins);
            Pins="";
            for (int i=0;i<current.PowerPins.count();i++)
            {
                Pins+=QString::number(current.PowerPins[i]+(12-(current.pinsNo/2)))+", ";
            }
            Pins=Pins.left(Pins.length()-2);
            qDebug()<<" Connect Vcc to SOCKET's pins: "<<qPrintable(Pins);
            qDebug() <<"And press Return to read chip.\n";
            QTextStream cin(stdin);
            cin.readLine();
        }

        QByteArray buf;
        qDebug() <<"Reading file...";
        QFile file(argv[5]);
        file.open(QIODevice::ReadOnly);
        buf=file.readAll();
        file.close();
        qDebug()<<"Verifying...";
        ROMAlgorithms ra(port,baudrate,&current,-1);

        int s=ra.Initialize();
        if (s!=0)
        {
            qDebug() <<"Device error "<<QString::number(s);
            return -4;
        }
        ra.rewind();
        ra.powerOn();
        for (int i=0;i<current.words;i++)
        {
            unsigned char z=ra.readByte();
            if (z!=(unsigned char)buf[i])
            {
                qDebug()<<"ERROR at "<<QString::number(i)<<" Got "<<QString::number(z,16)<<" Exp "<<QString::number((unsigned char)buf[i],16);
                ra.PowerOff();
                return -10;
            }

            if ((i+1)%64==0)
            {
                qDebug()<< i+1;
            }
        }
        qDebug()<<"Successfully verified.";
        ra.PowerOff();

    }

    //    qDebug()<<"Port: "<<argv[1];


    return 0;
}
