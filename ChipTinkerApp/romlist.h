//Copyright 2014,2015 MCbx, All rights reserved.
//http://mcbx.netne.net/ictester
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
#ifndef ROMLIST_H
#define ROMLIST_H
#include <QString>
#include <QList>

struct ROMModel {
    QString name;
    QString description;
    short pinsNo;    //-1 - category
    QList<short> GNDPins;
    QList<short> PowerPins;
    short bits;
    short words;
    QString algorithm;
    QString algorithmData;
};

class ROMList
{
public:
    ROMList(QString fileName);
    int saveToFile(QString fileName);
    QList<ROMModel> roms;
    void timeMark();
    QString generateListing(bool commandLine);
    void overwrite();
    ROMModel getByName(QString name);
private:
    QString fileName;
    QString modDate;
};

#endif // ROMLIST_H
