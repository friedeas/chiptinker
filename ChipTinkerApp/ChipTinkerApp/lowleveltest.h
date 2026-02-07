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

#ifndef LOWLEVELTEST_H
#define LOWLEVELTEST_H

#include <QDialog>
#include "../devicedriver.h"
#include <QString>

namespace Ui {
class LowLevelTest;
}

class LowLevelTest : public QDialog
{
    Q_OBJECT

public:
    explicit LowLevelTest(QString port, int baudRate, QWidget *parent = nullptr);
    ~LowLevelTest();

private slots:
    void on_btnConnect_clicked();

    void on_cbPowerOn_clicked(bool checked);

    void on_btnAllOn_clicked();

    void on_btnAllOff_clicked();

    void on_pin_changed();

private:
    Ui::LowLevelTest *ui;
    QString currentSituation;
    void displaySituation();
    DeviceDriver * tester;
    QString port;
    int baud;
};

#endif // LOWLEVELTEST_H
