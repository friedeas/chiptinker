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

#ifndef TESTSHEET_H
#define TESTSHEET_H

#include <qstring.h>
#include <QVector>
#include <QStringList>

class TestSheet
{
public:
    struct pinDesignation
    {
        QString pinTag;
        int pinType;
    };
    struct testCommand
    {
        int cmd;
        QString arg;
    };

    TestSheet(QString name, QString description, int numOfPins);
    TestSheet(QString fileName);

    QString getName() const;
    void setName(const QString &value);
    QString getDescription() const;
    void setDescription(const QString &value);
    int getNumOfPins() const;
    void setNumOfPins(int value);

    int saveToFile(QString fileName);
    QByteArray getPowerPins();
    QByteArray getGNDPins();

    //load from file
    //save to file
    QString initIO(bool visualization=false);
    QString compareStep(int step, QString data);

    QVector <pinDesignation> pins;
    QVector <testCommand> script;

    QStringList getPinsDescriptions();
    QString getFN();
private:
    QString name;
    QString description;
    int numOfPins;
    QString fileName;

};

#endif // TESTSHEET_H
