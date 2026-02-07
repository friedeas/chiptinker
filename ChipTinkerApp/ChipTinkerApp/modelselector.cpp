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


#include "modelselector.h"
#include "ui_modelselector.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QScrollBar>
#include "../testsheet.h"
#include "icvisualizer.h"
#include <QSettings>
#include <QList>
#include <QShortcut>


// Model Selector
// The fancy "open" window for test sheets


ModelSelector::ModelSelector(QWidget *parent, QString initPath, QString settingsPath) :
    QDialog(parent),
    ui(new Ui::ModelSelector)
{

    //restore settings from ini file
    // - window size
    // - last used directory
    // - splitter sizes
    QString p = settingsPath;
    this->iniPath=p;
    QSettings settings(this->iniPath,QSettings::IniFormat);
    settings.beginGroup("OpenDialog");
    int ww = settings.value("Width","720").toInt();
    int wh = settings.value("Height","406").toInt();
    int sh1= settings.value("TreeWidth","160").toInt();
    int sh2= settings.value("FilesWidth","400").toInt();
    int sh3= settings.value("PreviewWidth","160").toInt();
    if (initPath=="") initPath=settings.value("LastPlace",initPath=QDir::currentPath()).toString();
    settings.endGroup();

    //modify path
    this->fileName="";
    ui->setupUi(this);
    dirmodel = new QFileSystemModel(this);
    this->setWindowTitle(tr("Select test sheet"));
    this->directory=initPath;
    if ((initPath=="")||(!QDir(initPath).exists())) this->directory=QDir::currentPath();

    //prepare tree for displaying only directories
    dirmodel->setFilter(QDir::AllDirs|QDir::NoDotAndDotDot);
    dirmodel->setRootPath(this->directory);
    this->ui->dirView->setModel(dirmodel);
    this->ui->dirView->hideColumn(2);
    this->ui->dirView->hideColumn(1);
    this->ui->dirView->hideColumn(3);

    //open tree view to show directory
    QModelIndex qmi= dirmodel->index(this->directory);
    while(qmi.isValid())
    {
        this->ui->dirView->expand(qmi);
        qmi = qmi.parent();
    }
    ui->dirView->resizeColumnToContents(0);

#if QT_VERSION >= 0x050000
    ui->dirView->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents); //Qt5
#else
    ui->dirView->header()->setResizeMode(0,QHeaderView::ResizeToContents);  //Qt4
#endif
    ui->dirView->setColumnWidth(0,600);
    ui->dirView->header()->close();

    //Prepare file list widget
    QStringList aaa;
    aaa<<tr("File")<<tr("Type/description")<<tr("Path");
    ui->twFiles->setHeaderLabels(aaa);
    ui->twFiles->setColumnHidden(2,1);
    ui->twFiles->setRootIsDecorated(0);
    ui->twFiles->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(0);

    this->previewVisualizer = new ICVisualizer(this,ui->preview,0,"");

    //apply settings from ini
    ModelSelector::resize(ww,wh);
    QList<int> sizes;
    sizes<<sh1<<sh2<<sh3;
    ui->splitter->setSizes(sizes);
    this->on_dirView_clicked(dirmodel->index(this->directory));   //show contents in file manager

    QShortcut* fileshortcut = new QShortcut(Qt::Key_Slash, this);
    QObject::connect(fileshortcut,SIGNAL(activated()),this,SLOT(on_searchActivated()));

}

ModelSelector::~ModelSelector()
{
    delete ui;
}

void ModelSelector::on_searchActivated()
{
    ui->edFilter->setFocus();
}

void ModelSelector::on_dirView_clicked(const QModelIndex &index)
{
    ui->twFiles->clear();
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(0);

    this->directory = dirmodel->fileInfo(index).absoluteFilePath();
    //we've the path, we need to list mod files, verify them and show list to user.

   QDir fileLister(this->directory,"*.mod");
    QStringList models = fileLister.entryList();

    ui->twFiles->setSortingEnabled(0);
    foreach (QString a, models)
    {
        QFileInfo tmp(this->directory,a);
        QTreeWidgetItem *entry=new QTreeWidgetItem();
        entry->setText(0,a);

        //Now let's read description from file. VERY bad thing here.
        QString desc="";
        QFile plik(tmp.absoluteFilePath());
        if (plik.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            QTextStream in(&plik);
            QString line=in.readLine();
            if ((line.length()>0)&&(line.length()<17)) desc+=line.trimmed()+": ";
            line=in.readLine();
            if ((line.length()>0)&&(line.length()<129)) desc+=line.trimmed();
        }
        else desc=tr("[CANNOT READ DESCRIPTION]");
        entry->setText(1,desc);

        entry->setText(2,tmp.absoluteFilePath());

        //This is a whole search logic
        if ((ui->edFilter->text()=="")||(desc.toLower().contains(ui->edFilter->text().toLower()))||(a.toLower().contains(ui->edFilter->text().toLower())))
            ui->twFiles->addTopLevelItem(entry);
    }
    ui->twFiles->resizeColumnToContents(1);
    ui->twFiles->sortByColumn(0,Qt::AscendingOrder);
    ui->twFiles->setSortingEnabled(1);

    ui->dirView->scrollTo(index);
    QRect r = ui->dirView->visualRect(index);
    ui->dirView->horizontalScrollBar()->setValue(r.x()-20);
}

//search causes refresh, refresh does everything
void ModelSelector::on_edFilter_textChanged()
{
    this->on_dirView_clicked(dirmodel->index(this->directory));
    this->previewVisualizer->setNumOfPins(0);
}

void ModelSelector::on_buttonBox_clicked(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton stdButton = ui->buttonBox->standardButton(button);
    if (stdButton==QDialogButtonBox::Ok)
    {
        //send file name
        this->fileName=ui->twFiles->selectedItems()[0]->text(2);
        this->saveIni();
    }
    else
    {
        this->fileName="";
    }
}


QString ModelSelector::getFileName()
{
    return this->fileName;
}

void ModelSelector::on_dirView_expanded(const QModelIndex &index)
{
    ui->dirView->scrollTo(index);
    QRect r = ui->dirView->visualRect(index);
    ui->dirView->horizontalScrollBar()->setValue(r.x()-20);

}

void ModelSelector::on_twFiles_doubleClicked()
{
    if (ui->twFiles->selectedItems().count()>0)
    {
        //send file name
        this->fileName=ui->twFiles->selectedItems()[0]->text(2);
        this->saveIni();
        this->accept();
    }
}

void ModelSelector::saveIni()
{
    // write ini.
    QSettings settings(this->iniPath,QSettings::IniFormat);
    settings.beginGroup("OpenDialog");
    int tmp=ModelSelector::size().width();
    settings.setValue("Width",QString::number(tmp));
    tmp=ModelSelector::size().height();
    settings.setValue("Height",QString::number(tmp));
    settings.setValue("TreeWidth",QString::number(ui->splitter->sizes()[0]));
    settings.setValue("FilesWidth",QString::number(ui->splitter->sizes()[1]));
    settings.setValue("PreviewWidth",QString::number(ui->splitter->sizes()[2]));

    settings.setValue("LastPlace", this->directory);
    settings.endGroup();
}

void ModelSelector::on_twFiles_itemSelectionChanged()
{
    if (ui->twFiles->selectedItems().count()==0)
    {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(0);
        this->previewVisualizer->setNumOfPins(0);
    }
    else
    {
             ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(1);
             TestSheet t(ui->twFiles->selectedItems()[0]->text(2));

             this->previewVisualizer->setNumOfPins(t.getNumOfPins());
             this->previewVisualizer->setIO(t.initIO(1));
             this->previewVisualizer->setPinDescriptions(t.getPinsDescriptions());
             this->previewVisualizer->setSummary(t.getName(),3);

    }
}

//BUGFIX: Preview of IC does not resize when the slider is moved.
void ModelSelector::on_splitter_splitterMoved(int pos, int index)
{
    this->on_twFiles_itemSelectionChanged();
}
