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

#ifndef MODELSELECTOR_H
#define MODELSELECTOR_H

#include <QDialog>
#include <QModelIndex>
#include <QFileSystemModel>
#include <QString>
#include <QListView>
#include <QMessageBox>
#include <QDir>
#include <QTextStream>
#include <QPushButton>
#include "../testsheet.h"
#include "icvisualizer.h"

namespace Ui {
class ModelSelector;
}

class ModelSelector : public QDialog
{
    Q_OBJECT

public:
    explicit ModelSelector(QWidget *parent = nullptr, QString initPath="",QString settingsPath="");
    ~ModelSelector();
    QString getFileName();

private slots:
    void on_dirView_clicked(const QModelIndex &index);

    void on_edFilter_textChanged();

    void on_buttonBox_clicked(QAbstractButton *button);

//    void on_twFiles_clicked();

    void on_dirView_expanded(const QModelIndex &index);

    void on_twFiles_doubleClicked();

    void on_twFiles_itemSelectionChanged();

    void on_searchActivated();
    void on_splitter_splitterMoved(int pos, int index);

private:
    Ui::ModelSelector *ui;
    QFileSystemModel *dirmodel;
 //   QFileSystemModel *filemodel;
    QString iniPath;
    QListView *fileview;
    QString fileName;
    QString directory;
    ICVisualizer *previewVisualizer;
    void saveIni();
};

#endif // MODELSELECTOR_H
