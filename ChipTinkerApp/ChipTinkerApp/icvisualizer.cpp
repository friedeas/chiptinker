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


#include "icvisualizer.h"

//IC Visualizer
// This class draws visualization using data from parsed test sheet as base.


ICVisualizer::ICVisualizer(QObject *parent, QGraphicsView* canvas, int numOfPins, QString modelName)
{
    //set default colors
    this->canvas=canvas;
    this->DrwColor=QColor::fromRgb(0,0,0);
    this->BkgColor=QColor::fromRgb(230,230,230);
    this->NoColor=QColor::fromRgb(200,200,200);
    this->HiColor=QColor::fromRgb(255,65,65);
    this->LoColor=QColor::fromRgb(0,240,0);
    this->TxtColor=QColor::fromRgb(0,0,0);
    this->ErrColor=QColor::fromRgb(0,0,0);
    this->numOfPins=numOfPins;
    this->scene=new QGraphicsScene(parent);
    canvas->setScene(this->scene);
    scene->clear();
    this->ModelName=modelName;
    if (this->numOfPins==0)
    {
        scene->setBackgroundBrush(QBrush(this->NoColor));
    }
    else
    {
//        paint IC
        this->paintModel();
    }

}

int ICVisualizer::getNumOfPins() const
{
    return numOfPins;
}

void ICVisualizer::setNumOfPins(int value)
{
    numOfPins = value;
    this->paintModel();
}

void ICVisualizer::setIO(QString IO)
{
    //trim to numOfPins
    if (IO.length()>this->numOfPins)
    {
        IO=IO.left(IO.length()-(12-(this->numOfPins/2)));
        IO=IO.right(this->numOfPins);
    }
    this->pinsIO=IO;
    this->paintModel();
}

void ICVisualizer::setPinDescriptions(QStringList pinsDescr)
{
    this->pinsDescription=pinsDescr;
    this->paintModel();
}

void ICVisualizer::setPins(QString pinStatus)
{
    if (pinStatus.length()>this->numOfPins)
    {
        pinStatus=pinStatus.left(pinStatus.length()-(12-(this->numOfPins/2)));
        pinStatus=pinStatus.right(this->numOfPins);
    }
    this->pinsStatus=pinStatus;
    this->paintModel();
}

//paints actual model onto scene
void ICVisualizer::paintModel()
{
    this->scene->clear();
    if (this->numOfPins==0)
    {
        scene->setBackgroundBrush(QBrush(this->NoColor));
        return;
    }
    //paint IC skeleton

    this->scene->setBackgroundBrush(QBrush(this->BkgColor));

    //align horizontally scene in graph
    this->scene->addRect(0,0,1,1,QPen(this->BkgColor),QBrush(this->BkgColor));
    this->scene->addRect(269,0,1,1,QPen(this->BkgColor),QBrush(this->BkgColor));

    this->scene->addRect(70,30,120,6,QPen(this->DrwColor),QBrush(this->DrwColor));
    this->scene->addRect(110,36,40,15,QPen(this->DrwColor),QBrush(this->DrwColor));
    int caseHeight=40+(numOfPins/2)*20+((numOfPins/2)-1)*10;
    this->scene->addRect(70,30,6,caseHeight,QPen(this->DrwColor),QBrush(this->DrwColor));
    this->scene->addRect(184,30,6,caseHeight,QPen(this->DrwColor),QBrush(this->DrwColor));
    this->scene->addRect(70,26+caseHeight,120,6,QPen(this->DrwColor),QBrush(this->DrwColor));

    //paint summary
    QGraphicsTextItem * summary = new QGraphicsTextItem;
    summary->setDefaultTextColor(this->TxtColor);
    summary->setPlainText(this->Summary);
    int xxx=17;
    int yyy=46+caseHeight;
    if (this->summaryPosition==1) xxx=(130-(summary->boundingRect().width()/2));
    if (this->summaryPosition==2) xxx=240-summary->boundingRect().width();
    if (this->summaryPosition==3)
    {

        xxx=150-summary->boundingRect().height()/2;
        yyy=((46+caseHeight)/2)-(summary->boundingRect().width()/2);
        summary->setRotation(90);
    }
    summary->setPos(xxx,yyy);
    this->scene->addItem(summary);

    //paint name
     QGraphicsTextItem * title = new QGraphicsTextItem;
     title->setDefaultTextColor(this->TxtColor);
     title->setPlainText(this->ModelName);
     title->setPos((130-(title->boundingRect().width()/2)),0);
     this->scene->addItem(title);
    //paint labels and other things

    for (int i=0;i<numOfPins;i++)
    {
        QColor fillColor=this->NoColor;
        if (i+1>numOfPins/2)    //RIGHT-HAND HALF OF CHIP
        {
            int j=this->numOfPins-i;

            QGraphicsTextItem * io = new QGraphicsTextItem;
            QGraphicsTextItem * nu = new QGraphicsTextItem;
            io->setPos(190,(50+(j-1)*20+(j-1)*10));
            io->setDefaultTextColor(this->TxtColor);
            nu->setDefaultTextColor(this->TxtColor);
            nu->setPlainText(QString::number(i+1));

            QString pinLabel="";
            if (this->pinsIO.length()>=i+1)
            {
                if (pinsIO[i]=='0')
                    pinLabel+="<-- ";
                if (pinsIO[i]=='1')
                    pinLabel+="--> ";
                if (this->GNDPins.indexOf(i+1)>-1)
                    pinLabel="_";
                if (this->powerPins.indexOf(i+1)>-1)
                    pinLabel="*";
                if (this->NCPins.indexOf(i+1)>-1)
                    pinLabel="";
                if (this->ignoredPins.indexOf(i+1)>-1)
                    pinLabel="--X";
                if (this->pinsStatus.length()>=i+1)
                {
                    if ((pinsStatus[i]=='0')&&((pinsIO[i]=='1')||(pinsIO[i]=='0'))) fillColor=this->LoColor;
                    if ((pinsStatus[i]=='1')&&((pinsIO[i]=='1')||(pinsIO[i]=='0'))) fillColor=this->HiColor;
                }
            }
            this->scene->addRect(190,(50+(j-1)*20+(j-1)*10),50,20,QPen(this->DrwColor),QBrush(fillColor));
            io->setPlainText(pinLabel);
            nu->setPos(240-nu->boundingRect().width(),(50+(j-1)*20+(j-1)*10));
            this->scene->addItem(io);
            this->scene->addItem(nu);

            QGraphicsTextItem * de = new QGraphicsTextItem;
            de->setDefaultTextColor(this->TxtColor);
            if (this->pinsDescription.count()>=i+1)
            {
                de->setPlainText(this->pinsDescription[i]);
            }
            //5 pixels for margin
            de->setPos(185-de->boundingRect().width(),(50+(j-1)*20+(j-1)*10)); //50 not 80 - we've J
            this->scene->addItem(de);
            if ((this->ErrorMap.length()>=i+1)&&(this->ErrorMap[i]=='1'))
            {
                QGraphicsTextItem * err = new QGraphicsTextItem;
                err->setDefaultTextColor(this->ErrColor);
                err->setPlainText("!!!");
                if (this->ignoredPins.indexOf(i+1)>-1)
                {
                    err->setPlainText("...");
                }
                QFont ftmp = err->font();
                ftmp.setBold(1); err->setFont(ftmp);
                err->setPos(240,(50+(j-1)*20+(j-1)*10));
                this->scene->addItem(err);
            }

        }
        else    //LEFT-HAND HALF OF CHIP
        {
            QGraphicsTextItem * io = new QGraphicsTextItem;
            QGraphicsTextItem * nu = new QGraphicsTextItem;
            nu->setPos(20,(80+(i-1)*20+(i-1)*10));
            nu->setPlainText(QString::number(i+1));
            io->setDefaultTextColor(this->TxtColor);
            nu->setDefaultTextColor(this->TxtColor);

            QString pinLabel="";
            if (this->pinsIO.length()>=i+1)
            {
                if (pinsIO[i]=='0')
                    pinLabel+=" -->";
                if (pinsIO[i]=='1')
                    pinLabel+=" <--";
                if (this->GNDPins.indexOf(i+1)>-1)
                    pinLabel="_";
                if (this->powerPins.indexOf(i+1)>-1)
                    pinLabel="*";
                if (this->NCPins.indexOf(i+1)>-1)
                    pinLabel="";
                if (this->ignoredPins.indexOf(i+1)>-1)
                    pinLabel="X--";
                if (this->pinsStatus.length()>=i+1)
                {
                    if ((pinsStatus[i]=='0')&&((pinsIO[i]=='1')||(pinsIO[i]=='0'))) fillColor=this->LoColor;
                    if ((pinsStatus[i]=='1')&&((pinsIO[i]=='1')||(pinsIO[i]=='0'))) fillColor=this->HiColor;
                }
            }
            this->scene->addRect(20,(80+(i-1)*20+(i-1)*10),50,20,QPen(this->DrwColor),QBrush(fillColor));
            this->scene->addItem(nu);
            io->setPlainText(pinLabel);
            io->setPos(70-io->boundingRect().width(),(80+(i-1)*20+(i-1)*10));
            this->scene->addItem(io);

            QGraphicsTextItem * de = new QGraphicsTextItem;
            de->setPos(76,(80+(i-1)*20+(i-1)*10));
            de->setDefaultTextColor(this->TxtColor);
            if (this->pinsDescription.count()>=i+1)
            {
                de->setPlainText(this->pinsDescription[i]);
            }
            this->scene->addItem(de);
            if ((this->ErrorMap.length()>=i+1)&&(this->ErrorMap[i]=='1'))
            {
                QGraphicsTextItem * err = new QGraphicsTextItem;
                err->setDefaultTextColor(this->ErrColor);
                err->setPlainText("!!!");
                if (this->ignoredPins.indexOf(i+1)>-1)
                {
                    err->setPlainText("...");
                }
                QFont ftmp = err->font();
                ftmp.setBold(1); err->setFont(ftmp);
                err->setPos(0,(80+(i-1)*20+(i-1)*10));
                this->scene->addItem(err);
            }
        }
    }
    canvas->fitInView(scene->sceneRect(),Qt::KeepAspectRatio);
}

void ICVisualizer::repaintModel()
{
    //optional geometry checks
    this->paintModel();
}

QString ICVisualizer::getModelName() const
{
    return ModelName;
}

void ICVisualizer::setModelName(const QString &value)
{
    ModelName = value;
}


void ICVisualizer::setBkgColor(QColor color)
{
    this->BkgColor=color;
    this->paintModel();
}


void ICVisualizer::setHiColor(QColor color)
{
    this->HiColor=color;
    this->paintModel();
}

void ICVisualizer::setLoColor(QColor color)
{
    this->LoColor=color;
    this->paintModel();
}

void ICVisualizer::setDrwColor(QColor color)
{
    this->DrwColor=color;
    this->paintModel();
}

void ICVisualizer::setNoColor(QColor color)
{
    this->NoColor=color;
    this->paintModel();
}

void ICVisualizer::setTxtColor(QColor color)
{
    this->TxtColor=color;
    this->paintModel();
}

void ICVisualizer::setErrColor(QColor color)
{
    this->ErrColor=color;
    this->paintModel();
}

void ICVisualizer::clearErrorMap()
{
    this->ErrorMap="";
    this->paintModel();
}

void ICVisualizer::setGNDPins(QByteArray pins)
{
    this->GNDPins=pins;
}

void ICVisualizer::setPowerPins(QByteArray pins)
{
    this->powerPins=pins;
}

void ICVisualizer::setNCPins(QByteArray pins)
{
    this->NCPins=pins;
}

void ICVisualizer::setIgnoredPins(QByteArray pins)
{
    this->ignoredPins=pins;
}

//position: 0-left, 1-centre, 2-right, 3 - vertical
void ICVisualizer::setSummary(QString summary, int position)
{
    this->Summary=summary;
    this->summaryPosition=position;
    this->paintModel();
}

void ICVisualizer::setErrorMap(QString errors)
{
    if (errors.length()>this->numOfPins)
    {
        errors=errors.left(errors.length()-(12-(this->numOfPins/2)));
        errors=errors.right(this->numOfPins);
    }
    this->ErrorMap=errors;
    this->paintModel();
}
