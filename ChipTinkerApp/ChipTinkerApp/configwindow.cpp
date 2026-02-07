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

#include "configwindow.h"
#include "ui_configwindow.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QColor>
#include <QColorDialog>
#include <QPalette>
#include <../powertable.h>
#include <QMessageBox>
#include <QFontDialog>

ConfigWindow::ConfigWindow(QString cfgFileName, QString powerFileName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigWindow)
{
    ui->setupUi(this);
    ConfigWindow::setWindowTitle(tr("Configuration"));

    //get list ov available serial ports
    QSerialPortInfo port;
    for (int i=0;i<port.availablePorts().count();i++)
    {
        ui->cbSerialPort->addItem(port.availablePorts()[i].portName());
    }

    //fill baud rates
    ui->cbBaudRate->addItem("600");
    ui->cbBaudRate->addItem("1200");
    ui->cbBaudRate->addItem("2400");
    ui->cbBaudRate->addItem("4800");
    ui->cbBaudRate->addItem("9600");
    ui->cbBaudRate->addItem("19200");
    ui->cbBaudRate->addItem("38400");
    ui->cbBaudRate->addItem("19200");
    ui->cbBaudRate->addItem("38400");
    ui->cbBaudRate->addItem("57600");

    //fill languages
    ui->cbLang->addItem("AUTO");
    QStringList fileNames = QDir(":/i18n").entryList();
    fileNames.push_back("__en_US.");
    for (int i = 0; i < fileNames.size(); ++i)
    {
        QString locale;
        locale = fileNames[i];
        locale.truncate(locale.lastIndexOf('.'));
        locale.remove(0, locale.indexOf('_') + 1);
        locale.remove(0, locale.indexOf('_') + 1);
        ui->cbLang->addItem(locale);
    }

    //load settings
    this->cfgFileName=cfgFileName;
    QSettings settings(this->cfgFileName,QSettings::IniFormat);
    settings.beginGroup("Device");
    QString readSet=settings.value("Port","COM1").toString();
    #ifndef Q_OS_WIN32
    readSet=settings.value("Port","/dev/ttyS0").toString();
    #endif

    if (ui->cbSerialPort->findText(readSet)>-1)
    {
        ui->cbSerialPort->setCurrentIndex(ui->cbSerialPort->findText(readSet));
    }
    else
    {
        ui->cbSerialPort->addItem(readSet);
        ui->cbSerialPort->setCurrentIndex(ui->cbSerialPort->count()-1);
    }

   readSet=settings.value("Rate","19200").toString();
   if (ui->cbBaudRate->findText(readSet)>-1)
   {
       ui->cbBaudRate->setCurrentIndex(ui->cbBaudRate->findText(readSet));
   }
   else
   {
       ui->cbBaudRate->addItem(readSet);
       ui->cbBaudRate->setCurrentIndex(ui->cbBaudRate->count()-1);
   }


    int intRead=settings.value("TimeOut","1000").toInt();
    ui->sbTimeout->setValue(intRead);
    settings.endGroup();

    settings.beginGroup("GUI");
    QString l_over=settings.value("Lang","AUTO").toString();
    if (ui->cbLang->findText(l_over)<0)
    {
        ui->cbLang->setCurrentIndex(0);
    }
    else
    {
        ui->cbLang->setCurrentIndex(ui->cbLang->findText(l_over));
    }

    ui->cbClearLog->setChecked(settings.value("ClearLog",1).toBool());

    QString a=settings.value("LogFont","").toString();
    if (a=="")
    {
        ui->cbLogFont->setChecked(false);
    }
    else
    {
        ui->cbLogFont->setChecked(true);
        ui->btnLogSelect->setText(a);
        QFont x(a);
        x.fromString(a);
        ui->btnLogSelect->setFont(x);
    }

    a=settings.value("RomFont","").toString();
    if (a=="")
    {
        ui->cbRomFont->setChecked(false);
    }
    else
    {
        ui->cbRomFont->setChecked(true);
        ui->btnRomSelect->setText(a);
        QFont x(a);
        x.fromString(a);
        ui->btnRomSelect->setFont(x); //the only proper way to set font from string. Allo ther skip metrics or don't return the string back.
    }
    ui->cbToolbarSize->setCurrentText(settings.value("ToolbarSize","24").toString());
    ui->cbStartStopTB->setChecked(settings.value("StartStopTB","false").toBool());
    settings.endGroup();

    settings.beginGroup("Test");
    intRead=settings.value("StepDelay","2").toInt();
    ui->sbStepDelay->setValue(intRead);
    ui->cbWarnUnstable->setChecked(settings.value("warnUnstable","true").toBool());
    settings.endGroup();

    //change background function deprecated to make programmers use style sheets.
    //Style sheets don't work. Bug unfixed since 2012.
    //This temporary hack will be fixed if Qt team will fix stylesheets for such widgets
    //See you in A.D. 2096 :)
    QPalette pal=palette();

    settings.beginGroup("ICColors");
    int r,g,b;
    r=settings.value("BkgR","230").toInt();
    g=settings.value("BkgG","230").toInt();
    b=settings.value("BkgB","230").toInt();
    pal.setBrush(QPalette::Button,QColor::fromRgb(r,g,b));
    pal.setBrush(QPalette::Base,QColor::fromRgb(r,g,b));
    ui->btnBkgr->setPalette(pal);
    ui->btnBkgr->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");


    r=settings.value("HiR","255").toInt();
    g=settings.value("HiG","65").toInt();
    b=settings.value("HiB","65").toInt();
    pal.setBrush(QPalette::Button,QColor::fromRgb(r,g,b));
    pal.setBrush(QPalette::Base,QColor::fromRgb(r,g,b));
    ui->btnHi->setPalette(pal);
    ui->btnHi->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");

    r=settings.value("LowR","0").toInt();
    g=settings.value("LowG","240").toInt();
    b=settings.value("LowB","0").toInt();
    pal.setBrush(QPalette::Button,QColor::fromRgb(r,g,b));
    pal.setBrush(QPalette::Base,QColor::fromRgb(r,g,b));
    ui->btnLo->setPalette(pal);
    ui->btnLo->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");

    r=settings.value("NoR","200").toInt();
    g=settings.value("NoG","200").toInt();
    b=settings.value("NoB","200").toInt();
    pal.setBrush(QPalette::Button,QColor::fromRgb(r,g,b));
    pal.setBrush(QPalette::Base,QColor::fromRgb(r,g,b));
    ui->btnNC->setPalette(pal);
    ui->btnNC->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");

    r=settings.value("DrawR","0").toInt();
    g=settings.value("DrawG","0").toInt();
    b=settings.value("DrawB","0").toInt();
    pal.setBrush(QPalette::Button,QColor::fromRgb(r,g,b));
    pal.setBrush(QPalette::Base,QColor::fromRgb(r,g,b));
    ui->btnDraw->setPalette(pal);
    ui->btnDraw->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");

    r=settings.value("TextR","0").toInt();
    g=settings.value("TextG","0").toInt();
    b=settings.value("TextB","0").toInt();
    pal.setBrush(QPalette::Button,QColor::fromRgb(r,g,b));
    pal.setBrush(QPalette::Base,QColor::fromRgb(r,g,b));
    ui->btnTxt->setPalette(pal);
    ui->btnTxt->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");

    r=settings.value("ErrR","0").toInt();
    g=settings.value("ErrG","0").toInt();
    b=settings.value("ErrB","0").toInt();
    pal.setBrush(QPalette::Button,QColor::fromRgb(r,g,b));
    pal.setBrush(QPalette::Base,QColor::fromRgb(r,g,b));
    ui->btnErr->setPalette(pal);
    ui->btnErr->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");


    settings.endGroup();

    //Settings of AutoFill - automatically fills pin type in editor. This is a time-saver when filling chip info.
    settings.beginGroup("AutoSettings");
    ui->cbSetAuto->setChecked( settings.value("SetAuto",1).toBool() );
    ui->lePowerAuto->setText(settings.value("PowerAuto","Vcc").toString());
    ui->leGndAuto->setText(settings.value("GndAuto","GND; Vss").toString());
    ui->leOutputAuto->setText(settings.value("OutputAuto","Q?; Q??").toString());
    ui->leNcAuto->setText(settings.value("NcAuto","NC").toString());
    settings.endGroup();

    //external software settings
    settings.beginGroup("ExternalSoftware");
    int q=settings.value("ProgramsCount",0).toInt();
    for (int i=0;i<q;i++)
    {
        QString itemName=settings.value("Name"+QString::number(i),"").toString();
        QString itemCommand=settings.value("Command"+QString::number(i),"").toString();
        this->extItems.append(itemName);
        this->extCalls.append(itemCommand);
    }
    settings.endGroup();


    //prepare power table
    QString powerFile=powerFileName;
    pLUT = new PowerTable(powerFile);

    ui->sbSwitchesCount->setValue(pLUT->getCount());

    ui->twPower->setColumnCount(2);
    QStringList labels;
    labels<<tr("Function")<<tr("Pin");
    ui->twPower->setHorizontalHeaderLabels(labels);

    ui->twExtItems->setColumnCount(2);
    QStringList labels2;
    labels2<<tr("Name")<<tr("Command");
    ui->twExtItems->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->twExtItems->setHorizontalHeaderLabels(labels2);
    ui->twExtItems->setColumnWidth(0,136);
    ui->twExtItems->setColumnWidth(1,356);

    this->repaintPower(pLUT);
    this->repaintExtTools();
}

int ConfigWindow::PowerRoleToCode(QString role)
{
    if (role=="Vcc") return 1;
    if (role=="GND") return 0;
    return 2;
}

QString ConfigWindow::PowerCodeToRole(int code)
{
    if (code==0) return "GND";
    if (code==1) return "Vcc";
    return "NC";
}

ConfigWindow::~ConfigWindow()
{
    delete ui;
}

void ConfigWindow::on_btnBkgr_clicked()
{
    QColorDialog qc(ui->btnBkgr->palette().button().color());
    if (qc.exec())
    {
            QPalette pal=palette();
            pal.setBrush(QPalette::Button,qc.selectedColor());
            ui->btnBkgr->setPalette(pal);
            int r=qc.selectedColor().red();
            int g=qc.selectedColor().green();
            int b=qc.selectedColor().blue();
            ui->btnBkgr->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");
    }
}

void ConfigWindow::on_btnDraw_clicked()
{
    QColorDialog qc(ui->btnDraw->palette().button().color());
    if (qc.exec())
    {
            QPalette pal=palette();
            pal.setBrush(QPalette::Button,qc.selectedColor());
            ui->btnDraw->setPalette(pal);
            int r=qc.selectedColor().red();
            int g=qc.selectedColor().green();
            int b=qc.selectedColor().blue();
            ui->btnDraw->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");
    }
}

void ConfigWindow::on_btnHi_clicked()
{
    QColorDialog qc(ui->btnHi->palette().button().color());
    if (qc.exec())
    {
            QPalette pal=palette();
            pal.setBrush(QPalette::Button,qc.selectedColor());
            ui->btnHi->setPalette(pal);
            int r=qc.selectedColor().red();
            int g=qc.selectedColor().green();
            int b=qc.selectedColor().blue();
            ui->btnHi->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");
    }
}

void ConfigWindow::on_btnLo_clicked()
{
    QColorDialog qc(ui->btnLo->palette().button().color());
    if (qc.exec())
    {
            QPalette pal=palette();
            pal.setBrush(QPalette::Button,qc.selectedColor());
            ui->btnLo->setPalette(pal);
            int r=qc.selectedColor().red();
            int g=qc.selectedColor().green();
            int b=qc.selectedColor().blue();
            ui->btnLo->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");
    }
}

void ConfigWindow::on_btnNC_clicked()
{
    QColorDialog qc(ui->btnNC->palette().button().color());
    if (qc.exec())
    {
            QPalette pal=palette();
            pal.setBrush(QPalette::Button,qc.selectedColor());
            ui->btnNC->setPalette(pal);
            int r=qc.selectedColor().red();
            int g=qc.selectedColor().green();
            int b=qc.selectedColor().blue();
            ui->btnNC->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");
    }
}

void ConfigWindow::on_btnTxt_clicked()
{
    QColorDialog qc(ui->btnTxt->palette().button().color());
    if (qc.exec())
    {
            QPalette pal=palette();
            pal.setBrush(QPalette::Button,qc.selectedColor());
            ui->btnTxt->setPalette(pal);
            int r=qc.selectedColor().red();
            int g=qc.selectedColor().green();
            int b=qc.selectedColor().blue();
            ui->btnTxt->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");
    }
}

void ConfigWindow::on_btnErr_clicked()
{
    QColorDialog qc(ui->btnErr->palette().button().color());
    if (qc.exec())
    {
            QPalette pal=palette();
            pal.setBrush(QPalette::Button,qc.selectedColor());
            ui->btnErr->setPalette(pal);
            int r=qc.selectedColor().red();
            int g=qc.selectedColor().green();
            int b=qc.selectedColor().blue();
            ui->btnErr->setStyleSheet("background-color: rgb("+QString::number(r)+","+QString::number(g)+","+QString::number(b)+");");
    }
}

void ConfigWindow::on_twPower_cellDoubleClicked(int row, int column)
{
    if (column==0)
    {
        PowerTable::powerEntry ent;
        ent.number=row+1;
        QSpinBox * pinSpinner = qobject_cast<QSpinBox*>(ui->twPower->cellWidget(row,1));
        ent.pin=pinSpinner->value();
        ent.type=(PowerRoleToCode(ui->twPower->item(row,column)->text())+1)%3;

        pLUT->setEntry(ent,row);
        //this->repaintPower(this->pLUT);
        ui->twPower->item(row,column)->setText(PowerCodeToRole(ent.type));
    }
}

void ConfigWindow::repaintPower(PowerTable *pLUT)
{
    QSignalMapper * spinMapper = new QSignalMapper(this);
    ui->sbSwitchesCount->setValue(pLUT->getCount());
    ui->twPower->setUpdatesEnabled(0);
    while(ui->twPower->rowCount()>0)
    ui->twPower->removeRow(0);

    for (int i=0;i<pLUT->getCount();i++)
    {
            ui->twPower->insertRow(ui->twPower->rowCount());
            QTableWidgetItem * junk= new QTableWidgetItem(PowerCodeToRole(pLUT->getEntryFunc(i)));
            junk->setFlags(junk->flags() & ~Qt::ItemIsEditable);
            ui->twPower->setItem(i,0,junk);

            QSpinBox * pinCounter= new QSpinBox(nullptr);
            pinCounter->setMinimum(1);
            pinCounter->setMaximum(24);

            QObject::connect(pinCounter,SIGNAL(valueChanged(int)),spinMapper,SLOT(map()));
            spinMapper->setMapping(pinCounter,i);

            pinCounter->setValue(pLUT->getEntryPin(i));
            ui->twPower->setCellWidget(i,1,pinCounter);
    }
    QObject::connect(spinMapper,SIGNAL(mapped(int)),this,SLOT(on_pin_changed(int)));
    ui->twPower->resizeRowsToContents();

    ui->twPower->setUpdatesEnabled(1);
}

void ConfigWindow::on_pin_changed(int row)
{
    PowerTable::powerEntry ent;
    ent.number=row+1;
    QSpinBox * pinSpinner = qobject_cast<QSpinBox*>(ui->twPower->cellWidget(row,1));
    ent.pin=pinSpinner->value();
    ent.type=PowerRoleToCode(ui->twPower->item(row,0)->text());
    pLUT->setEntry(ent,row);
}

void ConfigWindow::on_buttonBox_accepted()
{
    //write configuration
    QSettings settings(this->cfgFileName,QSettings::IniFormat);
    settings.beginGroup("Device");
    settings.setValue("Port",ui->cbSerialPort->currentText());
    settings.setValue("Rate",ui->cbBaudRate->currentText());
    settings.setValue("TimeOut",QString::number(ui->sbTimeout->value()));
    settings.endGroup();

    settings.beginGroup("GUI");
    settings.setValue("Lang",ui->cbLang->currentText());
    settings.setValue("ClearLog",ui->cbClearLog->isChecked());
    QString a=ui->btnLogSelect->text();
    if ((!ui->cbLogFont->isChecked())||(ui->btnLogSelect->text()=="..."))
    {
        a="";
    }
    settings.setValue("LogFont",a);
    a=ui->btnRomSelect->text();
    if ((!ui->cbRomFont->isChecked())||(ui->btnRomSelect->text()=="..."))
    {
        a="";
    }
    settings.setValue("RomFont",a);
    settings.setValue("ToolbarSize",ui->cbToolbarSize->currentText().toInt());
    settings.setValue("StartStopTB",ui->cbStartStopTB->isChecked());
    settings.endGroup();

    settings.beginGroup("Test");
    settings.setValue("StepDelay",QString::number(ui->sbStepDelay->value()));
    settings.setValue("warnUnstable",ui->cbWarnUnstable->isChecked());
    settings.endGroup();

    settings.beginGroup("ICColors");
    settings.setValue("BkgR",QString::number(ui->btnBkgr->palette().button().color().red()));
    settings.setValue("BkgG",QString::number(ui->btnBkgr->palette().button().color().green()));
    settings.setValue("BkgB",QString::number(ui->btnBkgr->palette().button().color().blue()));
    settings.setValue("HiR",QString::number(ui->btnHi->palette().button().color().red()));
    settings.setValue("HiG",QString::number(ui->btnHi->palette().button().color().green()));
    settings.setValue("HiB",QString::number(ui->btnHi->palette().button().color().blue()));
    settings.setValue("LowR",QString::number(ui->btnLo->palette().button().color().red()));
    settings.setValue("LowG",QString::number(ui->btnLo->palette().button().color().green()));
    settings.setValue("LowB",QString::number(ui->btnLo->palette().button().color().blue()));
    settings.setValue("NoR",QString::number(ui->btnNC->palette().button().color().red()));
    settings.setValue("NoG",QString::number(ui->btnNC->palette().button().color().green()));
    settings.setValue("NoB",QString::number(ui->btnNC->palette().button().color().blue()));
    settings.setValue("DrawR",QString::number(ui->btnDraw->palette().button().color().red()));
    settings.setValue("DrawG",QString::number(ui->btnDraw->palette().button().color().green()));
    settings.setValue("DrawB",QString::number(ui->btnDraw->palette().button().color().blue()));
    settings.setValue("TextR",QString::number(ui->btnTxt->palette().button().color().red()));
    settings.setValue("TextG",QString::number(ui->btnTxt->palette().button().color().green()));
    settings.setValue("TextB",QString::number(ui->btnTxt->palette().button().color().blue()));
    settings.setValue("ErrR",QString::number(ui->btnErr->palette().button().color().red()));
    settings.setValue("ErrG",QString::number(ui->btnErr->palette().button().color().green()));
    settings.setValue("ErrB",QString::number(ui->btnErr->palette().button().color().blue()));
    settings.endGroup();

    settings.beginGroup("AutoSettings");
    settings.setValue("SetAuto",ui->cbSetAuto->isChecked());
    settings.setValue("PowerAuto",ui->lePowerAuto->text());
    settings.setValue("GndAuto",ui->leGndAuto->text());
    settings.setValue("OutputAuto",ui->leOutputAuto->text());
    settings.setValue("NcAuto",ui->leNcAuto->text());
    settings.endGroup();

    //external software settings
    settings.beginGroup("ExternalSoftware");
    int q=this->extItems.count();
    settings.setValue("ProgramsCount",q);
    for (int i=0;i<q;i++)
    {
        settings.setValue("Name"+QString::number(i),this->extItems.at(i));
        settings.setValue("Command"+QString::number(i),this->extCalls.at(i));
    }
    //TODO: Bad hack but works
    for (int i=q;i<150;i++)
    {
        settings.remove("Name"+QString::number(i));
        settings.remove("Command"+QString::number(i));
    }

    settings.endGroup();

    //save power config
    this->pLUT->saveTable();

}

void ConfigWindow::on_sbSwitchesCount_valueChanged(int arg1)
{
    this->pLUT->resize(arg1);
    this->repaintPower(this->pLUT);
}

void ConfigWindow::on_btnDefaultPower_clicked()
{
    this->pLUT->revertToDefault();
    this->repaintPower(this->pLUT);
}

void ConfigWindow::repaintExtTools()
{
    ui->twExtItems->setUpdatesEnabled(0);
    while (ui->twExtItems->rowCount()>0)
            ui->twExtItems->removeRow(0);
    for (int i=0;i<this->extItems.count();i++)
    {
        ui->twExtItems->insertRow(ui->twExtItems->rowCount());
        QTableWidgetItem * item1 = new QTableWidgetItem(this->extItems.at(i));
        ui->twExtItems->setItem(i,0,item1);
        QTableWidgetItem * item2 = new QTableWidgetItem(this->extCalls.at(i));
        ui->twExtItems->setItem(i,1,item2);
    }
    ui->twExtItems->resizeRowsToContents();
    ui->twExtItems->setUpdatesEnabled(1);
}

//add external tool
void ConfigWindow::on_btnAddExt_clicked()
{
    if (this->extItems.count()>150)
    {
        return;
    }

    this->extCalls.append("");
    this->extItems.append("");
    this->repaintExtTools();
}

//remove external tool
void ConfigWindow::on_btnRemoveExt_clicked()
{
    int cr=ui->twExtItems->currentRow();
    if (cr>-1)
    {
        this->extItems.removeAt(cr);
        this->extCalls.removeAt(cr);
        this->repaintExtTools();
    }
}

//record external tool change
void ConfigWindow::on_twExtItems_itemChanged(QTableWidgetItem *item)
{
    if (item->column()==0)
    {
        this->extItems[item->row()]=item->text();
    }
    if (item->column()==1)
    {
        this->extCalls[item->row()]=item->text();
    }
}

//select new font for Log. The font, encoded as string, is kept in button's text
void ConfigWindow::on_btnLogSelect_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,ui->btnLogSelect->font(),nullptr);
    if (!ok)
    {
        return;
    }
    ui->btnLogSelect->setText(font.toString());
    ui->btnLogSelect->setFont(font);
    ui->cbLogFont->setChecked(true);
}

//select new font for ROM dump. The font, encoded as strong, is kept in button's text.
void ConfigWindow::on_btnRomSelect_clicked()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok,ui->btnRomSelect->font(),nullptr);
    if (!ok)
    {
        return;
    }
    ui->btnRomSelect->setText(font.toString());
    ui->btnRomSelect->setFont(font);
    ui->cbRomFont->setChecked(true);
}
