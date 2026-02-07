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

#include "identifier.h"
#include "ui_identifier.h"
#include "../devicedriver.h"
#include "powervisualizer.h"
#include "../powertable.h"
#include "../romlist.h"
#include "../romalgorithms.h"
#include <QSettings>
#include <QFileDialog>
#include <QTreeView>
#include <QDir>
#include <QMessageBox>
#include <QDirIterator>
#include <QStringList>

identifier::identifier(QString port, QString pLUTPath, QString settingsPath, int baudRate, bool warn, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::identifier)
{
    ui->setupUi(this);
    this->settingsPath=settingsPath;
    this->port=port;
    this->pLUTPath=pLUTPath;
    this->baudRate=baudRate;
    this->setWindowTitle("IC Identifier");
    if (warn)
    {
    QMessageBox::information(nullptr,QObject::tr("Warning"),QObject::tr("WARNING\nThis feature is in highly experimental stage and\n"
                                                                        "it may even DAMAGE chip (no tri-state inputs in tester). \n"
                                                                        "or tester itself. Use at your own risk and do main testing\n"
                                                                        "as quickly as possible.\n"));
    }

    //load path from settings
    QSettings settings(this->settingsPath,QSettings::IniFormat);
    settings.beginGroup("ICIdentifier");
    this->filesPath = settings.value("detectPath","").toString();
    settings.endGroup();
    ui->lePathToSheets->setText(this->filesPath);
    //if path not exist, ask for path.
    if ((!QDir(filesPath).exists())||(filesPath==""))
    {
        this->selektDir();
    }
    if (this->filesPath=="")
    {
        ui->btnNew->setEnabled(0);
        ui->btnStart->setEnabled(0);
    }

    if (ui->btnNew->isEnabled())
    {
        //Reload - click "New"
        this->on_btnNew_clicked();
    }
}

//Select directory for processing
void identifier::selektDir()
{
    QFileDialog dialog;
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    if (dialog.exec())
    {
        this->filesPath=dialog.directory().absolutePath();
        ui->lePathToSheets->setText(this->filesPath);

        QSettings settings(this->settingsPath,QSettings::IniFormat);
        settings.beginGroup("ICIdentifier");
        settings.setValue("detectPath",this->filesPath);
        settings.endGroup();
        return;
    }
    return;
}

identifier::~identifier()
{
    delete ui;
}

void identifier::on_btnNew_clicked()
{
    //INITIALIZATION
    ui->lbStatus->setText(tr("Loading deinifitons..."));
    QApplication::processEvents(); //let it paint
    ics.clear();
    int count=0;
    ui->cbPins->clear();
    QStringList pinCounts;
    QDirIterator it(this->filesPath, QStringList() << "*.mod", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        TestSheet tsh(it.next());
        ics.push_back(tsh);
        count++;
        if (!pinCounts.contains(QString::number(tsh.getNumOfPins())))
        {
            pinCounts <<QString::number(tsh.getNumOfPins());
        }
    }
    pinCounts.sort();
    ui->cbPins->addItems(pinCounts);
    ui->lbStatus->setText(tr("Loaded ")+QString::number(count)+tr(" sheets."));
    QApplication::processEvents();

    this->ics_filt=this->ics;
    //REPAINT TABLE
    this->repaintList();
    //Enable instruments
    ui->cbPins->setEnabled(1);
    this->on_cbPins_activated("");

    //END
}

//select button
void identifier::on_btnSheetPath_clicked()
{
    this->selektDir();
    this->on_btnNew_clicked();
}

void identifier::repaintList()
{
    ui->twICList->clear();
    ui->twICList->clearContents();
    ui->twICList->setRowCount(0);
    ui->twICList->setColumnCount(3);
    QStringList TableHeader;
    TableHeader <<tr("Name")<<tr("Pins")<<tr("Description");
    ui->twICList->setHorizontalHeaderLabels(TableHeader);
    ui->twICList->setColumnWidth(1,50);
    ui->twICList->setColumnWidth(2,250);
    for (int i=0;i<this->ics_filt.count();i++)
    {
        ui->twICList->insertRow(i);
        ui->twICList->setItem(i,0,new QTableWidgetItem(ics_filt[i].getName()));
        ui->twICList->setItem(i,1,new QTableWidgetItem(QString::number(ics_filt[i].getNumOfPins())));
        ui->twICList->setItem(i,2,new QTableWidgetItem(ics_filt[i].getDescription()));
        ui->twICList->resizeRowToContents(i);
    }
}

void identifier::on_cbPins_activated(const QString &arg1)
{
    this->ics_filt.clear();
    for (int i=0;i<this->ics.count();i++)
    {
        if (QString::number(this->ics[i].getNumOfPins())==ui->cbPins->currentText())
        {
            this->ics_filt.push_back(this->ics[i]);
        }
    }
    ui->leGND->setEnabled(1);
    ui->leVcc->setEnabled(1);
    ui->btnStart->setEnabled(1);
    ui->leVcc->setText(ui->cbPins->currentText());
    ui->leGND->setText(QString::number(ui->cbPins->currentText().toInt()/2));
    this->repaintList();
}

bool identifier::powerDisplayer()
{
    PowerTable pLUT(this->pLUTPath);

    //evaluate PLUT
    QString DIPPos="";
    for (int i=0;i<pLUT.getCount();i++) DIPPos+="0";
    int pinsNo=ui->cbPins->currentText().toInt();
    QString VccPos;
    VccPos.fill('0',pinsNo);
    QString GNDPos;
    GNDPos.fill('0',pinsNo);
    QStringList GNDPins=ui->leGND->text().split(',');
    QStringList PowerPins=ui->leVcc->text().split(',');
    for (int j=0;j<GNDPins.count();j++)
    {
        int gnd = GNDPins[j].toInt();
        for (int i=0;i<25;i++)
        {
            if (i==gnd)
            {
                bool foundInDIP=0;
                //GND found on pin i+1
                for (int jj=0;jj<pLUT.getCount();jj++) //check switches
                {
                    if ((pLUT.getEntryFunc(jj)==0)&&(pLUT.getEntryPin(jj)==gnd+(12-(pinsNo/2))))
                    {
                        DIPPos[jj]='1';
                        foundInDIP=1;
                    }
                }
                //if foundInDip==0 then install the proper link in GNDPos
                if (!foundInDIP)
                {
                    GNDPos[i-1]='1';
                }
            }
        }
    }
    for (int j=0;j<PowerPins.count();j++)
    {
        int vcc = PowerPins[j].toInt();
        for (int i=0;i<25;i++)
        {
            if (i==vcc)
            {
                bool foundInDIP=0;
                //Vcc found on pin i+1
                for (int jj=0;jj<pLUT.getCount();jj++) //check switches
                {
                    if ((pLUT.getEntryFunc(jj)==1)&&(pLUT.getEntryPin(jj)==vcc+(12-(pinsNo/2))))
                    {
                        DIPPos[jj]='1';
                        foundInDIP=1;
                    }

                }                //if foundInDIP==0 then install the proper link in VccPos
                if (!foundInDIP)
                {
                    VccPos[i-1]='1';
                }
            }
        }
    }
    PowerVisualizer pvi(DIPPos+","+VccPos+","+GNDPos, pinsNo, this);
    return (pvi.exec());
}


void identifier::on_btnStart_clicked()
{
    //Check power for user error
    QString verify=ui->leGND->text();
    if (verify.length()==0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("GND pins should not be empty!"));
        return;
    }
    verify=verify.replace(",","");
    for (int i=0;i<10;i++)
    {
        verify=verify.replace(QString::number(i),"");
    }
    if (verify.length()>0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("GND pins should have only comma-separated numbers!"));
        return;
    }

    verify=ui->leVcc->text();
    if (verify.length()==0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Vcc pins should not be empty!"));
        return;
    }
    verify=verify.replace(",","");
    for (int i=0;i<10;i++)
    {
        verify=verify.replace(QString::number(i),"");
    }
    if (verify.length()>0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Vcc pins should have only comma-separated numbers!"));
        return;
    }
    QStringList occupiedPins;
    QStringList qsl = ui->leGND->text().split(',');
    for (int i=0;i<qsl.count();i++)
    {
        if (qsl[i].toInt()>ui->cbPins->currentText().toInt())
        {
            QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("One or more GND pins are outside pins count!"));
            return;
        }
    }
    occupiedPins=occupiedPins+qsl;
    qsl = ui->leVcc->text().split(',');
    for (int i=0;i<qsl.count();i++)
    {
        if (qsl[i].toInt()>ui->cbPins->currentText().toInt())
        {
            QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("One or more Power pins are outside pins count!"));
            return;
        }
    }
    occupiedPins=occupiedPins+qsl;
    QSet<QString> stringSet(occupiedPins.begin(),occupiedPins.end());
    //    QSet<QString> stringSet = QSet<QString>::fromList(occupiedPins);
    if (stringSet.count() < occupiedPins.count()) {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Some pins exist in more than one fields!"));
        return;
    }

    //Check power against list
    //We have to re-generate list
    QString v=ui->leVcc->text();
    QString g=ui->leGND->text();
    this->on_cbPins_activated("");
    ui->leVcc->setText(v);
    ui->leGND->setText(g);

    for (int i=0;i<this->ics_filt.count();i++)
    {
        QStringList qsl = ui->leGND->text().split(',');
        for (int k=0;k<qsl.count();k++)
        {
            //if any element of user input is not present in GNDPins - fail chip
            char iii=(char)qsl[k].toInt();
            if (this->ics_filt[i].getGNDPins().indexOf(iii)==-1)
            {
                this->ics_filt[i].setName("XXXXXXXXXX");
            }
        }

        qsl = ui->leVcc->text().split(',');
        for (int k=0;k<qsl.count();k++)
        {
            //if any element of user input is not present in GNDPins - fail chip
            char iii=(char)qsl[k].toInt();
            if (this->ics_filt[i].getPowerPins().indexOf(iii)==-1)
            {
                this->ics_filt[i].setName("");
            }
        }
    }

    QList<TestSheet> ics_fi2=ics_filt;
    ics_filt.clear();
    for (int i=0;i<ics_fi2.count();i++)
    {
        if (ics_fi2[i].getName()!="")
        {
            ics_filt.push_back(ics_fi2[i]);
        }
    }
    this->repaintList();

    if (this->ics_filt.count()==0)
    {
        ui->lbStatus->setText(tr("No chips with pins/power criteria."));
        return;
    }

    //Power visualizer
    if (!powerDisplayer())
    {
        return;
    }


    //Burn-in
    ui->lbStatus->setText(tr("Initializing tester... "));
    DeviceDriver tester(this->port,this->baudRate,1000);
    int k=tester.reset();
    if (k!=0)
    {
        ui->lbStatus->setText(tr("Tester error ")+QString::number(k));
        return;
    }
    ics_fi2=ics_filt;
    for (int i=0;i<ics_fi2.count();i++)
    {
        k=tester.reset();
        if (k!=0)
        {
            ui->lbStatus->setText(tr("Tester error ")+QString::number(k));
            return;
        }
       ui->lbStatus->setText(tr("Test ")+ics_fi2[i].getName());
       QApplication::processEvents();
       if (!test(&tester,&ics_fi2[i]))
       {
              ics_fi2[i].setName("");
       }
       else
       {
           if (ui->cbStopOnFirst->isChecked())
           {
               ui->lbStatus->setText(tr("TEST PASSED FOR: <b>")+ics_fi2[i].getName()+"</b>");
               ics_filt.clear();
               for (int i=0;i<ics_fi2.count();i++)
               {
                   if (ics_fi2[i].getName()!="")
                   {
                       ics_filt.push_back(ics_fi2[i]);
                   }
               }
               this->repaintList();
               return;
           }
       }
    }


    ics_filt.clear();
    for (int i=0;i<ics_fi2.count();i++)
    {
        if (ics_fi2[i].getName()!="")
        {
            ics_filt.push_back(ics_fi2[i]);
        }
    }
    this->repaintList();


    //Repaint
    this->repaintList();

    //status bar refresh
    ui->lbStatus->setText(tr("Test completed. See table for passed results."));
    if (this->ics_filt.count()==0)
    {
        ui->lbStatus->setText(tr("No chips passed tests."));
        return;
    }
}

//oversimplified test routine
bool identifier::test(DeviceDriver *tester, TestSheet *sheet)
{
    tester->reset();
    tester->setIO(sheet->initIO());

    QString lastIOSetup=sheet->initIO();
    for (int i=0;i<sheet->script.count();i++)
    {
       tester->qSleep(1); //wait for levels to settle
       switch (sheet->script[i].cmd)
       {
           case 0:     //reset and restore IO
               tester->reset();
               //Set I/O:
               tester->setIO(lastIOSetup);
               break;
           case 1:     //power on
                tester->powerON();
                break;
           case 2:     //set IO
               lastIOSetup=sheet->script[i].arg;
               tester->setIO(sheet->script[i].arg);
               break;
           case 3:     //send data
               tester->setData(sheet->script[i].arg);
               break;
           case 4:     //read data and compare
               QString ret=tester->getData();
               if (ret=="-3")
               {
                    QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Got Error -3 on test! Aborting."));
                    tester->reset();
                    return 0;
               }
               ret=ret.left((24+sheet->getNumOfPins())/2);
               ret=ret.right(sheet->getNumOfPins());
               QString comp=sheet->compareStep(i+1,ret);
               if (comp!="") //handling compare error
               {
                   tester->reset();
                   return 0;
               }
               break;
       }
    }
    tester->reset();
    return 1;
}
