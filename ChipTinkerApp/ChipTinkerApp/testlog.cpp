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


#include "testlog.h"
#include <QFont>

//TestLog
//Draws a nice test log with icons, columns etc.

TestLog::TestLog(QTreeWidget * component, QWidget *parent) :
    QWidget(parent)
{
    this->noRefresh=0;
    //set-up widget do display data
    //two columns, one for icon and 4-digit number, one for description
    this->logObject=component;

    QStringList aaa;
    aaa<<tr("Step")<<tr("Description");
    this->logObject->setHeaderLabels(aaa);
    this->logObject->setRootIsDecorated(0);
    this->logObject->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->logObject->clear();
    this->logObject->setColumnWidth(0,70); //wartość z głupa

    //THIS IS IMPORTANT
    //This thing makes font monospace. Thanks to it 0 has the same width as 1
    //and user can compare values which are below each other.
    QFont f=this->logObject->font();
    f.setFamily("Monospace");
    f.setStyleHint(QFont::TypeWriter);
    this->logObject->setFont(f);
}

void TestLog::setNoRefresh(bool value)
{
    noRefresh = value;
}


void TestLog::AddMessage(int messageType, QString message, int stepNo)
{
    //we'll add message now
    QIcon entryIcon;
    switch (messageType)
    {
    case 1:
        entryIcon =QIcon(":/Resources/logIcons/reset.png");
        break;
    case 2:
        entryIcon = QIcon(":/Resources/logIcons/devErr.png");
        break;
    case 3:
        entryIcon = QIcon(":/Resources/logIcons/info.png");
        break;
    case 4:
        entryIcon = QIcon(":/Resources/logIcons/power.png");
        break;
    case 5:
        entryIcon = QIcon(":/Resources/logIcons/io.png");
        break;
    case 6:
        entryIcon = QIcon(":/Resources/logIcons/data.png");
        break;
    case 7:
        entryIcon = QIcon(":/Resources/logIcons/read.png");
        break;
    case 8:
        entryIcon = QIcon(":/Resources/logIcons/compErr.png");
        break;
    case 9:
        entryIcon = QIcon(":/Resources/logIcons/failed.png");
        break;
    case 10:
        entryIcon = QIcon(":/Resources/logIcons/passed.png");
        break;
    default:
        entryIcon = QIcon(); //blank
        break;
    }
        QTreeWidgetItem *entry=new QTreeWidgetItem();
        entry->setIcon(0,entryIcon);
        if (stepNo>-1) entry->setText(0,QString::number(stepNo).rightJustified(4,'0'));
        entry->setText(1,message);
        if (!this->noRefresh) this->logObject->addTopLevelItem(entry);
        this->logObject->scrollToBottom();
}
