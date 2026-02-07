//Copyright 2014,2015..2019 MCbx, All rights reserved.
//http://oldcomputer.info/software/ictester/
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

#include "romdumper.h"
#include "ui_romdumper.h"
#include <QString>
#include <QByteArray>
#include <QFile>
#include <QDebug>
#include <QFont>
#include <QFileDialog>
#include <QMessageBox>
#include "../devicedriver.h"
#include "powervisualizer.h"
#include "../powertable.h"
#include "../romlist.h"
#include "../romalgorithms.h"
#include <QTemporaryFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardItemModel>
#include <QSettings>
#include <QSet>
#include <QRandomGenerator> //Qt6 only

QByteArray buffer;
bool save;
bool running;
QString execFile;

//ROM reader GUI
//This subroutine set is for reading ROMs, PROMs, EPROMs and similar. ALPHA VERSION. TO BE CORRECTED.

ROMDumper::ROMDumper(QString port, QString pLUTPath, QString settingsPath, int baudRate, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ROMDumper)
{
//    qsrand(qrand());
    ui->setupUi(this);
    ROMDumper::setWindowTitle(tr("(E/P)ROM reader/tester"));
    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    ui->pBufferView->setFont(font);
    ui->pBufferView->setReadOnly(1);
    this->port=port;
    this->pLUTPath=pLUTPath;
    this->baudRate=baudRate;
    this->iniPath=settingsPath;

    save=0;
    running=0;

    //Get filename from settings.
    QSettings settings(this->iniPath,QSettings::IniFormat);
    settings.beginGroup("ROMDumper");
    QString fileName = settings.value("ModelsPath","").toString();
    settings.endGroup();
    if (!QFile(fileName).exists())
    {
        this->on_btnOpenIni_clicked();
        ROMDumper::setWindowTitle(QString::number(ui->cbType->count()));
        if (ui->cbType->count()==0)
        {
            ui->lbStatus->setText(tr("ROM models not specified."));
            ui->tabWidget->setEnabled(0);
            ui->btnOpenIni->setEnabled(0);
            ui->cbType->setEnabled(0);
        }
    }
    else
    {
        this->readIni(fileName);
    }

    //font override
    settings.beginGroup("GUI");
    QString a=settings.value("RomFont","").toString();
    if (a!="")
    {
        QFont j(a);
        j.fromString(a); //This must be done exactly in these two steps.
        ui->pBufferView->setFont(j);
    }
}

ROMDumper::~ROMDumper()
{
    delete ui;
}

//refreshes information from Type checkbox.
void ROMDumper::refreshType()
{
    ui->cbType->blockSignals(1);
    ui->cbType->clear();
    QStringList things=this->roms->generateListing(0).split('\n');
    for (int i=0;i<things.count();i++)
    {
        if (things[i]!="")
        {
            ui->cbType->addItem(things[i]);
        }
    }
    ui->cbType->setCurrentIndex(1);

    for (int ii=0;ii<ui->cbType->count();ii++)
    {
        if (ui->cbType->itemText(ii).at(0)=='-') //block items starting with -
        {
            qobject_cast<QStandardItemModel *>(ui->cbType->model())->item(ii)->setEnabled(0);
        }
    }
    ui->cbType->blockSignals(0);
    for (int ii=0;ii<ui->cbType->count();ii++)
    {
        if (ui->cbType->itemText(ii).at(0)!='-')
        {
           ui->cbType->setCurrentIndex(ii);
           this->on_cbType_currentIndexChanged(ui->cbType->currentText());
           break;
        }
    }
}

//reads INI file with ROM definitions
int ROMDumper::readIni(QString fileName)
{
    this->roms = new ROMList(fileName);
    //fill the window
    this->refreshType();
    return 0;
}

//updates buffer hex view
void ROMDumper::repaintBuffer()
{
    ui->pBufferView->clear();
    QString linia;
    ui->pBufferView->appendPlainText("       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
    for (int i=0;i<buffer.size();i=i+16)
    {
        linia=QString::number(i,16).rightJustified(5,'0');
        linia+=":";
        for (int k=0;k<16;k++)
        {
            linia=linia+" "+QString::number((unsigned char)buffer[k+i],16).rightJustified(2,'0');
        }
        linia+=" | ";
        for (int k=0;k<16;k++)
        {
            if ((buffer[k+i]<char(32))||(buffer[k+i]>char(126)))
                linia=linia+".";
            else
                linia=linia+buffer[k+i];
        }

        linia+"\n";
        ui->pBufferView->appendPlainText(linia);
    }
    ui->pBufferView->moveCursor(QTextCursor::Start);
}

//if Type is changed, customize buffer to it.
void ROMDumper::on_cbType_currentIndexChanged(const QString &arg1)
{
    QStringList parts=arg1.split(" ");
  //  parm="."+parts[0];
    parts=parts[1].split("x");
    buffer.resize(parts[0].right(parts[0].length()-1).toInt());
    buffer.fill(0,parts[0].right(parts[0].length()-1).toInt());
    ui->pbProgress->setMaximum(parts[0].right(parts[0].length()-1).toInt());
    repaintBuffer();
    save=0;
}

//load file to buffer.
void ROMDumper::on_btnLoad_clicked()
{
    QString fileName=QFileDialog::getOpenFileName(this,tr("Open file"),"",tr("BIN(*.BIN);;ROM (*.ROM);;All Files(*.*)"));
    if (fileName=="") return;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) return;
    QByteArray bytes = file.readAll();
    file.close();
    int max=buffer.size();
    if (bytes.length()<max) max=bytes.length();
    for (int i=0;i<max;i++)
    {
        buffer[i]=bytes[i];
    }
    repaintBuffer();
    save=1;
}

// Save buffer to file
void ROMDumper::on_btnSave_clicked()
{
    QString fileName=QFileDialog::getSaveFileName(this,tr("Save file"),"",tr("BIN(*.BIN);;ROM (*.ROM);;All Files(*.*)"));
    if (fileName=="") return;
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) return;
    file.write(buffer);
    file.close();
    save=0;
}

//Clear buffer
void ROMDumper::on_btnClear_clicked()
{
    buffer.fill(0,buffer.length());
    repaintBuffer();
    save=0;
}

//Exit, ask for saving
void ROMDumper::on_btnExit_clicked()
{
    if (save)
    {
        QMessageBox::StandardButton reply;
         reply = QMessageBox::question(this, tr("Close window?"), tr("File not saved. Close window?"),
                                       QMessageBox::Yes|QMessageBox::No);
         if (reply == QMessageBox::No) return;
    }
    this->close();
}

//Display power dialog. Returns bool has user accepted dialog.
bool ROMDumper::powerDisplayer()
{
    PowerTable pLUT(this->pLUTPath);

    //get power and gnd pin no:
    QString thing = ui->cbType->currentText().split(' ')[0];
    ROMModel currentModel = this->roms->getByName(thing);

    //evaluate PLUT
    QString DIPPos="";
    for (int i=0;i<pLUT.getCount();i++) DIPPos+="0";
    QString VccPos;
    VccPos.fill('0',currentModel.pinsNo);
    QString GNDPos;
    GNDPos.fill('0',currentModel.pinsNo);
    for (int j=0;j<currentModel.GNDPins.count();j++)
    {
        int gnd = currentModel.GNDPins[j];
        for (int i=0;i<25;i++)
        {
            if (i==gnd)
            {
                bool foundInDIP=0;
                //GND found on pin i+1
                for (int jj=0;jj<pLUT.getCount();jj++) //check switches
                {
                    if ((pLUT.getEntryFunc(jj)==0)&&(pLUT.getEntryPin(jj)==gnd+(12-(currentModel.pinsNo/2))))
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
    for (int j=0;j<currentModel.PowerPins.count();j++)
    {
        int vcc = currentModel.PowerPins[j];
        for (int i=0;i<25;i++)
        {
            if (i==vcc)
            {
                bool foundInDIP=0;
                //Vcc found on pin i+1
                for (int jj=0;jj<pLUT.getCount();jj++) //check switches
                {
                    if ((pLUT.getEntryFunc(jj)==1)&&(pLUT.getEntryPin(jj)==vcc+(12-(currentModel.pinsNo/2))))
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
    PowerVisualizer pvi(DIPPos+","+VccPos+","+GNDPos, currentModel.pinsNo, this);
    return (pvi.exec());
}

//sets controls on/off
void ROMDumper::controls(bool state)
{
    ui->btnVerify->setEnabled(state);
    ui->btnClear->setEnabled(state);
    ui->btnLoad->setEnabled(state);
    ui->btnSave->setEnabled(state);
    ui->cbType->setEnabled(state);
    ui->btnExit->setEnabled(state);
    ui->btnOpenIni->setEnabled(state);
    ui->cbUseBuffer->setEnabled(state);
    ui->sbRandomTest->setEnabled(state);
    ui->sbRepeat->setEnabled(state);
    ui->sbWaiting->setEnabled(state);
    ui->cbCycleBetweenRepeats->setEnabled(state);
    ui->cbRandomPowerCycle->setEnabled(state);
    ui->rbBackwardn->setEnabled(state);
    ui->rbForward->setEnabled(state);
    ui->rbRandom->setEnabled(state);
    ui->btnGatesTest->setEnabled(state);
    ui->btnDataTest->setEnabled(state);
    if (state==0)
    {
        ui->btnRead->setText(tr("Abort"));
        ui->pushButton->setText(tr("Abort"));
        running=1;
    }
    else
    {
        ui->btnRead->setText(tr("Read"));
        ui->pushButton->setText(tr("Start test"));
        ui->tabWidget->setTabEnabled(0,1);
        running=0;
    }
}

void ROMDumper::on_btnRead_clicked()
{
    if (running)
    {
        running=0;
        return;
    }

    if (!powerDisplayer()) return;
    ui->lbStatus->setStyleSheet("");
    ui->lbStatus->setText(tr("Preparing..."));
    QApplication::processEvents();
    QByteArray prevBuffer = buffer;  //if aborted, data is restored from it.
    QString thing = ui->cbType->currentText().split(' ')[0];
    ROMModel currentModel = this->roms->getByName(thing);

    controls(0);
    ROMAlgorithms algo(this->port,this->baudRate,&currentModel,ui->sbWaiting->value());
    int aa=algo.Initialize();
    if (aa!=0)
    {
        ui->lbStatus->setText(tr("ERROR: ")+QString::number(aa)+tr(" when accessing device."));
        ui->lbStatus->setStyleSheet("QLabel {color : red; }");
        controls(1);
        return;
    }

    algo.rewind();
    ui->pbProgress->setValue(0);
    ui->pbProgress->setMaximum(currentModel.words);
    algo.powerOn();
    ui->lbStatus->setStyleSheet("");
    ui->lbStatus->setText(tr("Reading chip..."));
    QApplication::processEvents();
    buffer.clear();

    for (int i=0;i<currentModel.words;i++)
    {
        buffer.push_back(algo.readByte());
        if (i%32==0)
        {
            ui->pbProgress->setValue(i);
            QApplication::processEvents();
        }
        if (running==0)
        {
            buffer=prevBuffer;
            ui->lbStatus->setText(tr("Operation aborted."));
            ui->lbStatus->setStyleSheet("QLabel {color : red; }");
            ui->pbProgress->setValue(0);
            controls(1);
            repaintBuffer();
            return;
        }
    }
    algo.PowerOff();
    save=1;
    controls(1);
    repaintBuffer();
    ui->pbProgress->setValue(0);
    ui->lbStatus->setStyleSheet("");
    ui->lbStatus->setText(tr("Reading OK."));
}

//verifies against buffer
int ROMDumper::verify(ROMModel rom,bool verbose=1)
{
    ROMAlgorithms algo(this->port,this->baudRate,&rom,ui->sbWaiting->value());
    int aa=algo.Initialize();
    if (aa!=0)
    {
        if (verbose)
        {
            ui->lbStatus->setText(tr("ERROR: ")+QString::number(aa)+tr(" when accessing device."));
            ui->lbStatus->setStyleSheet("QLabel {color : red; }");
        }
        controls(1);
        return -1;
    }

    algo.rewind();
    ui->pbProgress->setValue(0);
    ui->pbProgress->setMaximum(rom.words);
    algo.powerOn();
    if (verbose)
    {
        ui->lbStatus->setStyleSheet("");
        ui->lbStatus->setText(tr("Comparing chip..."));
    }
    QApplication::processEvents();

    for (int i=0;i<rom.words;i++)
    {
        unsigned char word = algo.readByte();
        if (word!=(unsigned char)buffer[i])
        {
            if (verbose)
            {
            ui->lbStatus->setText(tr("Compare error at 0x")+QString::number(i,16).rightJustified(4,'0').toUpper()+tr(" - Buffer: 0x")+
                                  QString::number((unsigned char)buffer[i],16).rightJustified(2,'0').toUpper()+
                                  " ("+QString::number((unsigned char)buffer[i],2).rightJustified(8,'0')+"), "+tr(" Chip: 0x")+
                                  QString::number(word,16).rightJustified(2,'0').toUpper()+
                                  " ("+QString::number(word,2).rightJustified(8,'0')+").");
            ui->lbStatus->setStyleSheet("QLabel {color : red; }");
            }
            ui->pbProgress->setValue(0);
            controls(1);
            return 1;
        }
        if (i%32==0)
        {
            ui->pbProgress->setValue(i);
            QApplication::processEvents();
        }
        if (running==0)
        {
            if (verbose)
            {
            ui->lbStatus->setText(tr("Comparing aborted."));
            ui->lbStatus->setStyleSheet("QLabel {color : red; }");
            }
            ui->pbProgress->setValue(0);
            controls(1);
            repaintBuffer();
            return -2;
        }

    }
    algo.PowerOff();
    ui->pbProgress->setValue(0);
    return 0;
}

//on verify button click... verify against buffer
void ROMDumper::on_btnVerify_clicked()
{
    if (!powerDisplayer()) return;

    ui->lbStatus->setStyleSheet("");
    ui->lbStatus->setText(tr("Preparing..."));
    QApplication::processEvents();

    QString thing = ui->cbType->currentText().split(' ')[0];
    ROMModel currentModel = this->roms->getByName(thing);

    controls(0);
    if (this->verify(currentModel)==0)
    {
        ui->lbStatus->setStyleSheet("");
        ui->lbStatus->setText(tr("Compare OK. Ready."));
    }
    controls(1);
    repaintBuffer();

}

void ROMDumper::on_btnOpenIni_clicked()
{
    //ask for loading
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open ROM definitions..."), QString(), tr("INI Files (*.ini);;All Files (*)"));
    if (fileName.isEmpty())
    {
        fileName=this->iniPath;
        return;
    }
    QSettings settings(this->iniPath,QSettings::IniFormat); //save new file
    settings.beginGroup("ROMDumper");
    settings.setValue("ModelsPath",fileName);
    settings.endGroup();

    this->readIni(fileName);

}


//////////////////
/// READ TESTS ///
//////////////////

// This is a multi-pass, multi-directional read test.
void ROMDumper::on_pushButton_clicked()
{

    if (running)
    {
        running=0;
        return;
    }

    if (!powerDisplayer()) return;

    ui->lbStatus->setStyleSheet("");
    ui->lbStatus->setText(tr("Preparing..."));
    QApplication::processEvents();

    QString thing = ui->cbType->currentText().split(' ')[0];
    ROMModel currentModel = this->roms->getByName(thing);

    controls(0);

    ROMAlgorithms algo(this->port,this->baudRate,&currentModel,ui->sbWaiting->value());
    int aa=algo.Initialize();
    if (aa!=0)
    {
        ui->lbStatus->setText(tr("ERROR: ")+QString::number(aa)+tr(" when accessing device."));
        ui->lbStatus->setStyleSheet("QLabel {color : red; }");
        controls(1);
        return;
    }

    algo.powerOn();

    //test repeat loop
    for (int currPass=1;currPass<=ui->sbRepeat->value();currPass++)
    {
        algo.rewind();
        ui->pbProgress->setValue(0);
        ui->pbProgress->setMaximum(currentModel.words);

        ui->lbStatus->setStyleSheet("");
        ui->lbStatus->setText(tr("Pass ")+QString::number(currPass)+"/"+QString::number(ui->sbRepeat->value())+tr(" Comparing chip..."));
        QApplication::processEvents();

        if ((ui->rbForward->isChecked())||(ui->rbBackwardn->isChecked()))
        {
        for (int i=0;i<currentModel.words;i++)
        {
                if ((ui->cbRandomPowerCycle->isChecked())&&((QRandomGenerator::global()->generate()%100)>75))
            {
                algo.PowerOff();
                algo.Initialize();
                algo.Initialize(); //avoid underruns
                algo.powerOn();
            }
            int ii=i;
            if (ui->rbBackwardn->isChecked())
            {
                ii=currentModel.words-1-i;
            }
            unsigned char word = algo.readWord(ii);
            if (word!=(unsigned char)buffer[ii])
            {
                ui->lbStatus->setText(tr("Pass ")+QString::number(currPass)+"/"+QString::number(ui->sbRepeat->value())+
                                      tr(" Error at 0x")+QString::number(ii,16).rightJustified(4,'0').toUpper()+tr(" - Buff: 0x")+
                                      QString::number((unsigned char)buffer[ii],16).rightJustified(2,'0').toUpper()+
                                      " ("+QString::number((unsigned char)buffer[ii],2).rightJustified(8,'0')+"), "+
                                      tr(" Chip: 0x")+
                                      QString::number(word,16).rightJustified(2,'0').toUpper()+
                                      " ("+QString::number(word,2).rightJustified(8,'0')+").");
                ui->lbStatus->setStyleSheet("QLabel {color : red; }");
                ui->pbProgress->setValue(0);
                controls(1);
                return;
            }
            if (i%32==0)
            {
                ui->pbProgress->setValue(i);
                QApplication::processEvents();
            }
            if (running==0)
            {
                ui->lbStatus->setText(tr("Test aborted."));
                ui->lbStatus->setStyleSheet("QLabel {color : red; }");
                ui->pbProgress->setValue(0);
                controls(1);
                return;
            }
        }
        }

        if (ui->rbRandom->isChecked())
        {
        bool allTested=0;
        QByteArray times;
        int tries=0;
        times.fill(0,currentModel.words);

        while (!allTested)
        {
            if ((ui->cbRandomPowerCycle->isChecked())&&((QRandomGenerator::global()->generate()%100)>75))
            {
                algo.PowerOff();
                algo.Initialize();
                algo.Initialize(); //avoid underruns
                algo.powerOn();
            }
            int ii=QRandomGenerator::global()->generate()%currentModel.words;
            tries++;
            unsigned char word = algo.readWord(ii);
            if (word!=(unsigned char)buffer[ii])
            {
                ui->lbStatus->setText(tr("Pass ")+QString::number(currPass)+"/"+QString::number(ui->sbRepeat->value())+
                                      tr(" Error at 0x")+QString::number(ii,16).rightJustified(4,'0').toUpper()+tr(" - Buff: 0x")+
                                      QString::number((unsigned char)buffer[ii],16).rightJustified(2,'0').toUpper()+
                                      " ("+QString::number((unsigned char)buffer[ii],2).rightJustified(8,'0')+"), "+
                                      tr(" Chip: 0x")+
                                      QString::number(word,16).rightJustified(2,'0').toUpper()+
                                      " ("+QString::number(word,2).rightJustified(8,'0')+").");
                ui->lbStatus->setStyleSheet("QLabel {color : red; }");
                ui->pbProgress->setValue(0);
                controls(1);
                return;
            }
            if ((unsigned int)times[ii]<255)       //register each try in times table
            {
                times[ii]=times[ii]+1;
            }

            int nullCount=0;  //figure out number of items smaller than given minimum
            for (int k=0;k<times.length();k++)
            {
                if ((int)times[k]<ui->sbRandomTest->value())
                {
                    nullCount++;
                }
            }
            if (nullCount==0)  //finished?
            {
                allTested=1;
            }
            if (tries%32==0)
            {
                ui->pbProgress->setValue(currentModel.words-nullCount);
                ui->lbStatus->setText(tr("Pass ")+QString::number(currPass)+"/"+QString::number(ui->sbRepeat->value())+tr(" Comparing chip...")+
                                      tr("  Words left: ")+QString::number(nullCount));

                QApplication::processEvents();
            }
            if (running==0)
            {
                ui->lbStatus->setText(tr("Test aborted."));
                ui->lbStatus->setStyleSheet("QLabel {color : red; }");
                ui->pbProgress->setValue(0);
                controls(1);
                return;
            }
        }
        }

        if (ui->cbCycleBetweenRepeats->isChecked())
        {
            algo.PowerOff();
            algo.Initialize();
            algo.Initialize();
            algo.PowerOff();
            algo.Initialize();
            algo.Initialize(); //avoid underruns
            algo.powerOn();
        }
    } //main loop
    algo.PowerOff();
    save=1;
    controls(1);
    repaintBuffer();
    ui->pbProgress->setValue(0);
    ui->lbStatus->setStyleSheet("");
    ui->lbStatus->setText(tr("Compare OK. Ready."));

}

//tests does chip react if enable line is flipped
//tests one line at time. If it reacts - chip works.
void ROMDumper::on_btnGatesTest_clicked()
{
    for (int pass=0;pass<ui->sbRepeat->value();pass++)
    {
        if (!ui->cbUseBuffer->isChecked())
        {
            this->on_btnRead_clicked();     //read chip to buffer
            if (ui->lbStatus->text().contains(QObject::tr("aborted"),Qt::CaseInsensitive))
            {
                ui->lbStatus->setText(QObject::tr("Operation aborted."));
                ui->lbStatus->setStyleSheet("");
                controls(1);
                return;
            }
        }

        controls(0);
        QString thing = ui->cbType->currentText().split(' ')[0];
        ROMModel currentModel = this->roms->getByName(thing);
        if (currentModel.algorithm!="NORMAL")
        {
            ui->lbStatus->setText(QObject::tr("Only Normal mode chips can be tested."));
            return;
        }
        int enableComboStart=currentModel.algorithmData.indexOf("ENABLE_ON="); //get enable combination segment
        enableComboStart=enableComboStart+10;
        int enableComboLength=0;
        while ((currentModel.algorithmData.at(enableComboStart+enableComboLength)=='0')||
               (currentModel.algorithmData.at(enableComboStart+enableComboLength)=='1'))
        {
            enableComboLength++;
        }
        QString enableCombo = currentModel.algorithmData.mid(enableComboStart,enableComboLength);

        //if combinations are the same, we have to change the second one too.
        //so let's get ENABLE_OFF.
        int disableComboStart=currentModel.algorithmData.indexOf("ENABLE_OFF=");
        disableComboStart=disableComboStart+11;
        QString disableCombo = currentModel.algorithmData.mid(disableComboStart,enableComboLength);


        QString pinsLookup=currentModel.algorithmData.mid(currentModel.algorithmData.indexOf("ENABLE_PINS=")+12,32);
        QStringList pins=pinsLookup.split(',');   //create pins lookup table to display bad pins


        QList<short>testsOK;
        for (int i=0;i<enableComboLength;i++)   //for each enable pin toggle bit...
        {
            ui->lbStatus->setText(QObject::tr("Line ")+QString::number(i+1)+"/"+QString::number(enableComboLength)+"...");
            QString newCombo=enableCombo;
            if (newCombo.at(i)=='0')
            {
                newCombo.replace(i,1,'1');
            }
            else
            {
                newCombo.replace(i,1,'0');
            }
            currentModel.algorithmData.replace(enableComboStart,enableComboLength,newCombo);
            if (enableCombo==disableCombo)      //if enable==disable we have to replace it too
            {
                currentModel.algorithmData.replace(disableComboStart,enableComboLength,newCombo);
            }
            QApplication::processEvents();
            controls(0);
            testsOK.push_back(this->verify(currentModel,0));  //...and verify
            if (testsOK.last()==-2)                       //handle abort
            {
                ui->lbStatus->setText(QObject::tr("Operation aborted."));
                ui->lbStatus->setStyleSheet("");
                controls(1);
                return;
            }
        }

        ui->lbStatus->setStyleSheet("");
        QString lines;
        for (int i=0;i<testsOK.count();i++)
        {
            if (testsOK[i]==0)
            {
                lines=lines+" "+pins.at(i).left(2);
            }
        }
        if (lines.length()>0)
        {
            ui->lbStatus->setText(QObject::tr("Lines with no influence on contents: ")+lines);
            ui->lbStatus->setStyleSheet("QLabel {color : olive; }");
        }
        else
        {
            ui->lbStatus->setText(QObject::tr("Chip reacts on all lines"));
            ui->lbStatus->setStyleSheet("");

        }
        controls(1);
    }
}

//tests data bit changes in incrementing addresses
void ROMDumper::on_btnDataTest_clicked()
{
    for (int pass=0;pass<ui->sbRepeat->value();pass++)
    {
        if (!ui->cbUseBuffer->isChecked())
        {
            this->on_btnRead_clicked();     //read chip to buffer
            if (ui->lbStatus->text().contains(QObject::tr("aborted"),Qt::CaseInsensitive))
            {
                ui->lbStatus->setText(QObject::tr("Operation aborted."));
                ui->lbStatus->setStyleSheet("");
                controls(1);
                return;
            }
        }
        controls(0);

        ui->lbStatus->setText(QObject::tr("Checking..."));
        ui->lbStatus->setStyleSheet("");
        QApplication::processEvents();

        //test buffer against line locking
        QString thing = ui->cbType->currentText().split(' ')[0];
        ROMModel currentModel = this->roms->getByName(thing);
        unsigned char stuckZeros=0;
        unsigned char stuckOnes=0;
        for (int i=0;i<buffer.length();i++)
        {
            stuckZeros|=buffer[i];
            stuckOnes|=~buffer[i];
        }
        stuckZeros=~stuckZeros;
        stuckOnes=~stuckOnes;
        QString sstuckZeros=QString::number(stuckZeros,2).rightJustified(8,'0');
        QString sstuckOnes=QString::number(stuckOnes,2).rightJustified(8,'0');
        QString explainationZeros="";
        QString explainationOnes="";
        for (int i=7;i>7-currentModel.bits;i--)
        {
            if (sstuckZeros.at(i)=='1')
            {
                explainationZeros+=" D"+QString::number(7-i);
            }
            if (sstuckOnes.at(i)=='1')
            {
                explainationOnes+=" D"+QString::number(7-i);
            }
        }
        ui->lbStatus->setText(QObject::tr("No data lines with unchanged content."));

        if ((explainationOnes.length()>0)||(explainationZeros.length()>0))
        {
            ui->lbStatus->setText(QObject::tr("Found constant data bits:"));
            if (explainationZeros.length()>0)
            {
                ui->lbStatus->setText(ui->lbStatus->text()+QObject::tr(" Stuck 0: ")+explainationZeros);
            }
            if (explainationOnes.length()>0)
            {
                ui->lbStatus->setText(ui->lbStatus->text()+QObject::tr(" Stuck 1: ")+explainationOnes);
            }
            ui->lbStatus->setStyleSheet("QLabel {color : olive; }");
            controls(1);
            return;
        }

        ui->lbStatus->setStyleSheet("");
        controls(1);
    }
}


/////////////////////
/// MODELS EDITOR ///
/////////////////////

//refreshes editor list panel
void ROMDumper::repaintEditor()
{
    int selected=ui->lwItems->currentRow();
    ui->lwItems->clear();
    for (int i=0;i<this->roms->roms.count();i++)
    {
        ui->lwItems->addItem(this->roms->roms[i].name);
    }
    if (selected<ui->lwItems->count())
    {
        ui->lwItems->setCurrentRow(selected);
    }

}

//If current tab changed - lock type selector when in editor mode.
void ROMDumper::on_tabWidget_currentChanged(int index)
{
    if (index==2)
    {
        ui->cbType->setEnabled(0);
        this->repaintEditor();
    }
    else
    {
        if (!ui->cbType->isEnabled())
        {
            //refresh cbType, try to preserve item selected
            QString former=ui->cbType->currentText();
            this->refreshType();
            int k=-1;
            for (int i=0;i<ui->cbType->count();i++)
            {
                if (ui->cbType->itemText(i)==former)
                {
                    k=i;
                    break;
                }
            }
            ui->cbType->setEnabled(1);
            if (k>-1)
            {
                ui->cbType->setCurrentIndex(k);
            }
        }
    }
}

void ROMDumper::on_btnUp_clicked()
{
    int current=ui->lwItems->currentRow();
    if (current<0)
    {
        return;
    }
    int target=current-1;
    if (target<0)
    {
        target=current;
    }
    this->roms->roms.move(current,target);
    ui->lwItems->setCurrentRow(target);
    this->repaintEditor();
}

void ROMDumper::on_btnDown_clicked()
{
    int current=ui->lwItems->currentRow();
    if (current<0)
    {
        return;
    }
    int target=current+1;
    if (target>this->roms->roms.count()-1)
    {
        target=current;
    }
    this->roms->roms.move(current,target);
    ui->lwItems->setCurrentRow(target);
    this->repaintEditor();
}

void ROMDumper::on_btnAdd_clicked()
{
    ROMModel aa;
    aa.name="UNTITLED";
    aa.algorithm="NORMAL";
    aa.bits=4;
    aa.words=256;
    aa.pinsNo=12;
    aa.description="";
    this->roms->roms.push_back(aa);
    this->repaintEditor();
    ui->lwItems->setCurrentRow(ui->lwItems->count()-1);
    ui->sbWaitRead->setValue(2);
    ui->sbWaitEnable->setValue(2);
}

void ROMDumper::on_btnAddComment_clicked()
{
    ROMModel aa;
    aa.name="-CATEGORY-";
    aa.algorithm="";
    aa.bits=0;
    aa.words=0;
    aa.pinsNo=-1;
    aa.description="";
    this->roms->roms.push_back(aa);
    this->repaintEditor();
    ui->lwItems->setCurrentRow(ui->lwItems->count()-1);
}

void ROMDumper::on_btnDel_clicked()
{
    int current=ui->lwItems->currentRow();
    if (current<0)
    {
        return;
    }
    this->roms->roms.removeAt(current);

    if (current>this->roms->roms.count()-1)
    {
        current=this->roms->roms.count()-1;
    }

    this->repaintEditor();
    ui->lwItems->setCurrentRow(current);
}

//save new ROM models list.
void ROMDumper::on_btnSaveAs_clicked()
{
    QString fileName=QFileDialog::getSaveFileName(this,QObject::tr("Save file"),"",QObject::tr("INI(*.INI);;All Files(*.*)"));
    if (fileName=="") return;
    this->roms->timeMark(); //include timemark
    this->roms->saveToFile(fileName);
}

//if user clicks another item, display its information in editor
void ROMDumper::on_lwItems_currentRowChanged(int currentRow)
{
    if (currentRow==-1)
    {
        ui->leDescription->setEnabled(0);
        ui->tabWidget_2->setEnabled(0);
        ui->lePower->setEnabled(0);
        ui->leGround->setEnabled(0);
        ui->sbBits->setEnabled(0);
        ui->sbWords->setEnabled(0);
        ui->sbPinCount->setEnabled(0);
        return;
    }
    ui->leName->setText(this->roms->roms[currentRow].name);
    if (this->roms->roms[currentRow].name.at(0)=='-')
    {
        ui->leDescription->setEnabled(0);
        ui->tabWidget_2->setEnabled(0);
        ui->lePower->setEnabled(0);
        ui->leGround->setEnabled(0);
        ui->sbBits->setEnabled(0);
        ui->sbWords->setEnabled(0);
        ui->sbPinCount->setEnabled(0);
    }
    else
    {
        ui->leDescription->setEnabled(1);
        ui->tabWidget_2->setEnabled(1);
        ui->lePower->setEnabled(1);
        ui->leGround->setEnabled(1);
        ui->sbBits->setEnabled(1);
        ui->sbWords->setEnabled(1);
        ui->sbPinCount->setEnabled(1);
    }
    ui->leDescription->setText(this->roms->roms[currentRow].description);
    QString tmp="";
    for (int i=0;i<this->roms->roms[currentRow].PowerPins.count();i++)
    {
        tmp=tmp+QString::number(this->roms->roms[currentRow].PowerPins[i])+",";
    }
    tmp=tmp.left(tmp.length()-1);
    ui->lePower->setText(tmp);
    tmp="";
    for (int i=0;i<this->roms->roms[currentRow].GNDPins.count();i++)
    {
        tmp=tmp+QString::number(this->roms->roms[currentRow].GNDPins[i])+",";
    }
    tmp=tmp.left(tmp.length()-1);
    ui->leGround->setText(tmp);
    ui->sbPinCount->setValue(this->roms->roms[currentRow].pinsNo);
    ui->sbBits->setValue(this->roms->roms[currentRow].bits);
    ui->sbWords->setValue(this->roms->roms[currentRow].words);
    if (this->roms->roms[currentRow].algorithm=="NORMAL")
    {
        ui->tabWidget_2->setCurrentIndex(0);

        int starter=this->roms->roms[currentRow].algorithmData.indexOf("ADDRESS=")+8;
        int len=this->roms->roms[currentRow].algorithmData.mid(starter).indexOf("\r\n");
        QString data=this->roms->roms[currentRow].algorithmData.mid(starter,len);
        QStringList tmp=data.split(',');
        data="";
        for (int i=0;i<tmp.count();i++)
        {
            data=data+tmp[i]+",";
        }
        data=data.left(data.length()-1);
        ui->leAddressPins->setText(data);

        starter=this->roms->roms[currentRow].algorithmData.indexOf("DATA=")+5;
        len=this->roms->roms[currentRow].algorithmData.mid(starter).indexOf("\r\n");
        data=this->roms->roms[currentRow].algorithmData.mid(starter,len);
        tmp=data.split(',');
        data="";
        for (int i=0;i<tmp.count();i++)
        {
            data=data+tmp[i]+",";
        }
        data=data.left(data.length()-1);
        ui->leDataPins->setText(data);

        starter=this->roms->roms[currentRow].algorithmData.indexOf("ENABLE_PINS=")+12;
        len=this->roms->roms[currentRow].algorithmData.mid(starter).indexOf("\r\n");
        data=this->roms->roms[currentRow].algorithmData.mid(starter,len);
        tmp=data.split(',');
        data="";
        for (int i=0;i<tmp.count();i++)
        {
            data=data+tmp[i]+",";
        }
        data=data.left(data.length()-1);
        ui->leEnablePins->setText(data);

        starter=this->roms->roms[currentRow].algorithmData.indexOf("ENABLE_ON=")+10;
        len=this->roms->roms[currentRow].algorithmData.mid(starter).indexOf("\r\n");
        data=this->roms->roms[currentRow].algorithmData.mid(starter,len);
        ui->leEnableOn->setText(data);

        starter=this->roms->roms[currentRow].algorithmData.indexOf("ENABLE_OFF=")+11;
        len=this->roms->roms[currentRow].algorithmData.mid(starter).indexOf("\r\n");
        data=this->roms->roms[currentRow].algorithmData.mid(starter,len);
        ui->leEnableOff->setText(data);

        starter=this->roms->roms[currentRow].algorithmData.indexOf("WAIT_ENABLE=")+12;
        len=this->roms->roms[currentRow].algorithmData.mid(starter).indexOf("\r\n");
        data=this->roms->roms[currentRow].algorithmData.mid(starter,len);
        ui->sbWaitEnable->setValue(data.toInt());


        starter=this->roms->roms[currentRow].algorithmData.indexOf("WAIT_READ=")+10;
        len=this->roms->roms[currentRow].algorithmData.mid(starter).indexOf("\r\n");
        data=this->roms->roms[currentRow].algorithmData.mid(starter,len);
        ui->sbWaitRead->setValue(data.toInt());

    }

}

//save currently editor model to currently loaded ROM list
void ROMDumper::on_btnApply_clicked()
{
    if (ui->lwItems->currentRow()==-1)
    {
        return;
    }
    int current=ui->lwItems->currentRow();
    //comment evaluation is simple
    if (!ui->sbPinCount->isEnabled())
    {
        if (ui->leName->text().mid(0,1)!="-")
        {
            ui->leName->setText("-"+ui->leName->text());
        }
        this->roms->roms[current].name=ui->leName->text();
        this->repaintEditor();
        ui->lbStatus->setText(QObject::tr("Section information applied."));
        ui->lbStatus->setStyleSheet("");
        return;
    }

    //verify ROM model data

    //This part does not use QRegExp or QRegularExpression INTENTIONALLY.
    //Both of them are volatile, unstable and have problems with portability (especially in Haiku OS)
    if (ui->leName->text().length()==0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Name should not be empty!"));
        return;
    }
    if ((ui->leName->text().contains(' '))||(ui->leName->text().at(0)=='-'))
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Name should not contain spaces nor shouldn't start with dash!"));
        return;
    }

    QStringList occupiedPins;  //stores pins occupied for checking isn't a pin double-asigned

    if (ui->leGround->text().length()==0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Ground pins list should not be empty!"));
        return;
    }
    QString verify=ui->leGround->text();
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
    QStringList qsl = ui->leGround->text().split(',');
    for (int i=0;i<qsl.count();i++)
    {
        if (qsl[i].toInt()>ui->sbPinCount->value())
        {
            QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("One or more GND pins are outside pins count!"));
            return;
        }
    }
    occupiedPins=occupiedPins+qsl;

    if (ui->lePower->text().length()==0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Power pins list should not be empty!"));
        return;
    }
    verify=ui->lePower->text();
    verify=verify.replace(",","");
    for (int i=0;i<10;i++)
    {
        verify=verify.replace(QString::number(i),"");
    }
    if (verify.length()>0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Power pins should have only comma-separated numbers!"));
        return;
    }
    qsl = ui->lePower->text().split(',');
    for (int i=0;i<qsl.count();i++)
    {
        if (qsl[i].toInt()>ui->sbPinCount->value())
        {
            QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("One or more Power pins are outside pins count!"));
            return;
        }
    }
    occupiedPins=occupiedPins+qsl;

    if (ui->leAddressPins->text().length()==0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Address pins list should not be empty!"));
        return;
    }
    verify=ui->leAddressPins->text();
    verify=verify.replace(",","");
    for (int i=0;i<10;i++)
    {
        verify=verify.replace(QString::number(i),"");
    }
    if (verify.length()>0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Address pins should have only comma-separated numbers!"));
        return;
    }
    qsl = ui->leAddressPins->text().split(',');
    for (int i=0;i<qsl.count();i++)
    {
        if (qsl[i].toInt()>ui->sbPinCount->value())
        {
            QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("One or more Address pins are outside pins count!"));
            return;
        }
    }
    occupiedPins=occupiedPins+qsl;

    if (ui->leDataPins->text().length()==0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Data pins list should not be empty!"));
        return;
    }
    verify=ui->leDataPins->text();
    verify=verify.replace(",","");
    for (int i=0;i<10;i++)
    {
        verify=verify.replace(QString::number(i),"");
    }
    if (verify.length()>0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Data pins should have only comma-separated numbers!"));
        return;
    }
    qsl = ui->leDataPins->text().split(',');
    for (int i=0;i<qsl.count();i++)
    {
        if (qsl[i].toInt()>ui->sbPinCount->value())
        {
            QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("One or more Data pins are outside pins count!"));
            return;
        }
    }
    occupiedPins=occupiedPins+qsl;

    verify=ui->leEnablePins->text();
    verify=verify.replace(",","");
    for (int i=0;i<10;i++)
    {
        verify=verify.replace(QString::number(i),"");
    }
    if (verify.length()>0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Enable pins should have only comma-separated numbers!"));
        return;
    }
    qsl = ui->leEnablePins->text().split(',');
    for (int i=0;i<qsl.count();i++)
    {
        if (qsl[i].toInt()>ui->sbPinCount->value())
        {
            QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("One or more Enable pins are outside pins count!"));
            return;
        }
    }
    int enableLength=qsl.count();  //enable sequence length
    if (qsl[0]=="")
    {
        enableLength--;
    }
    occupiedPins=occupiedPins+qsl;

    if (ui->leEnableOff->text()=="")  //if the same sequence goes
    {
        ui->leEnableOff->setText(ui->leEnableOn->text());
    }

    if ((ui->leEnableOff->text().length()!=enableLength)||(ui->leEnableOn->text().length()!=enableLength))
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Invalid enable sequences length!"));
        return;
    }

    verify=ui->leEnableOff->text()+ui->leEnableOn->text();
    verify=verify.replace("1","");
    verify=verify.replace("0","");
    if (verify.length()>0)
    {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Enable sequences should contain only 0 and 1!"));
        return;
    }

    //check for pins both in GND, Vcc, Address or Data.
 //   QSet<QString> stringSet = QSet<QString>::fromList(occupiedPins); //No, in Qt6 you MUST use the iterators Qt wanted to get rid of
    QSet<QString> stringSet(occupiedPins.begin(),occupiedPins.end()); //and if You think it works with typical iterator aritmetic, fuck you, it doesnt.
    if (stringSet.count() < occupiedPins.count()) {
        QMessageBox::information(nullptr,QObject::tr("Error"),QObject::tr("Some pins exist in more than one fields!"));
        return;
    }

    //save to ROMs
    this->roms->roms[current].name=ui->leName->text();
    this->roms->roms[current].description=ui->leDescription->text();
    this->roms->roms[current].pinsNo=ui->sbPinCount->value();
    this->roms->roms[current].GNDPins.clear();
    qsl = ui->leGround->text().split(',');
    for (int i=0;i<qsl.count();i++)
    {
        this->roms->roms[current].GNDPins.push_back(qsl[i].toInt());
    }
    this->roms->roms[current].PowerPins.clear();
    qsl = ui->lePower->text().split(',');
    for (int i=0;i<qsl.count();i++)
    {
        this->roms->roms[current].PowerPins.push_back(qsl[i].toInt());
    }
    this->roms->roms[current].bits=ui->sbBits->value();
    this->roms->roms[current].words=ui->sbWords->value();
    if (ui->tabWidget_2->currentIndex()==0)
    {
        this->roms->roms[current].algorithm="NORMAL";
        QString algorithmData="ADDRESS="+ui->leAddressPins->text()+"\r\n";
        algorithmData+="DATA="+ui->leDataPins->text()+"\r\n";
        algorithmData+="ENABLE_PINS="+ui->leEnablePins->text()+"\r\n";
        algorithmData+="ENABLE_ON="+ui->leEnableOn->text()+"\r\n";
        algorithmData+="ENABLE_OFF="+ui->leEnableOff->text()+"\r\n";
        algorithmData+="WAIT_ENABLE="+QString::number(ui->sbWaitEnable->value())+"\r\n";
        algorithmData+="WAIT_READ="+QString::number(ui->sbWaitRead->value())+"\r\n";
        this->roms->roms[current].algorithmData=algorithmData;
    }

    ui->lbStatus->setText("Section information applied.");
    ui->lbStatus->setStyleSheet("");
    this->repaintEditor();
    return;
}
