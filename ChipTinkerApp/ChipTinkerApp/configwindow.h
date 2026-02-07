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

#ifndef CONFIGWINDOW_H
#define CONFIGWINDOW_H

#include <QDialog>
#include <QtGui>
#include <QSettings>
#include <QTableWidgetItem>
#include <../powertable.h>

namespace Ui {
class ConfigWindow;
}

class ConfigWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigWindow(QString cfgFileName, QString powerFileName, QWidget *parent = nullptr);
    ~ConfigWindow();

private slots:
    void on_btnBkgr_clicked();

    void on_btnDraw_clicked();

    void on_btnHi_clicked();

    void on_btnLo_clicked();

    void on_btnNC_clicked();

    void on_twPower_cellDoubleClicked(int row, int column);

    void on_pin_changed(int row);

    void on_buttonBox_accepted();

    void on_sbSwitchesCount_valueChanged(int arg1);

    void on_btnDefaultPower_clicked();

    void on_btnTxt_clicked();

    void on_btnErr_clicked();

    void on_btnAddExt_clicked();

    void on_btnRemoveExt_clicked();

    void on_twExtItems_itemChanged(QTableWidgetItem *item);

    void on_btnLogSelect_clicked();

    void on_btnRomSelect_clicked();

private:
    Ui::ConfigWindow *ui;

    QStringList extItems;
    QStringList extCalls;

    int PowerRoleToCode(QString role);
    QString PowerCodeToRole(int code);
    void repaintPower(PowerTable * pLUT);
    void repaintExtTools();
    PowerTable * pLUT;
    QString cfgFileName;
};

#endif // CONFIGWINDOW_H
