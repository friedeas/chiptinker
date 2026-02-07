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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "icvisualizer.h"
#include "testlog.h"
#include <../powertable.h>
#include <../testsheet.h>
#include <../devicedriver.h>
#include <QColor>
#include <QString>
#include <QLabel>
#include <QTranslator>
#include <QActionGroup>
#include <QProcess>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

protected:
    void changeEvent(QEvent*);

protected slots:
    void slotLanguageChanged(QAction* action);
    void slotExternalLaunched(QAction* action);
    void slotToolEndsExec(int,QProcess::ExitStatus);

public:
    explicit MainWindow(QStringList arguments, QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void repaintOnDisplay() { this->visualization->repaintModel(); }

private slots:
    void on_btnStart_clicked();
    void on_btnStop_clicked();
    void on_actionExit_triggered();
    void on_actionOpen_model_triggered();
    void on_actionShow_power_requirements_triggered();
    void on_actionCopy_log_to_clipboard_triggered();
    void on_actionDevice_reset_triggered();
    void on_actionConfiguration_triggered();
    void on_actionIdentify_device_triggered();
    void on_mru_used();
    void on_actionClear_history_triggered();
    void on_actionLow_level_interface_test_triggered();
    void on_actionEdit_current_test_sheet_triggered();
    void on_actionCreate_new_model_triggered();
    void on_actionAbout_triggered();
    void on_actionROM_dump_triggered();
    void on_actionIgnore_pins_triggered();
    void on_actionClear_log_triggered();
    void on_actionIC_identifier_triggered();
    void on_actionTest_series_triggered();
    void StartThing();

private:
    void closeEvent(QCloseEvent *clsEv);
    Ui::MainWindow *ui;
    ICVisualizer * visualization;
    TestLog * testLog;
    QString pLUTPath;
    TestSheet * ICModel;
    QString commPort;
    int baudRate;
    int timeout;
    int stepDelay;
    QColor BkgColor;
    QColor HiColor;
    QColor LoColor;
    QColor NoColor;
    QColor DrwColor;
    QColor TxtColor;
    QColor ErrColor;
    QStringList autoFillSettings;
    QString settingsPath;
    QFont defaultFont; //this is default for both ROM diag and log
    bool StartButtonNeeded;
    void saveSettings();
    void loadSettings();
    void loadModel(QString fileName);
    void enableEverything();
    bool isRunning;
    QLabel * statusText;
    bool hardwareError(int code, int step, DeviceDriver *devToReset);
    QStringList MRU;
    void refreshMRU();
    QLabel * lbSkippingInfo;
    void loadLanguage(const QString& rLanguage);
    void createLanguageMenu(void);
    QTranslator m_translator; // contains the translations for this application
    QTranslator m_translatorQt; // contains the translations for qt
    QString m_currLang; // contains the currently loaded language
    QActionGroup* langGroup;
    QActionGroup* extGroup; //external tools
    QString lang_override; //override system language
    bool clearLog;
    bool warnUnstable; //should it warn if using suspicious features like IC identifier
    bool testInSeries; //does it run in series mode?
    QProcess * executing; //process for executing external tools.
};

#endif // MAINWINDOW_H
