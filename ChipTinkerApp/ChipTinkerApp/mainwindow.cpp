//Copyright 2014,2015..2021 MCbx, All rights reserved.
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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "modelselector.h"
#include "powervisualizer.h"
#include "configwindow.h"
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QDateTime>
#include <QFile>
#include <QSettings>
#include <../powertable.h>
#include <../devicedriver.h>
#include <QClipboard>
#include <QElapsedTimer>
#include "lowleveltest.h"
#include "sheeteditor.h"
#include "romdumper.h"
#include <QInputDialog>
#include <QTranslator>
#include <QActionGroup>
#include <QFileInfo>
#include <QProcess>
#include "identifier.h"
#include <QToolButton>

MainWindow::MainWindow(QStringList arguments, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
   // ui->retranslateUi(this);
    this->ICModel=nullptr;
    this->isRunning=0;
    this->clearLog=1;
    this->testInSeries=0;

    this->settingsPath = "ictestrc.ini";
    #ifndef Q_OS_WIN32
    //and now we need to do a few things here. With version 201909xx we will need to move configfile
    //from ~/.ictestrc.ini to ~/.config/.ictestrc.ini IF IT DOES NOT EXIST.
    QFileInfo newfile(QDir::homePath()+"/.config/.ictestrc.ini");
    QFileInfo oldfile(QDir::homePath()+"/.ictestrc.ini");
    if ((!newfile.exists())&&(oldfile.exists()))
    {
        QFile::copy(QDir::homePath()+"/.ictestrc.ini", QDir::homePath()+"/.config/.ictestrc.ini");
    }
    this->settingsPath = QDir::homePath()+"/.config/.ictestrc.ini";
    #endif

    //window make-up
    this->createLanguageMenu();

    MainWindow::setWindowTitle("ChipTinker App");
    MainWindow::setWindowIcon(QIcon(":/Resources/logIcons/read.png"));
    testLog = new TestLog(ui->twTestLog,nullptr);
    visualization = new ICVisualizer(nullptr,ui->gvICPreview,0,"");
    ui->cbRealTimeTrace->setVisible(0);
    ui->cbUpdateTrace->setVisible(0);
    statusText=new QLabel("ChipTinkerApp ");
    ui->statusBar->addWidget(statusText);

    //get LUT path
    this->pLUTPath = ".icpower.rc";
    #ifndef Q_OS_WIN32

    //and now we have to do another transition to .config
    QFileInfo newfile1(QDir::homePath()+"/.config/.icpower.rc");
    QFileInfo oldfile1(QDir::homePath()+"/.icpower.rc");
    if ((!newfile1.exists())&&(oldfile1.exists()))
    {
        QFile::copy(QDir::homePath()+"/.icpower.rc", QDir::homePath()+"/.config/.icpower.rc");
    }

    this->pLUTPath = QDir::homePath()+"/.config/.icpower.rc";
    #endif

    this->defaultFont=ui->twTestLog->font(); //initialize default log font to comeback if needed.
    this->StartButtonNeeded=true; //don't we have a "Start" on toolbar?

    //get settings from configuration file
    this->loadSettings();

    //Status bar: Skipped pins
    lbSkippingInfo = new QLabel (tr("Not ignoring any pins."));
    lbSkippingInfo->setAlignment(Qt::AlignLeft);
    lbSkippingInfo->setMinimumSize(lbSkippingInfo->sizeHint());
    ui->statusBar->addPermanentWidget(lbSkippingInfo,0);

    //Refresh MRU list
    this->refreshMRU();
    QObject::connect(ui->MRU0,SIGNAL(triggered()),this,SLOT(on_mru_used()));
    QObject::connect(ui->MRU1,SIGNAL(triggered()),this,SLOT(on_mru_used()));
    QObject::connect(ui->MRU2,SIGNAL(triggered()),this,SLOT(on_mru_used()));
    QObject::connect(ui->MRU3,SIGNAL(triggered()),this,SLOT(on_mru_used()));
    QObject::connect(ui->MRU4,SIGNAL(triggered()),this,SLOT(on_mru_used()));
    QObject::connect(ui->MRU5,SIGNAL(triggered()),this,SLOT(on_mru_used()));
    QObject::connect(ui->MRU6,SIGNAL(triggered()),this,SLOT(on_mru_used()));
    QObject::connect(ui->MRU7,SIGNAL(triggered()),this,SLOT(on_mru_used()));
    QObject::connect(ui->MRU8,SIGNAL(triggered()),this,SLOT(on_mru_used()));
    QObject::connect(ui->MRU9,SIGNAL(triggered()),this,SLOT(on_mru_used()));

    //Toolbar make-up
    ui->mainToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    ui->menuBar->setContextMenuPolicy(Qt::PreventContextMenu);

    //We're ready to go
    QString platform=tr(" On ");
    #ifdef Q_OS_WIN
        platform+="Windows";
    #endif
    #ifdef Q_OS_LINUX
        platform+="Linux";
    #endif
    #ifdef Q_OS_MAC
        platform+="Mac OS";
    #endif
    #ifdef Q_OS_HAIKU
        platform+="Haiku";
    #endif
    if (platform==tr(" On "))
    {
        #ifdef Q_OS_UNIX
            platform+="Unix ";
        #endif
        platform+="Unknown";
    }
        platform+=tr("\nPlatform: ")+QSysInfo::currentCpuArchitecture()+tr(" Build: ")+QSysInfo::buildCpuArchitecture()+tr(" System Locale: ")+QLocale::system().name();
    testLog->AddMessage(TestLog::Informative,tr("ChipTinker App started, ")+QDate::currentDate().toString("yyyy-MM-dd")+" "+QTime::currentTime().toString("HH:mm:ss")+platform);
    //Parse command-line arguments
    if (arguments.count()==2)
    {
        if (arguments[1]=="-test")
        {
            this->on_actionLow_level_interface_test_triggered();
            exit(0);
        }
        if (arguments[1]=="-romread")
        {
            this->on_actionROM_dump_triggered();
            exit(0);
        }
        if (arguments[1]=="-reset")
        {
            this->on_actionDevice_reset_triggered();
            exit(0);
        }

        //handle file name
        if (QFile::exists(arguments[1]))
        {
            loadModel(arguments[1]);
            lbSkippingInfo->setText(tr("Not ignoring any pins."));
            QTimer::singleShot(0, this, SLOT(repaintOnDisplay()));    //crude hack sorry

        } else
        {
            QMessageBox::information(nullptr,tr("Error"),tr("File ")+arguments[1]+tr(" does not exist!"));
            exit(1);
        }
    }
}


////////////////////////////////
///   INTERNATIONALIZATION   ///
////////////////////////////////

void MainWindow::createLanguageMenu(void)
{
     QSettings settings(this->settingsPath,QSettings::IniFormat);
     settings.beginGroup("GUI");
     this->lang_override=settings.value("Lang","AUTO").toString();
     settings.endGroup();

     langGroup = new QActionGroup(ui->menuLanguage);
     QObject::connect(this->langGroup, SIGNAL(triggered(QAction*)),this,SLOT(slotLanguageChanged(QAction*)));

     langGroup->setExclusive(true);

     connect(langGroup, SIGNAL (triggered(QAction *)), this, SLOT (slotLanguageChanged(QAction *)));

     // format systems language
     QString defaultLocale = QLocale::system().name();


     QStringList fileNames = QDir(":/i18n").entryList();
     fileNames.push_back("__en_US.");

     for (int i = 0; i < fileNames.size(); ++i)
     {
          // get locale extracted by filename
          QString locale;
          locale = fileNames[i];
          locale.truncate(locale.lastIndexOf('.'));
          locale.remove(0, locale.indexOf('_') + 1);
          locale.remove(0, locale.indexOf('_') + 1);

         QString lang = QLocale::languageToString(QLocale(locale).language());

         QAction *action = new QAction(lang, this);
         action->setCheckable(true);
         action->setData(locale);

         ui->menuLanguage->addAction(action);
         langGroup->addAction(action);

         // set default translators and language checked

             if (((this->lang_override=="AUTO")&&(defaultLocale == locale))||(this->lang_override==locale))
             {
                action->setChecked(true);
                this->slotLanguageChanged(action);
             }


     }
}

void MainWindow::slotLanguageChanged(QAction* action)
{
     if(nullptr != action)  //changing 0 to nullptr because compiler screeched about it. This is nevertheless
     {                      //the monstrous hack and I'm not sure do we even need this
        // load the language dependant on the action content
        loadLanguage(action->data().toString());
     }
}

void switchTranslator(QTranslator& translator, const QString& filename)
{
     qApp->removeTranslator(&translator);
     if(translator.load(filename))
        qApp->installTranslator(&translator);
}

void MainWindow::loadLanguage(const QString& rLanguage)
{
     if(m_currLang != rLanguage)
     {
         if (rLanguage=="en_US")
         {
            m_currLang = rLanguage;
            qApp->removeTranslator(&m_translator);
            switchTranslator(m_translatorQt, QString("qt_%1.qm").arg(rLanguage));
            ui->retranslateUi(this);
         }
         else
         {
              m_currLang = rLanguage;
              QLocale locale = QLocale(m_currLang);
              QLocale::setDefault(locale);
              switchTranslator(m_translator, QString(":/i18n/_QtICTester_%1.qm").arg(rLanguage));
              switchTranslator(m_translatorQt, QString("qt_%1.qm").arg(rLanguage));
         }
     }
}

void MainWindow::changeEvent(QEvent* event)
{
     if(nullptr != event)
     {
           // this event is send if a translator is loaded
          if (event->type()==QEvent::LanguageChange)
          {
              ui->retranslateUi(this);
          }

           // this event is send, if the system, language changes
          if (event->type()==QEvent::LocaleChange)
          {
                QString locale = QLocale::system().name();
                locale.truncate(locale.lastIndexOf('_'));
                loadLanguage(locale);
          }
     }
     QMainWindow::changeEvent(event);
}

/////////////////////////////////
/// END INTERNATIONALIZATION  ///
/////////////////////////////////

void MainWindow::loadSettings()
{
    //Load settings from file
    QSettings settings(this->settingsPath,QSettings::IniFormat);
    settings.beginGroup("Device");
    this->commPort=settings.value("Port","COM1").toString();
    #ifndef Q_OS_WIN32
    this->commPort=settings.value("Port","ttyS0").toString();
    #endif
    this->baudRate=settings.value("Rate","19200").toInt();
    this->timeout=settings.value("TimeOut","1000").toInt();
    settings.endGroup();

    settings.beginGroup("GUI");
    int winX=MainWindow::width();
    int winY=MainWindow::height();
    winX=settings.value("Width",QString::number(winX)).toInt();
    winY=settings.value("Height",QString::number(winY)).toInt();
    MainWindow::resize(winX,winY);
    this->lang_override=settings.value("Lang","AUTO").toString();
    this->clearLog=settings.value("ClearLog",1).toBool();
    QString a=settings.value("LogFont","").toString();
    if (a!="")
    {
        QFont j(a);
        j.fromString(a); //this must be done in these two steps, no way set(QFont(a)).
        ui->twTestLog->setFont(j);
    }
    else
    {
        ui->twTestLog->setFont(this->defaultFont);
    }
    int tbSize=settings.value("ToolbarSize",24).toInt();
    int prevSize=ui->mainToolBar->iconSize().height();

    ui->mainToolBar->setIconSize(QSize(tbSize,tbSize));
    if (tbSize < prevSize)
    {
        MainWindow::setGeometry(MainWindow::x(),MainWindow::y(),MainWindow::width(),MainWindow::height()-(prevSize-tbSize));
    }
    if (tbSize > prevSize)
    {
        MainWindow::setGeometry(MainWindow::x(),MainWindow::y(),MainWindow::width(),MainWindow::height()+(tbSize-prevSize));
    }

    bool StartStopTB=settings.value("StartStopTB","false").toBool();
    if ((StartStopTB)&&(this->StartButtonNeeded))
    {
        QWidget *spacer = new QWidget(this);
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        spacer->setVisible(true);
        ui->mainToolBar->addWidget(spacer);

        QAction * Starter = new QAction("Start",this);
        Starter->setIcon(QIcon(":/Resources/logIcons/read.png"));
        Starter->setText("Start");
        connect(Starter,SIGNAL(triggered()),this,SLOT(StartThing()));

        //We need a nice text-icon combo, let's try the TDE trick
        QToolButton * tb = new QToolButton();
        tb->setDefaultAction(Starter);
        tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        ui->mainToolBar->addWidget(tb);
        this->StartButtonNeeded=false;
    }

    settings.endGroup();

    settings.beginGroup("Test");
    ui->cbUpdatePins->setChecked(settings.value("UpdatePins",1).toBool());
    ui->cbUpdateLog->setChecked(settings.value("UpdateLog",1).toBool());
    ui->cbTestFlight->setChecked(settings.value("ReadAfterSend","0").toBool());
    ui->rbRepeat->setChecked(settings.value("Repeat","0").toBool());
    ui->cbAbortOnError->setChecked(settings.value("AbortOnError","1").toBool());
    ui->cbSingleStep->setChecked(settings.value("StepByStep","0").toBool());
    ui->sbRepetitions->setValue(settings.value("Repeats","100").toInt());
    this->stepDelay=settings.value("StepDelay","2").toInt();
    this->warnUnstable=settings.value("warnUnstable",1).toBool();
    settings.endGroup();

    settings.beginGroup("AutoSettings");
    this->autoFillSettings.append(settings.value("SetAuto",1).toString());
    this->autoFillSettings.append(settings.value("OutputAuto","Q?; Q??").toString());
    this->autoFillSettings.append(settings.value("GndAuto","GND; Vss").toString());
    this->autoFillSettings.append(settings.value("PowerAuto","Vcc").toString());
    this->autoFillSettings.append(settings.value("NcAuto","NC").toString());
    settings.endGroup();

    settings.beginGroup("ICColors");
    int r,g,b;
    r=settings.value("BkgR","230").toInt();
    g=settings.value("BkgG","230").toInt();
    b=settings.value("BkgB","230").toInt();
    this->BkgColor=QColor::fromRgb(r,g,b);
    r=settings.value("HiR","255").toInt();
    g=settings.value("HiG","65").toInt();
    b=settings.value("HiB","65").toInt();
    this->HiColor=QColor::fromRgb(r,g,b);
    r=settings.value("LowR","0").toInt();
    g=settings.value("LowG","240").toInt();
    b=settings.value("LowB","0").toInt();
    this->LoColor=QColor::fromRgb(r,g,b);
    r=settings.value("NoR","200").toInt();
    g=settings.value("NoG","200").toInt();
    b=settings.value("NoB","200").toInt();
    this->NoColor=QColor::fromRgb(r,g,b);
    r=settings.value("DrawR","0").toInt();
    g=settings.value("DrawG","0").toInt();
    b=settings.value("DrawB","0").toInt();
    this->DrwColor=QColor::fromRgb(r,g,b);
    r=settings.value("TextR","0").toInt();
    g=settings.value("TextG","0").toInt();
    b=settings.value("TextB","0").toInt();
    this->TxtColor=QColor::fromRgb(r,g,b);
    r=settings.value("ErrR","0").toInt();
    g=settings.value("ErrG","0").toInt();
    b=settings.value("ErrB","0").toInt();
    this->ErrColor=QColor::fromRgb(r,g,b);
    settings.endGroup();

    settings.beginGroup("MRU");
    for (int i=0;i<10;i++)
    {
        QString item=settings.value("Item"+QString::number(i+1),"").toString();
        this->MRU.push_back(item);
    }
    settings.endGroup();

    visualization->setBkgColor(this->BkgColor);
    visualization->setHiColor(this->HiColor);
    visualization->setLoColor(this->LoColor);
    visualization->setNoColor(this->NoColor);
    visualization->setDrwColor(this->DrwColor);
    visualization->setErrColor(this->ErrColor);
    visualization->setTxtColor(this->TxtColor);

    //external software set-up
    ui->menuExternal_tools->clear();
    settings.beginGroup("ExternalSoftware");
    int q=settings.value("ProgramsCount",0).toInt();
    extGroup = new QActionGroup(ui->menuExternal_tools);
    QObject::connect(this->extGroup, SIGNAL(triggered(QAction*)),this,SLOT(slotExternalLaunched(QAction*)));
    for (int i=0;i<q;i++)
    {
        QString itemName=settings.value("Name"+QString::number(i),"").toString();
        QString itemCommand=settings.value("Command"+QString::number(i),"").toString();

        QAction *action = new QAction(itemName,this);
        action->setData(itemCommand);
        ui->menuExternal_tools->addAction(action);
        extGroup->addAction(action);
    }
    settings.endGroup();
}

//Start the test ordinary way
void MainWindow::StartThing()
{
    if (ui->btnStart->isEnabled())
    {
        this->on_btnStart_clicked();
    }
}

//Example of VERY bad procedure. Well, in CPPB it was easier to implement MRU.
void MainWindow::refreshMRU()
{
    while (MRU.length()<10) MRU.push_back(""); //rebuild MRU after history cleaning

    if (this->MRU[0]!="")
    {
        ui->MRU1->setText(QFileInfo(MRU[0]).fileName());
        ui->MRU1->setData(MRU[0]);
        ui->MRU1->setVisible(1);
    }
    else ui->MRU1->setVisible(0);
    if (this->MRU[1]!="")
    {
        ui->MRU2->setText(QFileInfo(MRU[1]).fileName());
        ui->MRU2->setData(MRU[1]);
        ui->MRU2->setVisible(1);
    }
    else ui->MRU2->setVisible(0);
    if (this->MRU[2]!="")
    {
        ui->MRU3->setText(QFileInfo(MRU[2]).fileName());
        ui->MRU3->setData(MRU[2]);
        ui->MRU3->setVisible(1);
    }
    else ui->MRU3->setVisible(0);
    if (this->MRU[3]!="")
    {
        ui->MRU4->setText(QFileInfo(MRU[3]).fileName());
        ui->MRU4->setData(MRU[3]);
        ui->MRU4->setVisible(1);
    }
    else ui->MRU4->setVisible(0);
    if (this->MRU[4]!="")
    {
        ui->MRU5->setText(QFileInfo(MRU[4]).fileName());
        ui->MRU5->setData(MRU[4]);
        ui->MRU5->setVisible(1);
    }
    else ui->MRU5->setVisible(0);
    if (this->MRU[5]!="")
    {
        ui->MRU6->setText(QFileInfo(MRU[5]).fileName());
        ui->MRU6->setData(MRU[5]);
        ui->MRU6->setVisible(1);
    }
    else ui->MRU6->setVisible(0);
    if (this->MRU[6]!="")
    {
        ui->MRU7->setText(QFileInfo(MRU[6]).fileName());
        ui->MRU7->setData(MRU[6]);
        ui->MRU7->setVisible(1);
    }
    else ui->MRU7->setVisible(0);
    if (this->MRU[7]!="")
    {
        ui->MRU8->setText(QFileInfo(MRU[7]).fileName());
        ui->MRU8->setData(MRU[7]);
        ui->MRU8->setVisible(1);
    }
    else ui->MRU8->setVisible(0);
    if (this->MRU[8]!="")
    {
        ui->MRU9->setText(QFileInfo(MRU[8]).fileName());
        ui->MRU9->setData(MRU[8]);
        ui->MRU9->setVisible(1);
    }
    else ui->MRU9->setVisible(0);
    if (this->MRU[9]!="")
    {
        ui->MRU0->setText(QFileInfo(MRU[9]).fileName());
        ui->MRU0->setData(MRU[9]);
        ui->MRU0->setVisible(1);
    }
    else ui->MRU0->setVisible(0);
}

void MainWindow::on_mru_used()
{
    QAction *action = qobject_cast<QAction *>(sender());
      if (action)
          loadModel(action->data().toString());
}

//external program specified from config has been launched
void MainWindow::slotExternalLaunched(QAction *action)
{
    QString command=action->data().toString();
    if (command=="")
    {
        return;
    }
    //replace templates
    command.replace("$P",this->commPort);
    command.replace("$B",QString::number(this->baudRate));
    command.replace("$T",QString::number(this->timeout));


    this->executing = new QProcess();
    connect(this->executing,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(slotToolEndsExec(int,QProcess::ExitStatus)));
    this->executing->setProperty("reset","0"); //I consider this an ugly hack, to send data this way.


    //act when reset by the end of tool is activated
    if (command.at(command.length()-1)=='!')
    {
        this->executing->setProperty("reset","1");  //ugly hack part zwei
        command=command.left(command.length()-1);
    }

    QString message=tr("Launching external software ");
    if (this->executing->property("reset")=="1")
    {
        message+=tr("with reset");
    }
    message+="\n"+command;
    testLog->AddMessage(TestLog::Informative,message);
    QApplication::processEvents();

    this->executing->start(command);
   // executing.waitForFinished(-1);
}

//An external tool launched from menu has ended its run
void MainWindow::slotToolEndsExec(int code, QProcess::ExitStatus)
{
    testLog->AddMessage(TestLog::Informative,tr("Exit with code ")+QString::number(code));
    if (QObject::sender()->property("reset")=="1") //ugly hack part drei, und final
    {
        this->on_actionDevice_reset_triggered();
    }
}

void MainWindow::saveSettings()
{
    //save settings
    QSettings settings(this->settingsPath,QSettings::IniFormat);
    settings.beginGroup("Device");
    settings.setValue("Port",this->commPort);
    settings.setValue("Rate",QString::number(this->baudRate));
    settings.setValue("TimeOut",QString::number(this->timeout));
    settings.endGroup();

    settings.beginGroup("GUI");
    settings.setValue("Width",QString::number(MainWindow::width()));
    settings.setValue("Height",QString::number(MainWindow::height()));
    settings.setValue("Lang",this->lang_override);
    settings.setValue("ClearLog",this->clearLog);
    settings.endGroup();

    settings.beginGroup("Test");
    settings.setValue("UpdatePins",QString::number(ui->cbUpdatePins->isChecked()));
    settings.setValue("UpdateLog",QString::number(ui->cbUpdateLog->isChecked()));
    settings.setValue("ReadAfterSend",QString::number(ui->cbTestFlight->isChecked()));
    settings.setValue("Repeat",QString::number(ui->rbRepeat->isChecked()));
    settings.setValue("AbortOnError",QString::number(ui->cbAbortOnError->isChecked()));
    settings.setValue("StepByStep",QString::number(ui->cbSingleStep->isChecked()));
    settings.setValue("Repeats",QString::number(ui->sbRepetitions->value()));
    settings.setValue("StepDelay",QString::number(this->stepDelay));
    settings.endGroup();

    settings.beginGroup("ICColors");
    settings.setValue("BkgR",QString::number(this->BkgColor.red()));
    settings.setValue("BkgG",QString::number(this->BkgColor.green()));
    settings.setValue("BkgB",QString::number(this->BkgColor.blue()));
    settings.setValue("HiR",QString::number(this->HiColor.red()));
    settings.setValue("HiG",QString::number(this->HiColor.green()));
    settings.setValue("HiB",QString::number(this->HiColor.blue()));
    settings.setValue("LowR",QString::number(this->LoColor.red()));
    settings.setValue("LowG",QString::number(this->LoColor.green()));
    settings.setValue("LowB",QString::number(this->LoColor.blue()));
    settings.setValue("NoR",QString::number(this->NoColor.red()));
    settings.setValue("NoG",QString::number(this->NoColor.green()));
    settings.setValue("NoB",QString::number(this->NoColor.blue()));
    settings.setValue("DrawR",QString::number(this->DrwColor.red()));
    settings.setValue("DrawG",QString::number(this->DrwColor.green()));
    settings.setValue("DrawB",QString::number(this->DrwColor.blue()));
    settings.setValue("TextR",QString::number(this->TxtColor.red()));
    settings.setValue("TextG",QString::number(this->TxtColor.green()));
    settings.setValue("TextB",QString::number(this->TxtColor.blue()));
    settings.setValue("ErrR",QString::number(this->ErrColor.red()));
    settings.setValue("ErrG",QString::number(this->ErrColor.green()));
    settings.setValue("ErrB",QString::number(this->ErrColor.blue()));
    settings.endGroup();

    settings.beginGroup("MRU");
    for (int i=0;i<10;i++)
    {
        settings.setValue("Item"+QString::number(i+1),this->MRU[i]);
    }
    settings.endGroup();

}

void MainWindow::loadModel(QString fileName)
{
    if (!QFile(fileName).exists())
    {
        QMessageBox::information(nullptr,tr("Error"),tr("File does not exist!"));
        if (this->MRU.contains(fileName))
        {
            this->MRU.removeOne(fileName);
            this->refreshMRU();
        }
        return;
    }
    else
    {
        if (!this->MRU.contains(fileName))
        {
            if (this->MRU.count()==10) this->MRU.removeLast();
        }
        else
        {
            this->MRU.removeOne(fileName);
        }
        this->MRU.push_front(fileName);

        this->refreshMRU();
    }

    delete this->ICModel;  //deallocate old model
    delete visualization;                                           //BUGFIX
    visualization = new ICVisualizer(nullptr,ui->gvICPreview,0,"");    //To make model always sit in the center
    visualization->setBkgColor(this->BkgColor);
    visualization->setHiColor(this->HiColor);
    visualization->setLoColor(this->LoColor);
    visualization->setNoColor(this->NoColor);
    visualization->setDrwColor(this->DrwColor);
    visualization->setErrColor(this->ErrColor);
    visualization->setTxtColor(this->TxtColor);

    visualization->setErrorMap("");
    visualization->setPins("");
    visualization->setNumOfPins(0); //turns visualisation off
    this->ICModel = new TestSheet(fileName);
 //   ui->statusBar->showMessage(" ");
    this->statusText->setText("");
    if (ICModel->getNumOfPins()<1)
    {
        testLog->AddMessage(TestLog::TestFailed,tr("There was an error loading file ")+fileName.split(QDir::separator()).last());
        //demobilize interface if it was enebled
        ui->btnStart->setEnabled(0);
        ui->actionShow_power_requirements->setEnabled(0);
        ui->actionEdit_current_test_sheet->setEnabled(0);
        ui->actionCreate_new_model->setEnabled(0);
        ui->actionIgnore_pins->setEnabled(0);
        ui->actionTest_series->setEnabled(0);
        return;
    }
    //Initialize a whole stuff
    this->enableEverything();
    //restore visualization
    visualization->setNumOfPins(ICModel->getNumOfPins());
    visualization->setModelName(ICModel->getName());
    visualization->setGNDPins(this->ICModel->getGNDPins());
    visualization->setPowerPins(this->ICModel->getPowerPins());
    visualization->setIO(ICModel->initIO(1));
    visualization->setPinDescriptions(ICModel->getPinsDescriptions());
    testLog->AddMessage(TestLog::Informative,tr("Loaded file ")+fileName.split(QDir::separator()).last().split('/').last());
    this->statusText->setText(ICModel->getName()+" - "+ICModel->getDescription());
}

//enables widgets disabled on startup. Call if model loaded.
void MainWindow::enableEverything()
{
    ui->btnStart->setEnabled(1);
    ui->actionShow_power_requirements->setEnabled(1);
    ui->actionEdit_current_test_sheet->setEnabled(1);
    ui->actionCreate_new_model->setEnabled(1);
    ui->actionIgnore_pins->setEnabled(1);
    ui->actionTest_series->setEnabled(1);
}

void MainWindow::closeEvent(QCloseEvent *clsEv)
{
    Q_UNUSED(clsEv);
    if (this->isRunning==1)
    {
        this->isRunning=0;

    }

    this->saveSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_btnStart_clicked()
{
    //display power requirements for user
    PowerTable pLUT(this->pLUTPath);
    QString powerString = pLUT.checkModel(*this->ICModel);
    PowerVisualizer pvi(powerString,this->ICModel->getNumOfPins(),this);
    if (pvi.exec())
    {

        //lock UI
        this->isRunning=1;
        ui->btnStop->setEnabled(1);
        ui->btnStart->setEnabled(0);
        ui->menuBar->setEnabled(0);
        ui->mainToolBar->setEnabled(0);
        ui->sbRepetitions->setEnabled(0);
        ui->rbRepeat->setEnabled(0);
        ui->rbSingle->setEnabled(0);
        ui->cbSingleStep->setEnabled(0);
        ui->actionTest_series->setEnabled(0);


        //Clear stats, prepare visualization
        if ((ui->cbUpdateLog->isChecked())&&(this->clearLog)) ui->twTestLog->clear();
        ui->lbFailed->setText("0");
        ui->lbFailed->setStyleSheet("");
        ui->lbPassed->setText("0");
        ui->lbStep->setText("0");
        ui->lbRun->setText("0");
        if (ui->cbUpdatePins->isChecked())
        {
            this->visualization->setPins("");
            this->visualization->setErrorMap("");
        }
        int successes=0;
        int failures=0;
        QByteArray notConnectedList;
        for (int iii=0;iii<this->ICModel->pins.count();iii++)
        {
            if (this->ICModel->pins[iii].pinType==5)
            {
                notConnectedList.push_back(iii+1);
            }
        }


        //Prepare pin skip routine. They will look like NC
        QByteArray ignoredPins;
        QString ignoredString = lbSkippingInfo->text();
        ignoredString=ignoredString.remove(QRegularExpression("[A-Za-z :\\.]"));
        if (ignoredString!="")
        {
            testLog->AddMessage(TestLog::Informative,tr("Using pins skip: ")+ignoredString);
        }
        QStringList ssIgnored = ignoredString.split(',');
        for (int ii=0;ii<ssIgnored.count();ii++)
        {
           if ((ssIgnored.at(ii).toInt()>0)&&(ssIgnored.at(ii).toInt()<this->ICModel->getNumOfPins()))
           {
               ignoredPins.append(ssIgnored.at(ii).toInt());
           }
        }
        visualization->setIgnoredPins(ignoredPins);
        visualization->setNCPins(notConnectedList);

        //We are ready for testing procedure

        //Create device link.
        DeviceDriver tester(this->commPort,this->baudRate,this->timeout);

        //spawn the timer for time measure
        QElapsedTimer timer;
        timer.start();

        //Now the loop - if consecutive testing is selected.
        int spins=1;
        if (ui->rbRepeat->isChecked()) spins=ui->sbRepetitions->value();
        int series_success=0; //number of successfully tested chips for series testing.
        int series_count=1; //number of chips tested in series
        int series_spins=1; //number of tests per series - default it's int_max.
                            //we are doing it as we may test a single chip in series e.g. in 4 repetiions.
        if (this->testInSeries) {
            series_spins=INT_MAX; //hack, correct this in future
            testLog->AddMessage(TestLog::Informative,tr("Testing chip series"));
        }

        for (series_count=1;series_count<=series_spins;series_count++) //series loop
        {


        for (int repCount=1;repCount<=spins;repCount++) //repeats loop
        {
            bool failed=0;
            //Check device by resetting it.
            if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::DeviceHandling,tr("Sending RESET..."));
            QApplication::processEvents();
            int devResp = tester.reset();
            if ((ui->cbUpdateLog->isChecked())&&(devResp==0)) testLog->AddMessage(TestLog::DeviceHandling,tr("Reset OK."));
            if (hardwareError(devResp,-1,&tester)) return;
            if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::DeviceHandling,tr("Sending initial I/O"));
            QApplication::processEvents();
            tester.qSleep(this->stepDelay);
            devResp = tester.setIO(this->ICModel->initIO());
            if (hardwareError(devResp,-1,&tester)) return;
            if (ui->cbUpdatePins->isChecked()) this->visualization->setIO(this->ICModel->initIO()); //set I/O on visualization
            if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::Informative,tr("Starting test script ")+this->ICModel->getName()+tr(" at ")+
                                                                  QDate::currentDate().toString("yyyy-MM-dd")+" "+QTime::currentTime().toString("HH:mm:ss"));
            //What's going on with this lastIO? In test, RESET always leads to loose of IO settings.
            //So after RESETting program automatically programs I/O pins.
            //Now imagine we've changed I/O during test and then announced RESET command few steps later.
            //We have to restore I/O settings not from the beginning (initIO), but last user set.
            QString lastIOSetup=this->ICModel->initIO();
            int delta=0; //scrpit lines vs program lines - visualization only.
            for (int i=0;i<this->ICModel->script.count();i++)
            {

                if ((ui->cbSingleStep->isChecked())&&(this->ICModel->script[i].cmd!=255))
                {
                    //MessageBox is moved not to cover log and visualization
                    QMessageBox mb(QMessageBox::Information, tr("Step-by-step"),tr("Press Return to go to the next step."),QMessageBox::Ok|QMessageBox::Cancel, this,Qt::Widget);
                    mb.move(MainWindow::pos().x(),MainWindow::pos().y()+ui->gvICPreview->height()+30);
                    if (mb.exec()==QMessageBox::Cancel)
                    {
                        tester.reset();
                        this->on_btnStop_clicked();
                        if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::TestFailed,tr("Aborted by user."));
                        return;
                    }
                }

                if (!this->isRunning)
                {
                    tester.reset();
                    tester.reset(); //BUGFIX - sometimes resetting in the middle of processing caused tester to respond errorneously next time.
                    if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::TestFailed,tr("Test aborted."));
                    return;
                }

                ui->lbStep->setText(QString::number(i+1));
                QApplication::processEvents();  //A demonstration of a very bad programming practice
                tester.qSleep(this->stepDelay); //wait for levels to settle
                 switch (this->ICModel->script[i].cmd)
                {
                    case 0:     //reset and restore IO
                        if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::DeviceHandling,tr("Sending RESET."),i+1-delta);
                        devResp = tester.reset();
                        if (hardwareError(devResp,i,&tester)) return;
                        if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::DeviceHandling,tr("Sending I/O after resetting"), i+1-delta);
                        tester.qSleep(this->stepDelay);
                        //Set I/O:
                        devResp=tester.setIO(lastIOSetup);
                        if (hardwareError(devResp,i,&tester)) return;
                        if (ui->cbUpdatePins->isChecked()) this->visualization->setIO(lastIOSetup); //set I/O on visualization
                        break;

                    case 1:     //power on
                        if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::PowerSet,tr("5V Power on"),i+1-delta);
                        devResp=tester.powerON();
                        if (hardwareError(devResp,i,&tester)) return;
                        break;

                    case 2:     //set IO
                        if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::IOSet,tr("Setting I/O configuration"),i+1-delta);
                        lastIOSetup=this->ICModel->script[i].arg;
                        devResp=tester.setIO(this->ICModel->script[i].arg);
                        if (ui->cbUpdatePins->isChecked()) this->visualization->setIO(lastIOSetup); //set I/O on visualization
                        if (hardwareError(devResp,i,&tester)) return;
                        break;

                    case 3:     //send data
                        if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::DataSend,tr("Send:  ").leftJustified(8,' ').left(7)+this->ICModel->script[i].arg,i+1-delta);
                        devResp = tester.setData(this->ICModel->script[i].arg);
                        if (hardwareError(devResp,i,&tester)) return;
                        if ((ui->cbTestFlight->isChecked())&&(ui->cbUpdatePins->isChecked()))
                        {
                            //read and present on pin display
                            QString ret = tester.getData();
                            this->visualization->setPins(ret);
                        }
                        break;

                    case 255: //comment
                        if (this->ICModel->script[i].arg.at(this->ICModel->script[i].arg.length()-1)==';')
                        { //if it ends with ; and update log is checked, display it without this last ;.
                            if (ui->cbUpdateLog->isChecked())
                            {
                                testLog->AddMessage(TestLog::Informative,this->ICModel->script[i].arg.left(this->ICModel->script[i].arg.length()-1), i+1-delta);
                            }
                        }
                        else
                        {   //if it's a "hidden" comment, just increase skip counter.
                            delta++; //skip the invisible comment line in counting
                        }
                        break;

                    case 4:     //read data and compare
                        if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::DataRead,tr("Exp:   ").leftJustified(8,' ').left(7)+this->ICModel->script[i].arg,i+1-delta);
                        QString ret=tester.getData();
                        if (ret=="-3")
                        {
                            hardwareError(-1,i,&tester);
                            return;
                        }
                        ret=ret.left((24+this->ICModel->getNumOfPins())/2);
                        ret=ret.right(this->ICModel->getNumOfPins());
                        if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::DataRead,tr("Read:  ").leftJustified(8,' ').left(7)+ret,i+1-delta);
                        QString comp=this->ICModel->compareStep(i+1,ret);
                        //update visualization
                        if (ui->cbUpdatePins->isChecked())
                        {
                            this->visualization->setPins(ret);
                            this->visualization->setErrorMap(comp);
                        }
                        if (comp!="") //handling compare error
                        {
                            if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::ChipBad,tr("Err:   ").leftJustified(8,' ').left(7)+comp,i+1-delta);
                            //Fault can be ignored if the errorneous pin is in pin skip list
                            for (int ii=0;ii<ignoredPins.length();ii++)
                            {
                                comp[ignoredPins.at(ii)-1]='0';
                            }
                            if (!comp.contains('1'))
                            {
                                testLog->AddMessage(TestLog::Informative,tr("All bad pins on skip list, continuing test..."),i+1-delta);
                                break;
                            }
                            //but it is not...
                            if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::ChipBad,tr("IC failed test."),i+1-delta);
                            failed=1;

                            //Aborting procedure
                            if (ui->cbAbortOnError->isChecked()) //Aborting procedure
                            {
                                tester.reset();
                                if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::TestFailed,tr("Test failed: IC failed test."));

                                if (!this->testInSeries) //for series we abort only current iteration, not a whole test
                                    this->on_btnStop_clicked();

                                //Warn user by incrementing failures label.
                                //not placing 1, because user may enable "abort on error" after few bad runs.
                                ui->lbFailed->setText(QString::number(ui->lbFailed->text().toInt()+1));
                                ui->lbFailed->setStyleSheet("QLabel { background-color : red; }");
                                if ((repCount==spins)&&(!this->testInSeries)) return; //shut down the procedure if we're testing not in series
                            }
                        }
                        break;
                }
            }
            //display test summary
            if (failed)
            {
                   if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::TestFailed,tr("Test failed: IC failed test."));
                   failures++;
            }
            else
            {
                   if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::TestPassed,tr("IC passed all tests."));
                   if (ui->cbUpdatePins->isChecked()) this->visualization->setPins("");
                   successes++;
            }

            //update successes/failures/runs labels
            ui->lbFailed->setText(QString::number(failures));
            if (failures>0) ui->lbFailed->setStyleSheet("QLabel { background-color : red; }");
            ui->lbPassed->setText(QString::number(successes));
            ui->lbRun->setText(QString::number(repCount));


        } //repeat test loop

        tester.reset(); //END OF TESTS - POWER OFF.

        if (failures==0)
        {
            series_success++; //Series test: increase OK count if it was OK.
        }


        //If this is a multi-pass verification, give a nice summary to user:
        if ((spins>1)&&(ui->cbUpdateLog->isChecked()))
        {
            testLog->AddMessage(TestLog::Informative,tr("Total runs made: ")+ui->lbRun->text());
            if (failures>0)
            {
                testLog->AddMessage(TestLog::Informative,tr("Successful runs: ")+QString::number(successes));
                testLog->AddMessage(TestLog::TestFailed,tr("Passes with errors: ")+QString::number(failures));
            }
            else
                 testLog->AddMessage(TestLog::TestPassed,tr("Successful runs: ")+QString::number(successes));
            //Proof for users who look only on icons - if there was an error, there won't be
            //icon of successful passes, but info icon not to introduce mistakes.

        }

        //end of test iteration. Supply user with information when testing in series.
        if (this->testInSeries)
        {
            QMessageBox mb(QMessageBox::Information, tr("Test in series"),tr("Press Return to test next chip."),QMessageBox::Ok|QMessageBox::Cancel, this,Qt::Widget);
            mb.move(MainWindow::pos().x(),MainWindow::pos().y()+ui->gvICPreview->height()+30);
            if (mb.exec()==QMessageBox::Cancel)
            {
                tester.reset();
                this->testInSeries=0;
                this->on_btnStop_clicked();
                if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::Informative,"\n"+tr("End of series test."));
            //    if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::Informative,tr("     SUMMARY"));
                QString a=QString::number(series_count)+tr(" chip(s) tested,");
                if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::Informative,a);
                a=QString::number(series_success)+tr(" chip(s) passed,");
                if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::Informative,a);
                a=QString::number(series_count-series_success)+tr(" chip(s) failed,");
                if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::Informative,a);
                if (ui->lbFailed->text()!="0")
                {
                    if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::ChipBad,tr("Some chips failed test!"));
                }
                testLog->AddMessage(TestLog::Informative,tr("Series test ")+this->ICModel->getName()+tr(" finished at ")+
                                                                                  QDate::currentDate().toString("yyyy-MM-dd")+" "+QTime::currentTime().toString("HH:mm:ss")+"\n");

                return;
            }
        }

        successes=0;
        failures=0; //reset before next series loop.
        } //series test loop
        if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::Informative,tr("Test end at ")+QDate::currentDate().toString("yyyy-MM-dd")+" "+QTime::currentTime().toString("HH:mm:ss")+tr(", Total time: ")+QString::number(timer.elapsed()/1000)+tr(" secs.")+"\n");


    } //if power view executed?
    this->on_btnStop_clicked();
}


void MainWindow::on_btnStop_clicked()
{
    this->isRunning=0;
    ui->btnStop->setEnabled(0);
    this->enableEverything();
    ui->menuBar->setEnabled(1);
    ui->mainToolBar->setEnabled(1);

    ui->cbTestFlight->setEnabled(1);
    ui->sbRepetitions->setEnabled(1);
    ui->rbRepeat->setEnabled(1);
    ui->rbSingle->setEnabled(1);
    ui->cbSingleStep->setEnabled(1);
}


void MainWindow::on_actionExit_triggered()
{
    MainWindow::close();
}

void MainWindow::on_actionOpen_model_triggered()
{
    ModelSelector openModelDialog(this,"",this->settingsPath);
    if (openModelDialog.exec())
    {
        this->loadModel(openModelDialog.getFileName());
    }
    //restore pin skipping, as usually we dont need it from earlier circuit
    lbSkippingInfo->setText(tr("Not ignoring any pins."));
}

void MainWindow::on_actionShow_power_requirements_triggered()
{
    PowerTable pLUT(this->pLUTPath);
    QString powerString = pLUT.checkModel(*this->ICModel);
    PowerVisualizer pvi(powerString,this->ICModel->getNumOfPins(),this);
    pvi.exec();
}

void MainWindow::on_actionCopy_log_to_clipboard_triggered()
{
    QString logDump="";
    for (int i=0;i<ui->twTestLog->topLevelItemCount();i++)
    {
        logDump += ui->twTestLog->topLevelItem(i)->text(0)+" : ";
        logDump += ui->twTestLog->topLevelItem(i)->text(1)+"\n";
    }
    QApplication::clipboard()->setText(logDump);

}

//Throws nice hardware error, restores UI and returns with 1. If no error, just returns with 0
bool MainWindow::hardwareError(int code, int step, DeviceDriver * devToReset)
{
    if (code<0)
    {
        devToReset->reset();
        QApplication::processEvents();
        if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::DeviceError,tr("Device error ")+QString::number(code), step+1);
        if (ui->cbUpdateLog->isChecked()) testLog->AddMessage(TestLog::TestFailed,tr("Test FAILED because of hardware error. "));
        this->on_btnStop_clicked();
        return 1;
    }
    return 0;
}

void MainWindow::on_actionDevice_reset_triggered()
{
    DeviceDriver tester(this->commPort,this->baudRate,this->timeout);
    testLog->AddMessage(TestLog::DeviceHandling,tr("Sending RESET..."));
    MainWindow::repaint();
    QApplication::processEvents();
    int devResp = tester.reset();
    if (devResp==0) testLog->AddMessage(TestLog::DeviceHandling,tr("Reset OK."));
    if (devResp<0)
    {
        testLog->AddMessage(TestLog::DeviceError,tr("Failed to reset device"));
        return;
    }
}

void MainWindow::on_actionConfiguration_triggered()
{
    if (ConfigWindow(this->settingsPath, this->pLUTPath, this).exec())
    {
        this->loadSettings();
    }
}

void MainWindow::on_actionIdentify_device_triggered()
{
    DeviceDriver tester(this->commPort,this->baudRate,this->timeout);
    int devResp = tester.reset();
    if (devResp<0)
    {
        testLog->AddMessage(TestLog::DeviceError,tr("IDENTIFY: Failed to reset device"));
        return;
    }
    tester.qSleep(25);
    testLog->AddMessage(TestLog::DeviceHandling,tr("Identify device..."));
    QApplication::processEvents();
    QString resp = tester.deviceVersion();
    testLog->AddMessage(TestLog::DeviceHandling,tr("Got: ")+resp.replace('\n',' ').trimmed());
    QApplication::processEvents();
    tester.qSleep(25);
    QApplication::processEvents();
    devResp = tester.reset();
    if (devResp<0)
    {
        testLog->AddMessage(TestLog::DeviceError,tr("IDENTIFY: Failed to reset device after ID"));
        return;
    }
}

void MainWindow::on_actionClear_history_triggered()
{
    this->MRU.clear();
    this->refreshMRU();
}

void MainWindow::on_actionLow_level_interface_test_triggered()
{
    LowLevelTest(this->commPort,this->baudRate).exec();
}

void MainWindow::on_actionEdit_current_test_sheet_triggered()
{
    TestSheet temp = *this->ICModel;
    if (SheetEditor(&temp,this->autoFillSettings).exec())
    {
        QString fn=this->ICModel->getFN();
        if (this->ICModel->getName()!=temp.getName()) //If name is different than original
        {
            fn = temp.getFN();
            if (QFile(fn).exists())             //if file exists
                QMessageBox::information(this,tr("Warning"),tr("Such file exists. It will be overwritten."));

            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,tr("Old file"),tr("Changing name will save sheet file with different name. Do you want to keep old file?"),QMessageBox::Yes|QMessageBox::No);
            if (reply==QMessageBox::No)
            {
                QFile(this->ICModel->getFN()).remove();
            }
        }
        temp.saveToFile(fn);
        //reload
        loadModel(fn);
    }
}

void MainWindow::on_actionCreate_new_model_triggered()
{
    //OK, here we will make a new model.
    //The model will be automatically saved and opened for editing.
    bool ok;

    QString dir=QFileInfo(this->ICModel->getFN()).dir().absolutePath();
    if (dir[dir.length()-1]!=QDir::separator()) dir=dir+QDir::separator(); //get current path from currently loaded model
    QString newName = QInputDialog::getText(this, tr("Create new sheet"),tr("Enter name of new sheet."), QLineEdit::Normal,"", &ok);
    if (!ok) return;
    dir=dir+newName+".mod";

    //Such file exists! what to do?
    if (QFile(dir).exists())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,tr("File exists"),tr("Sheet with such name exists in current directory. Overwrite??"),QMessageBox::Yes|QMessageBox::No);
        if (reply==QMessageBox::No)
        {
            return;
        }
    }

    //Now new model is created
    if (this->ICModel) delete this->ICModel;
    this->ICModel = new TestSheet(newName,tr("Type description"),14);
    this->ICModel->saveToFile(dir);
    //During creating a new model the worst thing is uncontrolled pin skipping, so we turn it off
    lbSkippingInfo->setText(tr("Not ignoring any pins."));

    //we've already loaded a model, we don't need toload it back, let's open it for editing
    this->on_actionEdit_current_test_sheet_triggered();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this,tr("About ChipTinker App"),tr("ChipTinker App\nFork of (QtICTester, ICTester v. 0.09\nQt6 port\nMCbx 2014..2025\nGNU General Public License v2.0.)"));
}

void MainWindow::on_actionROM_dump_triggered()
{
    ROMDumper(this->commPort,this->pLUTPath,this->settingsPath,this->baudRate).exec();
}

void MainWindow::on_actionIgnore_pins_triggered()
{
    //Here open window with list editor.
    //List goes to label as "Ignore: x,y,z,t..."
    QString initial = lbSkippingInfo->text();
    initial=initial.remove(QRegularExpression("[A-Za-z :\\.]"));
    QString ret = "a";
    while (ret.contains(QRegularExpression("[^0-9,]")))
    {
        bool ok=false;
        //Formulate dialog. We're doing it not by GetText to shift it away from visualization, where user
        //may look to decide which pin to exclude from next test.
        QInputDialog qid(this);
        qid.setInputMode(QInputDialog::TextInput);
        qid.setWindowTitle(tr("Skip pins"));
        qid.setLabelText(tr("Enter comma-separated list of pins to skip during test:"));
        qid.setTextValue(initial);
        qid.setGeometry((ui->gvICPreview->mapToGlobal(ui->gvICPreview->pos()).x()+ui->gvICPreview->width()+20),
                        ui->gvICPreview->mapToGlobal(ui->gvICPreview->pos()).y()+(ui->gvICPreview->height()/2),
                        qid.width(),
                        qid.height());
        ok = qid.exec();

        //Cancel clicked
        if (!ok)
            return;

        //OK clicked
        ret=qid.textValue();
        //prepare result
        ret=ret.remove(QRegularExpression("[ \\.]"));
        if (ret.contains(QRegularExpression("[^0-9,]")))
        {
            QMessageBox::information(nullptr,tr("Error"),tr("Use only numbers, spaces and commas!"));
        }
    }
    QStringList qsl=ret.split(',');
    for (int i=0;i<qsl.count();i++)
    {
        if (this->ICModel->getNumOfPins()<qsl[i].toInt())
        {
            QMessageBox::information(nullptr,tr("Error"),tr("Pin number ")+qsl[i]+tr(" exceeds IC's pin count!"));
            return;
        }
    }

    if (ret=="")
    {
        lbSkippingInfo->setText(tr("Not ignoring any pins."));
        QByteArray ignoredList;
        visualization->setIgnoredPins(ignoredList);
        visualization->repaintModel();
        return;
    }
    lbSkippingInfo->setText(tr("Ignore: ")+ret);


    //We need to rebuild ignored pins list.
    QByteArray ignoredList;
    QStringList ssIgnored = ret.split(',');
    for (int ii=0;ii<ssIgnored.count();ii++)
    {
       if ((ssIgnored.at(ii).toInt()>0)&&(ssIgnored.at(ii).toInt()<this->ICModel->getNumOfPins()))
       {
           ignoredList.push_back(ssIgnored.at(ii).toInt());
       }
    }
    visualization->setIgnoredPins(ignoredList);
    visualization->repaintModel();
}

void MainWindow::on_actionClear_log_triggered()
{
    ui->twTestLog->clear();
}

void MainWindow::on_actionIC_identifier_triggered()
{
    identifier(this->commPort,this->pLUTPath,this->settingsPath,this->baudRate,this->warnUnstable).exec();
}

void MainWindow::on_actionTest_series_triggered()
{
    this->testInSeries=1;
    this->on_btnStart_clicked();
    this->testInSeries=0;
}
