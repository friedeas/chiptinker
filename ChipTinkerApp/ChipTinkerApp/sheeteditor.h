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

#ifndef SHEETEDITOR_H
#define SHEETEDITOR_H

#include <QDialog>
#include "../testsheet.h"
#include <QString>
#include <QKeyEvent>
#include <QTableWidgetItem>

namespace Ui {
class SheetEditor;
}

class SheetEditor : public QDialog
{
    Q_OBJECT

public:
    explicit SheetEditor(TestSheet *sheet, QStringList AutoSettings, QWidget *parent = nullptr);
    ~SheetEditor();

protected:
    void keyPressEvent(QKeyEvent * event);

private slots:
    void on_leName_textChanged(const QString &arg1);

    void on_leDescription_textChanged(const QString &arg1);

    void on_sbPinsNo_valueChanged1(int arg1);

    void on_twFirst_cellDoubleClicked(int row, int column);

    void on_twSecond_cellDoubleClicked(int row, int column);

    void on_twFirst_changed(int row, int column);

    void on_twSecond_changed(int row, int column);
    void on_twFirst_cellClicked();

    void on_twSecond_cellClicked();

    void on_btnInsertReset_clicked();

    void on_btnInsertPower_clicked();

    void on_btnInsertConfig_clicked();

    void on_btnInsertSend_clicked();

    void on_btnInsertRead_clicked();

    void on_btnLineDelete_clicked();

    void on_btnLineToBottom_clicked();

    void on_BtnLineToTop_clicked();

    void on_btnLineDown_clicked();

    void on_btnLineUp_clicked();

    void on_twScript_cellDoubleClicked(int row, int column);

    void on_pushButton_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_btnDRC_clicked();

    void on_twScript_itemEntered(QTableWidgetItem *item);

    void on_btnInsertcomment_clicked();

    void on_twScript_itemChanged(QTableWidgetItem *item);

private:
    void done(int r);
    Ui::SheetEditor *ui;
    TestSheet * sheet;
    void repaintPins();
    void repaintScript();
    QString CodeToStr(int code);
    int strToCode(QString str);
    QString CommandToStr(int command);
    int StrToCommand(QString command);
    QStringList AutoSettings;
};

#endif // SHEETEDITOR_H
