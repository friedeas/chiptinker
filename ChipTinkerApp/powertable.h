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

#ifndef POWERTABLE_H
#define POWERTABLE_H

#include <QString>
#include <QVector>
#include "testsheet.h"

class PowerTable
{
public:
    struct powerEntry
    {
        short number;
        short type;
        short pin;
    };

    PowerTable(QString tableFile);

    int saveTable();
    int getCount();
    QString checkModel(TestSheet model);
    int getEntryFunc(int i);
    int getEntryPin(int i);
    bool setEntry(powerEntry ent, int i);
    void revertToDefault();
    void resize(int len);
private:
    QString filePath;
    QVector <powerEntry> entries;
    void createDefaultTable();
};

#endif // POWERTABLE_H
