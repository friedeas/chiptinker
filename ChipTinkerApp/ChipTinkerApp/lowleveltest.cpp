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

#include "lowleveltest.h"
#include "ui_lowleveltest.h"
#include "../devicedriver.h"
#include <QMessageBox>


//LOW LEVEL TEST
//This is a low level test window

LowLevelTest::LowLevelTest(QString port, int baudRate, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LowLevelTest)
{
    ui->setupUi(this);
    LowLevelTest::setWindowTitle(tr("Low level interface test"));
    this->port=port;
    this->baud=baudRate;

    QList<QPushButton*> buttons = this->findChildren<QPushButton*>();
    foreach (QPushButton * p, buttons)
    {
        //There are no buttons with names shorter than 3 chars than pin buttons
        if (p->text().length()<3)
            QObject::connect(p,SIGNAL(clicked()),this,SLOT(on_pin_changed()));
    }
    this->tester=nullptr;
}

LowLevelTest::~LowLevelTest()
{
    if (this->tester) tester->reset();
    if (this->tester) delete tester;
    delete ui;
}

void LowLevelTest::on_btnConnect_clicked()
{
    this->tester=new DeviceDriver(this->port,this->baud,3000);
    if (tester->reset()!=0)
    {
        QMessageBox::information(nullptr,tr("Error"),tr("Device does not respond to RESET command"));
        delete tester;
        tester=nullptr;
        return;
    }
    if (tester->setIO("000000000000000000000000")!=0)
    {
        QMessageBox::information(nullptr,tr("Error"),tr("Device does not respond to SET I/O command"));
        delete tester;
        tester=nullptr;
        return;
    }
    this->currentSituation="000000000000000000000000";
    if (tester->setData(this->currentSituation)!=0)
    {
        QMessageBox::information(nullptr,tr("Error"),tr("Device does not respond to SET DATA command"));
        delete tester;
        tester=nullptr;
        return;
    }

    ui->cbPowerOn->setEnabled(1);
    ui->btnAllOff->setEnabled(1);
    ui->btnAllOn->setEnabled(1);
    ui->btnConnect->setEnabled(0);

    QList<QPushButton*> buttons = this->findChildren<QPushButton*>();
    foreach (QPushButton * p, buttons)
    {
        //There are no buttons with names shorter than 3 chars than pin buttons
        if (p->text().length()<3)
            p->setEnabled(1);
    }

    this->displaySituation();
}


void LowLevelTest::displaySituation()
{
    //sync currentSituation with button states;
    QList<QPushButton*> buttons = this->findChildren<QPushButton*>();
    QPalette pal=palette();
    foreach (QPushButton * p, buttons)
    {
        if (p->text().length()<3)
        {
            if (this->currentSituation[p->text().toInt()-1]=='0')
            {
                pal.setBrush(QPalette::Button,QColor::fromRgb(0,200,0));
                p->setPalette(pal);
                p->setStyleSheet("background-color: rgb(0,200,0);");
            }
            else
            {
                pal.setBrush(QPalette::Button,QColor::fromRgb(200,0,0));
                p->setPalette(pal);
                p->setStyleSheet("background-color: rgb(200,0,0);");
            }
        }

    }


    //send situation to device. Always set I/O first, as it may perish by voltage on/off.
    if (tester->setIO("000000000000000000000000")!=0)
    {
        QMessageBox::information(nullptr,tr("Error"),tr("Device does not respond to SET I/O command"));
        delete tester;
        tester=nullptr;
        return;
    }
    int k = tester->setData(this->currentSituation);
    if (k!=0)
    {
        QMessageBox::information(nullptr,tr("Error"),tr("Device does not respond to SET DATA command on argument ")+this->currentSituation+tr(" code ")+QString::number(k));
        return;
    }
}


void LowLevelTest::on_cbPowerOn_clicked(bool checked)
{
    if (tester->reset()!=0)
    {
        QMessageBox::information(nullptr,tr("Error"),tr("Device does not respond to RESET command"));
        return;
    }
    if (checked)
    {
        if (tester->powerON()!=0)
        {
            QMessageBox::information(nullptr,tr("Error"),tr("Device does not respond to POWER ON command"));
            return;
        }
    }
    this->displaySituation();
}


void LowLevelTest::on_btnAllOn_clicked()
{
    this->currentSituation="111111111111111111111111";
    this->displaySituation();
}


void LowLevelTest::on_btnAllOff_clicked()
{
    this->currentSituation="000000000000000000000000";
    this->displaySituation();
}


void LowLevelTest::on_pin_changed()
{
    //modify CurrentSituation
    QPushButton * pb = qobject_cast<QPushButton *>(sender());
    if (pb)
    {
        if (this->currentSituation[pb->text().toInt()-1]=='0')
            this->currentSituation[pb->text().toInt()-1]='1';
        else
            this->currentSituation[pb->text().toInt()-1]='0';
    }

    this->displaySituation();
}
