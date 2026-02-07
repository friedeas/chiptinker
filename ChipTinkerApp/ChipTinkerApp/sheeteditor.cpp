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


#include "sheeteditor.h"
#include "ui_sheeteditor.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QScrollBar>

//Sheet Editor
//Edit specified test sheet in GUI

SheetEditor::SheetEditor(TestSheet * sheet, QStringList AutoSettings, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SheetEditor)
{
    ui->setupUi(this);
    this->sheet=sheet;
    ui->leName->blockSignals(1);
    ui->leDescription->blockSignals(1); //block "on edited" signals for initialization
    ui->leName->setText(this->sheet->getName());
    ui->leDescription->setText(this->sheet->getDescription());
    ui->leName->blockSignals(0);
    ui->leDescription->blockSignals(0);  //unblock signals
    ui->sbPinsNo->findChild<QLineEdit*>()->setReadOnly(true); //make sure user won't enter "tram rail" in here
    ui->sbPinsNo->setValue(this->sheet->getNumOfPins()); //this will not pull all graphics redraw yet

    this->AutoSettings=AutoSettings; //load auto-complete settings

    SheetEditor::setWindowTitle(tr("Sheet Editor"));
    QObject::connect(ui->sbPinsNo,SIGNAL(valueChanged(int)),this,SLOT(on_sbPinsNo_valueChanged1(int))); //now pull the graphics redraw on pins nb change

    //prepare pins widgets
    ui->twFirst->setColumnCount(3);
    QStringList labels;
    labels<<tr("#")<<tr("Usage")<<tr("Tag");
    ui->twFirst->setHorizontalHeaderLabels(labels);
    ui->twFirst->verticalHeader()->hide();
    ui->twSecond->setColumnCount(3);
    ui->twSecond->setHorizontalHeaderLabels(labels);
    ui->twSecond->verticalHeader()->hide();

    //COLUMN SIZES: Warning: Qt may not resize it with app scaling.
    ui->twFirst->setColumnWidth(0,30);
    ui->twSecond->setColumnWidth(0,30);
    ui->twFirst->setColumnWidth(2,ui->twFirst->width()-ui->twFirst->columnWidth(0)-ui->twFirst->columnWidth(1)-5);
    ui->twSecond->setColumnWidth(2,ui->twSecond->width()-ui->twSecond->columnWidth(0)-ui->twSecond->columnWidth(1)-5);

    QObject::connect(ui->twFirst,SIGNAL(cellChanged(int,int)),this,SLOT(on_twFirst_changed(int,int)));
    QObject::connect(ui->twSecond,SIGNAL(cellChanged(int,int)),this,SLOT(on_twSecond_changed(int,int)));

    this->repaintPins();
    this->repaintScript(); //make graphics

    //if first page has been changed, usually we've to edit script. Make user not point second tab every time editor is started.
    QString t=ui->leDescription->text();
    if (t!=tr("Type description"))
    {
        ui->tabWidget->setCurrentIndex(1);
    }
    else
    {
        ui->tabWidget->setCurrentIndex(0);
    }
}

SheetEditor::~SheetEditor()
{
    delete ui;
}

//Switching next row in key event
void SheetEditor::keyPressEvent(QKeyEvent * event)
{
     if ((event->key()==16777220)||(event->key()==Qt::Key_Enter)||(event->key()==Qt::Key_Return))
     {
         //BUGFIX: Program shuts down with crash when Return is pressed - determine what we're
         //editing and what shall be done. If no table is edited, then it means "accept".
        if ((ui->twFirst->selectedItems().count()<=0)&&(ui->twSecond->selectedItems().count()<=0))
        {
            this->accept();
            return;
        }
        if (ui->twFirst->selectedItems().count()>0)
        {
            //WARNING: Possible RC, this count quickly disappears!
            int cr=ui->twFirst->currentRow();
            int cc=ui->twFirst->currentColumn();
            QString currentText=ui->twFirst->item(cr,2)->text();
            //Please use cr and cc instead of CurrentColumn/CurrentRow.

            //Auto-fill of power role if enabled
            if (this->AutoSettings.at(0)=="true")
            {
                for (int autoClass=0;autoClass<5;autoClass++) //for each pin, iterating through structure
                {
                    QApplication::processEvents(); //please do not remove - fixes the RC some way!
                    QString wildcardList=AutoSettings.at(autoClass);
                    QStringList x = wildcardList.split(';');
                    for (int i=0;i<x.count();i++) //for each wildcard in pin class
                    {
                        QString y=x.at(i);
                        QRegularExpression rx(QRegularExpression::anchoredPattern(QRegularExpression::wildcardToRegularExpression(y.trimmed())),QRegularExpression::CaseInsensitiveOption);
                        if (rx.match(currentText).hasMatch())
                        {
                            if (autoClass==1) //this is because pin class 2 is unused I/O.
                            {
                                this->sheet->pins[cr].pinType=1;
                            }
                            else
                            {
                                this->sheet->pins[cr].pinType=autoClass+1;
                            }
                            this->repaintPins();
                        }
                    }
                }
            }

            ui->twSecond->clearSelection();

            if ((cc==2)&&(cr>-1)&&(cr<(this->sheet->getNumOfPins()/2)))
            {
                if (cr==(ui->twFirst->rowCount())-1)
                {
                    //switch to second
                    ui->twFirst->clearSelection();
                    ui->twSecond->setCurrentCell(ui->twSecond->rowCount()-1,2);
                    ui->twSecond->editItem(ui->twSecond->currentItem());
                }
                ui->twFirst->setCurrentCell(cr+1,2);
                ui->twFirst->editItem(ui->twFirst->currentItem());
                return;
            }

        }
        else //SECOND TABLE SET (right-hand)
        {
            //WARNING: Possible RC, this count quickly disappears!
            int cr=ui->twSecond->currentRow();
            QString currentText=ui->twSecond->item(cr,2)->text();
            //Please use cr and cc instead of CurrentColumn/CurrentRow.

            //Auto-fill of power role if enabled
            if (this->AutoSettings.at(0)=="true")
            {
                for (int autoClass=0;autoClass<5;autoClass++) //for each pin, iterating through structure
                {
                    QApplication::processEvents(); //please do not remove - fixes the RC some way!
                    QString wildcardList=AutoSettings.at(autoClass);
                    QStringList x = wildcardList.split(';');
                    for (int i=0;i<x.count();i++) //for each wildcard in pin class
                    {
                        QString y=x.at(i);
                        QRegularExpression rx(QRegularExpression::anchoredPattern(QRegularExpression::wildcardToRegularExpression(y.trimmed())),QRegularExpression::CaseInsensitiveOption);
                        if (rx.match(currentText).hasMatch())
                        {
                            if (autoClass==1) //this is because pin class 2 is unused I/O.
                            {
                                this->sheet->pins[(this->sheet->getNumOfPins()/2)+((this->sheet->getNumOfPins()/2)-cr)-1].pinType=1;
                            }
                            else
                            {
                                this->sheet->pins[(this->sheet->getNumOfPins()/2)+((this->sheet->getNumOfPins()/2)-cr)-1].pinType=autoClass+1;
                            }
                            this->repaintPins();
                        }
                    }
                }
            }

             //on 2nd row, go upwards
             ui->twSecond->setCurrentCell(cr-1,2);
             ui->twSecond->editItem(ui->twSecond->currentItem());
        }
    }
}

void SheetEditor::on_leName_textChanged(const QString &arg1)
{
    this->sheet->setName(arg1);
}

void SheetEditor::on_leDescription_textChanged(const QString &arg1)
{
    this->sheet->setDescription(arg1);
}

void SheetEditor::on_sbPinsNo_valueChanged1(int arg1)
{
    if(arg1!=this->sheet->getNumOfPins())
    {
        if (this->sheet->script.count()>0)
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,tr("Warning"),tr("Modifying pin count will ERASE test script.\nDo you want to continue?"),QMessageBox::Yes|QMessageBox::No);
            if (reply==QMessageBox::No)
            {
                ui->sbPinsNo->setValue(this->sheet->getNumOfPins());
                return;
            }
        }
        this->sheet->setNumOfPins(arg1);

        this->repaintPins();
        this->repaintScript();
    }
}

//Pin code to descriptive string
QString SheetEditor::CodeToStr(int code)
{
    if (code==0) return tr("Input");
    if (code==1) return tr("Output");
    if (code==2) return tr("I/O");
    if (code==3) return tr("GND");
    if (code==4) return tr("+V");
    if (code==5) return tr("NC");
    return "INT.ERROR";
}

//Descriptive string to pin code
int SheetEditor::strToCode(QString str)
{
    if (str==tr("Input")) return 0;
    if (str==tr("Output")) return 1;
    if (str==tr("I/O")) return 2;
    if (str==tr("GND")) return 3;
    if (str==tr("+V")) return 4;
    if (str==tr("NC")) return 5;
    return 0;
}

//Paint pins in editor
void SheetEditor::repaintPins()
{
    ui->twFirst->blockSignals(1);
    ui->twSecond->blockSignals(1);

    ui->twFirst->setUpdatesEnabled(0);
    ui->twSecond->setUpdatesEnabled(0);
    while (ui->twFirst->rowCount()>0) ui->twFirst->removeRow(0);
    while (ui->twSecond->rowCount()>0) ui->twSecond->removeRow(0);

    //populate first
    for (int i=0;i<this->sheet->getNumOfPins()/2;i++)
    {
        ui->twFirst->insertRow(ui->twFirst->rowCount());
        QColor col;
        if (this->sheet->pins[i].pinType==0) col=QColor::fromRgb(128,255,128);
        if (this->sheet->pins[i].pinType==1) col=QColor::fromRgb(0,128,255);
        if (this->sheet->pins[i].pinType==3) col=QColor::fromRgb(0,128,0);
        if (this->sheet->pins[i].pinType==4) col=QColor::fromRgb(255,64,64);
        if (this->sheet->pins[i].pinType==5) col=QColor::fromRgb(128,128,128);
        QBrush junk(col); //junk object for porting to qt6
        QTableWidgetItem * no = new QTableWidgetItem(QString::number(i+1));
        no->setFlags(no->flags() & ~Qt::ItemIsEditable);
//        no->setBackgroundColor(col);
        no->setBackground(junk);
        ui->twFirst->setItem(i,0,no);
        QTableWidgetItem * func = new QTableWidgetItem(CodeToStr(this->sheet->pins[i].pinType));
        func->setFlags(func->flags() & ~Qt::ItemIsEditable);
        func->setBackground(junk);
        ui->twFirst->setItem(i,1,func);
        QTableWidgetItem * tag = new QTableWidgetItem(this->sheet->pins[i].pinTag);
        tag->setBackground(junk);
        ui->twFirst->setItem(i,2,tag);
    }

    //populate second. Notice that they go reverse way as in IC numbering
    for (int i=this->sheet->getNumOfPins()-1;i>=this->sheet->getNumOfPins()/2;i--)
    {
        ui->twSecond->insertRow(ui->twSecond->rowCount());
        QColor col;
        if (this->sheet->pins[i].pinType==0) col=QColor::fromRgb(128,255,128);
        if (this->sheet->pins[i].pinType==1) col=QColor::fromRgb(0,128,255);
        if (this->sheet->pins[i].pinType==3) col=QColor::fromRgb(0,128,0);
        if (this->sheet->pins[i].pinType==4) col=QColor::fromRgb(255,64,64);
        if (this->sheet->pins[i].pinType==5) col=QColor::fromRgb(128,128,128);
        QBrush junk(col); //junk for qt6 compatibility
        QTableWidgetItem * no = new QTableWidgetItem(QString::number(i+1));
        no->setFlags(no->flags() & ~Qt::ItemIsEditable);
        no->setBackground(junk);
        ui->twSecond->setItem(ui->twSecond->rowCount()-1,0,no);
        QTableWidgetItem * func = new QTableWidgetItem(CodeToStr(this->sheet->pins[i].pinType));
        func->setFlags(func->flags() & ~Qt::ItemIsEditable);
        func->setBackground(junk);
        ui->twSecond->setItem(ui->twSecond->rowCount()-1,1,func);
        QTableWidgetItem * tag = new QTableWidgetItem(this->sheet->pins[i].pinTag);
        tag->setBackground(junk);
        ui->twSecond->setItem(ui->twSecond->rowCount()-1,2,tag);
    }

    ui->twFirst->resizeRowsToContents();
    ui->twSecond->resizeRowsToContents();
    ui->twFirst->setUpdatesEnabled(1);
    ui->twSecond->setUpdatesEnabled(1);

    ui->twFirst->blockSignals(0);
    ui->twSecond->blockSignals(0);

    //adjust visualization
    int yy=125+(ui->twFirst->rowHeight(1))*ui->twFirst->rowCount();
    ui->lineBottom->move(ui->lineBottom->x(),yy);
    ui->lineL->setGeometry(ui->lineL->x(),ui->lineL->y(),ui->lineL->width(),yy-112);
    ui->lineR->setGeometry(ui->lineR->x(),ui->lineR->y(),ui->lineR->width(),yy-112);
}

//Command code to descriptive string
QString SheetEditor::CommandToStr(int command)
{
    if (command==0) return tr("Reset");
    if (command==1) return tr("5V ON");
    if (command==2) return tr("Config");
    if (command==3) return tr("Send");
    if (command==4) return tr("Read");
    if (command==255) return tr("Info");
    return "!ERROR!";
}

//Descriptive string to command code
int SheetEditor::StrToCommand(QString command)
{
    if (command==tr("Reset")) return 0;
    if (command==tr("5V ON")) return 1;
    if (command==tr("Config")) return 2;
    if (command==tr("Send")) return 3;
    if (command==tr("Read")) return 4;
     if (command==tr("Info")) return 255;
    return 0;
}

//this nice function paints script in table
//TODO: optimize it
void SheetEditor::repaintScript()
{
    ui->twScript->blockSignals(1);
    ui->twScript->setUpdatesEnabled(0);

    while(ui->twScript->rowCount()>0) ui->twScript->removeRow(0); //perl-ish list clear clears attributes too
    QStringList header;
    header<<tr("Cmd");
    for (int i=0;i<this->sheet->getNumOfPins();i++)
    {
       header<<QString::number(i+1);
       // header<<QString::number(i+1)+"\n"+this->sheet->pins[i].pinTag; //monitor has fixed width, don't put pin tag there.
    }
    ui->twScript->setColumnCount(sheet->getNumOfPins()+1);
    ui->twScript->setHorizontalHeaderLabels(header);
    ui->twScript->setColumnWidth(0,60);
    ui->twScript->setWordWrap(0);
    for (int i=0;i<this->sheet->getNumOfPins();i++)
    {
       ui->twScript->setColumnWidth(i+1,25);
      //  ui->twScript->setColumnWidth(i+1,50);
    }

    //populate list
    //first, let's create initial I/O list
    QString currentUsage = this->sheet->initIO().left((24+this->sheet->getNumOfPins())/2);
    currentUsage=currentUsage.right(this->sheet->getNumOfPins());
    //colors for cells
    //In future they may be saved to config file
    QColor inpCol=QColor::fromRgb(128,255,128);
    QColor outCol=QColor::fromRgb(0,128,255);
    QColor gndCol=QColor::fromRgb(0,128,0);
    QColor vccCol=QColor::fromRgb(255,64,64);
    QColor ncCol=QColor::fromRgb(128,128,128);
    //draw nice color legend
    QPalette pal=palette();
    ui->lbLegendInp->setAutoFillBackground(1);
    ui->lbLegendInp->setAlignment(Qt::AlignCenter);
    pal.setColor(QPalette::Window, inpCol);
    ui->lbLegendInp->setPalette(pal);
    ui->lbLegendOut->setAutoFillBackground(1);
    ui->lbLegendOut->setAlignment(Qt::AlignCenter);
    pal.setColor(QPalette::Window, outCol);
    ui->lbLegendOut->setPalette(pal);
    ui->lbLegendNC->setAutoFillBackground(1);
    ui->lbLegendNC->setAlignment(Qt::AlignCenter);
    pal.setColor(QPalette::Window, ncCol);
    ui->lbLegendNC->setPalette(pal);
    ui->lbLegendPower->setAutoFillBackground(1);
    ui->lbLegendPower->setAlignment(Qt::AlignCenter);
    pal.setColor(QPalette::Window, vccCol);
    ui->lbLegendPower->setPalette(pal);
    ui->lbLegendGround->setAutoFillBackground(1);
    ui->lbLegendGround->setAlignment(Qt::AlignCenter);
    pal.setColor(QPalette::Window, gndCol);
    ui->lbLegendGround->setPalette(pal);

    int rowNumber=0;
    QStringList rowNumbers;
    for (int i=0;i<this->sheet->script.count();i++)
    {
        rowNumber++;
        QString rowLabel=QString::number(rowNumber);
        ui->twScript->insertRow(ui->twScript->rowCount());
        QTableWidgetItem * cmd = new QTableWidgetItem(CommandToStr(this->sheet->script[i].cmd));
        cmd->setFlags(cmd->flags() & ~Qt::ItemIsEditable);
        ui->twScript->setItem(i,0,cmd);
        //now the tail
        if ((this->sheet->script[i].cmd==0)||(this->sheet->script[i].cmd==1))
        {
            //reset or power on condition - no parameters
            for (int j=0;j<this->sheet->getNumOfPins();j++)
            {
              QTableWidgetItem * blind = new QTableWidgetItem(" ");
              blind->setBackground(QBrush(QColor::fromRgb(200,200,200)));
              blind->setFlags(blind->flags() & ~Qt::ItemIsEditable);
              ui->twScript->setItem(i,j+1,blind);
            }
        }
        if (this->sheet->script[i].cmd==2) //change I/O
        {
            currentUsage=this->sheet->script[i].arg;
            for (int j=0;j<this->sheet->getNumOfPins();j++)
            {
                QColor col=inpCol;
                QString k="I";
                if (this->sheet->script[i].arg[j]=='1')
                {
                    k="O";
                    col=outCol;
                }
                if (this->sheet->pins[j].pinType==3)
                {
                    k=" ";
                    col=gndCol;
                }
                if (this->sheet->pins[j].pinType==4)
                {
                    k=" ";
                    col=vccCol;
                }
                if (this->sheet->pins[j].pinType==5)
                {
                    k=" ";
                    col=ncCol;
                }
                QTableWidgetItem * item = new QTableWidgetItem(k);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setTextAlignment(Qt::AlignCenter);
                QBrush junk(col);
                item->setBackground(junk);
                item->setToolTip(tr("Pin ")+QString::number(j+1)+": "+this->sheet->pins[j].pinTag);
                ui->twScript->setItem(i,j+1,item);
            }

        }
        if (this->sheet->script[i].cmd==3) //Send
        {
            for (int j=0;j<this->sheet->getNumOfPins();j++)
            {
                QString k=" ";
                QColor col=inpCol;
                if (this->sheet->script[i].arg[j]=='0') k="L";
                if (this->sheet->script[i].arg[j]=='1') k="H";
                if (this->sheet->script[i].arg[j]=='X') k=" ";
                if (currentUsage[j]=='1')
                {
                    k=" ";
                    col=outCol;
                }
                if (this->sheet->pins[j].pinType==3)
                {
                    k=" ";
                    col=gndCol;
                }
                if (this->sheet->pins[j].pinType==4)
                {
                    k=" ";
                    col=vccCol;
                }
                if (this->sheet->pins[j].pinType==5)
                {
                    k=" ";
                    col=ncCol;
                }
                QTableWidgetItem * item = new QTableWidgetItem(k);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setTextAlignment(Qt::AlignCenter);
                QBrush junk(col);
                item->setBackground(junk);
                item->setToolTip(tr("Pin ")+QString::number(j+1)+": "+this->sheet->pins[j].pinTag);
                ui->twScript->setItem(i,j+1,item);
            }
        }
        if (this->sheet->script[i].cmd==4) //Read
        {
            for (int j=0;j<this->sheet->getNumOfPins();j++)
            {
                QString k=" ";
                QColor col=inpCol;
                if (this->sheet->script[i].arg[j]=='0') k="L";
                if (this->sheet->script[i].arg[j]=='1') k="H";
                if (this->sheet->script[i].arg[j]=='X') k="X";
                if (currentUsage[j]=='1')
                {
                    col=outCol;
                }
                if (this->sheet->pins[j].pinType==3)
                {
                    k=" ";
                    col=gndCol;
                }
                if (this->sheet->pins[j].pinType==4)
                {
                    k=" ";
                    col=vccCol;
                }
                if (this->sheet->pins[j].pinType==5)
                {
                    k=" ";
                    col=ncCol;
                }
                QTableWidgetItem * item = new QTableWidgetItem(k);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item->setTextAlignment(Qt::AlignCenter);
                QBrush junk(col);
                item->setBackground(junk);
                item->setToolTip(tr("Pin ")+QString::number(j+1)+": "+this->sheet->pins[j].pinTag);
                ui->twScript->setItem(i,j+1,item);
            }
        }
        if (this->sheet->script[i].cmd==255) //comment
        {
            QString k=this->sheet->script[i].arg.trimmed();
            QTableWidgetItem * item = new QTableWidgetItem(k);
            item->setTextAlignment(Qt::AlignLeft);

            ui->twScript->setItem(i,1,item);
            ui->twScript->setSpan(i,1,1,this->sheet->getNumOfPins());
            if (k.at(k.length()-1)!=';') //invisible comments get no numbers. They are invisible.
            {
                rowNumber--;
                rowLabel="#";
            }
        }

        rowNumbers.append(rowLabel);
    }
    ui->twScript->setVerticalHeaderLabels(rowNumbers);

    ui->twScript->resizeRowsToContents();
    ui->twScript->setUpdatesEnabled(1);
    ui->twScript->blockSignals(0);
}


//Change pin roles
void SheetEditor::on_twFirst_cellDoubleClicked(int row, int column)
{
    if (column==1)
    {
        //cycle through items
        int it=strToCode(ui->twFirst->item(row,column)->text());
        it=(it+1)%6;
        if (it==2) it++;
        int pin=ui->twFirst->item(row,0)->text().toInt()-1;
        this->sheet->pins[pin].pinType=it;
        this->repaintPins();
    }
}

//Change pin roles - second column
void SheetEditor::on_twSecond_cellDoubleClicked(int row, int column)
{
    if (column==1)
    {
        //cycle through items
        int it=strToCode(ui->twSecond->item(row,column)->text());
        it=(it+1)%6;
        if (it==2) it++;
        int pin=ui->twSecond->item(row,0)->text().toInt()-1;
        this->sheet->pins[pin].pinType=it;
        this->repaintPins();
    }
}

//change pins in listing
void SheetEditor::on_twFirst_changed(int row, int column)
{
    if (column==2)
    {
          int pin=ui->twFirst->item(row,0)->text().toInt()-1;
          this->sheet->pins[pin].pinTag=ui->twFirst->item(row,column)->text().replace(',',' ');
          QKeyEvent ew(QKeyEvent::KeyPress,16777220,Qt::NoModifier);
          this->keyPressEvent(&ew);
    }
}

//change pins in listing - second column
void SheetEditor::on_twSecond_changed(int row, int column)
{
    if (column==2)
    {
          int pin=ui->twSecond->item(row,0)->text().toInt()-1;
          this->sheet->pins[pin].pinTag=ui->twSecond->item(row,column)->text().replace(',',' ');
          QKeyEvent ew(QKeyEvent::KeyPress,16777220,Qt::NoModifier);
          this->keyPressEvent(&ew);
    }
}

//Swap first-second columns
void SheetEditor::on_twFirst_cellClicked()
{
    ui->twSecond->clearSelection();
}

//Swap second-first columns
void SheetEditor::on_twSecond_cellClicked()
{
    ui->twFirst->clearSelection();
}

//Insert "Reset" command into script editor
void SheetEditor::on_btnInsertReset_clicked()
{
    TestSheet::testCommand tmp;
    tmp.cmd=0;
    this->sheet->script.push_back(tmp);
    this->repaintScript();
    ui->twScript->scrollToBottom(); //always scroll to added line
    ui->twScript->selectRow(ui->twScript->rowCount()-1);
}

//Insert "Power On" command into script editor
void SheetEditor::on_btnInsertPower_clicked()
{
    TestSheet::testCommand tmp;
    tmp.cmd=1;
    this->sheet->script.push_back(tmp);
    this->repaintScript();
    ui->twScript->scrollToBottom(); //always scroll to added line
    ui->twScript->selectRow(ui->twScript->rowCount()-1);
}

//Insert "Configure I/O" command into script editor
void SheetEditor::on_btnInsertConfig_clicked()
{
    //By default config does not change anything ergo we need to get last I/O state
    QString currentUsage = this->sheet->initIO().left((24+this->sheet->getNumOfPins())/2);
    currentUsage=currentUsage.right(this->sheet->getNumOfPins());
    for (int i=0;i<this->sheet->script.count();i++)
    {
        if (this->sheet->script[i].cmd==2) currentUsage=this->sheet->script[i].arg;
    }
    //now introduce command
    TestSheet::testCommand tmp;
    tmp.cmd=2;
    tmp.arg=currentUsage;
    this->sheet->script.push_back(tmp);
    this->repaintScript();
    ui->twScript->scrollToBottom(); //always scroll to added line
    ui->twScript->selectRow(ui->twScript->rowCount()-1);
}

//Insert "Send" command into script editor
void SheetEditor::on_btnInsertSend_clicked()
{
    //Generate default combination
    QString currentUsage;
    for (int i=0;i<this->sheet->getNumOfPins();i++)
    {
        if ((this->sheet->pins[i].pinType==0)||(this->sheet->pins[i].pinType==1))
        {
            currentUsage+="0";
        }
        else
        {
            currentUsage+="X";
        }
    }
    //If we have some read/send command before, we will use its arguments
    if (this->sheet->script.count()>0)
    {
        if ((this->sheet->script.last().cmd==3)||(this->sheet->script.last().cmd==4))
            currentUsage=this->sheet->script.last().arg;
    }
    //now introduce command
    TestSheet::testCommand tmp;
    tmp.cmd=3;
    tmp.arg=currentUsage;
    this->sheet->script.push_back(tmp);
    this->repaintScript();
    ui->twScript->scrollToBottom(); //always scroll to added line
    ui->twScript->selectRow(ui->twScript->rowCount()-1);
}

//Insert "Read" command into script editor
void SheetEditor::on_btnInsertRead_clicked()
{
    //Generate default combination (0 for pins, doesn't matter for Vcc/GND)
    QString currentUsage;
    for (int i=0;i<this->sheet->getNumOfPins();i++)
    {
        if ((this->sheet->pins[i].pinType==0)||(this->sheet->pins[i].pinType==1))
        {
            currentUsage+="0";
        }
        else
        {
            currentUsage+="X";
        }
    }
    //If we have some read/send command before, we will use its arguments
    //it saves lot of time when programming a test (it's not needed to switch IC's inputs.
    if (this->sheet->script.count()>0)
    {
        if ((this->sheet->script.last().cmd==3)||(this->sheet->script.last().cmd==4))
            currentUsage=this->sheet->script.last().arg;
    }
    //now introduce command
    TestSheet::testCommand tmp;
    tmp.cmd=4;
    tmp.arg=currentUsage;
    this->sheet->script.push_back(tmp);
    this->repaintScript();
    ui->twScript->scrollToBottom(); //always scroll to added line
    ui->twScript->setCurrentCell(ui->twScript->rowCount()-1,1);
}


//Insert comment into script editor
void SheetEditor::on_btnInsertcomment_clicked()
{
    TestSheet::testCommand tmp;
    tmp.cmd=255;
    tmp.arg="...";
    this->sheet->script.push_back(tmp);
    this->repaintScript();
    ui->twScript->scrollToBottom(); //always scroll to added line
    ui->twScript->setCurrentCell(ui->twScript->rowCount()-1,1);
}


//Delete currently selected command
void SheetEditor::on_btnLineDelete_clicked()
{
    int cr=ui->twScript->currentRow();
    if (cr>-1)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,tr("Warning"),tr("OK to delete selected step?"),QMessageBox::Yes|QMessageBox::No);
        if (reply==QMessageBox::Yes)
        {
            this->sheet->script.remove(cr);
            this->repaintScript();
            if (cr<ui->twScript->rowCount())
            {
                ui->twScript->setCurrentCell(cr,1);
            }
            else
                ui->twScript->scrollToBottom();
        }
    }
}

//Push line to bottom
void SheetEditor::on_btnLineToBottom_clicked()
{
    int cr=ui->twScript->currentRow();
    if ((cr>-1)&&(cr!=ui->twScript->rowCount()-1))
    {
        int cc=ui->twScript->currentColumn();
        TestSheet::testCommand tmp=this->sheet->script[cr];
        this->sheet->script.remove(cr);
        this->sheet->script.push_back(tmp);
        this->repaintScript();
        ui->twScript->scrollToBottom();
        ui->twScript->setCurrentCell(ui->twScript->rowCount()-1,cc);
    }
}

//Push line to top
void SheetEditor::on_BtnLineToTop_clicked()
{
    int cr=ui->twScript->currentRow();
    if (cr>0)
    {
        int cc=ui->twScript->currentColumn();
        TestSheet::testCommand tmp=this->sheet->script[cr];
        this->sheet->script.remove(cr);
        this->sheet->script.push_front(tmp);
        this->repaintScript();
        ui->twScript->scrollToTop();
        ui->twScript->setCurrentCell(0,cc);
    }
}

void SheetEditor::on_btnLineDown_clicked()
{
    int cr=ui->twScript->currentRow();
    if ((cr>-1)&&(cr!=ui->twScript->rowCount()-1))
    {
          int cc=ui->twScript->currentColumn();
          TestSheet::testCommand tmp=this->sheet->script[cr+1];
          this->sheet->script[cr+1]=this->sheet->script[cr];
          this->sheet->script[cr]=tmp;
          this->repaintScript();
          ui->twScript->setCurrentCell(cr+1,cc);
    }
}

void SheetEditor::on_btnLineUp_clicked()
{
    int cr=ui->twScript->currentRow();
    if (cr>0)
    {
        int cc=ui->twScript->currentColumn();
        TestSheet::testCommand tmp=this->sheet->script[cr-1];
        this->sheet->script[cr-1]=this->sheet->script[cr];
        this->sheet->script[cr]=tmp;
        this->repaintScript();
        ui->twScript->setCurrentCell(cr-1,cc);
    }

}

//Change cell state in script
void SheetEditor::on_twScript_cellDoubleClicked(int row, int column)
{
    //If there's a text in a cell, it may be modified.
    //If there's H, there may be L. If command is "Read", there may be X too. H-L-X.
    //If there's I, there may be O. I-O.
    //Then the script must be synchronized.
    //Because this is made all time, no repainting should be performed in R/W operations.
    QString contents = ui->twScript->item(row,column)->text();
    if (contents=="H")
    {
        //transition to L
        ui->twScript->item(row,column)->setText("L");
        this->sheet->script[row].arg[column-1]='0';
    }
    if (contents=="L")
    {
        //if Read, transition to X, else transition to H
        if (ui->twScript->item(row,0)->text()=="Read")
        {
            ui->twScript->item(row,column)->setText("X");
            this->sheet->script[row].arg[column-1]='X';
        }
        else
        {
            ui->twScript->item(row,column)->setText("H");
            this->sheet->script[row].arg[column-1]='1';
        }
    }
    if (contents=="X")
    {
        ui->twScript->item(row,column)->setText("H");
        this->sheet->script[row].arg[column-1]='1';
    }
    if (contents=="I")
    {
        ui->twScript->item(row,column)->setText("O");
        this->sheet->script[row].arg[column-1]='1';
        //transition to O

        //here repainting must be made to keep with I/O changes.
        this->repaintScript();
        ui->twScript->setCurrentCell(row,column);
    }
    if (contents=="O")
    {
        ui->twScript->item(row,column)->setText("I");
        this->sheet->script[row].arg[column-1]='0';
        //transition to I

        //here repainting must be made to keep with I/O changes.
        this->repaintScript();
        ui->twScript->setCurrentCell(row,column);
    }

}

//Error/warning handling
void SheetEditor::done(int r)
{
    if (r==QDialog::Accepted)
    {
        //OK pressed
        if ((ui->leDescription->text()=="")||(ui->leName->text()==""))
        {
            QMessageBox::critical(this,tr("Error"),tr("Name and type cannot be blank."),QMessageBox::Ok);
            return;
        }
        //Evaluate pins for warnings
        QString warnings="";
        if (this->sheet->getGNDPins().length()>1)
            warnings+=tr("\n - More than 1 ground pins. This will require custom setting.");
        if (this->sheet->getPowerPins().length()>1)
             warnings+=tr("\n - More than 1 +5V pins. This will require custom setting.");
        if (this->sheet->getPowerPins().length()==0)
             warnings+=tr("\n - No ground pins at all.");
        if (this->sheet->getPowerPins().length()==0)
             warnings+=tr("\n - No voltage pins at all.");
        int inpCount=0;
        int outCount=0;
        for (int i=0;i<this->sheet->getNumOfPins();i++)
        {
            if (this->sheet->pins[i].pinType==0) inpCount++;
            if (this->sheet->pins[i].pinType==1) outCount++;
        }
        if (outCount==0)
            warnings+=tr("\n - No outputs at all.");
        if (inpCount==0)
            warnings+=tr("\n - No inputs at all.");

        if (warnings!="")
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,tr("Warning"),tr("The following non-standard situations have been found:")+warnings+tr("\nDo you want to continue?"),QMessageBox::Yes|QMessageBox::No);
            if (reply==QMessageBox::No)
            {
                return;
            }
        }
        QDialog::done(r);
        return;
    }
    else
    {
        QDialog::done(r);
        return;
    }
}

//This is a poorly written yet working clock generator routine.
void SheetEditor::on_pushButton_clicked()
{
    bool ok = false;
    int pin = QInputDialog::getInt(this,tr("Clock generator"),tr("Which pin to toggle?"),1,1,this->sheet->getNumOfPins(),1,&ok);
    if (!ok)
        return;
    pin--; //pins counted from 0
    //Generate I/O string
    QString currentUsage = this->sheet->initIO().left((24+this->sheet->getNumOfPins())/2);
    currentUsage=currentUsage.right(this->sheet->getNumOfPins());
    for (int i=0;i<this->sheet->script.count();i++)
    {
        if (this->sheet->script[i].cmd==2) currentUsage=this->sheet->script[i].arg;
    }
    if (currentUsage[pin]!='0')
    {
        QMessageBox::information(this,tr("Error"),tr("This pin is not an input!"),QMessageBox::Yes);
        return;
    }
    ok=false;
    int cycleCount = QInputDialog::getInt(this,tr("Clock generator"),tr("How many (2-step) cycles?"),1,1,1024,1,&ok);
    if (!ok)
        return;
    for (int i=0;i<cycleCount*2;i++) //Cycle is up-down so we have to multiply it by 2
    {
        //insert new Send command
        this->on_btnInsertSend_clicked();
        //toggle pin
        if (this->sheet->script.last().arg[pin]=='0')
            this->sheet->script.last().arg[pin]='1';
        else
            this->sheet->script.last().arg[pin]='0';
    }
    this->repaintScript();
}

//Changing pins I/O in first tab should be visible in second.
void SheetEditor::on_tabWidget_currentChanged(int index)
{
    if (index==1)
        this->repaintScript();
}

//Design rules checker
void SheetEditor::on_btnDRC_clicked()
{
    //DESIGN RULES CHECKER:
     // - Is there a ground, power, at least one input and one output?
     // - Are ping tagged?
     // - Does it contain at least one power on, send, read.
     // - Does it start with Reset-Power on?
     // - Are all inputs tested in high and low?
     // - Are all outputs tested at high/low?
     // - Does it end with READ?
    QString report=tr("Warnings: \n");

    if (this->sheet->script.count()<4)
    {
        QMessageBox::information(this,tr("Checker"),tr("Script shorter than 3 steps - cannot continue."),QMessageBox::Ok);
        return;
    }

    //Is there a ground, power, at least one input and one output?
    if (this->sheet->getGNDPins().length()>1)
        report+=tr("Design: More than 1 ground pins. This will require custom setting.\n");
    if (this->sheet->getPowerPins().length()>1)
         report+=tr("Design: More than 1 +5V pins. This will require custom setting.\n");
    if (this->sheet->getPowerPins().length()==0)
         report+=tr("Design: IC has no ground pins at all.\n");
    if (this->sheet->getPowerPins().length()==0)
         report+=tr("Design: IC has no voltage pins at all.\n");
    int inpCount=0;
    int outCount=0;
    for (int i=0;i<this->sheet->getNumOfPins();i++)
    {
        if (this->sheet->pins[i].pinType==0) inpCount++;
        if (this->sheet->pins[i].pinType==1) outCount++;
    }
    if (outCount==0)
        report+=tr("Design: IC has no outputs at all.\n");
    if (inpCount==0)
        report+=tr("Design: IC has no inputs at all.\n");

    //Does all pins have tags?
    for (int i=0;i<this->sheet->getNumOfPins();i++)
    {
        if (this->sheet->getPinsDescriptions()[i].trimmed().length()==0)
            report+=tr("IC: Pin ")+QString::number(i+1)+tr(" has no tag.\n");
    }

    //Does it contain at least one power, reset, send, read?
    bool noPower=1;
    bool noReset=1;
    bool noSend=1;
    bool noRead=1;
    for (int i=0;i<this->sheet->script.count();i++)
    {
        switch (this->sheet->script[i].cmd)
        {
            case 0:
                noReset=0;
                break;
            case 1:
                noPower=0;
                break;
            case 3:
                noSend=0;
                break;
            case 4:
                noRead=0;
                break;
        }
    }
    if (noPower)
        report+=tr("Script: Test has no \"Power on\" command.\n");
    if (noReset)
        report+=tr("Script: Test has no \"Reset\" command.\n");
    if (noSend)
        report+=tr("Script: Test has no \"Send\" command.\n");
    if (noRead)
        report+=tr("Script: Test has no \"Read\" command.\n");


    //Does it start with power on - reset?
    int k=0;
    for (int i=0;i<this->sheet->script.count();i++)
    {
        if (this->sheet->script.at(k).cmd==255)
            k++;
        else
            break;
    }
    //if (this->sheet->script.first().cmd!=0)
    if (this->sheet->script.at(k).cmd!=0)
    {

        report+=tr("1: Test doesn't start with Reset\n");
    }
    if ((this->sheet->script[k+1].cmd!=1)&&((this->sheet->script[k+2].cmd!=1)))
    {
        report+=tr("Script: No power on in second or 3rd step?\n");
    }


    //Does it test every input and output against Hi/lo?
    QString checkedAsOutputHi;
    checkedAsOutputHi.fill('0',this->sheet->getNumOfPins());
    QString checkedAsOutputLo;
    checkedAsOutputLo.fill('0',this->sheet->getNumOfPins());
    QString checkedAsInputHi;
    checkedAsInputHi.fill('0',this->sheet->getNumOfPins());
    QString checkedAsInputLo;
    checkedAsInputLo.fill('0',this->sheet->getNumOfPins());
    QString ExistsAsOutput;
    ExistsAsOutput.fill('0',this->sheet->getNumOfPins());
    QString ExistsAsInput;
    ExistsAsInput.fill('0',this->sheet->getNumOfPins());

    //First, we populate ExistsAsOutput/in for initial situation.
    QString currentUsage = this->sheet->initIO(1).left((24+this->sheet->getNumOfPins())/2);
    currentUsage=currentUsage.right(this->sheet->getNumOfPins());
    for (int i=0;i<currentUsage.length();i++)
    {
        if (currentUsage[i]=='0')
            ExistsAsInput[i]='1';
        if (currentUsage[i]=='1')
            ExistsAsOutput[i]='1';
    }
    //Then for all future changes
    for (int i=0;i<this->sheet->script.count();i++)
    {
        if (this->sheet->script[i].cmd==2)
        {
            for (int j=0;j<this->sheet->script[i].arg.length();j++)
            {
                if (this->sheet->script[i].arg[j]=='0')
                    ExistsAsInput[j]='1';
                if (this->sheet->script[i].arg[j]=='1')
                    ExistsAsOutput[j]='1';
            }
        }
    }
 //   report+=" EAI:"+ExistsAsInput+", EAO:"+ExistsAsOutput+"\n";
    for (int i=0;i<this->sheet->script.count();i++)
    {
        //If this pin is input, is it present in SEND commands both as H and L?
        //If this pin is output, is it present in READ commands both as H and L?
        if (this->sheet->script[i].cmd==3) //Send - input pin
        {
            for (int j=0;j<this->sheet->script[i].arg.length();j++)
            {
                if (this->sheet->script[i].arg[j]=='0')
                    checkedAsInputLo[j]='1';
                if (this->sheet->script[i].arg[j]=='1')
                    checkedAsInputHi[j]='1';
            }
        }
        if (this->sheet->script[i].cmd==4) //read - out pin
        {
            for (int j=0;j<this->sheet->script[i].arg.length();j++)
            {
                if (this->sheet->script[i].arg[j]=='0')
                    checkedAsOutputLo[j]='1';
                if (this->sheet->script[i].arg[j]=='1')
                    checkedAsOutputHi[j]='1';
            }
        }
    }
//    report+="in: h"+checkedAsInputHi+" l"+checkedAsInputLo+" out: h"+checkedAsOutputHi+" l"+checkedAsOutputLo+"\n";
    //Now compare strings to get which pins are not tested
    for (int i=0;i<this->sheet->getNumOfPins();i++)
    {
        if (ExistsAsInput[i]=='1')
        {
            if (checkedAsInputHi[i]=='0')
                report+=tr("Script: Pin ")+QString::number(i+1)+tr(" As input, not tested in 1 state.\n");
            if (checkedAsInputLo[i]=='0')
                report+=tr("Script: Pin ")+QString::number(i+1)+tr(" As input, not tested in 0 state.\n");
        }

        if (ExistsAsOutput[i]=='1')
        {
            //Because of pin classification, NC, power and ground are considered as o/p
            //we will omit them here.
            if (this->sheet->pins[i].pinType>1)
                continue;

            if (checkedAsOutputHi[i]=='0')
                report+=tr("Script: Pin ")+QString::number(i+1)+tr(" As output, not tested in 1 state.\n");
            if (checkedAsOutputLo[i]=='0')
                report+=tr("Script: Pin ")+QString::number(i+1)+tr(" As output, not tested in 0 state.\n");
        }
    }

    //Does it end with read?
    if ((this->sheet->script.last().cmd!=4)&&(this->sheet->script.last().cmd!=255))
    {
        report+=tr("Script: Sheet doesn't end with reading, do you need these instructions?.\n");
    }
    if (report==tr("Warnings: \n"))
        report+=tr("\nNo warnings.\n");
    else
        report+=tr("\nEnd of warnings.\n");

    QMessageBox::information(this,tr("Design rule checker"),report,QMessageBox::Ok);
}

//view pin names on status label during design. This speeds things up a bit.
void SheetEditor::on_twScript_itemEntered(QTableWidgetItem *item)
{
    ui->lbPinName->setText(item->toolTip());
}

//Save comments
void SheetEditor::on_twScript_itemChanged(QTableWidgetItem *item)
{
    if (this->sheet->script.at(item->row()).cmd==255)
    {
        this->sheet->script[item->row()].arg=item->text();
        int y = ui->twScript->verticalScrollBar()->value();
        this->repaintScript();
        ui->twScript->verticalScrollBar()->setValue(y);
    }
}
