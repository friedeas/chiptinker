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


#include "powervisualizer.h"
#include "ui_powervisualizer.h"

//Power Visualizer
//Spawns window with information how to connect power.


PowerVisualizer::PowerVisualizer(QString sequence, int pinsCount, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PowerVisualizer)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Power settings"));
    this->scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(this->scene);
    this->scene->setSceneRect(0,0,ui->graphicsView->width()-5,ui->graphicsView->height()-5);
    QPixmap pixmap;
    pixmap.load(":/Resources/Power/socket.png");
    QGraphicsPixmapItem * item = this->scene->addPixmap(pixmap);
    item->setPos(50,30);

    //draw IC
    pinsCount=pinsCount/2;
    this->scene->addRect(175,315-pinsCount*16,61,(pinsCount*16)+3,QPen(QColor::fromRgb(0,0,0)),QBrush(QColor::fromRgb(200,200,200)));
    this->scene->addRect(200-5,315-pinsCount*16,20,10,QPen(QColor::fromRgb(0,0,0)),QBrush(QColor::fromRgb(180,180,180)));
    for (int i=0;i<pinsCount;i++)
    {   //draw pins
        this->scene->addRect(167,319-((i+1)*16),8,10,QPen(QColor::fromRgb(0,0,0)),QBrush(QColor::fromRgb(200,200,200)));
        this->scene->addRect(236,319-((i+1)*16),8,10,QPen(QColor::fromRgb(0,0,0)),QBrush(QColor::fromRgb(200,200,200)));
    }

    QString statusText="";
    QString powerPart=sequence.split(',')[0];
    if (!powerPart.contains('1')) statusText=tr("No DIP switches on. ");
    else statusText=tr("DIP switches ");

    //draw DIP switches
    for (int i=0;i<powerPart.length();i++)
    {
        //draw DIP skeleton
        this->scene->addRect(430,110+(i*25),80,25,QPen(QColor::fromRgb(0,0,100)),QBrush(QColor::fromRgb(0,0,180)));
        this->scene->addRect(450,113+(i*25),40,19,QPen(QColor::fromRgb(0,0,100)),QBrush(QColor::fromRgb(70,70,70)));
        //draw DIP text
        QGraphicsTextItem * num = new QGraphicsTextItem;
        num->setPlainText(QString::number(i+1));
        num->setRotation(90);
        QFont f = num->font();
        f.setBold(1);
        num->setFont(f);
        num->setDefaultTextColor(QColor::fromRgb(255,255,255));
        num->setPos(450,114+(i*25));
        this->scene->addItem(num);

        if (powerPart[i]=='1')
        {
            statusText+=QString::number(i+1)+" ";
            //draw DIP ON
                   this->scene->addRect(472,115+(i*25),17,15,QPen(QColor::fromRgb(0,0,0)),QBrush(QColor::fromRgb(240,240,240)));
        }
        else
        {
            //draw DIP OFF
                  this->scene->addRect(452,115+(i*25),17,15,QPen(QColor::fromRgb(0,0,0)),QBrush(QColor::fromRgb(240,240,240)));
        }
    }

    //put "ON" marking on DIPs
    QGraphicsTextItem * on = new QGraphicsTextItem;
    on->setPlainText(tr("ON"));
    on->setRotation(90);
    QFont f = on->font();
    f.setBold(1);
    on->setFont(f);
    on->setDefaultTextColor(QColor::fromRgb(255,255,255));
    on->setPos(511,110);
    this->scene->addItem(on);

    statusText+=tr("ON  ; ");

    //draw power connections
    powerPart=sequence.split(',')[1];
    if (powerPart.contains('1'))
    {
        statusText+=tr("IC pins to 5V: ");
        for (int i=0;i<powerPart.length();i++)
        {
            if (powerPart[i]=='1')
            {

                statusText+=QString::number((i+1))+" ";
                if ((i+1)>pinsCount)
                {

                    this->scene->addRect(355,65,4,245-((i-pinsCount)*16),QPen(QColor::fromRgb(180,0,0)),QBrush(QColor::fromRgb(180,0,0)));
                    this->scene->addRect(325,61+245-((i-pinsCount)*16),30,4,QPen(QColor::fromRgb(180,0,0)),QBrush(QColor::fromRgb(180,0,0)));
                }
                else
                {
                    //draw pass-over
                    this->scene->addRect(355,20,4,45,QPen(QColor::fromRgb(180,0,0)),QBrush(QColor::fromRgb(180,0,0)));
                    this->scene->addRect(20,20,335,4,QPen(QColor::fromRgb(180,0,0)),QBrush(QColor::fromRgb(180,0,0)));
                    //draw links
                    this->scene->addRect(20,20,4,110+((i+(12-pinsCount))*16),QPen(QColor::fromRgb(180,0,0)),QBrush(QColor::fromRgb(180,0,0)));
                    this->scene->addRect(20,20+110+((i+(12-pinsCount))*16),70,4,QPen(QColor::fromRgb(180,0,0)),QBrush(QColor::fromRgb(180,0,0)));
                }
            }
        }
    }
    statusText+=" ;  ";

    //draw GND connections
    powerPart=sequence.split(',')[2];
    if (powerPart.contains('1'))
    {
        statusText+=tr("IC pins to GND: ");
        for (int i=0;i<powerPart.length();i++)
        {
            if (powerPart[i]=='1')
            {
                statusText+=QString::number((i+1))+" ";
                if ((i+1)<=pinsCount)
                {
                    //draw links
                    this->scene->addRect(60,70,4,62+((i+(12-pinsCount))*16),QPen(QColor::fromRgb(0,180,0)),QBrush(QColor::fromRgb(0,180,0)));
                    this->scene->addRect(60,70+60+((i+(12-pinsCount))*16),30,4,QPen(QColor::fromRgb(0,180,0)),QBrush(QColor::fromRgb(0,180,0)));
                }
                else
                {
                    //draw pass-over
                    this->scene->addRect(60,70,4,35,QPen(QColor::fromRgb(0,180,0)),QBrush(QColor::fromRgb(0,180,0)));
                    this->scene->addRect(60,105,320,4,QPen(QColor::fromRgb(0,180,0)),QBrush(QColor::fromRgb(0,180,0)));
                    //draw links
                    this->scene->addRect(380,105,4,205-(i-pinsCount)*16,QPen(QColor::fromRgb(0,180,0)),QBrush(QColor::fromRgb(0,180,0)));
                    this->scene->addRect(323,105+205-(i-pinsCount)*16,60,4,QPen(QColor::fromRgb(0,180,0)),QBrush(QColor::fromRgb(0,180,0)));
                }

            }
        }
    }
    //update text
     ui->lbHint->setText(statusText);
}

PowerVisualizer::~PowerVisualizer()
{
    delete ui;
}
