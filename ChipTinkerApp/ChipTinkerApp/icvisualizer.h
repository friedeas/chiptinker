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

#ifndef ICVISUALIZER_H
#define ICVISUALIZER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QString>
#include <QStringList>

class ICVisualizer
{
public:
    ICVisualizer(QObject* parent, QGraphicsView *canvas, int numOfPins, QString modelName="");

    void setIO(QString IO);
    int getNumOfPins() const;
    void setNumOfPins(int value);
    void setPinDescriptions(QStringList pinsDescr);
    void setPowerPins(QByteArray pins);
    void setGNDPins(QByteArray pins);
    void setNCPins(QByteArray pins);
    void setIgnoredPins(QByteArray pins);

    void setPins(QString pinStatus);
    void setBkgColor(QColor color);
    void setHiColor(QColor color);
    void setLoColor(QColor color);
    void setDrwColor(QColor color);
    void setNoColor(QColor color);
    void setTxtColor(QColor color);
    void setErrColor(QColor color);
    QString getModelName() const;
    void setModelName(const QString &value);
    void repaintModel();

    void setErrorMap(QString errors);
    void clearErrorMap();
    //position: 0-left, 1-centre, 2-right, 3 - vertical
    void setSummary(QString summary,int position=1);
private:
    QGraphicsScene * scene;
    QGraphicsView* canvas;
    int numOfPins;
    QStringList pinsDescription;
    QString pinsIO;
    QString pinsStatus;
    void paintModel();
    QString ModelName;
    QColor BkgColor;
    QColor HiColor;
    QColor LoColor;
    QColor NoColor;
    QColor DrwColor;
    QColor TxtColor;
    QColor ErrColor;
    QString ErrorMap;
    QString Summary;
    int summaryPosition;
    QByteArray powerPins;
    QByteArray GNDPins;
    QByteArray NCPins;
    QByteArray ignoredPins;
};

#endif // ICVISUALIZER_H
