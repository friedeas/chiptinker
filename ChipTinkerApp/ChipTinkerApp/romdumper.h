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

#ifndef ROMDUMPER_H
#define ROMDUMPER_H

#include <QDialog>
#include <QProcess>
#include <QByteArray>
#include "../devicedriver.h"
#include "../romlist.h"

namespace Ui {
class ROMDumper;
}

class ROMDumper : public QDialog
{
    Q_OBJECT

public:
    explicit ROMDumper(QString port, QString pLUTPath, QString settingsPath, int baudRate, QWidget *parent = nullptr);
    ~ROMDumper();
    void repaintBuffer();
    QString port;
    QString pLUTPath;
    bool powerDisplayer();
    int verify(ROMModel rom, bool verbose);
public slots:

private slots:
    void on_cbType_currentIndexChanged(const QString &arg1);

    void on_btnLoad_clicked();

    void on_btnSave_clicked();

    void on_btnClear_clicked();

    void on_btnExit_clicked();

    void on_btnRead_clicked();

    void on_btnVerify_clicked();

    void on_btnOpenIni_clicked();

    void on_pushButton_clicked();

    void on_btnGatesTest_clicked();

    void on_btnDataTest_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_btnUp_clicked();

    void on_btnDown_clicked();

    void on_btnAdd_clicked();

    void on_btnAddComment_clicked();

    void on_btnDel_clicked();

    void on_btnSaveAs_clicked();

    void on_lwItems_currentRowChanged(int currentRow);

    void on_btnApply_clicked();

private:
    Ui::ROMDumper *ui;
    QProcess *process;
    void controls(bool);
    int baudRate;
    DeviceDriver * tester;
    ROMList * roms;
    int readIni(QString fileName);
    QString iniPath;
    void repaintEditor();
    void refreshType();
    QFont defaultFont;
};

#endif // ROMDUMPER_H
